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
#include "syscalls.h"

#define CHILD_NO 4
#define SYSCALL_NUM 12

int flag_log = 0;
int flag_debug = 0;

int main (int argc, char *argv[])
{
	//get path from argv
	if (argc < 3) {
       fprintf(stderr,"usage:\t%s -v victimfile(absolutepath)\n", argv[0]);
       exit(0);
    }
    
    char *v = "-v";
    char *l = "-l";
    char *d = "-d";//debug
    char *dir_path;
    
    for (int i = 1;i < argc;  )
    {
    	if (strcmp(argv[i], v) == 0){
        	dir_path = argv[i+1];
        	i+=2;
        }
		else if (strcmp(argv[i], l) == 0)
		{
			flag_log = 1;
			i++;
			fprintf(stdout,"log mod activated.\n");
		}
		else if (strcmp(argv[i], d) == 0)
		{
			flag_log = 1;
			i++;
			fprintf(stdout,"debug mod activated.\n");
		}
		else{
			fprintf(stderr,"invalid arguments.\n");
			fprintf(stderr,"usage:\t%s -v victimfile(absolutepath)\n", argv[0]);
			exit(0);
		} 
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
	generator_mod(Pool);
	int cur_dir_num = fd_index + 1;
	
	// loop over remaining fd[] and full them with random numbers
	srand(time(NULL));
	    
	while (fd_index < files_number + 3 + 100)
	{
		Pool->fd_pool[fd_index] = rand();
		fd_index++;
	}

	//3. create log files FILE * child_log[].

	FILE * child_log[CHILD_NO];
	//open a log file, prepare to append new text.
 
	if(flag_log)
	{
		
		for(int i = 0;i < CHILD_NO; i++)
		{
			char log_name[12] = "./log/log_";
			log_name[10] = '0' +i;
			log_name[11] ='\0';
			//strcat(log_name,temp);
			fprintf(stdout,"log_name = %s\n",log_name); 
			child_log[i] = fopen(log_name, "a");

		}
	}
	
 	
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
						//after fork(), child will reinit seed to 0
						srand(time(NULL));
						
						while(1)
						{
						/*******************************************/
						/**************choose syscall()*************/
						/*******************************************/
						
						int index_chosen_syscall = rand()%SYSCALL_NUM; // SYSCALL_NUM is syscall numbers this draft supports right now.

						/*******************************************/
						/*****************rand para()***************/
						/*******************************************/
						
						struct fuzz_sys_call child_copy_syscall = {
						
							.entrypoint = fuzz_sys_call_table[index_chosen_syscall].entrypoint,
							.para1 = fuzz_sys_call_table[index_chosen_syscall].para1,
							.para2 = fuzz_sys_call_table[index_chosen_syscall].para2,
							.para3 = fuzz_sys_call_table[index_chosen_syscall].para3,
							.para4 = fuzz_sys_call_table[index_chosen_syscall].para4,
							.para5 = fuzz_sys_call_table[index_chosen_syscall].para5,
							.para6 = fuzz_sys_call_table[index_chosen_syscall].para6
						
						};
						
						switch(index_chosen_syscall)
						{
							case 0:
							//sys_read
							child_copy_syscall.para1 = Pool->fd_pool[rand()%(files_number + 3 + 100)];
							
							if(rand()%2)
								{
								static	char tmp_buf_0[32];
									child_copy_syscall.para2 = (unsigned long)tmp_buf_0;
								}
							else
								{
									child_copy_syscall.para2 = (unsigned long)rand();
								}
							
							child_copy_syscall.para3 = rand()%512;
							
							break;
						
							case 1:
							//sys_chmod
							child_copy_syscall.para1 =(unsigned long) Pool->dirs_pool[rand()%1000];
							if(rand()%2)
								{
									child_copy_syscall.para2 = Pool->mode_pool[rand()%27];
								}
							else
								{
									child_copy_syscall.para2 = (unsigned int)rand();
								}
							break;

							case 2:
								//sys_write
								child_copy_syscall.para1 = Pool->fd_pool[rand() % (files_number + 3 + 100)];

								if (rand() % 2)
								{
								static	char tmp_buf_2[32];

									for (int i; i < 32; i++)   // To genterate ramdom content in the buffer.
										tmp_buf_2[i] = (char)rand() % 128;

									child_copy_syscall.para2 = (unsigned long)tmp_buf_2;
								}
								else
								{
									child_copy_syscall.para2 = (unsigned long)rand();
								}

								child_copy_syscall.para3 = rand() % 512;

								break;

							case 3: /* sys_chdir() */
								child_copy_syscall.para1 = (unsigned long)Pool->dirs_pool[rand() % (cur_dir_num + 100)];

								break;

							case 4: /* sys_fchidr() */

								{
								unsigned int tmp4 = 0;

								for(;(tmp4 == 0) ||(tmp4 == 1)||(tmp4 == 2); )
								{
									tmp4 = Pool->fd_pool[rand() % (files_number + 3 + 100)];
								}
								child_copy_syscall.para1 =  tmp4;
								}

								break;

							case 5: /* sys_mkdir */
								child_copy_syscall.para1 = (unsigned long)Pool->dirs_pool[rand() % 1000];
								if (rand() % 2)
								{
									child_copy_syscall.para2 = Pool->mode_pool[rand() % 27];
								}
								else
								{
									child_copy_syscall.para2 = (unsigned int)rand();
								}
								break;

							case 6: /* sys_rmdir */
								child_copy_syscall.para1 = (unsigned long)Pool->dirs_pool[rand() % 1000];
								
								break;

							case 7: /* sys_rename */
								child_copy_syscall.para1 = (unsigned long)Pool->dirs_pool[rand() % 1000];
								child_copy_syscall.para2 = (unsigned long)Pool->dirs_pool[rand() % 1000];

								break;

							case 8:
							//sys_time
								{
								time_t *tmp_time = (time_t*)malloc(sizeof(time_t));
								child_copy_syscall.para1 = (unsigned long)tmp_time;
								}

								break;

							case 9:
							// sys_readlink
								child_copy_syscall.para1 = (unsigned long)Pool->dirs_pool[rand() % 1000];

								if (rand() % 2)
								{
								static	char tmp_buf_9[32];

									for (int i; i < 32; i++)   // To genterate ramdom content in the buffer.
										tmp_buf_9[i] = (char)rand() % 128;

									child_copy_syscall.para2 = (unsigned long)tmp_buf_9;
								}
								else
								{
									child_copy_syscall.para2 = (unsigned long)rand();
								}

								child_copy_syscall.para3 = (unsigned long)rand() % 512;

								break;

							case 10: //sys_fchmod
								{
								unsigned int tmp10 = 0;
								for(;(tmp10 == 0) ||(tmp10 == 1)||(tmp10 == 2) ; )
								{
									tmp10 = Pool->fd_pool[rand() % (files_number + 3 + 100)];
								}
								child_copy_syscall.para1 =  tmp10;
								}

								if (rand() % 2)
								{
								static	char tmp_buf_10[32];

									for (int i; i < 32; i++)   // To genterate ramdom content in the buffer.
										tmp_buf_10[i] = (char)rand() % 128;

									child_copy_syscall.para2 = (unsigned long)tmp_buf_10;
								}
								else
								{
									child_copy_syscall.para2 = (unsigned long)rand();
								}

								break;

							case 11:/*sys_sync*/

								/* No argument required*/

								break;
						
							default:
							fprintf(stderr," Something is terribly WRONG! Do something to fix your rand_para switch!\n");
						
						
						}
						
						/*******************************************/
						/**********************log()****************/
						/*******************************************/
						
						switch(index_chosen_syscall)
						{
							case 0:
							//sys_read
							if (child_copy_syscall.para1)
							//skip para1 = 0 , which is indeed fd =0. Reading from keyboard is annoying. 
								{
								if(flag_log)
									fprintf(child_log[i],"child = %d calling sys_read(%d,%x,%d)\n", getpid(), (int)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2, (int)child_copy_syscall.para3);
								if(flag_debug) fprintf(stdout,"child = %d calling sys_read(%d,%x,%d)\n", getpid(), (int)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2,(int) child_copy_syscall.para3);
								}
							
							break;
						
							case 1:
							//sys_chmod
							if(flag_log)
									fprintf(child_log[i],"child = %d calling sys_chmod(%s,%o)\n", getpid(), (char*)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2);
								if(flag_debug) fprintf(stdout,"child = %d calling sys_chmod(%s,%o)\n", getpid(), (char*)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2);
							break;

							case 2:
								//sys_write
								if (child_copy_syscall.para1)
									//skip para1 = 0 , which is indeed fd =0. Reading from keyboard is annoying. 
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d calling sys_write(%d,%x,%d)\n", getpid(), (int)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2, (int)child_copy_syscall.para3);
									if(flag_debug) fprintf(stdout, "child = %d calling sys_write(%d,%x,%d)\n", getpid(), (int)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2, (int)child_copy_syscall.para3);
								}

								break;

							case 3: /* sys_chdir(const char* dir) */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_chdir(%s)\n", getpid(), (char*)child_copy_syscall.para1);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_chdir(%s)\n", getpid(), (char*)child_copy_syscall.para1);
								break;

							case 4: /* sys_fchdir(ind fd)  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_fchdir(%d)\n", getpid(), (int)child_copy_syscall.para1);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_fchdir(%d)\n", getpid(), (int)child_copy_syscall.para1);

								break;

							case 5: /* sys_mkdir  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_mkdir(%s,%o)\n", getpid(), (char*)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2);
									 if(flag_debug) fprintf(stdout,"child = %d calling sys_mkdir(%s,%o)\n", getpid(), (char*)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2);
								break;

							case 6: /* sys_rmdir  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_rmdir(%s)\n", getpid(), (char*)child_copy_syscall.para1);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_rmdir(%s)\n", getpid(), (char*)child_copy_syscall.para1);
								break;

							case 7: /* sys_rename  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_rename(%s, %s)\n", getpid(), (char*)child_copy_syscall.para1, (char*)child_copy_syscall.para2);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_rename(%s, %s)\n", getpid(), (char*)child_copy_syscall.para1, (char*)child_copy_syscall.para2);
								break;

							case 8: /* sys_time  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_time(%p)\n", getpid(), (time_t*)child_copy_syscall.para1);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_time(%p)\n", getpid(), (time_t*)child_copy_syscall.para1);
								break;
						
							case 9: /* sys_readlink  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_readlink(%s, %u, %d)\n", getpid(), (char *)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2, (int)child_copy_syscall.para3);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_readlink(%s, %u, %d)\n", getpid(), (char *)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2, (int)child_copy_syscall.para3);
								break;

							case 10: /* sys_fchmod  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_fchmnod(%d, %d)\n", getpid(), (int)child_copy_syscall.para1, (int)child_copy_syscall.para2);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_fchmod(%d, %d)\n", getpid(), (int)child_copy_syscall.para1, (int)child_copy_syscall.para2);
								break;

							case 11: /* sys_sync  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_sync()\n", getpid());
								if(flag_debug) fprintf(stdout, "child = %d calling sys_sync()\n", getpid());
								break;

							default:
								fprintf(stderr," Something is terribly WRONG! Do something to fix your log switch!\n");
						
						
						}
						
						/*******************************************/
						/******************syscall()****************/
						/*******************************************/
						
						int ret = 0;
						
						switch(index_chosen_syscall)
						{
							case 0:
							//sys_read
							//skip para1 = 0 , which is inded fd =0. Reading from keyboard is annoying. 
							
							if (child_copy_syscall.para1)
							{
								ret = syscall(child_copy_syscall.entrypoint,
											child_copy_syscall.para1,
											child_copy_syscall.para2,
											child_copy_syscall.para3);
		
								if (ret == -1)
								{
									//int errsv = errno;
									if(flag_log)
										fprintf(child_log[i], "child = %d sys_read failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_read failed with errno = %d\n", getpid(), errno);
								}else 
								{
									if(flag_log)
										fprintf(child_log[i], "child = %d sys_read success with %s\n", getpid(),(char *) child_copy_syscall.para2 );
									if(flag_debug) fprintf(stdout, "child = %d sys_read success \n", getpid());
								}
							}
							break;
						
							case 1:
							//sys_chmod
							
							ret = syscall(child_copy_syscall.entrypoint,
											child_copy_syscall.para1,
											child_copy_syscall.para2);
		
							if (ret == -1)
							{
								//int errsv = errno;
								if(flag_log)
									fprintf(child_log[i], "child = %d sys_chmod failed with errno = %d\n", getpid(), errno);
								if(flag_debug) fprintf(stdout, "child = %d sys_chmod failed with errno = %d\n", getpid(), errno);
							}
							else
							{
								if (flag_log)
									fprintf(child_log[i], "child = %d sys_chmod success \n", getpid());
								if(flag_debug) fprintf(stdout, "child = %d sys_chmod success \n", getpid());
							}
						
							break;

							case 2:
								//sys_write
								//skip para1 = 0 , which is inded fd =0. Reading from keyboard is annoying. 

								if (child_copy_syscall.para1)
								{
									ret = syscall(child_copy_syscall.entrypoint,
										child_copy_syscall.para1,
										child_copy_syscall.para2,
										child_copy_syscall.para3);

									if (ret == -1)
									{
										//int errsv = errno;
										if (flag_log)
											fprintf(child_log[i], "child = %d sys_write failed with errno = %d\n", getpid(), errno);
										if(flag_debug) fprintf(stdout, "child = %d sys_write failed with errno = %d\n", getpid(), errno);
									}
									else
									{
										if (flag_log)
											fprintf(child_log[i], "child = %d sys_write success with %s\n", getpid(), (char *)child_copy_syscall.para2);
										if(flag_debug) fprintf(stdout, "child = %d sys_write success \n", getpid());
									}
								}
								break;
						
							case 3:
							// sys_chdir
								ret = syscall(child_copy_syscall.entrypoint, child_copy_syscall.para1);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_chdir failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_dir failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_chdir success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_chdir success \n", getpid());
								}

								break;

							case 4: /* sys_fchdir(int fd) */
								ret = syscall(child_copy_syscall.entrypoint, child_copy_syscall.para1);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_fchdir failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_fdir failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_fchdir success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_fchdir success \n", getpid());
								}

								break;

							case 5:
								//sys_mkdir

								ret = syscall(child_copy_syscall.entrypoint,
									child_copy_syscall.para1,
									child_copy_syscall.para2);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_mkdir failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_mkdir failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_mkdir success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_mkdir success \n", getpid());
								}

								break;

							case 6:
								//sys_rmdir

								ret = syscall(child_copy_syscall.entrypoint,
									child_copy_syscall.para1);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_rmdir failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_rmdir failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_rmdir success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_rmdir success \n", getpid());
								}

								break;

							case 7:
								//sys_rename

								ret = syscall(child_copy_syscall.entrypoint,
									child_copy_syscall.para1, child_copy_syscall.para2);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_rename failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_rename failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_rename success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_rename success \n", getpid());
								}

								break;

							case 8:
								//sys_time

								ret = syscall(child_copy_syscall.entrypoint,
									(time_t*)child_copy_syscall.para1);

								free((time_t*)child_copy_syscall.para1);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_time failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_time failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_time success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_time success \n", getpid());
								}

								break;

							case 9:
								//sys_readlink

								ret = syscall(child_copy_syscall.entrypoint,
									child_copy_syscall.para1, child_copy_syscall.para2, child_copy_syscall.para3);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_readlink failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_readlink failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_readlink success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_readlink success \n", getpid());
								}

								break;

							case 10:
								//sys_fchmod

								ret = syscall(child_copy_syscall.entrypoint,
									child_copy_syscall.para1, child_copy_syscall.para2);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_fchmod failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_fchmod failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_fchmod success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_fchmod success \n", getpid());
								}

								break;

							case 11:
								//sys_sync

								ret = syscall(child_copy_syscall.entrypoint);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_sync failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_sync failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_sync success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_sync success \n", getpid());
								}

								break;

							default:
								fprintf(stderr," Something is terribly WRONG! Do something to fix your log switch!\n");
							}
						//end of while loop
					}
					
					//return 0;
						//end of child proc
				}
						
						//parent proc
			}
			
			else // fork failed
			{
				printf("\n Forkchild = %d fork failed, quitting!!!!!!\n", i);
				return 1;
			}
	

	}

	/*******************************************/
	/******************watchdog()***************/
	/*******************************************/
	
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
		   fprintf(stderr, "Child = %d return result = -1  error\n",childPID[i]);
		   exit(1);
		} else {
			fprintf(stderr, "Child = %d return result = %d error\n",childPID[i],result);
		   exit(1);
				  //fprintf(stdout, "Child = %d exit and we respawn it here\n",childPID[i]);
		  
				  //respawn child here.
		  		 
				childPID[i] = fork();
			if(childPID[i] >= 0) // fork was successful
			{
				if(childPID[i] == 0) // child process
				{
						//after fork(), child will reinit seed to 0
						srand(time(NULL));
						
						while(1)
						{
						/*******************************************/
						/**************choose syscall()*************/
						/*******************************************/
						
						int index_chosen_syscall = rand()%SYSCALL_NUM; // SYSCALL_NUM is syscall numbers this draft supports right now.

						/*******************************************/
						/*****************rand para()***************/
						/*******************************************/
						
						struct fuzz_sys_call child_copy_syscall = {
						
							.entrypoint = fuzz_sys_call_table[index_chosen_syscall].entrypoint,
							.para1 = fuzz_sys_call_table[index_chosen_syscall].para1,
							.para2 = fuzz_sys_call_table[index_chosen_syscall].para2,
							.para3 = fuzz_sys_call_table[index_chosen_syscall].para3,
							.para4 = fuzz_sys_call_table[index_chosen_syscall].para4,
							.para5 = fuzz_sys_call_table[index_chosen_syscall].para5,
							.para6 = fuzz_sys_call_table[index_chosen_syscall].para6
						
						};
						
						switch(index_chosen_syscall)
						{
							case 0:
							//sys_read
							child_copy_syscall.para1 = Pool->fd_pool[rand()%(files_number + 3 + 100)];
							
							if(rand()%2)
								{
								static	char tmp_buf_0[32];
									child_copy_syscall.para2 = (unsigned long)tmp_buf_0;
								}
							else
								{
									child_copy_syscall.para2 = (unsigned long)rand();
								}
							
							child_copy_syscall.para3 = rand()%512;
							
							break;
						
							case 1:
							//sys_chmod
							child_copy_syscall.para1 =(unsigned long) Pool->dirs_pool[rand()%1000];
							if(rand()%2)
								{
									child_copy_syscall.para2 = Pool->mode_pool[rand()%27];
								}
							else
								{
									child_copy_syscall.para2 = (unsigned int)rand();
								}
							break;

							case 2:
								//sys_write
								child_copy_syscall.para1 = Pool->fd_pool[rand() % (files_number + 3 + 100)];

								if (rand() % 2)
								{
								static	char tmp_buf_2[32];

									for (int i; i < 32; i++)   // To genterate ramdom content in the buffer.
										tmp_buf_2[i] = (char)rand() % 128;

									child_copy_syscall.para2 = (unsigned long)tmp_buf_2;
								}
								else
								{
									child_copy_syscall.para2 = (unsigned long)rand();
								}

								child_copy_syscall.para3 = rand() % 512;

								break;

							case 3: /* sys_chdir() */
								child_copy_syscall.para1 = (unsigned long)Pool->dirs_pool[rand() % (cur_dir_num + 100)];

								break;

							case 4: /* sys_fchidr() */

								{
								unsigned int tmp4 = 0;

								for(;(tmp4 == 0) ||(tmp4 == 1)||(tmp4 == 2); )
								{
									tmp4 = Pool->fd_pool[rand() % (files_number + 3 + 100)];
								}
								child_copy_syscall.para1 =  tmp4;
								}

								break;

							case 5: /* sys_mkdir */
								child_copy_syscall.para1 = (unsigned long)Pool->dirs_pool[rand() % 1000];
								if (rand() % 2)
								{
									child_copy_syscall.para2 = Pool->mode_pool[rand() % 27];
								}
								else
								{
									child_copy_syscall.para2 = (unsigned int)rand();
								}
								break;

							case 6: /* sys_rmdir */
								child_copy_syscall.para1 = (unsigned long)Pool->dirs_pool[rand() % 1000];
								
								break;

							case 7: /* sys_rename */
								child_copy_syscall.para1 = (unsigned long)Pool->dirs_pool[rand() % 1000];
								child_copy_syscall.para2 = (unsigned long)Pool->dirs_pool[rand() % 1000];

								break;

							case 8:
							//sys_time
								{
								time_t *tmp_time = (time_t*)malloc(sizeof(time_t));
								child_copy_syscall.para1 = (unsigned long)tmp_time;
								}

								break;

							case 9:
							// sys_readlink
								child_copy_syscall.para1 = (unsigned long)Pool->dirs_pool[rand() % 1000];

								if (rand() % 2)
								{
								static	char tmp_buf_9[32];

									for (int i; i < 32; i++)   // To genterate ramdom content in the buffer.
										tmp_buf_9[i] = (char)rand() % 128;

									child_copy_syscall.para2 = (unsigned long)tmp_buf_9;
								}
								else
								{
									child_copy_syscall.para2 = (unsigned long)rand();
								}

								child_copy_syscall.para3 = (unsigned long)rand() % 512;

								break;

							case 10: //sys_fchmod
								{
								unsigned int tmp10 = 0;
								for(;(tmp10 == 0) ||(tmp10 == 1)||(tmp10 == 2) ; )
								{
									tmp10 = Pool->fd_pool[rand() % (files_number + 3 + 100)];
								}
								child_copy_syscall.para1 =  tmp10;
								}

								if (rand() % 2)
								{
								static	char tmp_buf_10[32];

									for (int i; i < 32; i++)   // To genterate ramdom content in the buffer.
										tmp_buf_10[i] = (char)rand() % 128;

									child_copy_syscall.para2 = (unsigned long)tmp_buf_10;
								}
								else
								{
									child_copy_syscall.para2 = (unsigned long)rand();
								}

								break;

							case 11:/*sys_sync*/

								/* No argument required*/

								break;
						
							default:
							fprintf(stderr," Something is terribly WRONG! Do something to fix your rand_para switch!\n");
						
						
						}
						
						/*******************************************/
						/**********************log()****************/
						/*******************************************/
						
						switch(index_chosen_syscall)
						{
							case 0:
							//sys_read
							if (child_copy_syscall.para1)
							//skip para1 = 0 , which is indeed fd =0. Reading from keyboard is annoying. 
								{
								if(flag_log)
									fprintf(child_log[i],"child = %d calling sys_read(%d,%x,%d)\n", getpid(), (int)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2, (int)child_copy_syscall.para3);
								if(flag_debug) fprintf(stdout,"child = %d calling sys_read(%d,%x,%d)\n", getpid(), (int)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2,(int) child_copy_syscall.para3);
								}
							
							break;
						
							case 1:
							//sys_chmod
							if(flag_log)
									fprintf(child_log[i],"child = %d calling sys_chmod(%s,%o)\n", getpid(), (char*)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2);
								if(flag_debug) fprintf(stdout,"child = %d calling sys_chmod(%s,%o)\n", getpid(), (char*)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2);
							break;

							case 2:
								//sys_write
								if (child_copy_syscall.para1)
									//skip para1 = 0 , which is indeed fd =0. Reading from keyboard is annoying. 
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d calling sys_write(%d,%x,%d)\n", getpid(), (int)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2, (int)child_copy_syscall.para3);
									if(flag_debug) fprintf(stdout, "child = %d calling sys_write(%d,%x,%d)\n", getpid(), (int)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2, (int)child_copy_syscall.para3);
								}

								break;

							case 3: /* sys_chdir(const char* dir) */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_chdir(%s)\n", getpid(), (char*)child_copy_syscall.para1);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_chdir(%s)\n", getpid(), (char*)child_copy_syscall.para1);
								break;

							case 4: /* sys_fchdir(ind fd)  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_fchdir(%d)\n", getpid(), (int)child_copy_syscall.para1);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_fchdir(%d)\n", getpid(), (int)child_copy_syscall.para1);

								break;

							case 5: /* sys_mkdir  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_mkdir(%s,%o)\n", getpid(), (char*)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_mkdir(%s,%o)\n", getpid(), (char*)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2);
								break;

							case 6: /* sys_rmdir  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_rmdir(%s)\n", getpid(), (char*)child_copy_syscall.para1);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_rmdir(%s)\n", getpid(), (char*)child_copy_syscall.para1);
								break;

							case 7: /* sys_rename  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_rename(%s, %s)\n", getpid(), (char*)child_copy_syscall.para1, (char*)child_copy_syscall.para2);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_rename(%s, %s)\n", getpid(), (char*)child_copy_syscall.para1, (char*)child_copy_syscall.para2);
								break;

							case 8: /* sys_time  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_time(%p)\n", getpid(), (time_t*)child_copy_syscall.para1);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_time(%p)\n", getpid(), (time_t*)child_copy_syscall.para1);
								break;
						
							case 9: /* sys_readlink  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_readlink(%s, %u, %d)\n", getpid(), (char *)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2, (int)child_copy_syscall.para3);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_readlink(%s, %u, %d)\n", getpid(), (char *)child_copy_syscall.para1, (unsigned int)child_copy_syscall.para2, (int)child_copy_syscall.para3);
								break;

							case 10: /* sys_fchmod  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_fchmnod(%d, %d)\n", getpid(), (int)child_copy_syscall.para1, (int)child_copy_syscall.para2);
								if(flag_debug) fprintf(stdout, "child = %d calling sys_fchmod(%d, %d)\n", getpid(), (int)child_copy_syscall.para1, (int)child_copy_syscall.para2);
								break;

							case 11: /* sys_sync  */
								if (flag_log)
									fprintf(child_log[i], "child = %d calling sys_sync()\n", getpid());
								if(flag_debug) fprintf(stdout, "child = %d calling sys_sync()\n", getpid());
								break;

							default:
								fprintf(stderr," Something is terribly WRONG! Do something to fix your log switch!\n");
						
						
						}
						
						/*******************************************/
						/******************syscall()****************/
						/*******************************************/
						
						int ret = 0;
						
						switch(index_chosen_syscall)
						{
							case 0:
							//sys_read
							//skip para1 = 0 , which is inded fd =0. Reading from keyboard is annoying. 
							
							if (child_copy_syscall.para1)
							{
								ret = syscall(child_copy_syscall.entrypoint,
											child_copy_syscall.para1,
											child_copy_syscall.para2,
											child_copy_syscall.para3);
		
								if (ret == -1)
								{
									//int errsv = errno;
									if(flag_log)
										fprintf(child_log[i], "child = %d sys_read failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_read failed with errno = %d\n", getpid(), errno);
								}else 
								{
									if(flag_log)
										fprintf(child_log[i], "child = %d sys_read success with %s\n", getpid(),(char *) child_copy_syscall.para2 );
									if(flag_debug) fprintf(stdout, "child = %d sys_read success \n", getpid());
								}
							}
							break;
						
							case 1:
							//sys_chmod
							
							ret = syscall(child_copy_syscall.entrypoint,
											child_copy_syscall.para1,
											child_copy_syscall.para2);
		
							if (ret == -1)
							{
								//int errsv = errno;
								if(flag_log)
									fprintf(child_log[i], "child = %d sys_chmod failed with errno = %d\n", getpid(), errno);
								if(flag_debug) fprintf(stdout, "child = %d sys_chmod failed with errno = %d\n", getpid(), errno);
							}
							else
							{
								if (flag_log)
									fprintf(child_log[i], "child = %d sys_chmod success \n", getpid());
								if(flag_debug) fprintf(stdout, "child = %d sys_chmod success \n", getpid());
							}
						
							break;

							case 2:
								//sys_write
								//skip para1 = 0 , which is inded fd =0. Reading from keyboard is annoying. 

								if (child_copy_syscall.para1)
								{
									ret = syscall(child_copy_syscall.entrypoint,
										child_copy_syscall.para1,
										child_copy_syscall.para2,
										child_copy_syscall.para3);

									if (ret == -1)
									{
										//int errsv = errno;
										if (flag_log)
											fprintf(child_log[i], "child = %d sys_write failed with errno = %d\n", getpid(), errno);
										if(flag_debug) fprintf(stdout, "child = %d sys_write failed with errno = %d\n", getpid(), errno);
									}
									else
									{
										if (flag_log)
											fprintf(child_log[i], "child = %d sys_write success with %s\n", getpid(), (char *)child_copy_syscall.para2);
										if(flag_debug) fprintf(stdout, "child = %d sys_write success \n", getpid());
									}
								}
								break;
						
							case 3:
							// sys_chdir
								ret = syscall(child_copy_syscall.entrypoint, child_copy_syscall.para1);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_chdir failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_dir failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_chdir success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_chdir success \n", getpid());
								}

								break;

							case 4: /* sys_fchdir(int fd) */
								ret = syscall(child_copy_syscall.entrypoint, child_copy_syscall.para1);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_fchdir failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_fdir failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_fchdir success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_fchdir success \n", getpid());
								}

								break;

							case 5:
								//sys_mkdir

								ret = syscall(child_copy_syscall.entrypoint,
									child_copy_syscall.para1,
									child_copy_syscall.para2);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_mkdir failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_mkdir failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_mkdir success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_mkdir success \n", getpid());
								}

								break;

							case 6:
								//sys_rmdir

								ret = syscall(child_copy_syscall.entrypoint,
									child_copy_syscall.para1);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_rmdir failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_rmdir failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_rmdir success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_rmdir success \n", getpid());
								}

								break;

							case 7:
								//sys_rename

								ret = syscall(child_copy_syscall.entrypoint,
									child_copy_syscall.para1, child_copy_syscall.para2);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_rename failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_rename failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_rename success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_rename success \n", getpid());
								}

								break;

							case 8:
								//sys_time

								ret = syscall(child_copy_syscall.entrypoint,
									(time_t*)child_copy_syscall.para1);

								free((time_t*)child_copy_syscall.para1);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_time failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_time failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_time success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_time success \n", getpid());
								}

								break;

							case 9:
								//sys_readlink

								ret = syscall(child_copy_syscall.entrypoint,
									child_copy_syscall.para1, child_copy_syscall.para2, child_copy_syscall.para3);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_readlink failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_readlink failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_readlink success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_readlink success \n", getpid());
								}

								break;

							case 10:
								//sys_fchmod

								ret = syscall(child_copy_syscall.entrypoint,
									child_copy_syscall.para1, child_copy_syscall.para2);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_fchmod failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_fchmod failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_fchmod success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_fchmod success \n", getpid());
								}

								break;

							case 11:
								//sys_sync

								ret = syscall(child_copy_syscall.entrypoint);

								if (ret == -1)
								{
									//int errsv = errno;
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_sync failed with errno = %d\n", getpid(), errno);
									if(flag_debug) fprintf(stdout, "child = %d sys_sync failed with errno = %d\n", getpid(), errno);
								}
								else
								{
									if (flag_log)
										fprintf(child_log[i], "child = %d sys_sync success \n", getpid());
									if(flag_debug) fprintf(stdout, "child = %d sys_sync success \n", getpid());
								}

								break;

							default:
								fprintf(stderr," Something is terribly WRONG! Do something to fix your log switch!\n");
							}
						//end of while loop
					}
					
					//return 0;
						//end of child proc
				}
						
						//parent proc
			}
			
			else // fork failed
			{
				printf("\n Forkchild = %d fork failed, quitting!!!!!!\n", i);
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
