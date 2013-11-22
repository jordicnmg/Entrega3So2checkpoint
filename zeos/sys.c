/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <system.h>

#include <entry.h>

#define LECTURA 0
#define ESCRIPTURA 1


int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

void task_switch (union task_union *new){
user_to_system(&(current()->st));
system_to_user(&(new->task.st));
__asm__("movl 8(%ebp), %ecx;"
"pushl %esi;"
"pushl %edi;"
"pushl %ebx;"
"pushl %ecx;"
"call inner_task_switch;"
"popl %ecx;"
"popl %ebx;"
"popl %edi;"
"popl %esi;"
);
}

void inner_task_switch (union task_union *new){
struct task_struct *tr = current();
if(current()->pos != new->task.pos)set_cr3(new->task.dir_pages_baseAddr);
tss.esp0 = (DWord) &(new->stack[KERNEL_STACK_SIZE]);
__asm__("\t movl %%ebp,%0" : "=r"(tr->kernel_esp));
__asm__ __volatile__ ("movl %0,%%esp" : "=g" (new->task.kernel_esp));
__asm__("popl %ebp;");
__asm__("ret;");
}

int sys_ni_syscall()
{
user_to_system(&(current()->st));
system_to_user(&(current()->st));
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	user_to_system(&(current()->st));
	system_to_user(&(current()->st));
	return current()->PID;
}



int sys_clone (void (*function)(void),void *stack) {
////user to system
  int PId;
  struct task_struct *child;
  union task_union *childu;
  if (access_ok(VERIFY_WRITE, stack , 4) == 0) return -1;
  if (access_ok(VERIFY_WRITE, function , 4) == 0) return -1;
  if (list_empty(&freequeue)) return -12;///system to user
  // creates the thread
	child = list_head_to_task_struct(freequeue.next);
	list_del(&child->list);
	copy_data(current(), child, sizeof(union task_union));
	childu = (union task_union *) child;
	++vdir[child->pos];	
	child->PID = nextPID;
	PId = nextPID;
	nextPID++;
	child->st.user_ticks = 0;
	child->st.system_ticks = 0;
	child->st.blocked_ticks = 0;
	child->st.ready_ticks = 0;
	child->st.elapsed_total_ticks = get_ticks();
	child->st.total_trans = 0;
	child->st.remaining_ticks = 0;
	childu->stack[KERNEL_STACK_SIZE - 2]= (unsigned long) stack;
	childu->stack[KERNEL_STACK_SIZE - 5]= (unsigned long) function;
	childu->stack[KERNEL_STACK_SIZE - 17]= (unsigned long) ret_from_fork;
	childu->stack[KERNEL_STACK_SIZE - 18]= 0;
   	child->kernel_esp = &(childu->stack[KERNEL_STACK_SIZE - 18]);
	list_add_tail(&(child->list),&readyqueue);
	child->estat = ST_READY;
////system to user
	return PId;
}



