/* Build me with:
 * $ as one.s -o one.o
 * $ ld -s -o one one.o
 * Disassible me with:
 * $ objdump -d --no-show-raw-insn one.o > one_dis.s
 */
.text
        .global _start # Entry point for the loader.

_start:
        movl    $1,%ebx # Exit code
        movl    $1,%eax # system call number (sys_exit)
        int     $0x80 # call kernel
