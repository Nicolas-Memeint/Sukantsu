#ifndef _DEBUG_H_
#define _DEBUG_H_

// Define NGDEBUG to remove all assertions

#ifdef NGDEBUG

#define ASSERT_BACKTRACE(expr) {}

#else // NGDEBUG

#include <assert.h>
#include <execinfo.h>

static void dump_trace() {
	void *buffer[63];
	const int calls = backtrace(buffer, 63);
	backtrace_symbols_fd(buffer, calls, 1);
}

#define ASSERT_BACKTRACE(expr)                                                 \
	if (!(expr)) {                                                             \
		dump_trace();                                                          \
		assert(expr);                                                          \
	}

#endif // NGDEBUG

#endif // _DEBUG_H_
