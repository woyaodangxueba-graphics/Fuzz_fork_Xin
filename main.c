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
#include <errno.h>

#include "generator.h"

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

// 	int t;
// 	for(t = 0; t < files_number + 3 + 100; t++)
// 		printf("File descriptors # %d : %d \n\n", t, Pool->fd_pool[t]);
	/*******************************************/
	/**************forkchildren()***************/
	/*******************************************/

#define CHILD_NO 4	
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

						while(1)
						{
	
							int para_1 = Pool->fd_pool[rand()%(files_number + 3 + 100)];
							char tmp[32];
							char *para_2 = tmp;
	
							int para_3 = rand()%512;
							fprintf(stdout,"child = %d calling sys_read(%d,%x,%d)\n", getpid(), para_1, (unsigned int)&tmp, para_3);
	
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

	

	int k; 
	for(k = 0; k < cur_dir_num; k++)
		free(Pool->dirs_pool[k]);
		
	free(Pool->fd_pool);


}
