#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


#define OP_MAX 100
#define OPS_NUM 4

#define FILENAME "/tmp/myfifo"

char OPERATIONS[OPS_NUM]={'+', '-', '*', '/'};
int iterations, fd_pipe;

void parent(){
    for(int i=0; i<iterations; i++){
        uint8_t 
            op1=random()%OP_MAX,
            op2=random()%OP_MAX;

        char oper=OPERATIONS[random()%OPS_NUM];

        printf("parent (pid=%d) iteration: %d\n", getpid(), i);
        printf("parent (pid=%d): %d %c %d = ?\n", getpid(), op1, oper, op2);
        
        // send operand 1
        if(write(fd_pipe, &op1, 1) != 1){
            perror("write operand1");
            exit(0);
        }

        // send operation
        if(write(fd_pipe, &oper, 1) != 1){
            perror("write operation");
            exit(0);
        }

        // send operand 2
        if(write(fd_pipe, &op2, 1) != 1){
            perror("write operand2");
            exit(0);
        }

        wait(NULL);
    }
    printf("parent (pid=%d) ends.\n", getpid());


    close(fd_pipe);
    unlink(FILENAME);
}

void child(){
    for(int i=0; i<iterations; i++){
        uint8_t op1=0, op2=0;
        char oper=' ';

        // read operand1
        if(read(fd_pipe, &op1, 1)!=1){
            perror("read1");
            exit(0);
        }

        // read operation
        if(read(fd_pipe, &oper, 1)!=1){
            perror("read2");
            exit(0);
        }

        // read operand2
        if(read(fd_pipe, &op2, 1)!=1){
            perror("read3");
            exit(0);
        }

        printf("child (pid=%d): ", getpid());
        switch(oper){
            case '+':
                printf("%d + %d = %d", op1, op2, op1+op2);
                break;
            
            case '-':
                printf("%d - %d = %d", op1, op2, (int) (op1-op2));
                break;

            case '*':
                printf("%d * %d = %d", op1, op2, op1*op2);
                break;

            default:
                printf("%d / %d = %d", op1, op2, op1/op2);
                break;
        }
        printf(" \n");
    }

    printf("child (pid=%d) ends.\n", getpid());
}


int main(int argc, char *argv[]){
    srandom(time(NULL));

    if(argc!=2){
        printf("this programm requires only one argument!\n");
        exit(0);
    }

    iterations=atoi(argv[1]);
    
    if(mkfifo(FILENAME, 0666)<0){
        perror("mkfifo");
        exit(0);
    }
    printf("main: created pipe.\n");

    fd_pipe=open(FILENAME, O_RDWR);
    if(fd_pipe<0){
        perror("open");
        exit(0);
    }

    printf("main: open pipe for read/write.\n");

    int pid=fork();
    if(pid<0){
        perror("fork");
        exit(0);
    }

    if(pid==0) child();
    else parent();
}