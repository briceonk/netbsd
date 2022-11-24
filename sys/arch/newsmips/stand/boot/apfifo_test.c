#include <stdint.h>
#include <lib/libsa/stand.h>
#include <machine/adrsmap.h>
#include <machine/apfifo.h>
#include "apfifo_test.h"

#pragma region Logging
static int LOG_LEVEL = 0;
#define TRACE 0
#define INFO 1
#define ERROR 2

static int tap = 0;
#define LOGRESULT(x) {               \
	tap++;                           \
	if (!x)                          \
	{                                \
		log(INFO, "not ");              \
	}                                \
	log(INFO, "ok %d - %s\n", tap, #x); \
}

void
log(int level, const char * format, ...)
{
	if (level >= LOG_LEVEL)
	{
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
	}
}

void
dump_apfifo_channel(int log_level, struct fifo_channel *fifo_ch)
{
	log(log_level, "FIFO configuration dump start ---\n");
	log(log_level, "fifo_mask = 0x%x\n", fifo_ch->size);
	log(log_level, "fifo_addr = 0x%x\n", fifo_ch->address);
	log(log_level, "fifo_intc = 0x%x\n", fifo_ch->intclr);
	log(log_level, "fifo_dmam = 0x%x\n", fifo_ch->dma_mode);
	log(log_level, "fifo_wait = 0x%x\n", fifo_ch->unknown0);
	log(log_level, "fifo_drqc = 0x%x\n", fifo_ch->unknown1);
	log(log_level, "fifo_ictl = 0x%x\n", fifo_ch->intctrl);
	log(log_level, "fifo_ista = 0x%x\n", fifo_ch->intstat);
	log(log_level, "fifo_wmrk = 0x%x\n", fifo_ch->unknown2);
	log(log_level, "fifo_dcnt = 0x%x\n", fifo_ch->unknown3);
	log(log_level, "fifo_dptr = 0x%x\n", fifo_ch->dma_pointer);
	log(log_level, "fifo_cptr = 0x%x\n", fifo_ch->register_pointer);
	log(log_level, "fifo_vcnt = 0x%x\n", fifo_ch->count);
	log(log_level, "fifo_data = 0x%x\n", fifo_ch->data);
	log(log_level, "FIFO configuration dump end ---\n");
}

void
log_intstat(int log_level)
{
	log(log_level, "INTST0 = 0x%x INTST1 = 0x%x INTST2 = 0x%x INTST3 = 0x%x INTST4 = 0x%x INTST5 = 0x%x\n", *((uint32_t*)NEWS5000_INTST0), *((uint32_t*)NEWS5000_INTST1), *((uint32_t*)NEWS5000_INTST2), *((uint32_t*)NEWS5000_INTST3), *((uint32_t*)NEWS5000_INTST4), *((uint32_t*)NEWS5000_INTST5));
}
#pragma endregion Logging

#pragma region Utility functions
void
intclr()
{
	*((uint32_t*)NEWS5000_INTCLR0) = 0xffffffff;
	*((uint32_t*)NEWS5000_INTCLR1) = 0xffffffff;
	*((uint32_t*)NEWS5000_INTCLR2) = 0xffffffff;
	*((uint32_t*)NEWS5000_INTCLR3) = 0xffffffff;
	*((uint32_t*)NEWS5000_INTCLR4) = 0xffffffff;
	*((uint32_t*)NEWS5000_INTCLR5) = 0xffffffff;
}

void
init_channel(struct fifo_channel *fifo_ch, uint32_t address, uint32_t size)
{
	fifo_ch->size = size;
	fifo_ch->address = address;
}

void 
usleep(uint32_t microseconds)
{
	uint32_t start = get_ustime();
	while (get_ustime() < start + microseconds) {}
}
#pragma endregion Utility functions

