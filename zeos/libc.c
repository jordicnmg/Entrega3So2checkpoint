/*
 * libc.c 
 */

#include <libc.h>
#include <asm.h>

#include <types.h>

int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}



int write (int fd, char *buffer, int size) {
	int i;
	__asm__ ("push %edx;"
	"push %ecx;"
	"push %ebx;"
	"movl 8(%ebp),%ebx;"
	"movl 12(%ebp),%ecx;"
	"movl 16(%ebp),%edx;"
	"movl $4,%eax;"
	"int $0x80;"
	"popl %ebx;"
	"popl %ecx;"
	"popl %edx;");
	asm("\t movl %%eax,%0" : "=r"(i));
	if (i<0) {
		errno=-i;
		return -1;
	}
	else {
	return i;
	}
}

int get_stats (int pid, struct stats *st){
int i;
	__asm__ ("push %ecx;"
	"push %ebx;"
	"movl 8(%ebp),%ebx;"
	"movl 12(%ebp),%ecx;"
	"movl $35,%eax;"
	"int $0x80;"
	"popl %ebx;"
	"popl %ecx;"
	);
	asm("\t movl %%eax,%0" : "=r"(i));
	if (i<0) {
		errno=-i;
		return -1;
	}
	else {
	return i;
	}
}

int clone (void (*function)(void), void *stack){
int i;
	__asm__ ("push %ecx;"
	"push %ebx;"
	"movl 8(%ebp),%ebx;"
	"movl 12(%ebp),%ecx;"
	"movl $19,%eax;"
	"int $0x80;"
	"popl %ebx;"
	"popl %ecx;"
	);
	asm("\t movl %%eax,%0" : "=r"(i));
	if (i<0) {
		errno=-i;
		return -1;
		//printc('W');
	}
	else {
	return i;
	//printc('S');
	}
}


int sem_init (int n_sem, unsigned int value) {
int i;
	__asm__ ("push %ecx;"
	"push %ebx;"
	"movl 8(%ebp),%ebx;"
	"movl 12(%ebp),%ecx;"
	"movl $21,%eax;"
	"int $0x80;"
	"popl %ebx;"
	"popl %ecx;"
	);
	asm("\t movl %%eax,%0" : "=r"(i));
	if (i<0) {
		errno=-i;
		return -1;
		//printc('W');
	}
	else {
	return i;
	//printc('S');
	}
}



int sem_wait (int n_sem) {
int i;
	__asm__ ("push %ebx;"
	"movl 8(%ebp),%ebx;"
	"movl $22,%eax;"
	"int $0x80;"
	"popl %ebx;"
	);
	asm("\t movl %%eax,%0" : "=r"(i));
	if (i<0) {
		errno=-i;
		return -1;
	}
	else {
	return i;
	}
}



int sem_signal (int n_sem) {
int i;
	__asm__ ("push %ebx;"
	"movl 8(%ebp),%ebx;"
	"movl $23,%eax;"
	"int $0x80;"
	"popl %ebx;"
	);
	asm("\t movl %%eax,%0" : "=r"(i));
	if (i<0) {
		errno=-i;
		return -1;
	}
	else {
	return i;
	}
}


int sem_destroy (int n_sem){
int i;
	__asm__ ("push %ebx;"
	"movl 8(%ebp),%ebx;"
	"movl $24,%eax;"
	"int $0x80;"
	"popl %ebx;"
	);
	asm("\t movl %%eax,%0" : "=r"(i));
	if (i<0) {
		errno=-i;
		return -1;
	}
	else {
	return i;
	}
}

int gettime() {
	int i;
	__asm__ ("movl $10,%eax;"
	"int $0x80;");
	asm("\t movl %%eax,%0" : "=r"(i));
	return i;
}

int getpid(void){
	int i;
	__asm__ ("movl $20,%eax;"
	"int $0x80;");
	asm("\t movl %%eax,%0" : "=r"(i));
	if (i<0) {
		errno=-i;
		return -1;
	}
	else {
	return i;
	}
}

int fork(void){
	int i;
	__asm__ ("movl $2,%eax;"
	"int $0x80;");
	asm("\t movl %%eax,%0" : "=r"(i));
	if (i<0) {
		errno=-i;
		return -1;
	}
	else {
	return i;
	}
}

void exit(void){
	int i;
	__asm__ ("movl $1,%eax;"
	"int $0x80;");
	asm("\t movl %%eax,%0" : "=r"(i));
	}

