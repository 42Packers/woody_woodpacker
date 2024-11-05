/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plouvel <plouvel@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/27 17:53:28 by plouvel           #+#    #+#             */
/*   Updated: 2024/11/05 13:57:48 by plouvel          ###   ########.fr       */
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

static uint8_t key[KEY_SIZE] = {0};

static void xor_cypher(uint8_t *data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		data[i] ^= key[i % KEY_SIZE];
}

static int fill_key(void)
{
	int fd = -1;

	if ((fd = open("/dev/urandom", O_RDONLY)) < 0)
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
	int ret = 1;
	struct stat st;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <file>\n", argv[0]);
		return (1);
	}
	if (fill_key() < 0)
	{
		fprintf(stderr, "Error: cannot generate random key.\n");
		return (1);
	}
	if ((out_fd = open(OUT_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0)
	{
		fprintf(stderr, "Error: cannot open '%s' for writing.\n", OUT_FILE);
		goto clean;
	}
	if ((in_fd = open(argv[1], O_RDONLY)) < 0)
	{
		fprintf(stderr, "Error: cannot open '%s' for reading.\n", argv[1]);
		goto clean;
	}
	if (fstat(in_fd, &st) < 0)
	{
		fprintf(stderr, "Error: cannot stat '%s'.\n", argv[1]);
		goto clean;
	}
	if (!S_ISREG(st.st_mode))
	{
		fprintf(stderr, "Error: '%s' is not a regular file.\n", argv[1]);
		goto clean;
	}
	if ((fptr = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, in_fd, 0)) == MAP_FAILED)
	{
		fprintf(stderr, "Error: cannot mmap '%s'.\n", argv[1]);
		goto clean;
	}

	close(in_fd);
	in_fd = -1;

	(*(uint64_t *)&WOODY[PAYLOAD_SIZE_OFFSET]) = st.st_size;
	for (size_t i = 0; i < KEY_SIZE; i++)
		WOODY[KEY_OFFSET + i] = key[i];

	xor_cypher(fptr, st.st_size);

	if (write(out_fd, WOODY, WOODY_LEN) != WOODY_LEN)
	{
		fprintf(stderr, "Error: cannot write to '%s'.\n", OUT_FILE);
		goto clean;
	}
	if (write(out_fd, fptr, st.st_size) != st.st_size)
	{
		fprintf(stderr, "Error: cannot write to '%s'.\n", OUT_FILE);
		goto clean;
	}

	print_key();

	ret = 0;
clean:
	munmap(fptr, st.st_size);
	close(in_fd);
	close(out_fd);

	return (ret);
}