; To be compiled with : nasm -f elf64 woody.s -o woody.o && ld woody.o -znoseparate-code -s --gc-sections -o woody && strip --strip-all woody
; Then xxd -i -C woody > woody.c

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
%define MFD_CLOEXEC 0x0001

; Flags for execveat
%define AT_EMPTY_PATH 0x1000

; Flags for mmap
%define PROT_READ 0x01
%define PROT_WRITE 0x02
%define MAP_SHARED 0x01

; Misc
%define STDOUT 1
%define KEY_LENGTH_IN_BYTES 32

section .rodata
    message db "....WOODY....", 0Ah, 0
    message_len equ $ - message
    self_path db '/proc/self/exe', 0
    empty_str db 0

    woody_size dq 776; This is the size of the packer.
    payload_size dq 0xDEADBEEF000000FF; This is the size of the payload it will unpack. This will get replaced by the injector.

    key db KEY_LENGTH_IN_BYTES dup(0xDF) ; This is the key it will use.

section .text
    global _start

; R12 stores the file descriptor of the decryptor.
; R13 stores the file descriptor of the memfd.
; R14 stores the memory address of the memfd, and argc prior to execve.

_start:
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
    lea rdi, [rel empty_str]
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
    mov r10, MAP_SHARED
    mov r8, r13
    mov r9, 0
    syscall

    test rax, rax
    js _exit

    mov r14, rax

    mov rcx, 0
decrypt:
    cmp rcx, [rel payload_size]
    je end_decrypt

    mov rax, rcx
    mov rbx, KEY_LENGTH_IN_BYTES
    xor rdx, rdx
    div rbx
    mov ah, BYTE [key + rdx]
    mov al, BYTE [r14 + rcx]
    xor al, ah
    mov BYTE [r14 + rcx], al
    inc rcx
    jmp decrypt
end_decrypt:

    mov rax, SYS_WRITE
    mov rdi, STDOUT
    lea rsi, [rel message]
    mov rdx, message_len
    syscall

    mov r14, [rsp] ; argc

    ; Execve the memory file
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