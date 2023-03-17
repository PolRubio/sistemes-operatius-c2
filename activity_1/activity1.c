#include <time.h> // time()
#include <stdio.h> //input & output
#include <sys/types.h> // fork()
#include <stdlib.h> 
#include <fcntl.h> // O_* constants
#include <unistd.h> // access to POSIX
#include <sys/mman.h> // mmap + constants
#include <semaphore.h> // semaphores

#define MAX_NUM 100

#define SHARED_MEM_NAME "/shm_act1"
#define SHARED_MEM_SIZE 4

#define SEM_NAME1 "/activity1_sem1"
#define SEM_NAME2 "/activity1_sem2"


int main(int argc, char *argv[]){
    srandom(time(0));

    // variable initialisation
    int resulting_pid,
        random_number=random()%(MAX_NUM+1);

    printf("bouncing for %d times.\n",random_number);

    // [ SHARED MEMORY ]
    // https://gist.github.com/garcia556/8231e844a90457c99cc72e5add8388e4


    // "create the shared memory space, you can use the shm_open function call"
    // http://man.he.net/man3/shm_open
    int shm_fd=shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666); // 0666 meaning all users can read/write 
    if(shm_fd<0){
        perror("shm_open");
        exit(0);
    }

    // "limit the space to 4 bytes you can use the ftruncate"
    // http://man.he.net/man2/ftruncate
    if(ftruncate(shm_fd, SHARED_MEM_SIZE)<0){
        perror("ftruncate");
        exit(0);
    }

    // "map the memory space in the process, you can use the mmap function"
    // http://man.he.net/man2/mmap
    int* shared_value=mmap(NULL, SHARED_MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shared_value==MAP_FAILED){
        perror("mmap");
        exit(0);
    }

    // "generate a random number which should be written to shared memory"
    *shared_value=random_number;


    // [ SEMAPHORES ]
    //      "semaphores will be named "/activity2_sem1" and "/activity2_sem2""
    //      "used to indicate each process's turn to write to shared memory"

    sem_t *sem1,*sem2;

    // "create and initialize semaphores, you can use the sem_open and sem_init"
    // https://man7.org/linux/man-pages/man3/sem_open.3.html
    sem1=sem_open(SEM_NAME1, O_CREAT, 0666); // parents
    sem2=sem_open(SEM_NAME2, O_CREAT, 0666); // childs

    if(sem1==SEM_FAILED || sem2==SEM_FAILED){
        perror("sem_open");
        exit(0);
    }
    
    if(sem_init(sem1,1,1)<0 || sem_init(sem2,1,0)<0){
        perror("sem_init");
        exit(0);
    }




    // [ FORK ]

    resulting_pid=fork(); // new process created.
    if(resulting_pid<0){
        perror("fork");
        exit(0);
    }



    printf((resulting_pid==0)?"child":"parent");
    printf(" (pid=%d) begins\n", getpid());

    // "get and release the semaphores, you can use the sem_wait and sem_post"
    while(*shared_value>1){
        sem_wait((resulting_pid==0)?sem2:sem1);

        (*shared_value)--;
        printf((resulting_pid==0)?"child":"parent");
        printf(" (pid=%d) bounce: %d.\n", getpid(), *shared_value);

        sem_post((resulting_pid==0)?sem1:sem2);
    }

    printf((resulting_pid==0)?"child":"parent");
    printf(" (pid=%d) ends.\n", getpid());




    if(resulting_pid>0){
        // "necessary to unmap, close and free the shared memory space and the file descriptor with the functions munmap, close and unlink."
        if(munmap(shared_value,SHARED_MEM_SIZE)!=0){
            perror("munmap");
            exit(0);
        }
        if(close(shm_fd)<0){
            perror("close");
            exit(0);
        }
        if(shm_unlink(SHARED_MEM_NAME)==-1){
            perror("shm_unlink");
            exit(0);
        }

        // "release the semaphores with sem_close"
        if(sem_close(sem1)!=0 || sem_close(sem2)!=0){
            perror("sem_close");
            exit(0);
        }
    }
}