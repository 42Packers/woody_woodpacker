bits 64

%define SYS_WRITE 1
%define SYS_EXIT 60
%define SYS_LSEEK 8
%define SYS_SENDFILE 40
%define SYS_MEMFD_CREATE 319
%define EXECVE 59
%define STDOUT 1
%define SYS_OPEN 2

%define MFD_CLOEXEC 0x0001

section .rodata
    message db "....WOODY....", 0x0A, 0
    message_len equ $ - message
    self_path db '/proc/self/exe', 0
    woody db 'woody', 0
    file_size dq 9040

section .text
    global _start

_start:
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    lea rsi, [rel message]
    mov rdx, message_len
    syscall

    ; Open self
    mov rax, SYS_OPEN
    mov rdi, self_path
    xor rsi, rsi
    syscall

    test rax, rax
    mov rdi, 1
    js _exit

    mov r11, rax

    ; Seek to file size
    mov rax, SYS_LSEEK
    mov rdi, r11
    mov rsi, file_size
    mov rdx, 0
    syscall

    ; Open memfd
    mov rax, SYS_MEMFD_CREATE
    lea rdi, [rel woody]
    mov rsi, MFD_CLOEXEC
    syscall

    test rax, rax
    mov rdi, 1
    js _exit

    mov r12, rax

    ; Transfer file to memfd
    mov rax, SYS_SENDFILE
    mov rdi, r12
    mov rsi, r11
    mov rdx, 0x00
    mov r10, 30
    syscall

    ; Execve the memory file
    mov r

    test rax, rax
    mov rdi, 1
    js _exit

    mov r12, rax

    xor rdi, rdi
_exit:
    mov rax, SYS_EXIT
    syscall