#pragma region FIFO CPU-side access tests
int
apfifo_byte_access_test(struct fifo_channel *fifo_ch, volatile uint8_t *data_ptr)
{
	int ok = 1;
	int i = 0;
	uint32_t cptr;
	uint32_t vcnt;
	uint8_t buf;
	uint8_t data[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	// Set pointer to beginning of the FIFO block
	fifo_ch->register_pointer = 0;
	vcnt = fifo_ch->count;

	// Read in the values we should get from the beginning of the fifo by reading from memory directly
	for (i = 0; i < 32; i++)
	{
		cptr = fifo_ch->register_pointer;
		data[i] = APFIFO0_BUF_8(i);
		buf = *(data_ptr + (i % 4));
		log(INFO, " data[%d] = 0x%x fifo_data = 0x%x %s\n", i, data[i], buf, buf == data[i] ? "PASS" : "FAIL");
		if(buf != data[i])
		{
			ok = 0;
		}

		if (fifo_ch->count != vcnt - 1)
		{
			log(INFO, " Count error! Expected 0x%x, got 0x%x!\n", vcnt - 1, fifo_ch->count);
			ok = 0;
		}
		vcnt -= 1;

		if (fifo_ch->register_pointer != (cptr + 1))
		{
			log(INFO, " Register pointer error! Expected 0x%x, got 0x%x!\n", cptr + 1, fifo_ch->register_pointer);
			ok = 0;
		}
	}

	return ok;
}

int
apfifo_halfword_access_test(struct fifo_channel *fifo_ch, volatile uint16_t *data_ptr)
{
	int ok = 1;
	int i = 0;
	uint32_t cptr;
	uint32_t vcnt;
	uint16_t buf;
	uint16_t data[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	// Set pointer to beginning of the FIFO block
	fifo_ch->register_pointer = 0;
	vcnt = fifo_ch->count;

	// Read in the values we should get from the beginning of the fifo by reading from memory directly
	for (i = 0; i < 16; i++)
	{
		cptr = fifo_ch->register_pointer;
		data[i] = APFIFO0_BUF_16(i);
		buf = *(data_ptr + (i % 2));
		log(INFO, " data[%d] = 0x%x fifo_data = 0x%x %s\n", i, data[i], buf, buf == data[i] ? "PASS" : "FAIL");
		if(buf != data[i])
		{
			ok = 0;
		}

		if (fifo_ch->count != vcnt - 2)
		{
			log(INFO, " Count error! Expected 0x%x, got 0x%x!\n", vcnt - 2, fifo_ch->count);
			ok = 0;
		}
		vcnt -= 2;

		if (fifo_ch->register_pointer != (cptr + 2))
		{
			log(INFO, " Register pointer error! Expected 0x%x, got 0x%x!\n", cptr + 2, fifo_ch->register_pointer);
			ok = 0;
		}
	}

	return ok;
}

int
apfifo_word_access_test(struct fifo_channel *fifo_ch)
{
	int ok = 1;
	int i = 0;
	uint32_t vcnt;
	uint32_t buf;
	uint32_t cptr;
	uint32_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	// Set pointer to beginning of the FIFO block
	fifo_ch->register_pointer = 0;
	vcnt = fifo_ch->count;

	// Read in the values we should get from the beginning of the fifo by reading from memory directly
	for (i = 0; i < 8; i++)
	{
		cptr = fifo_ch->register_pointer;
		data[i] = APFIFO0_BUF_32(i);
		buf = fifo_ch->data;
		log(INFO, " data[%d] = 0x%x fifo_data = 0x%x %s\n", i, data[i], buf, buf == data[i] ? "PASS" : "FAIL");
		if(buf != data[i])
		{
			ok = 0;
		}

		if (fifo_ch->count != vcnt - 4)
		{
			log(INFO, " Count error! Expected 0x%x, got 0x%x!\n", vcnt - 4, fifo_ch->count);
			ok = 0;
		}
		vcnt -= 4;

		if (fifo_ch->register_pointer != (cptr + 4))
		{
			log(INFO, " Register pointer error! Expected 0x%x, got 0x%x!\n", cptr + 4, fifo_ch->register_pointer);
			ok = 0;
		}
	}

	return ok;
}

int
apfifo_ooo_access_test(struct fifo_channel *fifo_ch, volatile uint8_t *data_ptr)
{
	int ok = 1;
	uint32_t word = 0;
	uint32_t assembled_word = 0;

	fifo_ch->register_pointer = 0;
	word = fifo_ch->data;

	fifo_ch->register_pointer = 0;
	assembled_word = *(data_ptr + 3) | (*(data_ptr + 2) << 8) | (*(data_ptr + 1) << 16) | (*(data_ptr + 0) << 24);
	log(INFO, " 32-bit access = 0x%x 4x8-bit access = 0x%x %s\n", word, assembled_word, word == assembled_word ? "PASS" : "FAIL");
	if (word != assembled_word)
	{
		ok = 0;
	}

	fifo_ch->register_pointer = 0;
	assembled_word = (*(data_ptr + 0) << 24) | (*(data_ptr + 1) << 16) | (*(data_ptr + 2) << 8) | *(data_ptr + 3);
	log(INFO, " 32-bit access = 0x%x 4x8-bit access = 0x%x %s\n", word, assembled_word, word == assembled_word ? "PASS" : "FAIL");
	if (word != assembled_word)
	{
		ok = 0;
	}

	fifo_ch->register_pointer = 0;
	assembled_word = *(data_ptr + 3) | (*(data_ptr + 1) << 16) | (*(data_ptr + 2) << 8) | (*(data_ptr + 0) << 24);
	log(INFO, " 32-bit access = 0x%x 4x8-bit access = 0x%x %s\n", word, assembled_word, word == assembled_word ? "PASS" : "FAIL");
	if (word != assembled_word)
	{
		ok = 0;
	}

	fifo_ch->register_pointer = 0;
	assembled_word = (*(data_ptr + 1) << 16) | (*(data_ptr + 0) << 24) | *(data_ptr + 3) | (*(data_ptr + 2) << 8);
	log(INFO, " 32-bit access = 0x%x 4x8-bit access = 0x%x %s\n", word, assembled_word, word == assembled_word ? "PASS" : "FAIL");
	if (word != assembled_word)
	{
		ok = 0;
	}

	return ok;
}

int
apfifo_partial_word_test(struct fifo_channel *fifo_ch, volatile uint8_t *data_ptr)
{
	#define REGPTR_ERR(expected, actual) log(INFO, " Register pointer error! Expected 0x%x, got 0x%x!\n", expected, actual)
	int ok = 1;
	uint32_t initial = 0;
	uint32_t final = 0;

	fifo_ch->register_pointer = 0;
	fifo_ch->data = 0xabbacaab;
	fifo_ch->register_pointer = 0;
	initial = fifo_ch->data;

	fifo_ch->register_pointer = 0;
	final = *data_ptr;
	if (fifo_ch->register_pointer != 1)
	{
		REGPTR_ERR(1, fifo_ch->register_pointer);
		ok = 0;
	}
	final = fifo_ch->data;
	log(INFO, " initial = 0x%x final = 0x%x %s\n", initial, final, initial == final ? "PASS" : "FAIL");
	if (initial != final)
	{
		ok = 0;
	}
	if (fifo_ch->register_pointer != 5)
	{
		REGPTR_ERR(5, fifo_ch->register_pointer);
		ok = 0;
	}

	fifo_ch->register_pointer = 0;
	final = *data_ptr;
	final = *data_ptr;
	if (fifo_ch->register_pointer != 2)
	{
		REGPTR_ERR(2, fifo_ch->register_pointer);
		ok = 0;
	}
	final = fifo_ch->data;
	log(INFO, " initial = 0x%x final = 0x%x %s\n", initial, final, initial == final ? "PASS" : "FAIL");
	if (initial != final)
	{
		ok = 0;
	}
	if (fifo_ch->register_pointer != 6)
	{
		REGPTR_ERR(6, fifo_ch->register_pointer);
		ok = 0;
	}

	fifo_ch->register_pointer = 0;
	final = *data_ptr;
	final = *data_ptr;
	final = *data_ptr;
	if (fifo_ch->register_pointer != 3)
	{
		REGPTR_ERR(3, fifo_ch->register_pointer);
		ok = 0;
	}
	final = fifo_ch->data;
	log(INFO, " initial = 0x%x final = 0x%x %s\n", initial, final, initial == final ? "PASS" : "FAIL");
	if (initial != final)
	{
		ok = 0;
	}
	if (fifo_ch->register_pointer != 7)
	{
		REGPTR_ERR(7, fifo_ch->register_pointer);
		ok = 0;
	}

	fifo_ch->register_pointer = 0;
	final = *data_ptr;
	final = *data_ptr;
	final = *data_ptr;
	final = *data_ptr;
	if (fifo_ch->register_pointer != 4)
	{
		REGPTR_ERR(4, fifo_ch->register_pointer);
		ok = 0;
	}
	final = fifo_ch->data;
	log(INFO, " initial = 0x%x final = 0x%x %s\n", initial, final, initial != final ? "PASS" : "FAIL"); // XXX right?
	if (initial == final)
	{
		ok = 0;
	}
	if (fifo_ch->register_pointer != 8)
	{
		REGPTR_ERR(8, fifo_ch->register_pointer);
		ok = 0;
	}

	fifo_ch->register_pointer = 0;
	final = *(data_ptr + 2);
	final = *(data_ptr + 1);
	final = *(data_ptr + 3);
	if (fifo_ch->register_pointer != 3)
	{
		REGPTR_ERR(3, fifo_ch->register_pointer);
		ok = 0;
	}
	final = fifo_ch->data;
	log(INFO, " initial = 0x%x final = 0x%x %s\n", initial, final, initial == final ? "PASS" : "FAIL");
	if (initial != final)
	{
		ok = 0;
	}
	if (fifo_ch->register_pointer != 7)
	{
		REGPTR_ERR(7, fifo_ch->register_pointer);
		ok = 0;
	}

	return ok;
}

int
apfifo_cnt_cptr_test(struct fifo_channel *fifo_ch, uint32_t size)
{
	int ok = 1;

	fifo_ch->register_pointer = fifo_ch->dma_pointer - 0x8;
	fifo_ch->data;
	fifo_ch->data;
	if(fifo_ch->register_pointer != fifo_ch->dma_pointer || fifo_ch->count != 0)
	{
		log(INFO, " adv to 0 failed! register_pointer = 0x%x dma_pointer = 0x%x count = 0x%x\n", fifo_ch->register_pointer, fifo_ch->dma_pointer, fifo_ch->count);
		ok = 0;
	}

	fifo_ch->data;
	fifo_ch->data;
	if(fifo_ch->register_pointer == fifo_ch->dma_pointer || fifo_ch->count != fifo_ch->size - 0x7)
	{
		log(INFO, " adv past 0 failed! register_pointer = 0x%x dma_pointer = 0x%x count = 0x%x\n", fifo_ch->register_pointer, fifo_ch->dma_pointer, fifo_ch->count);
		ok = 0;
	}

	// force overflow (wraps register_pointer around to 0)
	fifo_ch->register_pointer = size - 0x7;
	fifo_ch->data;
	fifo_ch->data;
	if(fifo_ch->register_pointer != 0)
	{
		log(INFO, " register pointer did not overflow to 0! Actual = 0x%x\n", fifo_ch->register_pointer);
		ok = 0;
	}

	return ok;
}
#pragma endregion FIFO CPU-side access tests

#pragma region FIFO write tests
int
apfifo_word_write_test(struct fifo_channel *fifo_ch, uint32_t size)
{
	log(INFO, " FAIL - not complete\n");
	int ok = 1;
	uint32_t buf;
	
	// Basic write test (aligned)
	fifo_ch->register_pointer = 0;
	fifo_ch->data = 0xdecaf;
	fifo_ch->register_pointer = 0;
	buf = fifo_ch->data;
	log(INFO, "buf = 0x%x %s\n", buf, buf == 0xdecaf ? "PASS" : "FAIL");
	
	// Basic write test (non-aligned, what exactly happens here?)
	fifo_ch->register_pointer = 0;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	fifo_ch->register_pointer = 1;
	fifo_ch->data = 0xdeadface;
	fifo_ch->register_pointer = 0;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);

	fifo_ch->register_pointer = 0;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	fifo_ch->register_pointer = 2;
	fifo_ch->data = 0xdeadface;
	fifo_ch->register_pointer = 0;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);

	fifo_ch->register_pointer = 0;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	fifo_ch->register_pointer = 3;
	fifo_ch->data = 0xdeadface;
	fifo_ch->register_pointer = 0;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);

	// Write at overflow boundary (aligned)
	fifo_ch->register_pointer = size - 3;
	fifo_ch->data = 0xdecaf;
	fifo_ch->data = 0xc0ffee;
	fifo_ch->register_pointer = size - 3;
	buf = fifo_ch->data;
	log(INFO, "buf = 0x%x cptr = 0x%x cnt = 0x%x %s\n", buf, fifo_ch->register_pointer, fifo_ch->count, buf == 0xdecaf ? "PASS" : "FAIL");
	if (buf != 0xdecaf)
	{
		ok = 0;
	}

	buf = fifo_ch->data;
	log(INFO, "buf = 0x%x cptr = 0x%x cnt = 0x%x %s\n", buf, fifo_ch->register_pointer, fifo_ch->count, buf == 0xc0ffee ? "PASS" : "FAIL");
	if (buf != 0xdecaf)
	{
		ok = 0;
	}

	// TODO: Write at overflow boundary (non-aligned)
	fifo_ch->register_pointer = size - 3;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	fifo_ch->register_pointer = size - 2;
	fifo_ch->data = 0xdeadface;
	fifo_ch->register_pointer = size - 3;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);

	fifo_ch->register_pointer = size - 3;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	fifo_ch->register_pointer = size - 1;
	fifo_ch->data = 0xfacebabe;
	fifo_ch->register_pointer = size - 3;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);

	fifo_ch->register_pointer = size - 3;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	fifo_ch->register_pointer = size;
	fifo_ch->data = 0xbeefdead;
	fifo_ch->register_pointer = size - 3;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);

	fifo_ch->register_pointer = size - 3;
	fifo_ch->data = 0xebfeadde;
	fifo_ch->register_pointer = size - 3;
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);
	log(INFO, "fifo[0x%x] = 0x%x\n", fifo_ch->register_pointer, fifo_ch->data);

	ok = 0;
	return ok;
}

