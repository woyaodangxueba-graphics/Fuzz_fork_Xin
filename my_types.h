struct node
{
	int val;
	struct node *next;
};

struct link
{
	struct node *head;
	struct node *last;
};

struct para_pool
{
	int *fd_pool;
	char *dirs_pool[1000];
	int *mode_pool[1000];
};
