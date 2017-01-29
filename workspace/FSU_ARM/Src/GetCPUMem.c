/*
 * top - a top users display for Unix
 *
 * SYNOPSIS:  Linux 1.2.x, 1.3.x, using the /proc filesystem
 *
 * DESCRIPTION:
 * This is the machine-dependent module for Linux 1.2.x or 1.3.x.
 *
 * LIBS:
 *
 * CFLAGS: -DHAVE_GETOPT
 *
 * AUTHOR: Richard Henderson <rth@tamu.edu>
 */

#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "GetCPUMem.h"
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#include <sys/param.h>		/* for HZ */
//#include <asm/page.h>		/* for PAGE_SHIFT */
#define PROC_SUPER_MAGIC 0x9fa0

//#define HASH_SIZE 1024 
#define NR_TASKS 1024;

/*=STATE IDENT STRINGS==================================================*/
#define NPROCSTATES 7
#define NCPUSTATES 4
#define NMEMSTATS 6
/*=SYSTEM STATE INFO====================================================*/

/* these are for calculating cpu state percentages */
static long cp_time[NCPUSTATES];
static long cp_old[NCPUSTATES];
static long cp_diff[NCPUSTATES];

/* these are for keeping track of processes */
#define HASH_SIZE	(NR_TASKS * 3 / 2)

/* these are for passing data back to the machine independant portion */
static int cpu_states[NCPUSTATES];
static int memory_stats[NMEMSTATS];

/* usefull macros */
#define bytetok(x)	(((x) + 512) >> 10)
#define pagetok(x)	((x) << (PAGE_SHIFT - 10))
#define HASH(x)		(((x) * 1686629713U) % HASH_SIZE)

/*======================================================================*/

static inline char *
skip_ws(const char *p)
{
    while (isspace(*p)) p++;
    return (char *)p;
}

static inline char *
skip_token(const char *p)
{
    while (isspace(*p)) p++;
    while (*p && !isspace(*p)) p++;
    return (char *)p;
}

long percentages(int cnt, int *out, long *newt, long *old, long *diffs) {
	register int i;
	register long change;
	register long total_change;
	register long *dp;
	long half_total;

	/* initialization */
	total_change = 0;
	dp = diffs;

	/* calculate changes for each state and the overall change */
	for (i = 0; i < cnt; i++) {
		if ((change = *newt - *old) < 0) {
			/* this only happens when the counter wraps */
			change = (int) ((unsigned long) *newt - (unsigned long) *old);
		}
		total_change += (*dp++ = change);
		*old++ = *newt++;
	}

	/* avoid divide by zero potential */
	if (total_change == 0) {
		total_change = 1;
	}

	/* calculate percentages based on overall change, rounding up */
	half_total = total_change / 2l;
	for (i = 0; i < cnt; i++) {
		*out++ = (int) ((*diffs++ * 1000 + half_total) / total_change);
	}

	/* return the total in case the caller wants to use it */
	return (total_change);
}

void get_system_info(system_info *info) {
	char buffer[4096 + 1];
	int fd, len;
	char *p;
	int i;

	/* get load averages */
	fd = open("/proc/loadavg", O_RDONLY);
	len = read(fd, buffer, sizeof(buffer) - 1);
	close(fd);
	buffer[len] = '\0';

	info->load_avg[0] = strtod(buffer, &p);
	info->load_avg[1] = strtod(p, &p);
	info->load_avg[2] = strtod(p, &p);
	p = skip_token(p); /* skip running/tasks */
	p = skip_ws(p);
	if (*p)
		info->last_pid = atoi(p);
	else
		info->last_pid = -1;

	/* get the cpu time info */
	fd = open("/proc/stat", O_RDONLY);
	len = read(fd, buffer, sizeof(buffer) - 1);
	close(fd);
	buffer[len] = '\0';

	p = skip_token(buffer); /* "cpu" */
	cp_time[0] = strtoul(p, &p, 0);

	cp_time[1] = strtoul(p, &p, 0);
	cp_time[2] = strtoul(p, &p, 0);
	cp_time[3] = strtoul(p, &p, 0);

	/* convert cp_time counts to percentages */
	percentages(4, cpu_states, cp_time, cp_old, cp_diff);

	/* get system wide memory usage */
	fd = open("/proc/meminfo", O_RDONLY);
	len = read(fd, buffer, sizeof(buffer) - 1);
	close(fd);
	buffer[len] = '\0';

	/* be prepared for extra columns to appear be seeking
	 to ends of lines */
	p = buffer;
	p = skip_token(p);
	memory_stats[0] = strtoul(p, &p, 10); /* total memory */

	p = strchr(p, '\n');
	p = skip_token(p);
	memory_stats[1] = strtoul(p, &p, 10); /* free memory */

	p = strchr(p, '\n');
	p = skip_token(p);
	memory_stats[2] = strtoul(p, &p, 10); /* buffer memory */

	p = strchr(p, '\n');
	p = skip_token(p);
	memory_stats[3] = strtoul(p, &p, 10); /* cached memory */

	for (i = 0; i < 8; i++) {
		p++;
		p = strchr(p, '\n');
	}

	p = skip_token(p);
	memory_stats[4] = strtoul(p, &p, 10); /* total swap */

	p = strchr(p, '\n');
	p = skip_token(p);
	memory_stats[5] = strtoul(p, &p, 10); /* free swap */

	/* set arrays and strings */
	info->cpustates = cpu_states;
	info->memory = memory_stats;
}
