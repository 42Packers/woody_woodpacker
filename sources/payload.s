%define WRITE_SYSNO 1
%define STDOUT_FILENO 1

payload:
    push rbp
    mov rbp, rsp
    sub rsp, 0x10

    mov DWORD [rbp - 4],  0x00000A2E ;    .
    mov DWORD [rbp - 8],  0x2E2E2E59 ; ...Y
    mov DWORD [rbp - 12], 0x444F4F57 ; DOOW
    mov DWORD [rbp - 16], 0x2E2E2E2E ; ....

    mov rax, WRITE_SYSNO
    mov rdi, STDOUT_FILENO
    lea rsi, [rbp - 16]
    mov rdx, 14
    syscall

    pop rbp

    mov rax, 0x0000000000000000 ; to be patched
    jmp rax