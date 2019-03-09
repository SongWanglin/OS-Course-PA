#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define check(exp, msg) if(exp) {} else {\
   printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
   exit();}
#define PROC 7

void spin(){
  int i = 0;
  int j = 0;
  int k = 0;
  for(i = 0; i < 50; ++i){
    for(j = 0; j < 10000000; ++j){
      k = j % 10;
      k = k + 1;
    }
  }
}

// below is what I used to print the t vs ticks data 
/*/
int pid[3];
int main(int argc, char** argv){
	struct pstat st;
	for (int i = 0; i< 3; i++){
		if(fork() == 0){
			pid[i] = getpid();
			settickets(2*(i+1));
			while(1){spin();}
		}
	}
	int time = 0;
	while ( time <= 1000){
		check(getpinfo(&st)==0, "getpinfo");
	       	printf(1, "\n*****p info at t = %d *****\n", time);
         	for(int i =0; i<NPROC; i++){
		    if(st.inuse[i]){
			printf(1, "pid: %d tickets: %d ticks: %d\n", st.pid[i], st.tickets[i], st.ticks[i]);
		      }
		 }
	      time +=100;
	      sleep(100);
          }
	 for (int i = 0; i<3; i++){
		kill(pid[i]);
	 }
	 exit();
	//return 0;
}

/*/
int
main(int argc, char *argv[])
{
   struct pstat st;
   int count = 0;
   int i = 0;
   int pid[NPROC];
   printf(1,"Spinning...\n");
   while(i < PROC)
   {
      pid[i] = fork();
	    if(pid[i] == 0)
     {
		    spin();
		    exit();
      }
	  i++;
   }
   sleep(500);
   //spin();
   check(getpinfo(&st) == 0, "getpinfo");
   printf(1, "\n**** PInfo ****\n");
   for(i = 0; i < NPROC; i++) {
      if (st.inuse[i]) {
	       count++;
         printf(1, "pid: %d tickets: %d ticks: %d\n", st.pid[i], st.tickets[i], st.ticks[i]);
      }
   }
   for(i = 0; i < PROC; i++)
   {
	    kill(pid[i]);
   }
   while (wait() > 0);
   printf(1,"Number of processes in use %d\n", count);
   check(count == 10, "getpinfo should return 10 processes in use\n");
   printf(1, "Should print 1 then 2");
   exit();
}
