/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <stats.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024

extern struct list_head freequeue;
extern struct list_head readyqueue;
extern int nextPID;
int vdir[NR_TASKS];
enum state_t { ST_RUN, ST_READY, ST_BLOCKED, ST_ZOMBIE };

struct task_struct {
  int PID;			/* Process ID */
  page_table_entry * dir_pages_baseAddr;
  struct list_head list;
  int quantum;
  struct stats st;
  enum state_t estat;
  int pos;
  unsigned long * kernel_esp;
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern union task_union task[NR_TASKS]; /* Vector de tasques */
extern struct task_struct *idle_task;
struct sem{
 int owner;
 unsigned int compt;
 struct list_head queuesemafor;
};
extern struct sem semafor[NR_TASKS];

#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void reset_stats(void);

void init_sched(void);

void actualitzar_quantum (void);

struct task_struct * current();

void task_switch(union task_union*t);

struct task_struct *list_head_to_task_struct(struct list_head *l);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

int allocate_DIR(struct task_struct *);

/* Headers for the scheduling policy */
void sched_next_rr();
void update_current_state_rr(struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();


void user_to_system(struct stats *st);
void system_to_user(struct stats *st);
void run_to_ready(struct stats *st);
void ready_to_run(struct stats *st);

#endif  /* __SCHED_H__ */