int
apfifo_halfword_write_test(struct fifo_channel *fifo_ch, volatile uint16_t *data_ptr, uint32_t size)
{
	int ok = 1;
	uint16_t buf;
	uint32_t lbuf;

	// Basic write test (aligned)
	fifo_ch->register_pointer = 0;
	*data_ptr = 0xabba;
	fifo_ch->register_pointer = 0;
	buf = *data_ptr;
	log(INFO, "buf = 0x%x %s\n", buf, buf == 0xabba ? "PASS" : "FAIL");
	if(buf != 0xabba)
	{
		ok = 0;
	}

	fifo_ch->register_pointer = 2;
	*(data_ptr + 1) = 0xb00b;
	fifo_ch->register_pointer = 2;
	buf = *(data_ptr + 1);
	log(INFO, "buf = 0x%x %s\n", buf, buf == 0xb00b ? "PASS" : "FAIL");
	if(buf != 0xb00b)
	{
		ok = 0;
	}

	// Like byte accesses, writing to the wrong slot will yield 0xffff
	fifo_ch->register_pointer = size - 1;
	*data_ptr = 0x9ae7;
	fifo_ch->register_pointer = size - 1;
	buf = *(data_ptr + 1);
	if(buf != 0xffff)
	{
		log(INFO, " Unexpected bad halfword 0x%x\n", buf);
		ok = 0;
	}

	// Write to the correct slot
	fifo_ch->register_pointer = size - 1;
	*(data_ptr + 1) = 0x3c7e;
	fifo_ch->register_pointer = size - 1;
	buf = *(data_ptr + 1);
	if(buf != 0x3c7e)
	{
		log(INFO, " Unexpected good halfword 0x%x\n", buf);
		ok = 0;
	}

	// Write a full word as halfwords, then readback
	fifo_ch->register_pointer = size - 0x3;
	*data_ptr = 0xbcab;
	*(data_ptr + 1) = 0x3412;
	fifo_ch->register_pointer = size - 0x3;
	lbuf = fifo_ch->data;
	if (lbuf != 0xbcab3412)
	{
		log(INFO, " Halfword write sequence 1 failed! Got 0x%x\n", lbuf);
		ok = 0;
	}

	// Writing to the same halfword address should break
	fifo_ch->register_pointer = size - 0x3;
	*data_ptr = 0xabcd;
	*data_ptr = 0x9472;
	fifo_ch->register_pointer = size - 0x3;
	lbuf = fifo_ch->data;
	if (lbuf != 0xabcdffff)
	{
		log(INFO, " Halfword write sequence 2 failed! Got 0x%x\n", lbuf);
		ok = 0;
	}

	// Misaligned writes are ignored
	fifo_ch->register_pointer = size - 0x3;
	fifo_ch->data = 0x0;
	fifo_ch->register_pointer = size - 0x2;
	*data_ptr = 0x4321;
	fifo_ch->register_pointer = size - 0x3;
	lbuf = fifo_ch->data;
	if(lbuf != 0x210000) // part of the data is in the valid area
	{
		log(INFO, " Misaligned halfword write test 1 failed! Got 0x%x\n", lbuf);
		ok = 0;
	}

	fifo_ch->register_pointer = size - 0x3;
	fifo_ch->data = 0x0;
	fifo_ch->register_pointer = size;
	*data_ptr = 0x4321;
	fifo_ch->register_pointer = size - 0x3;
	lbuf = fifo_ch->data;
	if(lbuf != 0x0)
	{
		log(INFO, " Misaligned halfword write test 2 failed! Got 0x%x\n", lbuf);
		ok = 0;
	}

	return ok;
}

