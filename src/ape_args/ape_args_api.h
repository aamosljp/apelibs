#ifndef APE_ARGS_INCLUDED
#define APE_ARGS_INCLUDED

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define APE_ARGS_WINDOWS
#if defined(_WIN64)
#define APE_ARGS_WINDOWS_X64
#endif
#elif defined(__linux__) || defined(__unix__)
#define APE_ARGS_LINUX
#elif defined(__APPLE__)
#define APE_ARGS_APPLE
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef APE_ARGS_MALLOC
#if defined(APE_ARGS_REALLOC) || defined(APE_ARGS_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#else
#define APE_ARGS_MALLOC malloc
#define APE_ARGS_REALLOC realloc
#define APE_ARGS_FREE free
#endif
#else
#if !defined(APE_ARGS_REALLOC) || !defined(APE_ARGS_FREE)
#pragma GCC error "Need to define MALLOC, REALLOC and FREE or none of them"
#endif
#endif

#if defined(__cplusplus)
}
#endif

#if defined(APE_ARGS_STRIP_PREFIX)

#endif

#endif
