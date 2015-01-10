#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>

bool is_test_program(char *file_name) {
    // A test program is simply one that contains two consecutive underscores.

    char char1 = '\0', char2 = '\0';
    while (*file_name != '\0') {
        char2 = char1;
        char1 = *file_name;
        file_name++;

        if (char1 == '_' && char2 == '_') {
            return true;
        }
    }

    return false;
}

int run_test(char *test_program_name) {
    // We blindly assume that our test programs never have a file name
    // longer than 1024 bytes minus the name of the compiler executable.
    char *command = malloc(1024);

    // If it contains a 'return_NUMBER' file name, extract it.
    int expected_return = -1;
    char *return_position = strstr(test_program_name, "return_");
    if (return_position != NULL) {
        return_position += strlen("return_");

        expected_return = atoi(return_position);
    }

    snprintf(command, 1024, "./build/babyc test_programs/%s >/dev/null",
             test_program_name);
    int result = system(command);
    free(command);

    if (result != 0) {
        printf("[%s] Compilation failed!\n", test_program_name);
        return result;
    }

    if ((result = system("as out.s -o out.o --32")) != 0) {
        printf("[%s] Assembling failed!\n", test_program_name);
        return result;
    }

    if ((result = system("ld -m elf_i386 -s -o out out.o")) != 0) {
        printf("[%s] Linking failed!\n", test_program_name);
        return result;
    }

    result = WEXITSTATUS(system("./out"));

    system("rm out.s out.o out");

    if (result != expected_return) {
        printf("[%s] Expected %d, but got %d!\n", test_program_name,
               expected_return, result);
        return 1;
    } else {
        return 0;
    }
}

int main() {
    DIR *test_dir = opendir("test_programs");

    if (test_dir == NULL) {
        printf("Could not open test_programs directory!");
        return 1;
    }

    int tests_run = 0, tests_passed = 0;

    struct dirent *file;
    char *file_name;
    int test_result;
    while ((file = readdir(test_dir)) != NULL) {
        file_name = file->d_name;

        if (is_test_program(file_name)) {
            test_result = run_test(file_name);

            tests_run++;
            if (test_result == 0) {
                printf(".");
                tests_passed++;
            } else {
                printf("F");
            }
        }
    }
    printf("\n\n%d tests run, %d passed, %d failed.\n", tests_run, tests_passed,
           tests_run - tests_passed);

    closedir(test_dir);

    return tests_run - tests_passed;
}
