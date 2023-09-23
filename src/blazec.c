/*
 * Created by rakinar2 on 9/19/23.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include "arch.h"
#include "compile.h"
#include "file.h"
#include "lexer.h"
#include "log.h"
#include "parser.h"
#include "utils.h"

#if defined(__WIN32__)
#define BLAZE_WINDOWS
#include <windows.h>
#include <io.h>
#else
#include <sys/wait.h>
#endif

// FIXME
#define ASSEMBLER_PATH "/usr/bin/as"
#define LINKER_PATH "/usr/bin/ld"
#define DYNAMIC_LINKER_PATH "/lib64/ld-linux-x86-64.so.2"
#define LIB_BLAZE_PATH "./lib/libblazert.a"

static struct option const long_options[] = {
    { "output",      required_argument, NULL, 'o' },
    { "assembly",    no_argument,       NULL, 'S' },
    { "compile",     no_argument,       NULL, 'c' },
    { "executable",  no_argument,       NULL, 'x' },
    { "arch",  required_argument,       NULL, 'a' },
    { 0,             0,                 0,    0  }
};

enum blazec_generation_mode
{
    GEN_ASM,
    GEN_OBJ,
    GEN_LINK_EXEC,
    GEN_ASM_STDOUT
};

struct blazec_context
{
    char *outfile;
    char *infile;
    enum blazec_generation_mode mode;
    enum blazec_arch arch;
};

static void blazec_context_free(struct blazec_context *context)
{
    free(context->outfile);
    free(context->infile);
}

static const char *blazec_generation_mode_to_str(enum blazec_generation_mode mode)
{
    const char *translate[] = {
        [GEN_ASM] = "ASM",
        [GEN_LINK_EXEC] = "EXECUTABLE",
        [GEN_OBJ] = "OBJECT",
        [GEN_ASM_STDOUT] = "ASM_TO_STDOUT",
    };

    if ((sizeof (translate) / sizeof (translate[0])) <= mode)
        fatal_error("invalid generation mode");

    return translate[mode];
}

static void blazec_process_options(int argc, char **argv, struct blazec_context *context)
{
    int c;

    opterr = 0;

    while (true)
    {
        int option_index = 1;
        c = getopt_long(argc, argv, ":o:SGcxa:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            case 'o':
                context->outfile = strdup(optarg);
                break;

            case 'a':
                context->arch = arch_str_to_type(optarg);
                break;

            case 'S':
                context->mode = GEN_ASM;
                break;

            case 'G':
                context->mode = GEN_ASM_STDOUT;
                break;

            case 'c':
                context->mode = GEN_OBJ;
                break;

            case 'x':
                context->mode = GEN_LINK_EXEC;
                break;

            case ':':
                fatal_error("option '%s' requires an argument", argv[optind - 1]);
                break;

            case '?':
                fatal_error("Invalid option: '%s'", argv[optind - 1]);
                break;

            default:
                fatal_error("invalid option code");
                break;
        }
    }
}

static void blazec_write_assembly(struct compilation_context *compilation_context, asm_node_t *node, FILE *dest)
{
    asm_print_header(dest, &compilation_context->asm_data);
    asm_print(dest, compilation_context->arch, node);
}

static void blazec_write_assembly_file(struct compilation_context *compilation_context, asm_node_t *node, const char *filename)
{
    FILE *file = fopen(filename, "wb+");
    blazec_write_assembly(compilation_context, node, file);
    fclose(file);
}

static void blazec_write_object_file(struct compilation_context *compilation_context, asm_node_t *node, char *filename)
{
    char tmp_file_name[] = "/tmp/blazec-compiled-XXXXXX";
    int fd = mkstemp(tmp_file_name);
    FILE *file = fdopen(fd, "wb+");
    blazec_write_assembly(compilation_context, node, file);
    fclose(file);

#ifndef BLAZE_WINDOWS
    pid_t pid = fork();

    if (pid < 0)
        fatal_error("could not create new process: %s", strerror(errno));

    if (pid == 0)
    {
        exit(execlp(ASSEMBLER_PATH, ASSEMBLER_PATH, tmp_file_name, "-o", filename, NULL));
    }
    else
    {
        int code = 0;
        wait(&code);

        if (code != 0)
            fatal_error("assembler failed with exit code %d\n", code);
    }
#else
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char *command = NULL;
    asprintf(&command, ASSEMBLER_PATH " %s -o %s", tmp_file_name, filename);

    if(!CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        fatal_error("could not create new process: %s", strerror(errno));

    WaitForSingleObject(pi.hProcess, INFINITE);
    free(command);

    DWORD exitCode;

    if (!GetExitCodeProcess(pi.hProcess, &exitCode))
        fatal_error("failed to determine exit code of child process");

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#endif
    unlink(tmp_file_name);
}

static void blazec_write_executable_file(struct compilation_context *compilation_context, asm_node_t *node, char *filename)
{
    char tmp_file_name[36] = "/tmp/blazec-compiled-obj-XXXXXX";
    int fd = mkstemp(tmp_file_name);
#ifndef BLAZE_WINDOWS
    fchmod(fd, 0665);
#else
    if (_chmod(tmp_file_name, _S_IWRITE | _S_IREAD) == -1) 
        fatal_error("could not set file permissions");
#endif
    blazec_write_object_file(compilation_context, node, tmp_file_name);

#ifndef BLAZE_WINDOWS
    pid_t pid = fork();

    if (pid < 0)
        fatal_error("could not create new process: %s", strerror(errno));

    if (pid == 0)
    {
        exit(execlp(LINKER_PATH, LINKER_PATH, "-dynamic-linker",
                    DYNAMIC_LINKER_PATH, tmp_file_name, LIB_BLAZE_PATH,
                    "-lc", "-o", filename, NULL));
    }
    else
    {
        int code = 0;
        wait(&code);

        if (code != 0)
            fatal_error("ld failed with exit code %d\n", code);
    }
#else
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char *command = NULL;
    asprintf(&command, LINKER_PATH " -dynamic-linker " DYNAMIC_LINKER_PATH " %s " LIB_BLAZE_PATH " -lc -o %s", tmp_file_name, filename);

    if( !CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        fatal_error("could not create new process: %s", strerror(errno));

    WaitForSingleObject(pi.hProcess, INFINITE);
    free(command);

    DWORD exitCode;

    if (!GetExitCodeProcess(pi.hProcess, &exitCode))
        fatal_error("failed to determine exit code of child process");

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#endif

    close(fd);
    unlink(tmp_file_name);
}

static void blazec_compile_file(struct blazec_context *context)
{
    struct filebuf buf = filebuf_init(context->infile);
    filebuf_read(&buf);
    struct lex lex = lex_init(context->infile, buf.content);
    lex_analyze(&lex);
    struct parser parser = parser_init_from_lex(&lex);
    ast_node_t node = parser_create_ast_node(&parser);
#ifndef NDEBUG
    blaze_debug__print_ast(&node);
#endif
    struct compilation_context compilation_context = compilation_context_create();
    compilation_context.arch = context->arch;
    asm_node_t asm_node = compile(&compilation_context, &node);

    switch (context->mode)
    {
        case GEN_ASM:
            blazec_write_assembly_file(&compilation_context, &asm_node, context->outfile);
            break;
        case GEN_OBJ:
            blazec_write_object_file(&compilation_context, &asm_node, context->outfile);
            break;
        case GEN_LINK_EXEC:
            blazec_write_executable_file(&compilation_context, &asm_node, context->outfile);
            break;
        case GEN_ASM_STDOUT:
            blazec_write_assembly(&compilation_context, &asm_node, stdout);
            break;

        default:
            fatal_error("unsupported generation mode");
    }

    asm_node_free_inner(&asm_node);
    compilation_context_destroy(&compilation_context);
    parser_ast_free_inner(&node);
    parser_free(&parser);
    lex_free(&lex);
    filebuf_close(&buf);
    filebuf_free(&buf);
}

static char *blazec_determine_outfile_name(const char *restrict infile, const enum blazec_generation_mode mode)
{
    char *ret = NULL;
    char *outfile = strdup(infile);
    bool ext_changed = false;
    ssize_t len = (ssize_t) strlen(outfile);
    ssize_t index = len == 0 ? 0 : len - 1;

    if ((index - 3) >= 0 && outfile[index] == 'l'
        && outfile[index - 1] == 'b'
        && outfile[index - 2] == '.')
    {
        outfile[index - (mode == GEN_LINK_EXEC ? 2 : 1)] = 0;
        ext_changed = true;
    }

    if (!ext_changed && mode == GEN_LINK_EXEC)
        ret = strdup("a.out");
    else
        asprintf(&ret, "%s%s%s", outfile,
                 ext_changed ? "" : ".",
                 mode == GEN_LINK_EXEC ? "" :
                 mode == GEN_OBJ ? "o" : mode == GEN_ASM ? "s" : "out");
    free(outfile);
    return ret;
}

void blazec_context_set_default_arch(struct blazec_context *context)
{
    context->arch = arch_default_get();
}

int main(int argc, char **argv)
{
    struct blazec_context context = { 0 };
    context.mode = GEN_LINK_EXEC;

    blazec_process_options(argc, argv, &context);

    if (optind >= argc)
        fatal_error("no input files");

    if (optind < argc)
        context.infile = strdup(argv[optind]);

    if (context.outfile == NULL)
        context.outfile = blazec_determine_outfile_name(basename(context.infile), context.mode);

    if (context.arch == 0)
        blazec_context_set_default_arch(&context);

    log_debug("Compiling    '%s'", context.infile);
    log_debug("Generate     '%s'", context.outfile);
    log_debug("Mode         '%s'", blazec_generation_mode_to_str(context.mode));
    log_debug("Architecture '%s'", arch_to_str(context.arch));

    blazec_compile_file(&context);
    blazec_context_free(&context);
    return 0;
}
