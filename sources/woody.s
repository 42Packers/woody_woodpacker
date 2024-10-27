bits 64

; Syscall numbers
%define SYS_WRITE 1
%define SYS_EXIT 60
%define SYS_LSEEK 8
%define SYS_SENDFILE 40
%define SYS_MEMFD_CREATE 319
%define SYS_EXECVEAT 322
%define SYS_OPEN 2
%define SYS_MMAP 9

; Flags for memfd_create
%define MFD_CLOEXEC 0001h

; Flags for execveat
%define AT_EMPTY_PATH 1000h

; Flags for mmap
%define PROT_READ 1
%define PROT_WRITE 2
%define MAP_SHARED 1

; Misc
%define STDOUT 1

section .rodata
    message db "....WOODY....", 0Ah, 0
    message_len equ $ - message
    self_path db '/proc/self/exe', 0
    woody db 'woody', 0
    empty_str db 0

    woody_size dq 656; This is the size of the packer
    payload_size dq 0xDEADBEEF000000FF; This is the size of the payload it will unpack

    key dq 0xDEADBEEFDEADBEEF

section .text
    global _start

_start:
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    lea rsi, [rel message]
    mov rdx, message_len
    syscall

    ; Open self file
    mov rax, SYS_OPEN
    mov rdi, self_path
    xor rsi, rsi
    syscall

    test rax, rax
    js _exit

    mov r12, rax

    ; Seek to file size
    mov rax, SYS_LSEEK
    mov rdi, r12
    xor rsi, rsi
    mov rsi, [rel woody_size]
    mov rdx, 0
    syscall

    ; Open memfd
    mov rax, SYS_MEMFD_CREATE
    lea rdi, [rel woody]
    mov rsi, MFD_CLOEXEC
    syscall

    test rax, rax
    js _exit

    mov r13, rax

    ; Transfer file to the memfd
    mov rax, SYS_SENDFILE
    mov rdi, r13
    mov rsi, r12
    mov rdx, 0h
    mov r10, [rel payload_size]
    syscall

    ; Load the memfd into memory
    mov rax, SYS_MMAP
    mov rdi, 0x0
    mov rsi, [rel payload_size]
    mov rdx, PROT_READ | PROT_WRITE
    mov rcx, MAP_SHARED
    mov r8, r13
    mov r9, 0
    syscall

    test rax, rax
    js _exit

decrypt:
    mov rcx, 0
    cmp rcx, [rel payload_size]
    je end_decrypt
    mov rax, rcx ; ptr is gone!
    mov rbx, 8 ; divide by eight.
    xor rdx, rdx ; clean up rdx, remainder will be stored here.
    div rbx
    mov byte [rax + rcx],

end_decrypt:

    ; Execve the memory file

    mov r14, [rsp] ; argc

    mov rax, SYS_EXECVEAT
    mov rdi, r13
    mov rsi, empty_str
    lea rdx, [rsp + 8] ; argv
    lea r10, [rsp + 8 + (r14 * 8) + 8] ; envp
    mov r8, AT_EMPTY_PATH ; AT_EMPTY_PATH
    syscall

_exit:
    mov rax, SYS_EXIT
    mov rdi, 1
    syscall