#include <stdio.h>
#include <stdint.h>

uint32_t xorshift32(uint32_t *state){
  uint32_t x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *state = x;
  return x;
}

int main(int argc, char** argv){
  uint32_t rand = 611953;
  for (int i = 0 ; i<10; i++){
    xorshift32(&rand);
    printf("%d\n", rand);
  }
  return 0;
}

