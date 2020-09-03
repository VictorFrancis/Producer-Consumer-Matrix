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

struct matrix_class
{
	int arr1[3][3];
	int arr2[3][3];
};

void *runner(void* argument){
	struct matrix_class *arg = argument;
	printf("Parent Thread: 1st Matrix\n");
	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){
			printf("%d ",arg->arr1[i][j]);
		}
		printf("\n");
	}
	printf("Parent Thread: Second Matrix\n");
	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){
			printf("%d ",arg->arr2[i][j]);
		}
		printf("\n");
	}


	gsl_matrix * m = gsl_matrix_alloc(3,3);
	gsl_matrix * n = gsl_matrix_alloc(3,3);

	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){
			gsl_matrix_set(m,i,j, arg->arr1[i][j]);
			gsl_matrix_set(n,i,j, arg->arr2[i][j]);
		}
	} 
	gsl_matrix_mul_elements(m,n);


	//write to shared memory
	
	int(*bufptr)[3];
	//key_t key = ftok("file",60);
	int shmid = shmget(1222,sizeof(int[3][3]),IPC_CREAT|0666);
	if(shmid==-1){
		perror("memory attached");
		exit(1);
	}else{
		bufptr = shmat(shmid,0,0);
		if(bufptr==(void*)-1){
			perror("memory attached");
			exit(1);
		}else{
			for(int i=0;i<3;i++){
				for(int j=0;j<3;j++){
					bufptr[i][j] = gsl_matrix_get(m,i,j);
				}
			}
			printf("Parent Thread Writting successfull\n");
			shmdt(bufptr);
		}
	}
}

int main(){

	pid_t pid;
	srand(time(0));
	pid = fork();
	if(pid<0){
		//error
		fprintf(stderr, "fork fail\n");
		return 1;
	}else if(pid ==0){
		//child process
		key_t key = ftok("arrayfile",65);
		key_t key1 = ftok("arrayfile1",64);
		int(*buf1ptr)[3];
		int(*buf2ptr)[3];
		int shmid = shmget(key,sizeof(int[3][3]),IPC_CREAT|0666);
		int shmid1 = shmget(key1,sizeof(int[3][3]),IPC_CREAT|0666);
		if(shmid==-1 && shmid1==-1){
			perror("memmory attached");
			exit(1);
		}else{
			printf("Child creating new shared memory\n");
			buf1ptr = shmat(shmid,0,0);
			buf2ptr = shmat(shmid1,0,0);
			if(buf1ptr==(void*)-1 && buf2ptr==(void*)-1){
				perror("shmat");
				exit(1);
			}else{
				int i;
				for(i=0;i<3;i++){
					int j;
					for(j=0;j<3;j++){
						buf1ptr[i][j]=rand()%100;
						buf2ptr[i][j]=rand()%100;
					}
				}
				printf("Child writing successfull\n");
				shmdt(buf1ptr);
				shmdt(buf1ptr);
			}//end of array writting
		}//end of shared memory
	}else{
		//parent process
		wait(NULL);
		printf("Parent reading from shared memory\n");
		int shmid,shmid1;
		int(*array)[3];
		int(*array1)[3];
		key_t key = ftok("arrayfile",65);
		key_t key1 = ftok("arrayfile1",64);
		shmid = shmget(key,sizeof(int[3][3]),IPC_CREAT|0666);
		shmid1 = shmget(key1,sizeof(int[3][3]),IPC_CREAT|0666);

		array = shmat(shmid,0,SHM_RDONLY);
		array1 = shmat(shmid1,0,SHM_RDONLY);
		//passing array into thread
		//write thread here
		struct matrix_class arg;
		for(int i=0;i<3;i++){
			for(int j=0;j<3;j++){
				arg.arr1[i][j] = array[i][j];
				arg.arr2[i][j] = array1[i][j];
			}
		}
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&tid,&attr,runner,&arg);
		pthread_join(tid,NULL); 

		shmdt(array);
		shmdt(array1);
		shmctl(shmid,IPC_RMID,NULL);
		shmctl(shmid1,IPC_RMID,NULL);
	}
	return 0;
}
