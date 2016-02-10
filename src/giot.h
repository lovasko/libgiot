#ifndef GIOT_H
#define GIOT_H

#include <stdint.h>

#define GIOT_MAGIC    0x6107
#define GIOT_VERSION  1
#define GIOT_DESC_LEN 128

#define GIOT_MODE_EXACT 0
#define GIOT_MODE_BOOL  1

struct iotable_header {
	uint16_t magic;
	uint8_t version;
	uint8_t mode;
	uint8_t nin;
	uint8_t nout;
	uint8_t reserved[2];
	uint64_t nrows;
	char desc[GIOT_DESC_LEN];
};

#endif

