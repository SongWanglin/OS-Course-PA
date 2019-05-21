#include "param.h"

struct pstat{
	int pid[NPROC];
	int inuse[NPROC];
	int ticks[NPROC];
	int tickets[NPROC];
};
