#ifndef CORO_H
#define CORO_H

#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "list.h"

/* Coroutine stack size (64KB) */
#define CORO_STACK_SIZE (1024 * 64)

/* Coroutine states */
typedef enum {
    CORO_NEW,       /* Created but not started */
    CORO_READY,     /* Ready to run */
    CORO_RUNNING,   /* Currently running */
    CORO_SUSPENDED, /* Yielded/suspended */
    CORO_DEAD       /* Completed execution */
} coro_state_t;

/* Coroutine function type */
typedef void (*coro_func_t)(void);

/* Coroutine structure */
typedef struct coro {
    jmp_buf env;           /* Execution context */
    void *stack;           /* Stack pointer */
    void *stack_bottom;    /* Original stack allocation for cleanup */
    coro_func_t fn;        /* Function pointer */
    coro_state_t state;    /* Current state */
    struct list_head list; /* For ready queue */
    void *user_data;       /* Optional user data */
} coro_t;

/* Global state */
extern jmp_buf scheduler_ctx;        /* Scheduler context */
extern struct list_head ready_queue; /* Ready coroutines */
extern coro_t *current_coro;         /* Currently running coroutine */

#endif /* CORO_H */