#ifndef APFIFO_TEST
#define APFIFO_TEST

void apfifo_test(void);

void log(int level, const char* format, ...);
void dump_apfifo_channel(int log_level, struct fifo_channel *fifo_ch);
void log_intstat(int log_level);
void intclr(void);
void clear_fifo_ram(void);
void reset_channel(struct fifo_channel *fifo_ch);
int assert_intstat(struct fifo_channel *fifo_ch, uint32_t expected_value, char* fail_msg);
int assert_intst0(uint32_t expected_value, char* fail_msg);

// CPU read/register count tests
void apfifo_prep_read_test(struct fifo_channel *fifo_ch);
int apfifo_byte_access_test(struct fifo_channel *fifo_ch, volatile uint8_t *data_ptr);
int apfifo_halfword_access_test(struct fifo_channel *fifo_ch, volatile uint16_t *data_ptr);
int apfifo_word_access_test(struct fifo_channel *fifo_ch);
int apfifo_ooo_access_test(struct fifo_channel *fifo_ch, volatile uint8_t *data_ptr);
int apfifo_partial_word_test(struct fifo_channel *fifo_ch, volatile uint8_t *data_ptr);
int apfifo_cnt_cptr_test(struct fifo_channel *fifo_ch, uint32_t size);
int apfifo_misaligned_read_test(struct fifo_channel *fifo_ch);

// CPU write tests
int apfifo_byte_write_test(struct fifo_channel *fifo_ch, volatile uint8_t *data_ptr, uint32_t size);
int apfifo_halfword_write_test(struct fifo_channel *fifo_ch, volatile uint16_t *data_ptr, uint32_t size);
int apfifo_word_write_test(struct fifo_channel *fifo_ch, uint32_t size);

// FIFO interrupt tests
int apfifo_delay_interrupt_test(struct fifo_channel *fifo_ch);
int apfifo_delay_autoreset_test(struct fifo_channel *fifo_ch);
int apfifo_threshold_interrupt_test(struct fifo_channel *fifo_ch);
int apfifo_intclr_test(struct fifo_channel *fifo_ch);

// FIFO configuration tests
int apfifo_mask_test(struct fifo_channel *fifo_ch, uint32_t address, uint32_t size);
int apfifo_reconfigure_test(struct fifo_channel *fifo_ch);

// FIFO DMA tests
int apfifo_dma_read_test(void);
int apfifo_dma_write_test(void);

void boot(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t get_ustime(void);
void init_channel(struct fifo_channel *fifo_ch, uint32_t address, uint32_t size);
uint32_t calculate_delay_count_interval(struct fifo_channel *fifo_ch, uint32_t count);
void usleep(uint32_t microseconds);

#endif
