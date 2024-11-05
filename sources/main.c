#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <error.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <elf.h>
#include <stdio.h>
#include "stub.h"

int main(int argc, char **argv)
{
	uint8_t *fileptr = NULL;
	struct stat fs;
	int fd = 0;
	long page_size = sysconf(_SC_PAGE_SIZE);

	if (argc != 2)
	{
		error(1, 0, "file name missing");
	}

	if ((fd = open(argv[1], O_RDONLY)) < 0)
	{
		error(1, errno, "open");
	}
	if (fstat(fd, &fs) < 0)
	{
		error(1, errno, "fstat");
	}
	if ((fileptr = mmap(NULL, fs.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
	{
		error(1, errno, "mmap");
	}
	close(fd);

	Elf64_Ehdr *ehdr = fileptr;

	Elf64_Addr parasite_virtual_address = 0;

	size_t original_text_segment_file_size = 0;
	size_t end_of_text_segment = 0;

	for (size_t i = 0; i < ehdr->e_phnum; i++)
	{
		Elf64_Phdr *curr_phdr = fileptr + ehdr->e_phoff + (ehdr->e_phentsize * i);

		if (curr_phdr->p_type == PT_LOAD && curr_phdr->p_flags & PF_R && curr_phdr->p_flags & PF_X)
		{
			original_text_segment_file_size = curr_phdr->p_filesz;
			end_of_text_segment = curr_phdr->p_offset + curr_phdr->p_filesz;
			parasite_virtual_address = curr_phdr->p_vaddr + original_text_segment_file_size;

			curr_phdr->p_filesz += stub_bin_len;
			curr_phdr->p_memsz += stub_bin_len;

			for (size_t j = i + 1; j < ehdr->e_phnum; j++)
			{
				Elf64_Phdr *next_phdr = (Elf64_Phdr *)(fileptr + ehdr->e_phoff + (ehdr->e_phentsize * j));

				if (next_phdr->p_offset > curr_phdr->p_offset + original_text_segment_file_size)
				{
					next_phdr->p_offset += page_size;
				}
			}
			break;
		}
	}

	for (size_t i = 0; i < ehdr->e_shnum; i++)
	{
		Elf64_Shdr *curr_shdr = fileptr + ehdr->e_shoff + (ehdr->e_shentsize * i);

		if (curr_shdr->sh_addr > parasite_virtual_address)
		{
			curr_shdr->sh_offset += page_size;
		}
		else if (curr_shdr->sh_addr + curr_shdr->sh_size == parasite_virtual_address)
		{
			puts("Patched last section of the text segment");
			/* Increase the last section of the text segment so it contains the parasite code so it remaisn strip-safe. */
			curr_shdr->sh_size += stub_bin_len;
		}
		else if (curr_shdr->sh_addr == 0 && curr_shdr->sh_type != SHT_NULL)
		{
			curr_shdr->sh_offset += page_size;
		}
	}

	*(uint64_t *)(&stub_bin[0x1c4]) = (uint64_t)parasite_virtual_address;
	*(uint64_t *)(&stub_bin[0x1cc]) = (uint64_t)ehdr->e_entry;

	ehdr->e_shoff += page_size;
	ehdr->e_entry = parasite_virtual_address;

	int out_fd = 0;

	if ((out_fd = open("woody", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IXUSR | S_IWUSR)) < 0)
	{
		error(1, errno, "open");
	}

	write(out_fd, fileptr, end_of_text_segment);
	write(out_fd, stub_bin, stub_bin_len);
	lseek(out_fd, page_size - stub_bin_len, SEEK_CUR);
	fileptr += end_of_text_segment;
	write(out_fd, fileptr, fs.st_size - end_of_text_segment);
	close(out_fd);

	return (0);
}
