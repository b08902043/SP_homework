#include "threadtools.h"

/*
1) You should state the signal you received by: printf('TSTP signal caught!\n') or printf('ALRM signal caught!\n')
2) If you receive SIGALRM, you should reset alarm() by timeslice argument passed in ./main
3) You should longjmp(SCHEDULER,1) once you're done.
*/
void sighandler(int signo){
	/* Please fill this code section. */
	if(signo == SIGTSTP){
		printf("TSTP signal caught!\n");
		if (sigismember(&waiting_mask, SIGALRM))
        {
            alarm(timeslice);
        }
	}
	else if(signo == SIGALRM){
		printf("ALRM signal caught!\n");
		alarm(timeslice);
	}
	//set back alarm
	//set back signal mask
	sigprocmask(SIG_SETMASK,&base_mask,NULL);
	longjmp(SCHEDULER,1);
}

/*
1) You are stronly adviced to make 
	setjmp(SCHEDULER) = 1 for ThreadYield() case
	setjmp(SCHEDULER) = 2 for ThreadExit() case
2) Please point the Current TCB_ptr to correct TCB_NODE
3) Please maintain the circular linked-list here
*/
void scheduler(){
	//start from Head
	Current = Current->Next;
	int n = setjmp(SCHEDULER);
	if(n == 1){
		//jump from yield
		Current = Current->Next;
	}
	else if(n == 2){
		//jump from exit
		if(Current->Next != Current){
			Current->Prev->Next = Current->Next;
			Current->Next->Prev = Current->Prev;
			Current = Current->Next;
		}
		else{
			exit(0);
		}
	}
	//trap in next function
	longjmp(Current->Environment,1);
	/* Please fill this code section. */
}