int
apfifo_byte_write_test(struct fifo_channel *fifo_ch, volatile uint8_t *data_ptr, uint32_t size)
{
	int ok = 1;
	uint8_t buf;
	uint32_t lbuf;

	// Write a byte and read it back
	fifo_ch->register_pointer = 0;
	*data_ptr = 0xfb;
	fifo_ch->register_pointer = 0;
	buf = *data_ptr;
	log(INFO, "buf = 0x%x %s\n", buf, buf == 0xfb ? "PASS" : "FAIL");
	if(buf != 0xfb)
	{
		ok = 0;
	}

	// Writing a byte to the wrong "spot" for the count will cause 0xff to be written
	fifo_ch->register_pointer = size;
	*data_ptr = 0xbf;
	fifo_ch->register_pointer = size;
	buf = *(data_ptr + 3);
	fifo_ch->register_pointer = size -0x3;
	lbuf = fifo_ch->data;
	if (buf != 0xff)
	{
		log(INFO, " Unexpected bad byte 0x%x!\n", buf);
		ok = 0;
	}
	else if (lbuf != 0xff)
	{
		log(INFO, " Unexpected bad word readback 0x%x!\n", lbuf);
		ok = 0;
	}

	// Fixing the alignment should let us write the correct byte
	fifo_ch->register_pointer = size;
	*(data_ptr + 3) = 0xbf;
	fifo_ch->register_pointer = size;
	buf = *(data_ptr + 3);
	if (buf != 0xbf)
	{
		log(INFO, " Unexpected good byte 0x%x!\n", buf);
		ok = 0;
	}

	// Write a full word as bytes, then readback
	fifo_ch->register_pointer = size - 0x3;
	*data_ptr = 0xab;
	*(data_ptr + 1) = 0xbc;
	*(data_ptr + 2) = 0x12;
	*(data_ptr + 3) = 0x34;
	fifo_ch->register_pointer = size - 0x3;
	lbuf = fifo_ch->data;
	if (lbuf != 0xabbc1234)
	{
		log(INFO, " Byte write sequence 1 failed! Got 0x%x\n", lbuf);
		ok = 0;
	}

	// Writing to the same byte address should break
	fifo_ch->register_pointer = size - 0x3;
	*data_ptr = 0xab;
	*data_ptr = 0xbc;
	*data_ptr = 0x12;
	*data_ptr = 0x34;
	fifo_ch->register_pointer = size - 0x3;
	lbuf = fifo_ch->data;
	if (lbuf != 0xabff0000)
	{
		log(INFO, " Byte write sequence 2 failed! Got 0x%x\n", lbuf);
		ok = 0;
	}

	return ok;
}
#pragma endregion FIFO write tests

#pragma region FIFO configuration tests
int
apfifo_reconfigure_test(struct fifo_channel *fifo_ch)
{
	int ok = 1;
	uint32_t buf;

	fifo_ch->size = 0x7fff;
	fifo_ch->address = 0x0;
	fifo_ch->register_pointer = 0;
	fifo_ch->data = 0xca5cade;
	fifo_ch->register_pointer = 0;
	buf = fifo_ch->data;
	log(INFO, "buf = 0x%x\n", buf);
	ok = buf == 0xca5cade;

	fifo_ch->size = 0x4fff;
	fifo_ch->address = 0x3000;
	fifo_ch->register_pointer = 0;
	fifo_ch->data = 0xca5cade;
	fifo_ch->register_pointer = 0;
	buf = fifo_ch->data;
	log(INFO, "buf = 0x%x\n", buf);
	if(ok)
	{
		ok = buf == 0xca5cade;
	}

	// TODO: double-check overflow in this condition
	// TODO: size that doesn't end in f?

	return ok;
}
#pragma endregion FIFO configuration tests

#pragma region FIFO interrupt tests
uint32_t calculate_delay_count_interval(struct fifo_channel *fifo_ch, uint32_t count)
{
	fifo_ch->register_pointer = fifo_ch->dma_pointer;
	fifo_ch->unknown3 = count;
	uint32_t start_time = get_ustime();
	fifo_ch->register_pointer = fifo_ch->dma_pointer - 8;
	while (fifo_ch->intstat != 0x1c) {}
	uint32_t run_time = get_ustime() - start_time;
	uint32_t tick_rate = run_time / count;
	log(INFO, "  %u/%u = %u us per tick when starting from 0x%x\n", run_time, count, tick_rate, count);
	return tick_rate;
}

