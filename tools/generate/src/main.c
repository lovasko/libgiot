#include <sys/endian.h>

#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <giot.h>
#include <m_intcombo.h>

static struct iotable_header header;
static char* fn_name;
static char* source_path;

/**
 * Type of the user-supplied function that transforms the generated input
 * vector to the output vector.
 *
 * Note that the first argument is an array of input bytes and the second
 * argument is a pointer to the similar output array.
 */
typedef void (*fn_t)(uint8_t*, uint8_t**);

static void 
generate(void)
{
	uint8_t i;
	intmax_t* next;
	uint8_t* in;
	uint8_t* out;
	void* source;
	fn_t fn;
	struct m_intcombo ic;

	source = dlopen(source_path, RTLD_NOW);
	if (source == NULL) {
		fprintf(stderr, "dlopen: %s\n", dlerror());
		return;
	}

	fn = (fn_t)dlfunc(source, fn_name);
	if (fn == NULL) {
		fprintf(stderr, "dlfunc: %s\n", dlerror());
		return;
	}

	header.magic = GIOT_MAGIC;
	header.version = GIOT_VERSION;
	header.nrows = (1 << (header.nin * 8));
	write(STDOUT_FILENO, &header, sizeof(struct iotable_header));

	m_intcombo_init(&ic);
	for (i = 0; i < header.nin; i++)
		m_intcombo_add(&ic, 0, 255);
	m_intcombo_finalize(&ic, &next);

	in = malloc(header.nin * sizeof(uint8_t));
	out = malloc(header.nout * sizeof(uint8_t));

	while (m_intcombo_next(&ic) == M_INTCOMBO_OK) {
		for (i = 0; i < header.nin; i++)
			in[i] = (uint8_t)next[i];

		write(STDOUT_FILENO, in, header.nin);
		fn(in, &out);
		write(STDOUT_FILENO, out, header.nout);
	}

	free(in);
	free(out);
	m_intcombo_free(&ic);
	dlclose(source);
}

/**
 * Determine the lesser value of two size_t variables.
 */
static size_t
size_t_min(size_t a, size_t b)
{
	return (a < b) ? a : b;
}

/**
 * Print the usage string to the stdout.
 */
static void
print_usage(void)
{
	printf("giotgen -- Gemer IO Table Generator\n"
	       "  -d string         description (max %d characters)\n"
				 "  -f string         function name\n"  
				 "  -i N              number of input columns\n"
				 "  -m [exact|bool]   operation mode\n"
				 "  -o N              number of output columns\n", GIOT_DESC_LEN);
}

static void
parse_column_count(char* string, char* type, uint8_t* out)
{
	long number;

	errno = 0;
	number = strtol(string, NULL, 10);

	if (errno != 0) {
		perror("strtol");
		exit(EXIT_FAILURE);
	}

	if (number < 1) {
		fprintf(stderr, "ERROR: %s column count must be more than 0.\n", type);
		exit(EXIT_FAILURE);
	}

	if (number > 256) {
		fprintf(stderr, "ERROR: %s columnt count has to be less than 257.\n", type);
		exit(EXIT_FAILURE);
	}

	*out = (uint8_t)number;
}

static void
parse_mode(char* string)
{
	if (strcmp(string, "exact") == 0)
		header.mode = GIOT_MODE_EXACT;
	else if (strcmp(string, "bool") == 0)
		header.mode = GIOT_MODE_BOOL;
	else {
		fprintf(stderr, "ERROR: mode has to be either 'exact' or 'bool'.\n");
		exit(EXIT_FAILURE);
	}
}

static void
parse_arguments(int argc, char* argv[])
{
	int option;
	char desc[GIOT_DESC_LEN];

	while ((option = getopt(argc, argv, "d:f:hi:m:n:o:")) != -1) {
		switch(option) {
			case 'd':
				memset(desc, '\0', GIOT_DESC_LEN);
				memcpy(desc, optarg, size_t_min(GIOT_DESC_LEN, strlen(optarg)));
				memcpy(header.desc, desc, GIOT_DESC_LEN);
			break;

			case 'f':
				fn_name = strdup(optarg);
				asprintf(&source_path, "./%s.so", fn_name);
			break;

			case 'i': parse_column_count(optarg, "input", &header.nin); break;
			case 'o': parse_column_count(optarg, "output", &header.nout); break;

			case 'm': parse_mode(optarg); break;

			case 'h':
			case '?':
				print_usage();
				exit(EXIT_FAILURE);

			default: 
				fprintf(stderr, "ERROR: unknown error while parsing arguments\n");
				exit(EXIT_FAILURE);
		}
	}
}

/**
 * Gemer IO Table generator that executes a user-supplied shared object
 * function.
 */
int
main(int argc, char* argv[])
{
	memset(&header, 0, sizeof(struct iotable_header));
	parse_arguments(argc, argv);
	generate();

	return EXIT_SUCCESS;
}

