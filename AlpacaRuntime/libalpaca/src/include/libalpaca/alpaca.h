#ifndef ALPACA_H
#define ALPACA_H

#include <stddef.h>
#include <stdint.h>
#include <libmsp/mem.h>

typedef void (task_func_t)(void);

/** @brief Execution context */
typedef struct _context_t {
	/** @brief current running task */
	task_func_t *task;
	/** @brief indicate whether to jump to commit stage on power failure*/
	unsigned numRollback;
} context_t;

extern uint8_t* data_src[];
extern uint8_t* data_dest[];
extern unsigned data_size[];
extern uint8_t** data_src_base;
extern uint8_t** data_dest_base;
extern unsigned* data_size_base;
extern volatile unsigned _numBoots;
extern context_t * volatile curctx;
extern context_t context_0;
extern context_t context_1;
/** @brief LLVM generated function that clears all isDirty_ array */
extern void clear_isDirty();
/** @brief Function called on every reboot
 *  @details This function usually initializes hardware, such as GPIO
 *           direction. The application must define this function because
 *           different app uses different GPIO.
 */
extern void init();
extern task_func_t* _init;

void log_backup(uint8_t *data_src, uint8_t *data_dest, size_t var_size);

/** @brief Declare a task
 *
 *  @param idx      Global task index, zero-based
 *  @param func     Pointer to task function
 *
 */
#define TASK(func) \
	void func();

#define TRANSITION_TO(next_task) \
	context_t *next_ctx;\
	next_ctx = (curctx == &context_0 ? &context_1 : &context_0);\
	next_ctx->task = &next_task;\
	next_ctx->numRollback = 0;\
	curctx = next_ctx;\
	return;

// for compatibility
#define TASK_REF(task) \
	*task

/** @brief Declare the first task of the application
 *  @details This macro defines a function with a special name that is
 *           used to initialize the current task pointer.
 *
 *           This does incur the penalty of an extra task transition, but it
 *           happens only once in application lifetime.
 *
 *           The alternatives are to force the user to define functions
 *           with a special name or to define a task pointer symbol outside
 *           of the library.
 */
#define ENTRY_TASK(_task) \
	__nv context_t context_0 = {\
		.task = &_task,\
		.numRollback = 0,\
	};
	//task_func_t* _entry_task = &task;

/** @brief Init function prototype
 *  @details We rely on the special name of this symbol to initialize the
 *           current task pointer. The entry function is defined in the user
 *           application through a macro provided by our header.
 */
//void _init();

/** @brief Declare the function to be called on each boot
 *  @details The same notes apply as for entry task.
 */
#define INIT_FUNC(func) \
	task_func_t* _init = &func;

/**
 *  @brief way to simply rename vars. I don't need it actually.
 *  I should remove it or rename it..
 *  Actually I should just remove this thing!
 */
#define GLOBAL_SB(type, name, ...) GLOBAL_SB_(type, name, ##__VA_ARGS__, 3, 2)
#define GLOBAL_SB_(type, name, size, n, ...) GLOBAL_SB##n(type, name, size)
#define GLOBAL_SB2(type, name, ...) __nv type _global_ ## name
#define GLOBAL_SB3(type, name, size) __nv type _global_ ## name[size]

/**
 *  @brief way to simply reference renamed vars. I don't need it actually.
 *  I should remove it or rename it..
 *  Actually I should just remove this thing!
 */
#define GV(type, ...) GV_(type, ##__VA_ARGS__, 2, 1)
#define GV_(type, i, n, ...) GV##n(type, i)
#define GV1(type, ...) _global_ ## type
#define GV2(type, i) _global_ ## type[i]

#endif // ALPACA_H
