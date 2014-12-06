struct fuzz_sys_call{

	int entrypoint;
	unsigned long para1;
	unsigned long para2;
	unsigned long para3;
	unsigned long para4;
	unsigned long para5;
	unsigned long para6;
};

struct fuzz_sys_call fuzz_sys_call_table[] = {
	{	
		.entrypoint = 3, 
		.para1 = (unsigned long) NULL, 
		.para2 = (unsigned long) NULL, 
		.para3 = (unsigned long) NULL, 
		.para4 = (unsigned long) NULL, 
		.para5 = (unsigned long) NULL, 
		.para6 = (unsigned long) NULL
	},	//sys_read 3

	{.entrypoint = 15, .para1 = (unsigned long) NULL, .para2 = (unsigned long) NULL, .para3 = (unsigned long) NULL, .para4 = (unsigned long) NULL, .para5 = (unsigned long) NULL, .para6 = (unsigned long) NULL},	//sys_chmod 15
	{ .entrypoint = 4, .para1 = (unsigned long)NULL, .para2 = (unsigned long)NULL, .para3 = (unsigned long)NULL, .para4 = (unsigned long)NULL, .para5 = (unsigned long)NULL, .para6 = (unsigned long)NULL },	//sys_write 4

	{ 
		.entrypoint = 12,
		.para1 = (unsigned long)NULL,
		.para2 = (unsigned long)NULL,
		.para3 = (unsigned long)NULL,
		.para4 = (unsigned long)NULL,
		.para5 = (unsigned long)NULL,
		.para6 = (unsigned long)NULL 
	},	//sys_chdir 12

	{
		.entrypoint = 133,
		.para1 = (unsigned long)NULL,
		.para2 = (unsigned long)NULL,
		.para3 = (unsigned long)NULL,
		.para4 = (unsigned long)NULL,
		.para5 = (unsigned long)NULL,
		.para6 = (unsigned long)NULL
	},	//sys_fchdir 133

	{
		.entrypoint = 39,
		.para1 = (unsigned long)NULL,
		.para2 = (unsigned long)NULL,
		.para3 = (unsigned long)NULL,
		.para4 = (unsigned long)NULL,
		.para5 = (unsigned long)NULL,
		.para6 = (unsigned long)NULL
	},	//sys_mkdir 39

	{
		.entrypoint = 40,
		.para1 = (unsigned long)NULL,
		.para2 = (unsigned long)NULL,
		.para3 = (unsigned long)NULL,
		.para4 = (unsigned long)NULL,
		.para5 = (unsigned long)NULL,
		.para6 = (unsigned long)NULL
	},	//sys_rmdir 40

	{
		.entrypoint = 38,
		.para1 = (unsigned long)NULL,
		.para2 = (unsigned long)NULL,
		.para3 = (unsigned long)NULL,
		.para4 = (unsigned long)NULL,
		.para5 = (unsigned long)NULL,
		.para6 = (unsigned long)NULL
	},	//sys_rename 38

	{
		.entrypoint = 13,
		.para1 = (unsigned long)NULL,
		.para2 = (unsigned long)NULL,
		.para3 = (unsigned long)NULL,
		.para4 = (unsigned long)NULL,
		.para5 = (unsigned long)NULL,
		.para6 = (unsigned long)NULL
	},	//sys_time 13; time_t is defined as long int.

	{
		.entrypoint = 85,
		.para1 = (unsigned long)NULL,
		.para2 = (unsigned long)NULL,
		.para3 = (unsigned long)NULL,
		.para4 = (unsigned long)NULL,
		.para5 = (unsigned long)NULL,
		.para6 = (unsigned long)NULL
	},	//sys_readlink 85

	{
		.entrypoint = 94,
		.para1 = (unsigned long)NULL,
		.para2 = (unsigned long)NULL,
		.para3 = (unsigned long)NULL,
		.para4 = (unsigned long)NULL,
		.para5 = (unsigned long)NULL,
		.para6 = (unsigned long)NULL
	},	//sys_readlink 94

	{
		.entrypoint = 36,
		.para1 = (unsigned long)NULL,
		.para2 = (unsigned long)NULL,
		.para3 = (unsigned long)NULL,
		.para4 = (unsigned long)NULL,
		.para5 = (unsigned long)NULL,
		.para6 = (unsigned long)NULL
	},	//sys_sync 36

};
