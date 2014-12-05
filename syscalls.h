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
	{.entrypoint = 3, .para1 = (unsigned long) NULL, .para2 = (unsigned long) NULL, .para3 = (unsigned long) NULL, .para4 = (unsigned long) NULL, .para5 = (unsigned long) NULL, .para6 = (unsigned long) NULL},	//sys_read 3
	{.entrypoint = 15, .para1 = (unsigned long) NULL, .para2 = (unsigned long) NULL, .para3 = (unsigned long) NULL, .para4 = (unsigned long) NULL, .para5 = (unsigned long) NULL, .para6 = (unsigned long) NULL},	//sys_chmod 15
};
