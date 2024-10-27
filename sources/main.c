/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouvel <plouvel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/27 17:53:28 by plouvel           #+#    #+#             */
/*   Updated: 2024/10/27 18:37:22 by plouvel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <stdint.h>
#include <stdio.h>
#include "woody.h"

#define OUT_FILE "woody"

uint8_t key[8] = {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x7A, 0x8B};

void xor_cypher(uint8_t *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        data[i] ^= key[i % 8];
}

int main(int argc, char **argv)
{
    uint8_t *fptr = NULL;
    int in_fd = -1;
    int out_fd = -1;
    struct stat st;

    if (argc != 2)
        return (1);

    /* Open the out and in file. */
    if ((out_fd = open(OUT_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0)
        return (1);
    if ((in_fd = open(argv[1], O_RDONLY)) < 0)
        return (1);

    if (fstat(in_fd, &st) < 0)
        return (1);

    /* Map the input file in memory. */
    if ((fptr = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, in_fd, 0)) == MAP_FAILED)
        return (1);
    close(in_fd);

    /* Encrypt the file in memory. */
    xor_cypher(fptr, st.st_size);

    (*(uint64_t *)&WOODY[PAYLOAD_SIZE_OFFSET]) = st.st_size;

    /* Write the decryptor to the out file. */
    if (write(out_fd, WOODY, WOODY_LEN) != WOODY_LEN)
        return (1);
    /* Write the encrypted file to the out file. */
    if (write(out_fd, fptr, st.st_size) != st.st_size)
        return (1);

    close(out_fd);

    printf("Patched binary written to %s\n", OUT_FILE);

    return (0);
}