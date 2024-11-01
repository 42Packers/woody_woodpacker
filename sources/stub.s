%define WRITE_SYSNO 1
%define STDOUT_FILENO 1

bits 64

enter 0x10, 0

mov DWORD [rbp - 4],  0x00000A2E ;    .
mov DWORD [rbp - 8],  0x2E2E2E59 ; ...Y
mov DWORD [rbp - 12], 0x444F4F57 ; DOOW
mov DWORD [rbp - 16], 0x2E2E2E2E ; ....

mov rax, WRITE_SYSNO
mov rdi, STDOUT_FILENO
lea rsi, [rbp - 16]
mov rdx, 14
syscall

leave

xor rdx, rdx

mov rax, 0x401040
jmp rax
