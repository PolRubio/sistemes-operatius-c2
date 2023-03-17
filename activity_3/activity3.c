#include <time.h> // time()
#include <stdio.h> //input & output
#include <sys/types.h> // fork()
#include <stdlib.h> 
#include <fcntl.h> // O_* constants
#include <unistd.h> // access to POSIX
#include <sys/mman.h> // mmap + constants
#include <pthread.h>
#include <stdint.h>

#define MAX_NUM 100

#define SHARED_MEM_NAME "/shm_act1"
#define SHARED_MEM_SIZE 4

#define NUM_THREADS 2



typedef struct {
    uint32_t *data_ptr;
    pthread_mutex_t * mutex1;
    pthread_mutex_t * mutex2;
} thread_data_t;

int current_thread=1;
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

void* handle_thread(void* arg){
    // get thread count (thread1, thread2, ...)

    thread_data_t data=*((thread_data_t*) arg);
    
    printf("thread %lu", pthread_self());

    while((int)(data->data_ptr)<0){
        pthread_mutex_lock(data->mutex2); // mutex1 for parent or thread1



        pthread_mutex_unlock(data->mutex1); // mutex2 for parent or thread1
    }
}


int main(int argc, char *argv[]){
    srandom(time(0));

    int random_number=random()%(MAX_NUM+1);
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

    // [ MUTEXES ]
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    
    // [ THREADS ]

	pthread_t tid[NUM_THREADS];

    thread_data_t data;
    data.data_ptr=shared_value;
    data.mutex1=&mutex1;
    data.mutex2=&mutex2;


    for(int i=0; i<NUM_THREADS; i++)
        pthread_create(&tid[i], NULL, handle_thread, (void *)&data);
    

    for(int i=0; i<NUM_THREADS; i++)
        pthread_join(tid[i], NULL);
    

    // destroy mutexes
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);

   
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

    
}