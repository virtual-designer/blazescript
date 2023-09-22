/*
 * Created by rakinar2 on 9/21/23.
 */

#include "arch.h"
#include "utils.h"
#include <string.h>

const char *arch_to_str(enum blazec_arch arch)
{
    const char *translate[] = {
        [ARCH_I386] = "i386",
        [ARCH_X86_64] = "x86_64",
        [ARCH_AARCH64] = "aarch64",
        [ARCH_ARM] = "arm",
    };

    if ((sizeof (translate) / sizeof (translate[0])) <= arch)
        fatal_error("invalid architecture");

    return translate[arch];
}

enum blazec_arch arch_str_to_type(const char *restrict str)
{
    if (strcmp(str, "x86_64") == 0)
        return ARCH_X86_64;
    if (strcmp(str, "i386") == 0)
        return ARCH_I386;
    if (strcmp(str, "arm") == 0)
        return ARCH_ARM;
    if (strcmp(str, "aarch64") == 0)
        return ARCH_AARCH64;

    fatal_error("invalid architecture: %s", str);
}

enum blazec_arch arch_default_get()
{
#if defined(__i386__) || defined(_M_IX86)
    return ARCH_I386;
#elif defined(__x86_64__) || defined(_M_X64)
    return ARCH_X86_64;
#elif defined(__arm__) || defined(_M_ARM)
    return ARCH_ARM;
#elif defined(__aarch64__) || defined(_M_ARM64)
    return ARCH_AARCH64;
#else
#error "Unsupported architecture"
#endif
}