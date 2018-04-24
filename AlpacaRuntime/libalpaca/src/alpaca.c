#include <stdarg.h>
#include <string.h>
#include <libio/log.h>
#include <msp430.h>

#ifndef LIBCHAIN_ENABLE_DIAGNOSTICS
#define LIBCHAIN_PRINTF(...)
#else
#include <stdio.h>
#define LIBCHAIN_PRINTF printf
#endif

#include "alpaca.h"

/**
 * @brief dirtylist to save src address
 */
__nv uint8_t** data_src_base = &data_src;
/**
 * @brief dirtylist to save dst address
 */
__nv uint8_t** data_dest_base = &data_dest;
/**
 * @brief dirtylist to save size
 */
__nv unsigned* data_size_base = &data_size;

/**
 * @brief len of dirtylist
 */
__nv volatile unsigned num_dirty_gv=0;

/**
 * @brief double buffered context
 */
__nv context_t context_1 = {0};
/**
 * @brief double buffered context
 */
__nv context_t context_0 = {
	.task = _entry_task,
	.numRollback = 0,
};
/**
 * @brief current context
 */
__nv context_t * volatile curctx = &context_0;
/**
 * @brief current version which updates at every reboot or transition
 */
__nv volatile unsigned _numBoots = 0;

/**
 * @brief Function to be invoked at the beginning of every task
 */
void task_prologue()
{
	// increment version
	if(_numBoots == 0xFFFF){
		clear_isDirty();
		++_numBoots;
	}
	++_numBoots;
	// commit if needed
	while (curctx->numRollback) {
		unsigned rollBackIdx = curctx->numRollback - 1;
		uint8_t* w_data_dest = *(data_dest_base + rollbackIdx);
		uint8_t* w_data_src= *(data_src_base + rollbackIdx);
		unsigned w_data_size = *(data_size_base + rollbackIdx);
		memcpy(w_data_dest, w_data_src, w_data_size);
		curctx->numRollback--;
	}
}

/**
 * @brief Transfer control to the given task
 * @details Finalize the current task and jump to the given task.
 *          This function does not return.
 *
 */
//void transition_to(task_t *next_task)
//{
//	// double-buffered update to deal with power failure
//	context_t *next_ctx;
//	next_ctx = (curctx == &context_0 ? &context_1 : &context_0);
//	next_ctx->task = next_task;
//	next_ctx->needCommit = 1;
//
//	// atomic update of curctx
//	curctx = next_ctx;
//
//	// fire task prologue
//	task_prologue();
//	// jump to next tast
//	__asm__ volatile ( // volatile because output operands unused by C
//			"mov #0x2400, r1\n"
//			"br %[ntask]\n"
//			:
//			: [ntask] "r" (next_task->func)
//			);
//}

/**
 * @brief save info for rollback: TODO: This can get optimized
 *
 */
void log_backup(uint8_t *orig_addr, uint8_t *backup_addr, size_t var_size)
{
	// save info for rollback
	unsigned numRollback = curctx->numRollback;
	*(data_size_base + numRollback) = var_size;
	*(data_dest_base + numRollback) = orig_addr;
	*(data_src_base + numRollback) = backup_addr;
	// increment count
	curctx->numRollback = numRollback + 1;
}

/** @brief Entry point upon reboot */
int main() {
	_init();

	// (better alternative: transition_to(curctx->task);

	// check for update
	task_prologue();
	while (1) {
		curctx->task();
	}
	return 0;
}
