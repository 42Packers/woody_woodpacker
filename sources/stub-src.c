/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stub-src.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouvel <plouvel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/04 13:25:09 by plouvel           #+#    #+#             */
/*   Updated: 2024/11/05 13:15:33 by plouvel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define PIC_RESOLVE(addr) (get_rip() - ((char *)&get_rip_label - (char *)addr))

static unsigned long new_entry_point __attribute__((section(".data"))) = 0x00; /* To be patched */
static unsigned long old_entry_point __attribute__((section(".data"))) = 0x00; /* To be patched */

unsigned long get_rip();
void do_virus();
long _write(int fd, const void *buf, unsigned long count);

extern unsigned long get_rip_label;

__attribute__((noreturn)) void volatile _start()
{
    unsigned long host_entry_point;

    asm volatile(
        "push rax\n"
        "push rbx\n"
        "push rcx\n"
        "push rdx\n"
        "push rsi\n"
        "push rdi\n"
        "push rbp\n"
        "push rsp\n"
        "push r8\n"
        "push r9\n"
        "push r10\n"
        "push r11\n"
        "push r12\n"
        "push r13\n"
        "push r14\n"
        "push r15\n"
        : : :);

    host_entry_point = PIC_RESOLVE(&_start) - new_entry_point + old_entry_point;

    do_virus();

    asm volatile(
        "pop r15\n"
        "pop r14\n"
        "pop r13\n"
        "pop r12\n"
        "pop r11\n"
        "pop r10\n"
        "pop r9\n"
        "pop r8\n"
        "pop rsp\n"
        "pop rbp\n"
        "pop rdi\n"
        "pop rsi\n"
        "pop rdx\n"
        "pop rcx\n"
        "pop rbx\n"
        "pop rax\n"
        "jmp %0" : : "g"(host_entry_point) :);

    __builtin_unreachable();
}

void do_virus()
{
    const char *woody = "....WOODY....\n";

    _write(1, woody, 15);
}

long _write(int fd, const void *buf, unsigned long count)
{
    long ret;

    asm volatile(
        "mov rax, 1\n"
        "mov rdi, %1\n"
        "mov rsi, %2\n"
        "mov rdx, %3\n"
        "syscall"
        : "=r"(ret)
        : "g"((unsigned long)fd), "g"((unsigned long)buf), "g"(count)
        : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory");

    return ret;
}

unsigned long get_rip()
{
    unsigned long rip;

    asm volatile(
        "call get_rip_label\n"
        ".global get_rip_label\n"
        "get_rip_label:\n"
        "pop rax\n"
        "mov %0, rax\n"
        : "=r"(rip)
        :
        : "rax");

    return rip;
}