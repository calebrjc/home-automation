#ifndef __JCFW_UTIL_RESULT_H__
#define __JCFW_UTIL_RESULT_H__

// TODO(Caleb): Assign groups for error codes?
typedef enum
{
    JCFW_RESULT_OK = 0x00000000,
    JCFW_RESULT_ERROR,
    JCFW_RESULT_INVALID_ARGS,

    // System
    JCFW_RESULT_NOT_INITIALIZED = 0x00000100,
    JCFW_RESULT_ALLOCATION_FAILURE,

    // Connection
    JCFW_RESULT_NOT_CONNECTED = 0x00000200,
    JCFW_RESULT_DISCONNECTED,

    // DSA
    JCFW_RESULT_EMPTY = 0x00000300,
    JCFW_RESULT_FULL,
    JCFW_RESULT_OUT_OF_BOUNDS,
} jcfw_result_e;

// TODO(Caleb): Result check macro

#endif // __JCFW_UTIL_RESULT_H__
