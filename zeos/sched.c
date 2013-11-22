/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>
#include <system.h>
#include <utils.h>

struct sem semafor[NR_TASKS];
int vdir[NR_TASKS] = {0};

void actualitzar_quantum (void){
current_quantum = current()->quantum;
current()->st.remaining_ticks = current_quantum;
}

void user_to_system(struct stats *st){
st->user_ticks += get_ticks() - st->elapsed_total_ticks;
st->elapsed_total_ticks = get_ticks();
st->remaining_ticks = current_quantum;
}

void system_to_user(struct stats *st){
st->system_ticks += get_ticks()- st->elapsed_total_ticks;
st->elapsed_total_ticks = get_ticks();
st->remaining_ticks = current_quantum;
}

void run_to_ready(struct stats *st){
st->system_ticks += get_ticks()- st->elapsed_total_ticks;
st->elapsed_total_ticks = get_ticks();
}

void ready_to_run(struct stats *st){
st->ready_ticks += get_ticks() - st->elapsed_total_ticks;
st->elapsed_total_ticks = get_ticks();
st->remaining_ticks = current_quantum;
st->total_trans++;
}

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));



struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
 

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


int allocate_DIR(struct task_struct *t) 
{
	int pos = -1;
	int j;

	for (j = 0; j < NR_TASKS && pos == -1;++j){
		if (vdir[j] == 0) pos = j;
	}
	if (pos == -1) return -1;
	++vdir[pos];
	t->pos = pos;
	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

	return 1;
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
	while(1)
	{
	;
	}
}

int needs_sched_rr (void){
return (current_quantum == 0);
}

void update_sched_data_rr(void){
current_quantum--;
current()->st.remaining_ticks = current_quantum;
}

void update_current_state_rr (struct list_head *dest){
list_add_tail(&(current()->list),dest); //l'afegim abans pel cas que només hi hagi un procés executant-se
}

void sched_next_rr(void){
union task_union *p;
run_to_ready(&current()->st);
if ((list_empty(&readyqueue))){
	ready_to_run(&task[0].task.st);
	current()->estat = ST_READY;
	task[0].task.estat = ST_RUN;
	task_switch(&task[0]);
}
else {
	p = (union task_union *)(list_head_to_task_struct(readyqueue.next));
	current()->estat = ST_READY;
	list_del(&(p->task.list));
	p->task.estat = ST_RUN;
	ready_to_run(&p->task.st);
	actualitzar_quantum();
	current_quantum = p->task.quantum;
	task_switch(p);
	}
}

int get_quantum (struct task_struct *t){
return t->quantum;
}

void set_quantum (struct task_struct *t, int new_quantum){
t->quantum = new_quantum;
}

void init_idle (void)
{
task[0].task.PID = 0;
task[0].stack[KERNEL_STACK_SIZE-1]= (unsigned long) cpu_idle;
task[0].stack[KERNEL_STACK_SIZE-2]= 0;
task[0].task.kernel_esp = &task[0].stack[KERNEL_STACK_SIZE-2];
//vdir[task[0].task.pos] = 0;
//allocate_DIR(&task[0].task);
set_quantum(&task[0].task,18);
task[0].task.estat = ST_READY;
}

void init_task1(void)
{

task[1].task.PID = nextPID;
nextPID++;
allocate_DIR(&task[1].task);
set_user_pages(&task[1].task);
tss.esp0=(DWord)&task[1].stack[KERNEL_STACK_SIZE];
set_cr3(task[1].task.dir_pages_baseAddr);
set_quantum(&task[1].task,18);
current_quantum=get_quantum(&task[1].task);
reset_stats();
ready_to_run(&current()->st);	
task[1].task.estat = ST_RUN;
}

void reset_stats(void){
current()->st.user_ticks = 0;
current()->st.system_ticks = 0;
current()->st.blocked_ticks = 0;
current()->st.ready_ticks = 0;
current()->st.elapsed_total_ticks=get_ticks();
current()->st.total_trans = 0;
current()->st.remaining_ticks = 0;
}

void init_sched(){
int i;
  INIT_LIST_HEAD(&freequeue);
  INIT_LIST_HEAD(&readyqueue);
for (i = 0; i < NR_TASKS; ++i){
	semafor[i].owner = -1;
}

for(i=2;i<10;++i) {
	list_add(&task[i].task.list,&freequeue);
	task[i].task.estat = ST_ZOMBIE;
  }
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

