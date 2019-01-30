#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv){
  printf("code loc: %p\n", (void *)main);
  printf("heap loc: %p\n", (void *)malloc(1));
  int x = 3;
  printf("stack loc:%p\n", (void *) &x);
  return x;
}
