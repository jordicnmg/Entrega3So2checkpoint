#include <libc.h>
#include <types.h>
/*#include <interrupt.h> // la funció de configurar interrupcions està a interrupt.c
#include <zeos_interrupt.h>*/

char buff[24];

int pid;

void perror() {
	char *str="Ha anat be";
	if (errno==22) str="Error de parametres\n";
	if (errno==11) str="Torna-ho a provar\n";
	write(1,str,strlen(str));
}
int __attribute__ ((__section__(".text.main")))



 main(void)
{
	//struct stats s;
	//int d;
	//int t;
	//char m[1000];
	//char *buffer = "Q \n";
	
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
	/*d=write(-1,buffer,strlen(buffer));
	if (d==-1) perror();
	t=gettime();
	itoa(t,m);
	
	write(1,m,strlen(m));*/
	//d = fork();
	//char pila[4096];
	//runjp_rank(5,5);
	runjp();
	//clon_ranke(funcio,&pila[4095]);
	//d = getpid();
//	if (d > 0)exit();
//	if (get_stats(4,&s) < 0) d = 1;
//	else d = 0;
//	itoa(d,buffer);
//	d=write(1,buffer,strlen(buffer));	
	while(1){
	//d = getpid();
	//get_stats(1,&s);
	//if (d==2) exit();
	//itoa(d,buffer);
	//d=write(1,buffer,strlen(buffer));
	}
	return 0;
}
