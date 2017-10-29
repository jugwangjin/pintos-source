#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */


typedef int pfloat;
#define F 16384
#define ITOF(x) (pfloat) (x * F)
#define FTOI(x) (int)(x / F)
#define ADD(x, y) (x + y)
#define SUB(x, y) (x - y)
#define ADDn(x, n) (x + n * F)
#define SUBn(x, n) (x - n * F)
#define MULT(x, y) (((int64_t) x) * y / F)
#define MULTn(x, n) (x * n)
#define DIV(x, y) (((int64_t) x) * F / y)
#define DIVn(x, n) (x / n)
#define PRI_UPDATE(recent_cpu, nice) FTOI(SUB(SUB(ITOF(PRI_MAX), DIVn(recent_cpu, 4)), MULTn(ITOF(nice), 2)))
#define CPU_UPDATE(load, recent_cpu, nice) ADDn(MULT(DIV(MULTn(load, 2), ADDn(MULTn(load, 2), 1)), recent_cpu), nice)
#define LOAD_UPDATE(load, ready) ADD(MULT(DIVn(ITOF(59), 60), load), DIVn(ITOF(ready), 60))




/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */
    struct list_elem slpelem;           /* List element for sleeping threads list */
    int64_t alarm;                      /* Alarm schedule */
    int nice;                           /* Niceness */
    pfloat recent_cpu;                  /* Recent CPU occupation */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */
    struct list_elem semaelem;          /* Semaphore List element, no more shared with synch.c. */
#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */

    /* Priority Donation */
    int donated_priority;
    struct list lock_holding_list;       /* for nested priority donation */
    struct lock *acquiring_lock;         /* for nested priority donation */
  };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;
/* load_avg and ready_trheads are used in both timer.c and thread.c */
extern pfloat load_avg;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

/* Additional functions for alarm clock */
void thread_sleep(int64_t);
int64_t thread_wake(int64_t);

/* Maximum OS timer ticks. */
#define MAX_TIMER 0x7fffffffffffffLL

/*Additional functions for priority scheduling (1-3) */
void update_cpu (struct thread *t, void *aux);
void update_priority (struct thread *t, void *aux);
void thread_relocate (void);

/* thread_get_priority_by_pointer */
int thread_get_priority_from_pointer (struct thread *);

/* added for less function */
bool thread_priority_less (struct list_elem *, struct list_elem *, void *);

int real_ready_threads(void);

#endif /* threads/thread.h */
