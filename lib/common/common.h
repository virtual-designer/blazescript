#ifndef BLAZE_LIB_COMMON_COMMON_H
#define BLAZE_LIB_COMMON_COMMON_H

#ifdef I386
#define I386
#define BLAZE_ARCH i386
#elif defined(__x86_64__) || defined(_M_X64)
#define X86_64
#define BLAZE_ARCH x86_64
#elif defined(__arm__) || defined(_M_ARM)
#define ARM
#define BLAZE_ARCH arm
#elif defined(__aarch64__) || defined(_M_ARM64)
#define AARCH64
#define BLAZE_ARCH aarch64
#else
#error "Unsupported architecture"
#endif

#define BLAZE(fn) BLAZE_ARCH##_libblaze_##fn

#endif
