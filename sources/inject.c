/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   inject.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouvel <plouvel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/21 08:05:11 by plouvel           #+#    #+#             */
/*   Updated: 2024/10/21 09:30:35 by plouvel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <elf.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

unsigned char payload[] = {
    0x55, 0x48, 0x89, 0xe5, 0x48, 0x83, 0xec, 0x10, 0xc7, 0x45, 0xfc, 0x2e,
    0x0a, 0x00, 0x00, 0xc7, 0x45, 0xf8, 0x59, 0x2e, 0x2e, 0x2e, 0xc7, 0x45,
    0xf4, 0x57, 0x4f, 0x4f, 0x44, 0xc7, 0x45, 0xf0, 0x2e, 0x2e, 0x2e, 0x2e,
    0xb8, 0x01, 0x00, 0x00, 0x00, 0xbf, 0x01, 0x00, 0x00, 0x00, 0x48, 0x8d,
    0x75, 0xf0, 0xba, 0x0e, 0x00, 0x00, 0x00, 0x0f, 0x05, 0x5d, 0xb8, 0x00,
    0x00, 0x00, 0x00, 0xff, 0xe0};

static int write_new_binary(uint8_t *file, size_t filesz, size_t end_of_data, Elf64_Addr orig_entry);

int inject_payload(uint8_t *file, size_t filesz)
{
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)file;
    Elf64_Phdr *phdr = NULL;
    Elf64_Shdr *shdr = NULL;
    bool data_segment_found = false;
    size_t payloadsz = sizeof(payload);
    size_t end_of_data = 0;
    Elf64_Off orig_shoff = ehdr->e_shoff;
    Elf64_Addr orig_entry = ehdr->e_entry;

    ehdr->e_shoff += payloadsz;
    for (size_t i = 0; i < ehdr->e_phnum; i++)
    {
        phdr = (Elf64_Phdr *)(file + (ehdr->e_phoff + (ehdr->e_phentsize * i)));
        if (phdr->p_type == PT_LOAD && phdr->p_flags & PF_R && phdr->p_flags & PF_W)
        {
            ehdr->e_entry = phdr->p_vaddr + phdr->p_filesz;
            end_of_data = phdr->p_offset + phdr->p_filesz;
            phdr->p_filesz += payloadsz;
            phdr->p_memsz += payloadsz;
            data_segment_found = true;
            puts("Patched entry point and data segment.");
            break;
        }
    }
    if (!data_segment_found)
    {
        puts("Failed to find data segment.");
        return (1);
    }
    for (size_t i = 0; i < ehdr->e_shnum; i++)
    {
        // Find .bss section
        shdr = (Elf64_Shdr *)(file + (orig_shoff + (ehdr->e_shentsize * i)));
        if (shdr->sh_type == SHT_NOBITS)
        {
            shdr->sh_offset += payloadsz;
            shdr->sh_addr += payloadsz;
            puts("Patched .bss section.");
        }
        else if (shdr->sh_offset >= end_of_data)
        {
            shdr->sh_offset += payloadsz;
        }
    }
    phdr->p_flags |= PF_X;
    puts("Marked data segment as executable.");
    write_new_binary(file, filesz, end_of_data, orig_entry);
}

static int write_new_binary(uint8_t *file, size_t filesz, size_t end_of_data, Elf64_Addr orig_entry)
{
    int fd = open("./tmp", O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR);
    if (fd == -1)
    {
        puts("Failed to open output file.");
        return (1);
    }
    write(fd, file, end_of_data);
    uint32_t *addr = (uint32_t *)(payload + 59);
    *addr = orig_entry;
    write(fd, payload, sizeof(payload));
    write(fd, file + end_of_data, filesz - end_of_data);
    close(fd);
    return (0);
}