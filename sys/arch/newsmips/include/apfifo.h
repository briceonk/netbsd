#ifndef __MACHINE_APFIFO_H__
#define __MACHINE_APFIFO_H__

struct fifo_channel
{
    volatile uint32_t size;
    volatile uint32_t address;
    volatile uint32_t intclr;
    volatile uint32_t dma_mode;
    volatile uint32_t unknown0; // WAIT
    volatile uint32_t unknown1; // DRQ clear
    volatile uint32_t intctrl;
    volatile uint32_t intstat;
    volatile uint32_t watermark; // watermark
    volatile uint32_t time_delay_count;
    volatile uint32_t dma_pointer;
    volatile uint32_t register_pointer;
    volatile uint32_t count;
    volatile uint32_t data;
};

#define APFIFO0_CH0 ((struct fifo_channel*)(0xbed00000))
#define APFIFO0_CH1 ((struct fifo_channel*)(0xbed10000))
#define APFIFO0_FD ((struct fifo_channel*)(0xbed20000))
#define APFIFO0_CH3 ((struct fifo_channel*)(0xbed30000))

#define APFIFO0_CHIP_MODE *((volatile uint32_t*)0xbec80000)
#define APFIFO0_BUF_32(x) *((volatile uint32_t*)(0xbed80000 + 4 * (x)))
#define APFIFO0_BUF_16(x) *((volatile uint16_t*)(0xbed80000 + 2 * (x)))
#define APFIFO0_BUF_8(x) *((volatile uint8_t*)(0xbed80000 + (x)))

#define APFIFO1_CH0 ((struct fifo_channel*)(0xbe900000))
#define APFIFO1_CH1 ((struct fifo_channel*)(0xbe910000))
#define APFIFO1_CH2 ((struct fifo_channel*)(0xbe920000))
#define APFIFO1_CH3 ((struct fifo_channel*)(0xbe930000))

#define APFIFO1_CHIP_MODE *((volatile uint32_t*)0xbe880000)
#define APFIFO1_BUF_32(x) *((volatile uint32_t*)(0xbe980000 + 4 * (x)))
#define APFIFO1_BUF_16(x) *((volatile uint16_t*)(0xbe980000 + 2 * (x)))
#define APFIFO1_BUF_8(x) *((volatile uint8_t*)(0xbe980000 + (x)))

#endif	/* !__MACHINE_APFIFO_H__ */