#include "header.h"
#include "child.h"

// free child stack operations: begin
//==============================================================

child_t	*	g_p_child_free = NULL;	// free child list (free child stack head)
child_t	*	g_p_child_data = NULL;	// child record array

// put free child to stack g_p_child_free
//---------------------------------------
void put_free_child(child_t *p_child)
{
    p_child->p_next = g_p_child_free;
    g_p_child_free = p_child;
}

// get free child from stack g_p_child_free
// if g_p_child_free == NULL, the stack is empty
//----------------------------------------------
child_t * get_free_child()
{
    child_t * p_child = g_p_child_free;
    if (g_p_child_free) g_p_child_free = g_p_child_free->p_next;
    return p_child;
}

// set up free child stack
//------------------------
// all child data is g_p_child_data (array)
// put all free child to stack
// this function process at program startup
// and only process once.
//-----------------------------------------
void set_free_child(int i_child_nums)
{
    if (g_p_child_data) return;

    g_p_child_data = (child_t*)malloc(sizeof(child_t) * i_child_nums);
    memset(g_p_child_data, 0, sizeof(child_t) * i_child_nums);
    
    int i;
    for (i = 0; i < i_child_nums; i++)
    {
        child_t * p_child = (child_t*)&(g_p_child_data[i]);
        put_free_child(p_child);
    }
}

// active child list operations:
//==============================================================

pollfd_t *	g_p_child_fds;		// parent process poll child status.
child_t *	g_p_child_list = NULL;	// first child list node
child_t *	g_p_child_last = NULL;	// last child list node
int		g_i_max_files_nums	= 1024;
int		g_i_max_child_nums	= 1012;

child_t * add_child(child_t *p_child)
{
    p_child->p_prev = g_p_child_last;
    if (g_p_child_list == NULL)
         g_p_child_list = p_child;
    else g_p_child_last->p_next = p_child;
    g_p_child_last = p_child;
    
    return p_child;
}

child_t * add_child(pid_t i_child_id, int i_child_fi, int i_child_fo)
{
    child_t * p_child = get_free_child();
    if (p_child == NULL) return p_child;

    memset(p_child,0,sizeof(child_t));
    p_child->i_child_id = i_child_id;
    p_child->i_child_fi = i_child_fi;
    p_child->i_child_fo = i_child_fo;

    return add_child(p_child);
}

void del_child(child_t *p_child)
{
    close(p_child->i_child_fi);
    close(p_child->i_child_fo);

    if (p_child->p_prev)
         p_child->p_prev->p_next = p_child->p_next;
    else g_p_child_list = p_child->p_next;
    if (p_child->p_next)
         p_child->p_next->p_prev = p_child->p_prev;
    else g_p_child_last = p_child->p_prev;
                
    put_free_child(p_child);
}

int init_child_manager(int i_max_child_nums)
{
    if (g_p_child_fds) return g_i_max_child_nums;

    struct rlimit rlim;
    int res = getrlimit(RLIMIT_NOFILE, &rlim);
    if (res == 0) g_i_max_files_nums = (int)rlim.rlim_max;
    g_i_max_child_nums = (g_i_max_files_nums - 12) / 2;
    if (g_i_max_child_nums > i_max_child_nums) g_i_max_child_nums = i_max_child_nums;
    
    g_p_child_fds = (struct pollfd*)malloc(sizeof(pollfd_t) * g_i_max_files_nums);
    memset(g_p_child_fds, 0, sizeof(pollfd_t) * g_i_max_child_nums);

    set_free_child(g_i_max_child_nums);    
    return g_i_max_child_nums;
}

void free_child_manager()
{
    child_t *p_child = g_p_child_list;
    while (p_child)
    {
        child_t *p_temp = p_child->p_next;
        del_child(p_child);
        p_child = p_temp;
    }
    free(g_p_child_data);
    free(g_p_child_fds);
}

int read_child_status(int i_timeout)
{
    if (g_p_child_list == NULL) return 0;
    
    int i_index = 0;
    child_t * p_child = g_p_child_list;
    while (p_child)
    {
        pollfd_t *p_pollfd = &(g_p_child_fds[i_index++]);
        p_child->p_child_fd = p_pollfd;
        p_pollfd->fd = p_child->i_child_fi;
        p_pollfd->events = 0;
        p_pollfd->revents = 0;
        p_child = p_child->p_next;
    }
    
    return poll(g_p_child_fds,i_index,i_timeout);
}
