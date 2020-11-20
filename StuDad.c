#include <semaphore.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

int random_int(int min, int max);
void  ParentProcess(int SharedMem[], sem_t* mutex);
void  ChildProcess(int SharedMem[], sem_t* mutex);

int  main(int  argc, char *argv[])
{
	int    ShmID;
	int    *ShmPTR;
	pid_t  pid;
  
	// Allocating the shared memory, getting it's ID back.
  ShmID = shmget(IPC_PRIVATE, 1*sizeof(int), IPC_CREAT | 0666);
  if (ShmID < 0) {
      printf("*** shmget error ***\n");
      exit(1);
  }
  printf("Process has received a shared memory of one integers...\n");

  // Attach the memory segment to our address space.
  ShmPTR = (int *) shmat(ShmID, NULL, 0);
  if (*ShmPTR == -1) {
      printf("*** shmat error (Process) ***\n");
      exit(1);
  }
  printf("Process has attached the shared memory...\n");
	
	// Mutex Creation.
	sem_t *mutex;
	
	/* create, initialize semaphore */
  if ((mutex = sem_open("balancesemaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
    perror("semaphore initilization");
    exit(1);
  }
	
	// Forking.
  printf("Orig Bank Account = %d\n", ShmPTR[0]);
	pid = fork();
	// ERROR.
	if (pid < 0) {
    printf("*** fork error ***\n");
    exit(1);
	}
	// CHILD.
	else if (pid == 0) {
    time_t t;
    srand((unsigned) time(&t));
    ChildProcess(ShmPTR, mutex);
	}
	// Parent.
	else{
    srand(3478);
		ParentProcess(ShmPTR, mutex);
	}
}

void ParentProcess(int SharedMem[], sem_t* mutex)
{
	int localBalance = SharedMem[0];
	// Loop endlessly.
	while(1){
    sleep(random_int(0, 5));
		printf("Dear Old Dad: Attempting to Check Balance\n");
		int chanceNumber = random_int(0, 10);
		if(chanceNumber % 2 == 0){
			if(localBalance < 100){
				// Even number, try to deposit money.
				localBalance = SharedMem[0];
				int amt = random_int(0, 100);
				if(amt % 2 == 0){
					localBalance += amt;
					printf("Dear old Dad: Deposits $%d / Balance = $%d\n", amt, localBalance);
					// Copy contents into shared variable.
					sem_wait(mutex);
					SharedMem[0] = localBalance;
					sem_post(mutex);
				}else{
					printf("Dear old Dad: Doesn't have any money to give\n");
				}
			}
		}else{
			// Odd number, check last balance.
			printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
		}
	}
}


void  ChildProcess(int  SharedMem[], sem_t* mutex)
{
	int localBalance = SharedMem[0];
	// Loop endlessly.
	while(1){
    sleep(random_int(0, 5));
		
		printf("Poor Student: Attempting to Check Balance\n");
		int chanceNumber = random_int(0, 10);
		if(chanceNumber % 2 == 0){
			// Even number, attempt to withdraw money.
			localBalance = SharedMem[0];
			int need = random_int(0, 50);
			printf("Poor Student needs $%d\n", need);
			if(need <= localBalance){
				localBalance -= need;
				printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, localBalance);
			}else{
				printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
			}
			// Copy contents into shared variable.
			sem_wait(mutex);
			SharedMem[0] = localBalance;
			sem_post(mutex);
		}else{
			// Odd number, check last balance.
			printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
		}
	}
}

// https://stackoverflow.com/questions/29381843/generate-random-number-in-range-min-max
int random_int(int min, int max)
{
   return min + rand() % (max+1 - min);
}
