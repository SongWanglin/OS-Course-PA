#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "mmu.h"

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}
// user level lock init, acquire and release function
void lock_init(lock_t *lock){
	lock->locked = 0;
}
void lock_acquire(lock_t *lock){
	while(xchg(&lock->locked, 1));
}
void lock_release(lock_t *lock){
	lock->locked = 0;
}
// thread functions
int
thread_create(void(*start_routine)(void *, void *), void *arg1, void *arg2)
{
	void *stack = malloc(2*PGSIZE);
	// page align or not?
	if((uint)stack%PGSIZE!=0){
		stack += PGSIZE - (uint)stack%PGSIZE;
	}
	// call clone and check return value
	int res = clone(start_routine, arg1, arg2, stack);
	if (res == -1){
		void *new_stack;
	        new_stack = malloc(PGSIZE);
		if((uint)stack%PGSIZE != 0)
			new_stack = stack + (PGSIZE-(uint)stack % PGSIZE);
		return clone(start_routine, arg1, arg2, new_stack);
	}
	return res;
}
int
thread_join(void)
{
	void *stack;
	int res = join(&stack);
	free(stack);
	return res;
}

	