int sys_fork()
{
user_to_system(&(current()->st));
  int PId;
  int error;
  int pag;
  int page2;
  int new_ph_pag;
  unsigned int afpare;
  struct task_struct *child;
  union task_union *childu;
  page_table_entry *process_PT_father =  get_PT(current());
  if(list_empty(&freequeue))return -12;
  // creates the child process
  else {
	//---------DATA SISTEMA-----------//
 	child = list_head_to_task_struct(freequeue.next);
 	list_del(&child->list);
	copy_data(current(), child, sizeof(union task_union));
	error = allocate_DIR(child); // inicialització d'un nou directori
	if (error < 0) return -11;
	++vdir[child->pos];
	page_table_entry * process_PT_child =  get_PT(child);
	//---------DATA USUARI-----------//
	/* CODE */
  	for (pag=0;pag<NUM_PAG_CODE;pag++){ //copiem les entrades de la TP del codi usuari del pare al fill
		afpare = get_frame (process_PT_father,pag+NUM_PAG_KERNEL);
		set_ss_pag(process_PT_child,pag+NUM_PAG_KERNEL,afpare);
  	}	
	/* DATA */
	for (pag=0;pag<NUM_PAG_DATA;pag++){//busquem frame, assignem a un nou espai lògic del pare i del fill. Copiem  DATA+STACK del pare al nou frame del pare (que està mapejat al mateix lloc que el fill).
	new_ph_pag = alloc_frame();
		if (new_ph_pag < 0){
			for (page2=0;page2<pag;page2++) {
			new_ph_pag = get_frame(process_PT_father,page2+NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA);
			free_frame(new_ph_pag);	
			del_ss_pag(process_PT_father,page2+NUM_PAG_CODE+NUM_PAG_DATA+NUM_PAG_KERNEL);
			del_ss_pag(process_PT_child,page2+NUM_PAG_CODE+NUM_PAG_KERNEL);
			}
			set_cr3(get_DIR(current()));
			system_to_user(&(current()->st));
			return -12;
		}

		else {
			set_ss_pag(process_PT_child,pag+NUM_PAG_CODE+NUM_PAG_KERNEL,new_ph_pag);
			set_ss_pag(process_PT_father,pag+NUM_PAG_CODE+NUM_PAG_KERNEL+NUM_PAG_DATA,new_ph_pag);
			copy_data((void *)((pag+NUM_PAG_CODE+NUM_PAG_KERNEL)<<12),(void *)((pag+NUM_PAG_CODE+NUM_PAG_DATA+NUM_PAG_KERNEL)<<12),sizeof(union task_union));
		}
	}
	for (pag=0;pag<NUM_PAG_DATA;pag++){ //Eliminem la copia de DATA+STACK del pare
		del_ss_pag(process_PT_father,pag+NUM_PAG_CODE+NUM_PAG_DATA+NUM_PAG_KERNEL);
  	}
	set_cr3(get_DIR(current()));
	child->PID = nextPID;
	PId = nextPID;
	nextPID++;
	childu = (union task_union *) child;
	childu->stack[KERNEL_STACK_SIZE - 17] = (unsigned long) ret_from_fork;
	childu->stack[KERNEL_STACK_SIZE - 18] = 0;
	child->kernel_esp = &childu->stack[KERNEL_STACK_SIZE - 18];
	child->st.user_ticks = 0;
	child->st.system_ticks = 0;
	child->st.blocked_ticks = 0;
	child->st.ready_ticks = 0;
	child->st.elapsed_total_ticks=get_ticks();
	child->st.total_trans = 0;
	child->st.remaining_ticks = 0;
	list_add_tail(&(child->list),&readyqueue);
	child->estat = ST_READY;
	}
	system_to_user(&(current()->st));
	if (PId < 0) return -11;
	return PId;
}



void sys_exit()
{
user_to_system(&(current()->st));
unsigned int frame_free;
int ii;
int pag;
int pos = -1;
page_table_entry *process_PT =  get_PT(current());
pos = current()->pos;
if (pos != -1) --vdir[pos];
if (vdir[pos] == 0){
	for (pag = 0; pag < NUM_PAG_DATA; pag++){
		frame_free = get_frame(process_PT,pag+NUM_PAG_KERNEL+NUM_PAG_CODE);
		free_frame(frame_free);		
	}
	for (pag = 0; pag < NUM_PAG_DATA; pag++){ //Eliminem la copia de DATA+STACK del pare
		del_ss_pag(process_PT,pag+NUM_PAG_CODE+NUM_PAG_KERNEL);
	}
}
	reset_stats();
for (ii = 0; ii < NR_TASKS; ++ii){
	if(semafor[ii].owner == current()->PID)sem_destroy(ii);
}
current()->estat = ST_ZOMBIE;
current()->PID = -1;
update_current_state_rr(&freequeue);
update_sched_data_rr();
sched_next_rr();
}

int sys_stats (int pid, struct stats *st){
int i;
int j;
int flag = 0;
user_to_system(&(current()->st));
j = 0;
if (st == NULL){
	system_to_user(&(current()->st));
	return -14;
}
if (access_ok(VERIFY_WRITE, st, sizeof(struct stats)) == 0){
	system_to_user(&(current()->st));
	return -14;
}
if (pid < 0){
	system_to_user(&(current()->st));
	return -22;
}
if (pid >= nextPID){
	system_to_user(&(current()->st));
	return -3;
}
if (current()->PID == pid) {
	i = copy_to_user(&current()->st,st,sizeof(struct stats));
	system_to_user(&current()->st);
	if (i < 0) return -1;
	else return 0; 
}
for (i = 0; i < NR_TASKS && j == 0 && flag == 0; ++i){
	//if((task[i].task.PID == pid) && ((task[i].task.estat == ST_READY) || (task[i].task.estat == ST_RUN))){
	if(task[i].task.PID == pid){
	flag = 1;
	if (task[i].task.estat != ST_ZOMBIE){
		j = 1;
		j = copy_to_user(&task[i].task.st,st,sizeof(struct stats));
		if (j < 0){
			system_to_user(&current()->st);
			return -1;
		}
		else j = 1;
		}	
	}
}
system_to_user(&(current()->st));
if (j == 1) return 0;
else return -3;
}

