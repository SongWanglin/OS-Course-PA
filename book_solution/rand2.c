#include <stdio.h>
#include <stdlib.h>
int main(void){
  srand(655123);
  for(int i = 0; i<10; i++){
    printf("%d\n", rand());
  }
  return 0;
}
