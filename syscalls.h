struct fuzz_sys_call{

	int entrypoint;
	unsigned long para1;
	unsigned long para2;
	unsigned long para3;
	unsigned long para4;
	unsigned long para5;
	unsigned long para6;
}

struct fuzz_sys_call fuzz_sys_call_table[] = {
	{3, NULL, NULL, NULL, NULL, NULL, NULL},	//sys_read 3
	{15, NULL, NULL, NULL, NULL, NULL, NULL},	//sys_chmod 15
}