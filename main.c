/*this is our main*/
#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#include "generator.h"

#define CHILD_NO 4

int main (int argc, char *argv[])
{
	//get path from argv
	if (argc != 3) {
       fprintf(stderr,"usage:\t%s -v victimfile(absolutepath)\n", argv[0]);
       exit(0);
    }
    
    char *v = "-v";
    char *dir_path;
    
    if (strcmp(argv[1], v) == 0){
         dir_path = argv[2];
         //fprintf(stderr,"%s\n",dir_path);
        }
    else
    {
    	fprintf(stderr,"invalid arguments\n");
    	fprintf(stderr,"usage:\t%s -v victimfile(absolutepath)\n", argv[0]);
    	exit(0);
    } 
	
	/*******************************************/
	/*******************init()******************/
	/*******************************************/
	//1. figure out memory needed for para_pool and malloc to it
	
	struct para_pool *Pool = (struct para_pool*)malloc(sizeof(struct para_pool));
	int files_number = 0;
	
	generator_init(dir_path, &files_number);
	
	//malloc additional 3 for fd 0,1,2, 100 for "invalid_pool"
	Pool->fd_pool = ( int * )malloc(sizeof(int) * (files_number + 3 + 100));
	
	//2. generator file descriptors pool
	int fd_index = 0;
	int *fd_index_p = &fd_index;
	generator_fd_dir(Pool, dir_path, fd_index_p);
	int cur_dir_num = fd_index + 1;
	
	// loop over remaining fd[] and full them with random numbers
	srand(time(NULL));
	    
	while (fd_index < files_number)
	{
		Pool->fd_pool[fd_index] = rand();
		fd_index++;
	}

	//3. create log files FILE * child_log[].
	
	FILE child_log[CHILD_NO];

	
	/*******************************************/
	/**************forkchildren()***************/
	/*******************************************/

	
    pid_t childPID[CHILD_NO];

	
	for (int i = 0; i < CHILD_NO; i++)

	{
		    childPID[i] = fork();
			if(childPID[i] >= 0) // fork was successful
			{
				if(childPID[i] == 0) // child process
				{
						/*******************************************/
						/*******************syscall()***************/
						/*******************************************/
						
						//after fork(), child will reinit seed to 0
						srand(time(NULL));
						
						//open a log file, prepare to append new text.
						
						char log_name[] = "./log/log_";
						strcat(log_name,itoa(i));
						 
						child_log[i] = fopen(log_name, "a");
						
						
						while(1)
						{
	
							int para_1 = Pool->fd_pool[rand()%(files_number + 3 + 100)];
							char tmp[32];
							char *para_2 = tmp;
	
							int para_3 = rand()%512;
							 
							  
      						
							if (para_1)
								fprintf(child_log[i],"child = %d calling sys_read(%d,%x,%d)\n", getpid(), para_1, (unsigned int)&tmp, para_3);
	
							int ret = 0;
							// skip fd = 0, since it will read inputs from screen
							if (para_1)
							{
								ret = syscall(SYS_read, para_1, para_2, para_3);
		
								if (ret == -1)
								{
									//int errsv = errno;
									fprintf(stdout, "child = %d sys_read failed with errno = %d\n", getpid(), errno);
								}else 
								{
									fprintf(stdout, "child = %d sys_read success with %s\n", getpid(), para_2 );
								}
							}
	
						}

					return 0;
				}
				
			}
			else // fork failed
			{
				printf("\n Fork failed, quitting!!!!!!\n");
				return 1;
			}
	
	}

	
	//Wait signal from children, if one was killed respawn one.
	
	int status;
	while(1)
	
	{
		for (int i = 0; i < CHILD_NO; i++)
	{
		pid_t result = waitpid(childPID[i], &status, WNOHANG);
		if (result == 0) {
		  //fprintf(stdout, "Child = %d still alive\n",childPID[i]);
		} else if (result == -1) {
		   //fprintf(stdout, "Child = %d error\n",childPID[i]);
		} else {
		  //fprintf(stdout, "Child = %d exit and we respawn it here\n",childPID[i]);
		  
		  //respawn child here.
		  
		  childPID[i] = fork();
			if(childPID[i] >= 0) // fork was successful
			{
				if(childPID[i] == 0) // child process
				{
						/*******************************************/
						/*******************syscall()***************/
						/*******************************************/
						
						//after fork(), child will reinit seed to 0
						srand(time(NULL));
						
						//open a log file, prepare to append new text.
						
						char log_name[] = "./log/log_";
						strcat(log_name,itoa(i));
						 
						child_log[i] = fopen(log_name, "a");
						
						while(1)
						{
	
							int para_1 = Pool->fd_pool[rand()%(files_number + 3 + 100)];
							char tmp[32];
							char *para_2 = tmp;
	
							int para_3 = rand()%512;
							if (para_1)
								fprintf(child_log[i],"child = %d calling sys_read(%d,%x,%d)\n", getpid(), para_1, (unsigned int)&tmp, para_3);
	
							int ret = 0;
							// skip fd = 0, since it will read inputs from screen
							if (para_1)
							{
								ret = syscall(SYS_read, para_1, para_2, para_3);
		
								if (ret == -1)
								{
									//int errsv = errno;
									fprintf(stdout, "child = %d sys_read failed with errno = %d\n", getpid(), errno);
								}else 
								{
									fprintf(stdout, "child = %d sys_read success with %s\n", getpid(), para_2 );
								}
							}
	
						}

					return 0;
				}
				
			}
			else // fork failed
			{
				printf("\n Fork failed, quitting!!!!!!\n");
				return 1;
			}
		}
	}
	}
	
	

	int k; 
	for(k = 0; k < cur_dir_num; k++)
		free(Pool->dirs_pool[k]);
		
	free(Pool->fd_pool);


}