int
apfifo_delay_interrupt_test(struct fifo_channel *fifo_ch)
{
	int i;
	uint32_t inst, temp;
	int ok = 1;

	fifo_ch->register_pointer = 0x100;
	fifo_ch->dma_pointer = 0x100;

	log_intstat(INFO);
	intclr();
	*((uint32_t*)NEWS5000_INTEN0) = 0xffffffff; // enable all lvl0 interrupts - TODO: does INTST still get set if this is all 0?
	log_intstat(INFO);

	log(INFO, " Check platform-level interrupt\n"); // TODO: see if FIFO INTCLR reg de-asserts platform int rather than internal int register
	fifo_ch->intctrl = 0x0;
	fifo_ch->register_pointer = fifo_ch->dma_pointer - 8;
	if (fifo_ch->intstat != 0x18)
	{
		log(INFO, "  Interrupt error! Time delay interrupt was not set! INTST = 0x%x\n", fifo_ch->intstat);
		ok = 0;
	}
	else if (*((uint32_t*)NEWS5000_INTST0) != 0)
	{
		log(INFO, "  Interrupt error! INTST0 was set! 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
		ok = 0;
	}

	fifo_ch->intctrl = 0x4; // Enable time delay interrupt - todo: test with this disabled
	if (fifo_ch->intstat != 0x1c)
	{
		log(INFO, "  Interrupt error! Time delay interrupt was not set! INTST = 0x%x\n", fifo_ch->intstat);
		ok = 0;
	}
	else if (*((uint32_t*)NEWS5000_INTST0) == 0)
	{
		log(INFO, "  Interrupt error! INTST0 was not set! 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
		ok = 0;
	}
	else
	{
		log(INFO, "  This channel's INTST0 flag is 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
	}

	fifo_ch->data;
	fifo_ch->data;
	if (fifo_ch->intstat != 0x0)
	{
		log(INFO, "  Interrupt error! Time delay interrupt was set! INTST = 0x%x\n", fifo_ch->intstat);
		ok = 0;
	}
	else if (*((uint32_t*)NEWS5000_INTST0) != 0)
	{
		log(INFO, "  Interrupt error! INTST0 was set! 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
		ok = 0;
	}

	log(INFO, " Set time delay count, then read more data to trigger time delay interrupt\n");
	fifo_ch->unknown3 = 0xfff; // max 0xfff
	fifo_ch->data;
	if ((fifo_ch->intstat & 0xff) != 0x10)
	{
		log(INFO, "  Interrupt error! Time delay interrupt was not masked! INTST = 0x%x\n", fifo_ch->intstat);
		ok = 0;
	}
	else if (*((uint32_t*)NEWS5000_INTST0) != 0)
	{
		log(INFO, "  Interrupt error! INTST0 was set! 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
		ok = 0;
	}

	log(INFO, " Dead spin 1 start at 0x%x....\n", get_ustime());
	inst = 0;
	for (i = 0; i < 100000; ++i) {
		if ((i % 10) == 0)
		{
			temp = fifo_ch->intstat;
			if (inst != temp)
			{
				inst = temp;
				if(inst == 0x1c)
				{
					log(INFO, "\n Finish at approx 0x%x", get_ustime());
				}
				else
				{
					log(INFO, "0x%x ", inst);
				}
			}
		}
	}
	log(INFO, "\n");

	log(INFO, " Check that the next interrupt is not delayed if the delay count is not armed\n");
	fifo_ch->register_pointer = fifo_ch->dma_pointer;

	fifo_ch->register_pointer = fifo_ch->dma_pointer - 8;
	if (fifo_ch->intstat != 0x1c)
	{
		log(INFO, "  Interrupt error! Time delay interrupt was not set! INTST = 0x%x\n", fifo_ch->intstat);
		ok = 0;
	}
	else if (*((uint32_t*)NEWS5000_INTST0) == 0)
	{
		log(INFO, "  Interrupt error! INTST0 was not set! 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
		ok = 0;
	}

	fifo_ch->unknown3 = 0xfff; 
	if ((fifo_ch->intstat & 0xff) != 0x10)
	{
		log(INFO, "  Interrupt error! Time delay interrupt was not masked! INTST = 0x%x\n", fifo_ch->intstat);
		ok = 0;
	}
	else if (*((uint32_t*)NEWS5000_INTST0) != 0)
	{
		log(INFO, "  Interrupt error! INTST0 was set! 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
		ok = 0;
	}

	for (i = 0; i < 100000; ++i) { if (i % 1000 == 0) log(INFO, "."); }
	log(INFO, "\n");

	// Time delay count should only apply once - it should instantly trigger
	fifo_ch->data;
	fifo_ch->data;
	if ((fifo_ch->intstat & 0xff) != 0x0)
	{
		log(INFO, "  Interrupt error! Time delay interrupt was not masked! INTST = 0x%x\n", fifo_ch->intstat);
		dump_apfifo_channel(INFO, fifo_ch);
		ok = 0;
	}
	else if (*((uint32_t*)NEWS5000_INTST0) != 0)
	{
		log(INFO, "  Interrupt error! INTST0 was set! 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
		ok = 0;
	}

	fifo_ch->data;
	for (i = 0; i < 100000; ++i) { if (i % 1000 == 0) log(INFO, "."); }
	log(INFO, "\n");
	fifo_ch->register_pointer = fifo_ch->dma_pointer;
	if ((fifo_ch->intstat & 0xff) != 0x0)
	{
		log(INFO, "  Interrupt error! Time delay interrupt was not masked! INTST = 0x%x\n", fifo_ch->intstat);
		dump_apfifo_channel(INFO, fifo_ch);
		ok = 0;
	}
	else if (*((uint32_t*)NEWS5000_INTST0) != 0)
	{
		log(INFO, "  Interrupt error! INTST0 was set! 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
		ok = 0;
	}

	fifo_ch->register_pointer = fifo_ch->dma_pointer - 8;
	if (fifo_ch->intstat != 0x1c)
	{
		log(INFO, "  Interrupt error! Time delay interrupt was not set! INTST = 0x%x\n", fifo_ch->intstat);
		ok = 0;
	}
	else if (*((uint32_t*)NEWS5000_INTST0) == 0)
	{
		log(INFO, "  Interrupt error! INTST0 was not set! 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
		ok = 0;
	}

	log(INFO, " Check that interrupting count by reading out data allows count to continue\n");
	fifo_ch->register_pointer = fifo_ch->dma_pointer;
	fifo_ch->register_pointer = fifo_ch->dma_pointer - 8;
	if (fifo_ch->intstat != 0x1c)
	{
		log(INFO, "  Interrupt error! Time delay interrupt was not set! INTST = 0x%x\n", fifo_ch->intstat);
		ok = 0;
	}
	else if (*((uint32_t*)NEWS5000_INTST0) == 0)
	{
		log(INFO, "  Interrupt error! INTST0 was not set! 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
		ok = 0;
	}

	fifo_ch->unknown3 = 0xfff; 
	if ((fifo_ch->intstat & 0xff) != 0x10)
	{
		log(INFO, "  Interrupt error! Time delay interrupt was not masked! INTST = 0x%x\n", fifo_ch->intstat);
		ok = 0;
	}
	else if (*((uint32_t*)NEWS5000_INTST0) != 0)
	{
		log(INFO, "  Interrupt error! INTST0 was set! 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
		ok = 0;
	}

	for (i = 0; i < 1000; ++i) { if (i % 1000 == 0) log(INFO, "."); }
	log(INFO, "\n");

	fifo_ch->data;
	fifo_ch->data;
	if((fifo_ch->intstat & 0xff) != 0x0)
	{
		log(INFO, "  Interrupt error! FIFO INTST was set to 0x%x!\n", fifo_ch->intstat);
	}
	else if(fifo_ch->intstat == 0x0)
	{
		log(INFO, "  Interrupt error! Count stopped unexpectedly!\n");
	}

	for (i = 0; i < 200000; ++i) { if (i % 2000 == 0) log(INFO, "."); }
	log(INFO, "\n");
	if(fifo_ch->intstat != 0x0)
	{
		log(INFO, "  Interrupt error! Count did not complete! INTST=0x%x\n", fifo_ch->intstat);
	}

	log(INFO, " Check a smaller count and counter reset on dcnt write\n");
	fifo_ch->register_pointer = fifo_ch->dma_pointer;
	fifo_ch->unknown3 = 0xf00;
	fifo_ch->register_pointer = fifo_ch->dma_pointer - 8;

	log(INFO, "Dead spin 3 start at 0x%x....\n", get_ustime());
	inst = 0;
	for (i = 0; i < 100000; ++i) {
		if ((i % 10) == 0)
		{
			temp = fifo_ch->intstat;
			if (inst != temp)
			{
				inst = temp;
				if(inst == 0x1c)
				{
					log(INFO, "\nFinish at approx 0x%x", get_ustime());
				}
				else
				{
					log(INFO, "0x%x ", inst);
				}
			}
		}
		if (i == 50)
		{
			fifo_ch->data;
			log(INFO, "\nRead data!\n");
		}
		if (i == 100)
		{
			log(INFO, "\nAbout to reset dcnt! FIFO INTST = 0x%x INTST0 = 0x%x\n", fifo_ch->intstat, *((uint32_t*)NEWS5000_INTST0));
			fifo_ch->unknown3 = 0xf00;
			log(INFO, "Reset dcnt! FIFO INTST = 0x%x INTST0 = 0x%x\n", fifo_ch->intstat, *((uint32_t*)NEWS5000_INTST0));
		}
	}
	log(INFO, "\n");

	uint32_t total = 0;
	total += calculate_delay_count_interval(fifo_ch, 0xfff);
	total += calculate_delay_count_interval(fifo_ch, 0xf00);
	total += calculate_delay_count_interval(fifo_ch, 0xbaa);
	total += calculate_delay_count_interval(fifo_ch, 0x800);
	total += calculate_delay_count_interval(fifo_ch, 0x250);
	log(INFO, " Average us per tick: %u\n", total / 5);

	return ok;
}
#pragma endregion FIFO interrupt tests

#pragma region FIFO DMA tests
int
apfifo_dma_read_test()
{
	log(INFO, "Starting FDC DMA test!\n");
	dump_apfifo_channel(INFO, APFIFO0_FD);
	volatile uint32_t *sra = (uint32_t*)0xbed60000;
	// volatile uint8_t *srb = (uint8_t*)0xbed60004;
	volatile uint32_t *dor = (uint32_t*)0xbed60008;
	// volatile uint8_t *tdr = (uint8_t*)0xbed6000c;
	volatile uint32_t *msr_dsr = (uint32_t*)0xbed60010;
	volatile uint32_t *fdc_fifo = (uint32_t*)0xbed60014;
	volatile uint32_t *dir_ccr = (uint32_t*)0xbed6001c;
	volatile uint32_t *fdc_aux1 = (uint32_t*)0xbed60204;

	log(INFO, "\nreset FDC\n");
	// [:fdc] dor = 00
	// [:fdc] dor = 04
	// [:fdc] dsr_w 80 (':cpu' (9FC10D2C))
	// [:fdc] dor = 00
	// [:fdc] dsr_w 40 (':cpu' (9FC10D48))
	// [:cpu] ':cpu' (9FC10D54): unmapped program memory write to 1ED60200 = 0000000000000001 & 00000000FFFFFFFF
	// [:fdc] dor = 14

	log(INFO, "MSR = 0x%x ", *msr_dsr);
	*dor = 0x00;
	log(INFO, "dor = 0x%x\n", *dor);
	*dor = 0x04;
	log(INFO, "dor = 0x%x\n", *dor);
	// *msr_dsr = 0x80;
	// log(INFO, "dsr = 0x%x\n", *msr_dsr);
	*dor = 0x00;
	// log(INFO, "dor = 0x%x\n", *dor);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	// *msr_dsr = 0x40;
	// log(INFO, "msr = 0x%x\n", *msr_dsr);

	log(INFO, "Read INTST0, expect unset? 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
	*fdc_aux1 = 0x1;
	log(INFO, "Set FDC AUX to 0x1\n");
	log(INFO, "Read INTST0, expect unset?: 0x%x\n", *((uint32_t*)NEWS5000_INTST0));

	*dor = 0x1C;
	log(INFO, "dor = 0x%x\n", *dor);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	
	// internal stuff?
	// [:fdc] polled 0 : 0 -> 1
	// [:fdc] polled 1 : 0 -> 1
	// [:fdc] polled 2 : 0 -> 1
	// [:fdc] polled 3 : 0 -> 1

	// [:] generic_irq_w: INTST0 IRQ 16 set to 1
	log(INFO, "Read INTST0, expect set: 0x%x\n", *((uint32_t*)NEWS5000_INTST0));

	log(INFO, "\nExecute command sense interrupt status\n");
	*fdc_fifo = 0x08;
	while ((*msr_dsr & 0x80) != 0x80) {}
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res1 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res2 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);


	log(INFO, "\nExecute command sense interrupt status\n");
	*fdc_fifo = 0x08;
	while ((*msr_dsr & 0x80) != 0x80) {}
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res1 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res2 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);

	log(INFO, "\nExecute command sense interrupt status\n");
	*fdc_fifo = 0x08;
	while ((*msr_dsr & 0x80) != 0x80) {}
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res1 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res2 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);

	log(INFO, "\nExecute command sense interrupt status\n");
	*fdc_fifo = 0x08;
	while ((*msr_dsr & 0x80) != 0x80) {}
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res1 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res2 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);

	// Check ready
	// [:fdc] ':cpu' (9FC10EE8): sra_r = 0xcc
	log(INFO, "\nRead sra, expect 0xcc: 0x%x\n", *sra);

	log(INFO, "Execute specify df 10 command");
	*msr_dsr = 0x1c;
	*dir_ccr = 0x00;
	*fdc_fifo = 0x03;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0xdf;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x10;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);

	// [:fdc] dsr_w 1c (':cpu' (9FC0FEC8))
	// [:fdc] ':cpu' (9FC0FEE0): ccr_w(0x00)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x03)
	// [:] generic_irq_w: INTST0 IRQ 16 set to 0
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0xdf)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x10)
	// [:fdc] command specify df 10: step_rate=3 ms, head_unload=240 ms, head_load=16 ms, non_dma=false
	log(INFO, "\nRead INTST0, expect unset: 0x%x\n", *((uint32_t*)NEWS5000_INTST0));

	log(INFO, "Execute perpindicular command\n");
	*fdc_fifo = 0x12;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x05;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x12)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x05)
	// [:fdc] command perpendicular
	log(INFO, "Read msr: 0x%x\n", *msr_dsr);
	log(INFO, "Read INTST0, expect unset: 0x%x\n", *((uint32_t*)NEWS5000_INTST0));

	log(INFO, "Wait for msr");
	while ((*msr_dsr & 0xff) != 0x80) { log(INFO, "."); }

	log(INFO, "\nExecute configure 00 08 00 command\n");
	*fdc_fifo = 0x13;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x00;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x08;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x00;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x13)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x00)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x08)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x00)
	// [:fdc] command configure 00 08 00
	log(INFO, "Read msr: 0x%x\n", *msr_dsr);
	log(INFO, "Read INTST0, expect unset: 0x%x\n", *((uint32_t*)NEWS5000_INTST0));

	log(INFO, "Execute recalibrate 0 command\n");
	*fdc_fifo = 0x07;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x00;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x07)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x00)
	// [:fdc] command recalibrate 0

	log(INFO, "Wait for 3 seconds...");
	usleep(3000000);
	log(INFO, " Done!\n");
	log(INFO, "Read sra: 0x%x\n", *sra);
	log(INFO, "Read msr: 0x%x\n", *msr_dsr);
	// log(INFO, "Wait for interrupt\n");
	//while (*((uint32_t*)NEWS5000_INTST0) == 0) { log(INFO, "int = 0x%x  sra = 0x%x msr = 0x%x", *((uint32_t*)NEWS5000_INTST0), *sra, *msr_dsr); }
	// [:] intst_r: INTST0 = 0x0
	// ...
	// [:] generic_irq_w: INTST0 IRQ 16 set to 1
	// [:] intst_r: INTST0 = 0x10

	// log(INFO, "\nExecute command sense interrupt status\n");
	// *fdc_fifo = 0x08;
	// [:fdc] ':cpu' (9FC11760): fifo_w(0x08)
	// [:] generic_irq_w: INTST0 IRQ 16 set to 0
	// [:fdc] command sense interrupt status (fid=0 20 00) (':cpu' (9FC11760))

	//log(INFO, "Read results 0x%x 0x%x (should be 0x20 0x00)\n", *fdc_fifo, *fdc_fifo);
	// [:fdc] ':cpu' (9FC117B0): fifo_r = 0x20
	// [:fdc] ':cpu' (9FC117B0): fifo_r = 0x00

	/*
	log(INFO, "Execute recalibrate 0 command\n");
	*fdc_fifo = 0x07;
	*fdc_fifo = 0x00;

	log(INFO, "Read sra: 0x%x\n", *sra);
	log(INFO, "Read msr: 0x%x\n", *msr_dsr);
	log(INFO, "Wait for 3 seconds...");
	usleep(3000000);
	log(INFO, " Done!\n");
	log(INFO, "Read sra: 0x%x\n", *sra);
	log(INFO, "Read msr: 0x%x\n", *msr_dsr);
	*/

	log(INFO, "LED_FLOPPY = ON\n"); // don't feel like setting LEDs for now - will add library for this later if it doesn't already exist

	log(INFO, "Configure fifo channel...\n");
	APFIFO0_FD->size = 0x7fff;
	APFIFO0_FD->address = 0x0;
	APFIFO0_FD->intctrl = 0x0; // todo: check if enabling this in other tests changes anything
	APFIFO0_FD->dma_pointer  = 0x0;
	APFIFO0_FD->register_pointer  = 0x0;
	APFIFO0_FD->dma_mode  = 0x0;
	APFIFO0_FD->unknown2 = 0x10000; // watermark?
	APFIFO0_FD->dma_mode = 0x1; // enable DMA mode
	// [:apfifo0] FIFO CH2: Setting fifo_size to 0x7fff
	// [:apfifo0] FIFO CH2: Setting address to 0x0
	// [:apfifo0] FIFO CH2: Set intctrl = 0x0 (':cpu' (9FC118C4))
	// [:apfifo0] FIFO CH2: Set dma pointer = 0x0 (':cpu' (9FC118CC))
	// [:apfifo0] FIFO CH2: Set register pointer = 0x0 (':cpu' (9FC118D4))
	// [:apfifo0] FIFO CH2: Setting DMA mode to 0x0 (':cpu' (9FC1198C))
	// [:apfifo0] FIFO CH2: Setting watermark to 0x10000
	// [:apfifo0] FIFO CH2: Setting DMA mode to 0x1 (':cpu' (9FC1199C))

	log(INFO, "Trigger FDC command\n");
	*fdc_fifo = 0x46;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x00;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x00;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x00;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x01;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x02;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x10;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x1b;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0xff;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x46)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x00)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x00)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x00)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x01)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x02)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x10)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x1b)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0xff)
	// [:fdc] command read data mfm cmd=46 sel=0 chrn=(0, 0, 1, 512) eot=10 gpl=1b dtl=ff rate=500000

	// Then, poll INTST0 until floppy IRQ (0x10) is set, then results should??? be ready
	log(INFO, "Wait for interrupt...\n");
	log(INFO, "Read sra: 0x%x\n", *sra);
	log(INFO, "Read msr: 0x%x\n", *msr_dsr);
	while (*((uint32_t*)NEWS5000_INTST0) == 0) { 
		log(INFO, "Read sra: 0x%x ", *sra);
		log(INFO, "Read msr: 0x%x\n", *msr_dsr);
		usleep(10000);
	}

	log(INFO, "FDC command done! Reading out data...\n");
	while (APFIFO0_FD->count)
	{
		log(INFO, "0x%x ", APFIFO0_FD->data);
	}
	log(INFO, "\n");

	dump_apfifo_channel(INFO, APFIFO0_FD);

	return 0;
}

