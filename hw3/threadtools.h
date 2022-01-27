#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

extern int timeslice, switchmode;

typedef struct TCB_NODE *TCB_ptr;
typedef struct TCB_NODE{
    jmp_buf  Environment;
    int      Thread_id;
    TCB_ptr  Next;
    TCB_ptr  Prev;
    int i, N;
    int w, x, y, z;
} TCB;

extern jmp_buf MAIN, SCHEDULER;//jump buffer
extern TCB_ptr Head;
extern TCB_ptr Current;
extern TCB_ptr Work;
extern sigset_t base_mask, waiting_mask, tstp_mask, alrm_mask;

void sighandler(int signo);
void scheduler();

// Call function in the argument that is passed in
#define ThreadCreate(function, thread_id, number)                                         \
{                                                                                         \
    /*first time: call function*/                                                         \
    if(setjmp(MAIN) == 0){                                                                \
        function(thread_id,number);                                                       \
    }                                                                                     \
    /*else : back to main function*/                                                      \
}                                                                                         \
/*Build up TCB_NODE for each function, insert it into circular linked-list*/              
#define ThreadInit(thread_id, number)                                                     \
{                                                                                         \
    TCB_ptr temp = (TCB *)(malloc(sizeof(TCB)));                                          \
    temp->Thread_id = thread_id;                                                          \
    if(thread_id == 1){                                                                   \
        temp->i = 0;                                                                      \
        temp->N = number;                                                                 \
        temp->x = number;                                                                 \
        Head = temp;                                                                      \
        Current = Head;                                                                   \
    }                                                                                     \
    else if(thread_id == 2){                                                              \
        temp->N = number;                                                                 \
        temp->Prev = Head;                                                                \
        temp->x = 1;                                                                      \
        temp->y = 1;                                                                      \
        temp->z = 2;                                                                      \
        temp->i = 2;                                                                      \
        Head->Next = temp;                                                                \
        Current = temp;                                                                   \
    }                                                                                     \
    else{                                                                                 \
        temp->N = number;                                                                 \
        temp->i = 0;                                                           \
        temp->x = 0;                                                                      \
        temp->Prev = Current;                                                             \
        temp->Next = Head;                                                                \
        Head->Prev = temp;                                                                \
        Current->Next = temp;                                                             \
        Current = temp;                                                                   \
    }                                                                                     \
    /*jump back to ThreadCreate , return to main and schedule 3 functions*/               \
    if(setjmp(Current->Environment) == 0){                                                \
        longjmp(MAIN,1);                                                                  \
    }                                                                                     \
    /*jump from schedule: back to functions*/                                             \
}                                                                                         \

// Call this while a thread is terminated
#define ThreadExit()                                                                      \
{                                                                                         \
	longjmp(SCHEDULER,2);                                                                 \
}

// Decided whether to "context switch" based on the switchmode argument passed in main.c
#define ThreadYield()                                                                     \
{                                                                                         \
    int n = setjmp(Current->Environment);                                                 \
    if(n == 0){                                                                           \
        if(switchmode == 0)                                                               \
            longjmp(SCHEDULER,1);                                                         \
        if(switchmode == 1){                                                              \
            /*check which signal has been generate*/                                      \
            sigpending(&waiting_mask);                                                    \
            /*deal with SIGSTP first*/                                                    \
            if(sigismember(&waiting_mask,SIGTSTP)){                                       \
                /*catch SIGSTP and block SIGALRM*/                                        \
                /*call signal_handler*/                                                   \
                sigsuspend(&alrm_mask);                                                   \
            }                                                                             \
            /*deal with SIGALRM*/                                                         \
            if(sigismember(&waiting_mask,SIGALRM)){                                       \
                sigsuspend(&tstp_mask);                                                   \
            }                                                                             \
        }                                                                                 \
    }                                                                                     \
}                                                                                           
