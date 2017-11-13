#include <ctype.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// We blindly assume that our test programs never have a file name
// longer than 1024 bytes minus the name of the compiler executable.

static char command[1024];
static unsigned char data_input[1024];
static unsigned char data_output[1024];
static unsigned char data_expected[1024];
static char path_test_file[1024];
static char short_test_name[1024];
static volatile int *shared_result;

static bool debug = false;

static void print_err(char *msg) { printf("[%s] %s!\n", short_test_name, msg); }

/* --------------------------------------------------------------
 * Run test programs (check "baby C" features)
 * -------------------------------------------------------------- */

static int run_test_c(volatile int *return_value) {
    int result;

    snprintf(command, 1024, "./a.out");
    result = WEXITSTATUS(system(command));
    *return_value = result;
    return 0;
}

/* --------------------------------------------------------------
 * Run test programs (check "baby stdlib" features)
 * -------------------------------------------------------------- */

static void allocate_shared_memory(void) {
    shared_result = mmap(0, sizeof(int), PROT_READ | PROT_WRITE,
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static int parse_data(char *line, unsigned char *data, int *data_len) {
    *data = 0;
    *data_len = 0;
    char *pt = strstr(line, ":");

    if (!pt)
        return 1;
    char *hex = strstr(pt + 1, "\"");

    if (!hex)
        return 1;
    pt = strstr(++hex, "\"");
    if (!pt)
        return 1;
    *pt = 0;

    while (*hex) {
        if (!isxdigit(*hex)) {
            return 1;
        }
        if (!isxdigit(*(hex + 1))) {
            return 1;
        }
        sscanf(hex, "%02hhx", &data[(*data_len)++]);
        hex += 2;
    }
    return 0;
}

static int run_test_stdlib(volatile int *return_value) {
    int input_len, expected_len, output_len;
    int ret;

    input_len = 0;
    expected_len = 0;
    *return_value = 0x7fffffff;
    *data_input = 0;
    *data_expected = 0;

    /* input and expected data are stored in the C source code */
    FILE *ftext = fopen(path_test_file, "rt");

    if (!ftext) {
        print_err("Unable to read C code");
        if (debug) {
            exit(1);
        }
    }
    while (fgets(command, 1024, ftext)) {
        if (!memcmp(command, "#INPUT:", 7)) {
            ret = parse_data(command, data_input, &input_len);
            if (ret) {
                print_err("Invalid #INPUT: tag in C code");
                return 1;
            }
        }
        if (!memcmp(command, "#EXPECTED:", 10)) {
            ret = parse_data(command, data_expected, &expected_len);
            if (ret) {
                print_err("Invalid EXPECTED: tag in C code");
                return 1;
            }
        }
    }

    fclose(ftext);

#define PARENT_READ_FD (pipes[1][0])
#define PARENT_WRITE_FD (pipes[0][1])

#define CHILD_READ_FD (pipes[0][0])
#define CHILD_WRITE_FD (pipes[1][1])

    int pipes[2][2];

    *shared_result = 0x7fffffff;

    // pipes for parent to write and read
    ret = pipe(pipes[0]);
    if (ret != 0) {
        print_err("Unable to open temp pipe");
        exit(1);
    }
    ret = pipe(pipes[1]);
    if (ret != 0) {
        print_err("Unable to open temp pipe");
        exit(1);
    }

    if (!fork()) {
        dup2(CHILD_READ_FD, 0);
        dup2(CHILD_WRITE_FD, 1);

        /* Close fds not required by child. Also, we don't
           want the exec'ed program to know these existed */
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);
        close(PARENT_READ_FD);
        close(PARENT_WRITE_FD);

        run_test_c(shared_result);
        exit(0);
    } else {
        int write_count = 0;

        /* close fds not required by parent */
        close(CHILD_READ_FD);
        close(CHILD_WRITE_FD);

        // Write to child’s stdin
        while (write_count < input_len) {
            ret = write(PARENT_WRITE_FD, &data_input[write_count],
                        input_len - write_count);
            if (ret < 0) {
                perror("write");
                print_err("Unable to write pipe");
                exit(1);
            }
            write_count += ret;
        }

        if (write_count != input_len) {
            print_err("write incomplete");
            if (debug) {
                printf("[%s] Provided %d bytes, but wrote %d bytes!\n",
                       short_test_name, input_len, write_count);
                exit(1);
            }
            return 1;
        }
        close(PARENT_WRITE_FD);

        // Read from child’s stdout
        output_len = 0;
        while (output_len < expected_len) {
            ret = read(PARENT_READ_FD, &data_output[output_len],
                       expected_len - output_len);
            if (ret < 0) {
                perror("read");
                print_err("Unable to read pipe");
                exit(1);
            }
            output_len += ret;
        }

        close(PARENT_READ_FD);

        if (expected_len != output_len) {
            print_err("Expected data length and output data length differ");
            if (debug) {
                printf("[%s] Expected %d bytes, but got %d bytes!\n",
                       short_test_name, expected_len, output_len);
                exit(1);
            }
            return 1;
        }
        if (memcmp(data_expected, data_output, expected_len)) {
            print_err("Expected data and output data differ");
            if (debug) {
                int offset = 0;

                for (offset = 0; offset < expected_len; offset++) {
                    if (data_output[offset] != data_expected[offset])
                        break;
                }

                printf("[%s] Expected and output bytes not identical at offset "
                       "%d!\n",
                       short_test_name, offset);
                exit(1);
            }
            return 1;
        }

        int timeout = 0;

        while (*shared_result == 0x7fffffff) {
            usleep(100);
            timeout += 100;
            if (timeout > 1000000) {
                printf("[%s] run slower than expected!\n", short_test_name);
                break;
            }
        }
        while (*shared_result == 0x7fffffff) {
            usleep(100);
        }
        *return_value = *shared_result;
    }

    return 0;
}

/* --------------------------------------------------------------
 * Iterate over test files
 * -------------------------------------------------------------- */

static bool is_test_file(char *file_name) {
    // A test program is simply one that contains two consecutive underscores.
    return (strstr(file_name, "__") != NULL);
}

static int get_expected_result(char *file_name) {
    int expected_return = -1;
    char *return_position = strstr(file_name, "__return_");

    if (return_position != NULL) {
        return_position += strlen("__return_");
        expected_return = atoi(return_position);
    }
    return expected_return;
}

static int test_iterate(char *compiler_options, char *dir_name,
                        int (*run)(volatile int *return_value)) {
    int tests_run = 0, tests_passed = 0;
    struct dirent *file;
    char *file_name;
    int result;

    DIR *test_dir = opendir(dir_name);

    if (test_dir == NULL) {
        printf("Could not open %s directory!", dir_name);
        exit(1);
    }

    while ((file = readdir(test_dir)) != NULL) {
        file_name = file->d_name;

        if (is_test_file(file_name)) {

            // If it contains a 'return_NUMBER' file name, extract it.
            int expected_return = get_expected_result(file_name);

            tests_run++;

            snprintf(path_test_file, 1024, "%s/%s", dir_name, file_name);
            snprintf(short_test_name, 1024, "%s", file_name);
            snprintf(command, 1024, "./build/babyc %s -o a.out %s",
                     compiler_options, path_test_file);
            result = WEXITSTATUS(system(command));

            if (result != 0) {
                print_err("Compilation failed");
                if (debug) {
                    printf("[%s] Compiler exit with status = %d!\n", file_name,
                           result);
                }
                printf("F");
            } else {
                int got_value = 0x7fffffff;

                result = run(&got_value);
                if (result != 0) {
                    print_err("Wrong execution exit code");
                    if (debug) {
                        printf("[%s] Expected %d, but got %d!\n", file_name, 0,
                               result);
                        exit(1);
                    }
                    printf("F");
                } else {

                    if (got_value != expected_return) {
                        print_err("Wrong return value");
                        if (debug) {
                            printf("[%s] Expected %d, but got %d!\n", file_name,
                                   expected_return, got_value);
                            exit(1);
                        }
                        printf("F");
                    } else {
                        tests_passed++;
                        printf(".");
                    }
                }
            }

            snprintf(command, 1024, "rm -f out.s out.o a.out");
            result = WEXITSTATUS(system(command));
            if (result) {
                print_err("Unable to delete temp files");
                if (debug) {
                    exit(1);
                }
            }

            fflush(stdout);
        }
    }
    printf("\n\n%d tests run, %d passed, %d failed.\n", tests_run, tests_passed,
           tests_run - tests_passed);

    closedir(test_dir);

    return tests_run - tests_passed;
}

/* --------------------------------------------------------------
 * test entry point
 * -------------------------------------------------------------- */

void print_help(void) {
    printf("Run babyc compiler tests\n");
    printf("-h : this message\n");
    printf("-g : more debug, compile with -g, stop on first error\n");
}

int main(int argc, char **argv) {
    int total_failed = 0;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_help();
            exit(0);
        }
        if (!strcmp(argv[i], "-g")) {
            debug = true;
            continue;
        }
        print_help();
        exit(1);
    }
    allocate_shared_memory();
    printf("-------------------------------------------------------------------"
           "-\n");
    printf("Non optimized code -O0\n");
    printf("-------------------------------------------------------------------"
           "-\n");
    printf("\n");
    total_failed += test_iterate("-O0", "./test_programs", run_test_c);
    total_failed += test_iterate("-O0", "./test_stdlib", run_test_stdlib);
    printf("-------------------------------------------------------------------"
           "-\n");
    printf("Optimized code -O1\n");
    printf("-------------------------------------------------------------------"
           "-\n");
    printf("\n");
    total_failed += test_iterate("-O1", "./test_programs", run_test_c);
    total_failed += test_iterate("-O1", "./test_stdlib", run_test_stdlib);

    printf("-------------------------------------------------------------------"
           "-\n");
    printf("\n");
    printf("\n%d tests to fix.\n", total_failed);

    return total_failed ? 1 : 0;
}
