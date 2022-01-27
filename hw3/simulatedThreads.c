#include "threadtools.h"

// Please complete this three functions. You may refer to the macro function defined in "threadtools.h"

// Mountain Climbing
// You are required to solve this function via iterative method, instead of recursion.
void MountainClimbing(int thread_id, int number){
	//first trap in this function
	ThreadInit(thread_id, number);
	//long jump back this fuction
	if(Current->N == 0){
		sleep(1);
		printf("Mountain Climbing: 1\n");
		ThreadYield();
	}
	if(Current->N == 1){
		sleep(1);
		printf("Mountain Climbing: 1\n");
		ThreadYield();
	}
	while(Current->i <= Current->N){
		sleep(1);
		Current->i ++;
		Current->z = Current->x+Current->y;
		Current->y = Current->x;
		Current->x = Current->z;
		printf("Mountain Climbing: %d\n",Current->z);
		ThreadYield();
	}
	ThreadExit();
}

// Reduce Integer
// You are required to solve this function via iterative method, instead of recursion.
void ReduceInteger(int thread_id, int number){
	/* Please fill this code section. */
	ThreadInit(thread_id, number);
	if(Current->N == 1){
		sleep(1);
		printf("Reduce Integer: 0\n");
		ThreadYield();
	}
	while(Current->x != 1){
		sleep(1);
		if(Current->x % 2 == 0){
			Current->x/=2;
		}
		else{
			if(Current->x == 3) Current->x = 2;
			else Current->x = ((Current->x+1)%4 == 0)? Current->x+1:Current->x-1;
		}
		Current->i ++;
		printf("Reduce Integer: %d\n",Current->i);
		ThreadYield();
	}
	ThreadExit();
}

// Operation Count
// You are required to solve this function via iterative method, instead of recursion.
void OperationCount(int thread_id, int number){
	/* Please fill this code section. */
	ThreadInit(thread_id, number);
	int middle;
	if(Current->N%2 == 0){
		Current->w = 1;
	}
	else{
		Current->w = 2;
	}
	while(Current->i < Current->N/2){
		sleep(1);
		Current->x += Current->w;
		Current->w += 2;
		Current->i ++;
		printf("Operation Count: %d\n",Current->x);
		ThreadYield();
	}
	if(Current->N == 1){
		sleep(1);
		printf("Operation Count: %d\n",Current->x);
		ThreadYield();
	}
	ThreadExit();
}