int
apfifo_dma_write_test()
{
	log(INFO, "Starting FDC DMA test!\n");
	dump_apfifo_channel(INFO, APFIFO0_FD);
	volatile uint32_t *sra = (uint32_t*)0xbed60000;
	// volatile uint8_t *srb = (uint8_t*)0xbed60004;
	volatile uint32_t *dor = (uint32_t*)0xbed60008;
	// volatile uint8_t *tdr = (uint8_t*)0xbed6000c;
	volatile uint32_t *msr_dsr = (uint32_t*)0xbed60010;
	volatile uint32_t *fdc_fifo = (uint32_t*)0xbed60014;
	volatile uint32_t *dir_ccr = (uint32_t*)0xbed6001c;
	volatile uint32_t *fdc_aux1 = (uint32_t*)0xbed60204;

	log(INFO, "\nreset FDC\n");
	// [:fdc] dor = 00
	// [:fdc] dor = 04
	// [:fdc] dsr_w 80 (':cpu' (9FC10D2C))
	// [:fdc] dor = 00
	// [:fdc] dsr_w 40 (':cpu' (9FC10D48))
	// [:cpu] ':cpu' (9FC10D54): unmapped program memory write to 1ED60200 = 0000000000000001 & 00000000FFFFFFFF
	// [:fdc] dor = 14

	log(INFO, "MSR = 0x%x ", *msr_dsr);
	*dor = 0x00;
	log(INFO, "dor = 0x%x\n", *dor);
	*dor = 0x04;
	log(INFO, "dor = 0x%x\n", *dor);
	// *msr_dsr = 0x80;
	// log(INFO, "dsr = 0x%x\n", *msr_dsr);
	*dor = 0x00;
	// log(INFO, "dor = 0x%x\n", *dor);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	// *msr_dsr = 0x40;
	// log(INFO, "msr = 0x%x\n", *msr_dsr);

	log(INFO, "Read INTST0, expect unset? 0x%x\n", *((uint32_t*)NEWS5000_INTST0));
	*fdc_aux1 = 0x1;
	log(INFO, "Set FDC AUX to 0x1\n");
	log(INFO, "Read INTST0, expect unset?: 0x%x\n", *((uint32_t*)NEWS5000_INTST0));

	*dor = 0x1C;
	log(INFO, "dor = 0x%x\n", *dor);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	
	// internal stuff?
	// [:fdc] polled 0 : 0 -> 1
	// [:fdc] polled 1 : 0 -> 1
	// [:fdc] polled 2 : 0 -> 1
	// [:fdc] polled 3 : 0 -> 1

	// [:] generic_irq_w: INTST0 IRQ 16 set to 1
	log(INFO, "Read INTST0, expect set: 0x%x\n", *((uint32_t*)NEWS5000_INTST0));

	log(INFO, "\nExecute command sense interrupt status\n");
	*fdc_fifo = 0x08;
	while ((*msr_dsr & 0x80) != 0x80) {}
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res1 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res2 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);


	log(INFO, "\nExecute command sense interrupt status\n");
	*fdc_fifo = 0x08;
	while ((*msr_dsr & 0x80) != 0x80) {}
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res1 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res2 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);

	log(INFO, "\nExecute command sense interrupt status\n");
	*fdc_fifo = 0x08;
	while ((*msr_dsr & 0x80) != 0x80) {}
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res1 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res2 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);

	log(INFO, "\nExecute command sense interrupt status\n");
	*fdc_fifo = 0x08;
	while ((*msr_dsr & 0x80) != 0x80) {}
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res1 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	log(INFO, "res2 = 0x%x\n", *fdc_fifo);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);

	// Check ready
	// [:fdc] ':cpu' (9FC10EE8): sra_r = 0xcc
	log(INFO, "\nRead sra, expect 0xcc: 0x%x\n", *sra);

	log(INFO, "Execute specify df 10 command");
	*msr_dsr = 0x1c;
	*dir_ccr = 0x00;
	*fdc_fifo = 0x03;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0xdf;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x10;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);

	// [:fdc] dsr_w 1c (':cpu' (9FC0FEC8))
	// [:fdc] ':cpu' (9FC0FEE0): ccr_w(0x00)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x03)
	// [:] generic_irq_w: INTST0 IRQ 16 set to 0
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0xdf)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x10)
	// [:fdc] command specify df 10: step_rate=3 ms, head_unload=240 ms, head_load=16 ms, non_dma=false
	log(INFO, "\nRead INTST0, expect unset: 0x%x\n", *((uint32_t*)NEWS5000_INTST0));

	log(INFO, "Execute perpindicular command\n");
	*fdc_fifo = 0x12;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x05;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x12)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x05)
	// [:fdc] command perpendicular
	log(INFO, "Read msr: 0x%x\n", *msr_dsr);
	log(INFO, "Read INTST0, expect unset: 0x%x\n", *((uint32_t*)NEWS5000_INTST0));

	log(INFO, "Wait for msr");
	while ((*msr_dsr & 0xff) != 0x80) { log(INFO, "."); }

	log(INFO, "\nExecute configure 00 08 00 command\n");
	*fdc_fifo = 0x13;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x00;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x08;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x00;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x13)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x00)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x08)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x00)
	// [:fdc] command configure 00 08 00
	log(INFO, "Read msr: 0x%x\n", *msr_dsr);
	log(INFO, "Read INTST0, expect unset: 0x%x\n", *((uint32_t*)NEWS5000_INTST0));

	log(INFO, "Execute recalibrate 0 command\n");
	*fdc_fifo = 0x07;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x00;
	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x07)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x00)
	// [:fdc] command recalibrate 0

	log(INFO, "Wait for 3 seconds...");
	usleep(3000000);
	log(INFO, " Done!\n");
	log(INFO, "Read sra: 0x%x\n", *sra);
	log(INFO, "Read msr: 0x%x\n", *msr_dsr);
	// log(INFO, "Wait for interrupt\n");
	//while (*((uint32_t*)NEWS5000_INTST0) == 0) { log(INFO, "int = 0x%x  sra = 0x%x msr = 0x%x", *((uint32_t*)NEWS5000_INTST0), *sra, *msr_dsr); }
	// [:] intst_r: INTST0 = 0x0
	// ...
	// [:] generic_irq_w: INTST0 IRQ 16 set to 1
	// [:] intst_r: INTST0 = 0x10

	// log(INFO, "\nExecute command sense interrupt status\n");
	// *fdc_fifo = 0x08;
	// [:fdc] ':cpu' (9FC11760): fifo_w(0x08)
	// [:] generic_irq_w: INTST0 IRQ 16 set to 0
	// [:fdc] command sense interrupt status (fid=0 20 00) (':cpu' (9FC11760))

	//log(INFO, "Read results 0x%x 0x%x (should be 0x20 0x00)\n", *fdc_fifo, *fdc_fifo);
	// [:fdc] ':cpu' (9FC117B0): fifo_r = 0x20
	// [:fdc] ':cpu' (9FC117B0): fifo_r = 0x00

	/*
	log(INFO, "Execute recalibrate 0 command\n");
	*fdc_fifo = 0x07;
	*fdc_fifo = 0x00;

	log(INFO, "Read sra: 0x%x\n", *sra);
	log(INFO, "Read msr: 0x%x\n", *msr_dsr);
	log(INFO, "Wait for 3 seconds...");
	usleep(3000000);
	log(INFO, " Done!\n");
	log(INFO, "Read sra: 0x%x\n", *sra);
	log(INFO, "Read msr: 0x%x\n", *msr_dsr);
	*/

	log(INFO, "LED_FLOPPY = ON\n"); // don't feel like setting LEDs for now - will add library for this later if it doesn't already exist

	log(INFO, "Configure fifo channel...\n");
	APFIFO0_FD->size = 0x7fff;
	APFIFO0_FD->address = 0x0;
	APFIFO0_FD->intctrl = 0x2; // todo: check if enabling this in other tests changes anything
	APFIFO0_FD->dma_pointer  = 0x0;
	APFIFO0_FD->register_pointer  = 0x0;
	// APFIFO0_FD->dma_mode  = 0xa; // prep for transfer out
	APFIFO0_FD->unknown2 = 0x10000; // watermark?

	log(INFO, "Read INTEN0, expect unset: 0x%x\n", *((uint32_t*)NEWS5000_INTEN0));
	*((uint32_t*)NEWS5000_INTEN0) = 0x0;
	
	log(INFO, "Prepping data...\n");
	//APFIFO0_BUF_32(0) = 0x12345678;
	//APFIFO0_BUF_32(1) = 0xabcdef12;
	APFIFO0_FD->dma_mode = 0x2; // set DMA direction TODO: check if this changes anything in the status or how the dma/reg pointers increment
	APFIFO0_FD->data = 0x12345678;
	APFIFO0_FD->data = 0xabcdef12; // todo: see if it always repeats the last byte - does enabling interrupts change how this works?

	APFIFO0_FD->intctrl = 0x0; // MROM disables FIFO interrupts when formatting - is this correct? Maybe it hooks off of the FDC interrupt? TODO: see if interrupts can be enabled here
	APFIFO0_FD->dma_mode = 0x3; // enable DMA mode
	// [:apfifo0] FIFO CH2: Setting fifo_size to 0x7fff
	// [:apfifo0] FIFO CH2: Setting address to 0x0
	// [:apfifo0] FIFO CH2: Set intctrl = 0x0 (':cpu' (9FC118C4))
	// [:apfifo0] FIFO CH2: Set dma pointer = 0x0 (':cpu' (9FC118CC))
	// [:apfifo0] FIFO CH2: Set register pointer = 0x0 (':cpu' (9FC118D4))
	// [:apfifo0] FIFO CH2: Setting DMA mode to 0x0 (':cpu' (9FC1198C))
	// [:apfifo0] FIFO CH2: Setting watermark to 0x10000
	// [:apfifo0] FIFO CH2: Setting DMA mode to 0x1 (':cpu' (9FC1199C))

	log(INFO, "Trigger FDC command\n");
	*fdc_fifo = 0x45;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x00;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x00;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x00;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x01;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x02;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x10;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0x1b;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	*fdc_fifo = 0xff;

	usleep(1000);
	log(INFO, "MSR = 0x%x\n", *msr_dsr);
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x46) command
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x00) select
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x00) C
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x00) H
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x01) R
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x02) N
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x10) EOT (# of sectors)
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0x1b) GPL
	// [:fdc] ':cpu' (9FC113DC): fifo_w(0xff) DTL
	// [:fdc] command read data mfm cmd=46 sel=0 chrn=(0, 0, 1, 512) eot=10 gpl=1b dtl=ff rate=500000

	// Then, poll INTST0 until floppy IRQ (0x10) is set, then results should??? be ready
	log(INFO, "Wait for interrupt...\n");
	log(INFO, "Read sra: 0x%x\n", *sra);
	log(INFO, "Read msr: 0x%x\n", *msr_dsr);
	while (*((uint32_t*)NEWS5000_INTST0) == 0) { 
		log(INFO, "Read sra: 0x%x ", *sra);
		log(INFO, "Read msr: 0x%x\n", *msr_dsr);
		usleep(10000);
	}

	log(INFO, "FDC command done! Reading out data...\n");
	while (APFIFO0_FD->count)
	{
		APFIFO0_FD->data;
		log(INFO, ".");
	}
	log(INFO, "\n");

	dump_apfifo_channel(INFO, APFIFO0_FD);

	return 0;
}
#pragma endregion FIFO DMA tests

