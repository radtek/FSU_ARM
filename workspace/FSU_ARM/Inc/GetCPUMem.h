#ifndef _GETCPUMEM_H_
#define _GETCPUMEM_H_
#include <unistd.h>

/*
 *  This file defines the interface between top and the machine-dependent
 *  module.  It is NOT machine dependent and should not need to be changed
 *  for any specific machine.
 */

/*
 * the statics struct is filled in by machine_init
 */
typedef struct
{
    char **procstate_names;
    char **cpustate_names;
    char **memory_names;
#ifdef ORDER
    char **order_names;
#endif
}statics;

/*
 * the system_info struct is filled in by a machine dependent routine.
 */


typedef struct
{
    int    last_pid;
    double load_avg[3];
    int    p_total;
    int    p_active;     /* number of procs considered "active" */
    int    *procstates;
    int    *cpustates;
    int    *memory;
}system_info;

/* cpu_states is an array of percentages * 10.  For example, 
   the (integer) value 105 is 10.5% (or .105).
 */

/*
 * the process_select struct tells get_process_info what processes we
 * are interested in seeing
 */

struct process_select
{
    int idle;		/* show idle processes */
    int system;		/* show system processes */
    int uid;		/* only this uid (unless uid == -1) */
    char *command;	/* only this command (unless == NULL) */
};

/* non-int routines typically used by the machine dependent module */
char *printable();

int machine_init(statics *statics);
void get_system_info(system_info *info);

#endif