int sys_write (int fd, char * buffer, int size) {
user_to_system(&(current()->st));
	//Comprovacio file description
	int fd_correcte;
	int bytes_escrits;
	int j;
	char buffersistema[size];
	fd_correcte = check_fd(fd,ESCRIPTURA);
	if (fd_correcte != 0) {
		//EBADF
		system_to_user(&(current()->st));
		return -9;
	}
	else if (buffer==NULL) {
		//EFAULT
		system_to_user(&(current()->st));
		return -14;
	}
	else if (size < 0) {
		//errno=22; //invalid param
		system_to_user(&(current()->st));
		return -22;
	}
	else { //(fd_correcte==0 & *buffer!=NULL & size>=0)
		j = copy_from_user(buffer,&buffersistema, size);
		if(j == 0) {
			bytes_escrits= sys_write_console(&buffersistema[0],size);
			system_to_user(&(current()->st));
			return bytes_escrits;
		}
		else {
			system_to_user(&(current()->st));
			return -11;
		}
	}
}

int sys_ticks() {
user_to_system(&(current()->st));
system_to_user(&(current()->st));
return zeos_ticks;
}

//semafors
int sem_init (int n_sem, unsigned int value) {
printc('S');
	if ((n_sem >= NR_TASKS) || (n_sem < 0))return -22;
	else if((semafor[n_sem].owner > 0) || (value < 0)) return -16;
	else {
	semafor[n_sem].compt = value;
	semafor[n_sem].owner = current()->PID;
	INIT_LIST_HEAD(&semafor[n_sem].queuesemafor);
	return 0;
	}
}

int sem_wait (int n_sem) {
union task_union *p;
	if ((n_sem >= NR_TASKS) || (n_sem < 0)) return -22;
 	if (semafor[n_sem].owner == -1) return -22;
	if (semafor[n_sem].compt <= 0) {
		current()->estat = ST_BLOCKED;
		list_add_tail(&(current()->list),&(semafor[n_sem].queuesemafor));
		if ((list_empty(&readyqueue))){
			ready_to_run(&task[0].task.st);
			task[0].task.estat = ST_RUN;
			task_switch(&task[0]);
			return 0;
		}
		else {
			sched_next_rr();
			if (semafor[n_sem].owner == -1)return -22;
			else return 0;
			}
	}
	else {
		semafor[n_sem].compt--;
		return 0;
	}
	return -22;
}

int sem_signal (int n_sem) {
struct list_head *llis;
struct task_struct* s;
	if (n_sem > NR_TASKS || n_sem < 0 || semafor[n_sem].owner == -1) return -22;
	else if (list_empty(&(semafor[n_sem].queuesemafor))) {
			semafor[n_sem].compt++;
			return 0;
		}
		else {
			llis = (semafor[n_sem].queuesemafor.next);
			s = list_head_to_task_struct(llis);
			list_del(&(s->list));
			s->estat = ST_READY;
			list_add_tail(&s->list,&readyqueue);
			return 0;
		}
	return -1;
}


int sem_destroy (int n_sem){
struct list_head *llis;
struct task_struct *tasc;
	if ((n_sem >= NR_TASKS) || (n_sem < 0) || (semafor[n_sem].owner == -1)) return -22;
	if (semafor[n_sem].owner != current()->PID) return -1;
	else {
		semafor[n_sem].owner = -1;
		while(!(list_empty(&semafor[n_sem].queuesemafor))) {
			llis = list_first(&(semafor[n_sem].queuesemafor));
			tasc = list_head_to_task_struct(llis);
			list_del(llis);
			list_add_tail(&tasc->list,&readyqueue);
		}
		return 0;
 
	}
}
