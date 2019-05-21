#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define check(exp, msg) if(exp) {} else {\
   printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
   exit();}

void spin(){
  int k = 0;
  for(int i = 0; i < 50; ++i){
    for(int j = 0; j < 10000000; ++j)
    {
      k = j % 10;
      k = k + 1;
    }
  }
}

int pid[3];

int main(int argc, char **argv){
   struct pstat st;
   for(int i = 0; i<3; i++){
	   if(fork()==0){
		   pid[i] = getpid();
		   settickets((3-i)*2);
		   while(1){spin();}
	   }
   }
   int time = 0;
   while (time <= 100){
	   check(getpinfo(&st)==0, "getpinfo");
//	   printf(1, "\n*****p info at t = %d *****\n", time); 
	   printf(1,"%d ",time);
	   for(int i =0; i<NPROC; i++){
		  if(st.inuse[i] && st.pid[i]>3){
			 //printf(1, "pid: %d tickets: %d ticks: %d\n", st.pid[i], st.tickets[i], st.ticks[i]);   
		 	printf(1,"%d ", st.ticks[i]);
		  }
	   }
	   printf(1,"\n");
	   time +=1;  
	   sleep(1);
   }
  for(int i = 0; i<3; i++){
	 kill(pid[i]);
  } 
  exit();
}
