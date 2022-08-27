#ifndef APFIFO_TEST
#define APFIFO_TEST

void apfifo_test(void);
void dump_apfifo_channel(struct fifo_channel *fifo_ch);
void print_intstat(void);
void intclr(void);
int apfifo_word_access_test(struct fifo_channel *fifo_ch);
int apfifo_halfword_access_test(struct fifo_channel *fifo_ch, volatile uint16_t *data_ptr);
int apfifo_byte_access_test(struct fifo_channel *fifo_ch, volatile uint8_t *data_ptr);
int apfifo_ooo_access_test(struct fifo_channel *fifo_ch, volatile uint8_t *data_ptr);
int apfifo_partial_word_test(struct fifo_channel *fifo_ch, volatile uint8_t *data_ptr);
int apfifo_cnt_cptr_test(struct fifo_channel *fifo_ch, uint32_t size);
int apfifo_word_write_test(struct fifo_channel *fifo_ch, uint32_t size);
int apfifo_halfword_write_test(struct fifo_channel *fifo_ch, volatile uint16_t *data_ptr, uint32_t size);
int apfifo_byte_write_test(struct fifo_channel *fifo_ch, volatile uint8_t *data_ptr, uint32_t size);
int apfifo_reconfigure_test(struct fifo_channel *fifo_ch);
int apfifo_delay_interrupt_test(struct fifo_channel *fifo_ch);
void boot(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
uint32_t get_ustime(void);
void init_channel(struct fifo_channel *fifo_ch, uint32_t address, uint32_t size);
void calculate_delay_count_interval(struct fifo_channel *fifo_ch, uint32_t count);

#endif
