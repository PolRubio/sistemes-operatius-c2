#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

#define OP_MAX 100
#define OPS_NUM 4

// creates two processes (i.e., parent and child)
// named pipe that allows them to communicate

// parent process must generate three random number
    // correspond to two ( uint8_t ) operands [0,100] and a mathematical operation ( char )
    // operation of the formula axb, where:
        // a is the first operand
        // x is the mathematical operation
        // b is the second operand
    // possible mathematical operations [+, -, *, /]

// once generated, write one by one through the pipe


// child must perform three 1-byte read operations
// child process must perform the mathematical operation

// must be repeated a number of times passed by parameter to the program via arguments

char OPERATIONS[OPS_NUM]={'+', '-', '*', '/'};



void parent(int *pipe){
    uint8_t 
        op1=random()%OP_MAX,
        op2=random()%OP_MAX;

    char oper=OPERATIONS[random()%OPS_NUM];

    int uint_size=sizeof(uint8_t);

    printf("parent (pid=%d): %d %c %d = ?\n", getpid(), op1, oper, op2);


    // send operand 1
    if(write(pipe[1], &op1, uint_size) != uint_size){
        perror("write operand1");
        exit(0);
    }

    // send operation
    if(write(pipe[1], &oper, sizeof(char)) != sizeof(char)){
        perror("write operation");
        exit(0);
    }

    // send operand 2
    if(write(pipe[1], &op2, uint_size) != uint_size){
        perror("write operand2");
        exit(0);
    }
}

void child(int *pipe){
    int uint_size=sizeof(uint8_t);
    uint8_t op1=0, op2=0;
    char oper=' ';

    // read operand1
    if(read(pipe[0], &op1, uint_size)!=uint_size){
        perror("read1");
        exit(0);
    }

    // read operation
    if(read(pipe[0], &oper, uint_size)!=uint_size){
        perror("read2");
        exit(0);
    }

    // read operand2
    if(read(pipe[0], &op2, uint_size)!=uint_size){
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


int main(int argc, char *argv[]){
    srandom(time(NULL));

    if(argc!=2){
        printf("this programm requires only one argument!\n");
        exit(0);
    }

    int 
        iterations=atoi(argv[1]),
        pipe_fd[2];
    
    if(pipe(pipe_fd)<0){
        perror("pipe");
        exit(0);
    }
    
    int pid=fork();
    for (int i=0; i < iterations; i++){
        if(pid>0){
            printf("parent (pid=%d): iteration %d\n", getpid(), i);
            parent(pipe_fd);
        } else child(pipe_fd);
    }


}