/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouvel <plouvel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/27 17:53:28 by plouvel           #+#    #+#             */
/*   Updated: 2024/10/28 13:02:51 by plouvel          ###   ########.fr       */
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

// deadbeef key
static uint8_t key[KEY_SIZE] = {0};

void xor_cypher(uint8_t *data, size_t size)
{
    for (size_t i = 0; i < size; i++)
        data[i] ^= key[i % KEY_SIZE];
}

int fill_key(void)
{
    int fd = -1;

    if ((fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC)) < 0)
    {
        return (-1);
    }

    if (read(fd, &key, KEY_SIZE) < 0)
    {
        close(fd);
        return (-1);
    }

    close(fd);

    return (0);
}

void print_key(void)
{
    for (size_t i = 0; i < KEY_SIZE; i++)
        printf("%02X", key[i]);
    printf("\n");
}

int main(int argc, char **argv)
{
    uint8_t *fptr = NULL;
    int in_fd = -1;
    int out_fd = -1;
    struct stat st;

    if (argc != 2)
        return (1);
    if (fill_key() < 0)
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

    (*(uint64_t *)&WOODY[PAYLOAD_SIZE_OFFSET]) = st.st_size;
    for (size_t i = 0; i < KEY_SIZE; i++)
        WOODY[KEY_OFFSET + i] = key[i];

    /* Encrypt the file in memory. */
    xor_cypher(fptr, st.st_size);

    /* Write the decryptor to the out file. */
    if (write(out_fd, WOODY, WOODY_LEN) != WOODY_LEN)
        return (1);
    /* Write the encrypted file to the out file. */
    if (write(out_fd, fptr, st.st_size) != st.st_size)
        return (1);

    close(out_fd);

    print_key();
    printf("Patched binary written to %s\n", OUT_FILE);

    return (0);
}