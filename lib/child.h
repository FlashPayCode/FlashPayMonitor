#ifndef	__CHILD_H__
#define	__CHILD_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <poll.h>

//           struct pollfd {
//               int   fd;         /* file descriptor */
//               short events;     /* requested events */
//               short revents;    /* returned events */
//           };

typedef struct pollfd pollfd_t;
typedef struct child_rec child_t;

struct child_rec
{
    pid_t	i_child_id;	// child pid
    int		i_child_fi;	// child pipe input fd
    int		i_child_fo;	// child pipe output fd
    pollfd_t *	p_child_fd;	// child poll fd rec ptr.

    child_t *	p_prev;		// previous child node
    child_t *	p_next;		// next child node
};

// free child stack operations: begin
//==============================================================

extern	child_t	*	g_p_child_free;	// free child list (free child stack head)
extern	child_t	*	g_p_child_data;	// child record array

// put free child to stack g_p_child_free
//---------------------------------------
void	put_free_child(child_t *p_child);

// get free child from stack g_p_child_free
// if g_p_child_free == NULL, the stack is empty
//----------------------------------------------
child_t * get_free_child();

extern	child_t *	g_p_child_list;	// first child list node
extern	child_t *	g_p_child_last;	// first child list node
extern	int		g_i_max_files_nums;
extern	int		g_i_max_child_nums;

child_t * add_child(child_t *p_child);
child_t * add_child(pid_t i_child_id, int i_child_fi, int i_child_fo);
void	del_child(child_t *p_child);

int	init_child_manager(int i_max_child_nums);
int	read_child_status(int i_timeout);
void	free_child_manager();

#endif
