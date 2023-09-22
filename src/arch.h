/*
 * Created by rakinar2 on 9/21/23.
 */

#ifndef BLAZESCRIPT_ARCH_H
#define BLAZESCRIPT_ARCH_H

enum blazec_arch
{
    ARCH_ARM = 1,
    ARCH_AARCH64,
    ARCH_I386,
    ARCH_X86_64
};

const char *arch_to_str(enum blazec_arch arch);
enum blazec_arch arch_str_to_type(const char *restrict str);
enum blazec_arch arch_default_get();

#endif /* BLAZESCRIPT_ARCH_H */