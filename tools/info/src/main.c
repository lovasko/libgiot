#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <giot.h>

static char*
mode_name(uint8_t mode)
{
	if (mode == 0)
		return "exact";

	if (mode == 1)
		return "bool";

	return "unknown";
}

int
main(int argc, char* argv[])
{
	int fd;
	struct iotable_header header;

	if (argc != 2) {
		fprintf(stderr, "ERROR: expected one argument.\n");
		return EXIT_FAILURE;
	}

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("open");
		return EXIT_FAILURE;
	}

	if (read(fd, &header, sizeof(struct iotable_header)) != sizeof(struct iotable_header)) {
		fprintf(stderr, "ERROR: unable to read the header.\n");
		close(fd);
		return EXIT_FAILURE;
	}

	printf("Description: %s\n", header.desc);
	printf("    Version: %d\n", header.version);
	printf(" Match mode: %s\n", mode_name(header.mode));
	printf("      Input: %d\n", header.nin);
	printf("     Output: %d\n", header.nout);
	printf("  Row count: %lld\n", header.nrows);

	return EXIT_SUCCESS;
}

