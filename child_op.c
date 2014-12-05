/* This is our child's operation*/
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

/* Child syscall table initialization */

struct fuzz_sys_call* rand_para(struct para_pool *Pool, struct fuzz_sys_call fuzz_sys_call_table[], int index_chosen_syscall)
{
	/* allocate memnory for child's syscall table. */
	struct fuzz_sys_call *child_copy_syscall = (struct fuzz_sys_call*)malloc(sizeof(struct fuzz_sys_call));

	child_copy_syscall->entrypoint = fuzz_sys_call_table[index_chosen_syscall].entrypoint;
	child_copy_syscall->para1 = fuzz_sys_call_table[index_chosen_syscall].para1;
	child_copy_syscallpara2 = fuzz_sys_call_table[index_chosen_syscall].para2;
	child_copy_syscallpara3 = fuzz_sys_call_table[index_chosen_syscall].para3;
	child_copy_syscallpara4 = fuzz_sys_call_table[index_chosen_syscall].para4;
	child_copy_syscallpara5 = fuzz_sys_call_table[index_chosen_syscall].para5;
	child_copy_syscallpara6 = fuzz_sys_call_table[index_chosen_syscall].para6;

	switch (index_chosen_syscall)
	{
	case 0:
		//sys_read
		child_copy_syscall->para1 = Pool->fd_pool[rand() % (files_number + 3 + 100)];

		if (rand() % 2)
		{
			char tmp[32];
			child_copy_syscall->para2 = (unsigned long)tmp;
		}
		else
		{
			child_copy_syscall->para2 = (unsigned long)rand();
		}

		child_copy_syscall->para3 = rand() % 512;

		break;

	case 1:
		//sys_chmod
		child_copy_syscall->para1 = (unsigned long)Pool->dirs_pool[rand() % 1000];
		if (rand() % 2)
		{
			child_copy_syscall->para2 = Pool->mode_pool[rand() % 27];
		}
		else
		{
			child_copy_syscall->para2 = (unsigned int)rand();
		}

		break;

	default:
		fprintf(stderr, " Something is terribly WRONG! Do something to fix your rand_para switch!\n");
	}

	return child_copy_syscall;
}


/* Log file operations */
void log_file(struct fuzz_sys_call *child_copy_syscall, int index_chosen_syscall, int flag_log, int i)
{
	switch (index_chosen_syscall)
	{
	case 0:
		/* sys_read */
		if (child_copy_syscall->para1) //skip para1 = 0 , which is inded fd =0. Reading from keyboard is annoying. 
		{
			if (flag_log)
				fprintf(child_log[i], "child = %d calling sys_read(%d,%x,%d)\n",
				getpid(), (int)child_copy_syscall->para1,
				(unsigned int)child_copy_syscall->para2,
				(int)child_copy_syscall->para3);
			fprintf(stdout, "child = %d calling sys_read(%d,%x,%d)\n",
				getpid(), (int)child_copy_syscall->para1,
				(unsigned int)child_copy_syscall->para2,
				(int)child_copy_syscall->para3);
		}

		break;

	case 1:
		/* sys_chmod */
		if (flag_log)
			fprintf(child_log[i], "child = %d calling sys_chmod(%s,%o)\n",
			getpid(), (char*)child_copy_syscall->para1,
			(unsigned int)child_copy_syscall->para2);
		fprintf(stdout, "child = %d calling sys_chmod(%s,%o)\n",
			getpid(), (char*)child_copy_syscall->para1,
			(unsigned int)child_copy_syscall->para2);
		break;

	default:
		fprintf(stderr, " Something is terribly WRONG! Do something to fix your log switch!\n");
	}
}

/* Syscall Opercation */

//void my_syscall()

void child_op(struct para_pool *Pool, int flag_log, FILE * child_log[], struct fuzz_sys_call fuzz_sys_call_table[], int i) // i is the index of the forked child
{
	/* after fork(), child will reinit seed to 0 */
	srand(time(NULL));
						
	while(1)
	{
		/*******************************************/
		/**************choose syscall()*************/
		/*******************************************/
		int index_chosen_syscall = rand() % 2; // 2 is syscall numbers this draft supports right now.

		/*******************************************/
		/*****************rand para()***************/
		/*******************************************/
		
		struct fuzz_sys_call *child_copy_syscall = rand_para(Pool, fuzz_sys_call_table, index_chosen_syscall); // Don't forget to free the memory for child table in the end.
					
		/*******************************************/
		/**********************log_file()****************/
		/*******************************************/

		log_gile(child_copy_syscall, flag_log, i);
					
		/*******************************************/
		/******************syscall()****************/
		/*******************************************/
						
		int ret = 0;
						
		switch(index_chosen_syscall)
		{
		case 0:
			//sys_read
			//skip para1 = 0 , which is inded fd =0. Reading from keyboard is annoying. 
							
			if (child_copy_syscall->para1)
			{
				ret = syscall(child_copy_syscall->entrypoint, child_copy_syscall->para1,
									child_copy_syscall->para2, child_copy_syscall->para3);
		
				if (ret == -1)
				{
					//int errsv = errno;
					if(flag_log)
						fprintf(child_log[i], "child = %d sys_read failed with errno = %d\n", getpid(), errno);
					fprintf(stdout, "child = %d sys_read failed with errno = %d\n", getpid(), errno);
				}
				else 
				{
					if(flag_log)
						fprintf(child_log[i], "child = %d sys_read success with %s\n", getpid(),(char *) child_copy_syscall->para2);
					fprintf(stdout, "child = %d sys_read success \n", getpid());
				}
			}
						
			break;
						
		case 1:
			//sys_chmod
							
			ret = syscall(child_copy_syscall->entrypoint, child_copy_syscall->para1,
					child_copy_syscall->para2);
		
			if (ret == -1)
			{
				//int errsv = errno;
				if(flag_log)
					fprintf(child_log[i], "child = %d sys_chmod failed with errno = %d\n", getpid(), errno);
				fprintf(stdout, "child = %d sys_chmod failed with errno = %d\n", getpid(), errno);
			}
			else 
			{
				if(flag_log)
					fprintf(child_log[i], "child = %d sys_chmod success \n", getpid() );
				fprintf(stdout, "child = %d sys_chmod success \n", getpid());
			}
				
			break;
						
						
		default:
			fprintf(stderr," Something is terribly WRONG! Do something to fix your log switch!\n");
		}
	}

	/* Free the child's table */
	free(child_copy_syscall);

}