void
apfifo_test()
{
	log(INFO, "Starting CXD8442Q WSC-FIFOQ functional tests...\n");
	volatile uint8_t *byte_data_accessor = (uint8_t *)&APFIFO0_FD->data;

	dump_apfifo_channel(INFO, APFIFO0_FD);

	log(INFO, "Starting tests...\n");
	log(INFO, "start time = 0x%x\n", get_ustime());

	// TODO: DMA testing	
	// TODO: count when doing a DMA transfer out? See if count changes to cpu - dma when DMA dir is set?
	// LOGRESULT(apfifo_dma_read_test());
	// LOGRESULT(apfifo_dma_write_test());
	
	// Common configuration for first round of tests
	init_channel(APFIFO0_FD, 0x0, 0x1fff);
	init_channel(APFIFO0_CH0, 0x2000, 0x1fff);
	init_channel(APFIFO0_CH1, 0x4000, 0x1fff);
	init_channel(APFIFO0_CH3, 0x6000, 0x1fff);

	// Basic read tests
	LOGRESULT(apfifo_byte_access_test(APFIFO0_FD, byte_data_accessor));
	LOGRESULT(apfifo_halfword_access_test(APFIFO0_FD, (volatile uint16_t *)&APFIFO0_FD->data));
	LOGRESULT(apfifo_word_access_test(APFIFO0_FD));

	// Edge-case-y read tests
	LOGRESULT(apfifo_ooo_access_test(APFIFO0_FD, byte_data_accessor));
	LOGRESULT(apfifo_partial_word_test(APFIFO0_FD, byte_data_accessor));
	LOGRESULT(apfifo_cnt_cptr_test(APFIFO0_FD, APFIFO0_FD->size));

	// Basic write tests
	LOGRESULT(apfifo_byte_write_test(APFIFO0_FD, byte_data_accessor, APFIFO0_FD->size));
	LOGRESULT(apfifo_halfword_write_test(APFIFO0_FD, (volatile uint16_t *)&APFIFO0_FD->data, APFIFO0_FD->size));
	LOGRESULT(apfifo_word_write_test(APFIFO0_FD, APFIFO0_FD->size));

	LOGRESULT(apfifo_delay_interrupt_test(APFIFO0_FD));
	// TODO: multichannel delay interrupt test

	// FIFO channel control tests
	LOGRESULT(apfifo_reconfigure_test(APFIFO0_FD));
	
	log(INFO, "1..%d\nTests complete!\n", tap);
	log(INFO, "end time = 0x%x\n\n", get_ustime());
	usleep(100);
	log(INFO, "end time2 = 0x%x\n\n", get_ustime());

	dump_apfifo_channel(INFO, APFIFO0_FD);

	log(INFO, "\nExiting to APmonitor...\n");
	return;
}
