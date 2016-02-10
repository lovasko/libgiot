#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <giot.h>
#include <its.h>

#define HEX_FORMAT ITS_BASE_HEX
#define DEC_FORMAT ITS_BASE_DEC
#define OCT_FORMAT ITS_BASE_OCT
#define BIN_FORMAT ITS_BASE_BIN

static uint8_t* row_buffer;
static uint32_t format;
static int fd;
static struct iotable_header header;
static uint64_t offset;
static uint64_t count;

static void
print_byte(uint8_t byte)
{
	char* str;

	str = its(&byte, 8, ITS_UNSIGNED, format);
	printf("%s", str);
	free(str);
}

static void
print_row(void)
{
	uint8_t i;

	read(fd, row_buffer, header.nin + header.nout);

	for (i = 0; i < header.nin; i++) {
		print_byte(row_buffer[i]);
		printf(" ");
	}

	printf("-> ");

	for (i = 0; i < header.nout; i++) {
		print_byte(row_buffer[i+header.nin]);
		printf(" ");
	}

	printf("\n");
}

/**
 * Print the usage string to the stdout.
 */
static void
print_usage(void)
{
	printf("giotgen -- Gemer IO Table Listing\n"
	       "  -h                help message\n"
	       "  -c N              row count\n"
	       "  -f [hex|oct|bin]  number formatting (default: dec)\n"
				 "  -o N              row offset\n");
}

static void
parse_format(char* string)
{
	if (strcmp(string, "hex") == 0)
		format = HEX_FORMAT;
	else if (strcmp(string, "dec") == 0)
		format = DEC_FORMAT;
	else if (strcmp(string, "oct") == 0)
		format = OCT_FORMAT;
	else if (strcmp(string, "bin") == 0)
		format = BIN_FORMAT;
	else {
		fprintf(stderr, "ERROR: unknown format\n");
		exit(EXIT_FAILURE);
	}
}

static void
parse_positive_number(char* string, uint64_t*  out_number)
{
	int64_t number;

	errno = 0;
	number = strtoll(string, NULL, 10);
	if (number == 0 && errno != 0) {
		perror("strtoll");
		exit(EXIT_FAILURE);
	}

	if (number < 0) {
		fprintf(stderr, "ERROR: the parameter has to be greater than zero.\n");
		exit(EXIT_FAILURE);
	}

	*out_number = (uint64_t)number;
}

static void
parse_arguments(int argc, char* argv[])
{
	int option;

	while ((option = getopt(argc, argv, "hc:f:o:")) != -1) {
		switch(option) {
			case 'f': parse_format(optarg); break;
			case 'c': parse_positive_number(optarg, &count); break;
			case 'o': parse_positive_number(optarg, &offset); break;

			case 'h':
			case '?':
				print_usage();
				exit(EXIT_SUCCESS);

			default: 
				fprintf(stderr, "ERROR: unknown error while parsing arguments\n");
				exit(EXIT_FAILURE);
		}
	}
}

int
main(int argc, char* argv[])
{
	uint64_t i;

	offset = 0;
	count = INT64_MAX;
	parse_arguments(argc, argv);	

	if (optind >= argc) {
		fprintf(stderr, "ERROR: expected the IO table file.\n");
		return EXIT_FAILURE;
	}

	fd = open(argv[optind], O_RDONLY);
	if (fd < 0) {
		perror("open");
		return EXIT_FAILURE;
	}

	read(fd, &header, sizeof(struct iotable_header));
	row_buffer = malloc(header.nin + header.nout);

	lseek(fd, (off_t)sizeof(struct iotable_header) +
	          offset * (header.nin + header.nout), SEEK_SET);

	for (i = 0; i < count; i++)
		print_row();

	return EXIT_SUCCESS;
}

