
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int inject_payload(void *file, size_t filesz);

int main(int argc, char **argv)
{
	size_t file_size;
	struct stat st;
	int fd;
	void *file;

	file_size = 0;

	if (stat(argv[1], &st) != 0)
	{
		return (1);
	}
	if ((fd = open(argv[1], O_RDONLY)) == -1)
	{
		return (1);
	}
	file_size = st.st_size;
	if ((file = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
	{
		return (1);
	}

	inject_payload(file, file_size);

	(void)argc;
	(void)argv;
	return (0);
}
