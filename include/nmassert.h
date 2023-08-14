#ifndef __NM_ASSERT_H__
#define __NM_ASSERT_H__
#include <cstdlib>
#include "stdio.h"

#ifdef NDEBUG
#  define NMASSERT(condition) ((void)0)
#else
#  define NMASSERT(expr) if(!(expr)) { printf("Assertion \'%s\' failed:%s:%d\n", #expr, __FILE__, __LINE__); fflush(stdout); exit(1); }
#endif

#ifdef NDEBUG
#   define NMASSERT_MSG(expr, ...) ((void)0)
#else
#   define NMASSERT_MSG(expr, ...) if(!(expr)) { printf("Assertion \'%s\' failed:%s:%d\t", #expr, __FILE__, __LINE__); \
                                    printf(__VA_ARGS__); \
                                    printf("\n"); \
                                    fflush(stdout);            \
                                    exit(1); }
#endif

#endif //__NM_ASSERT_H__
