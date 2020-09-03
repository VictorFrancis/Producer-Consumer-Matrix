#include <sys/shm.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <gsl/gsl_matrix.h>
#include <pthread.h>

int main(){
	int shmid;
	int (*bufptr)[3];
	key_t key = ftok("file",60);
	shmid = shmget(1222,sizeof(int[3][3]),IPC_CREAT|0666);
	bufptr = shmat(shmid,0,SHM_RDONLY);
	if(bufptr == (void *)-1){
		perror("memory attached");
		exit(1);
	}else{
		printf("Consumer Program: Result Matrix \n");
		for(int i=0;i<3;i++){
			for(int j=0;j<3;j++){
				printf("%d ",bufptr[i][j]);
			}
			printf("\n");
		}
	}
	printf("\nRead to memory successful--\n");
	shmdt(bufptr);
	shmctl(shmid,IPC_RMID,NULL);
	return 0;
}
//consumer
