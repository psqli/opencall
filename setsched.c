#include <stdio.h>

#include <sched.h>

#include "setsched.h"

#define REALTIME_PRIORITY 80

int
setsched_realtime(void)
{
#if 0
	int max_prio;

	struct sched_param schedp;

	if(sched_getparam(0, &schedp))
	{
		perror("Error in `sched_getparam()`");
		return -1;
	}

	max_prio = sched_get_priority_max(SCHED_FIFO);
	schedp.sched_priority = REALTIME_PRIORITY;

	if(schedp.sched_priority > max_prio)
	{
		printf("Error, %d priority couldn't be set\n",
		REALTIME_PRIORITY);
		return -1;
	}

	if(sched_setscheduler(0, SCHED_FIFO, &schedp))
	{
		perror("Error in `sched_setscheduler()`");
		return -1;
	}

#endif

	return 0;
}