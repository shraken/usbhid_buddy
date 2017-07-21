/**
 * @file 23lc1024.h
 * @author Nicholas Shrake
 * @date 5 May 2017
 * @brief Driver enumerations and prototypes for the Microchip
 *			23LC1024 1MBit SPI SRAM
 *
 */
 
#ifndef _23LC1204_H_
#define _23LC1024_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//#define SRAM_23LC1024_BUFFER_SIZE 32

#define SRAM_23LC1024_INSTR_OFFSET 0
#define SRAM_23LC1024_WMRM_LENGTH 2
#define SRAM_23LC1024_HEADER_LENGTH 4

typedef enum _SRAM_23LC1024_ACTION {
    SRAM_23LC1024_ACTION_READ = 0x03,
    SRAM_23LC1024_ACTION_WRITE = 0x02,
    SRAM_23LC1024_ACTION_EDIO = 0x3B,
    SRAM_23LC1024_ACTION_EQIO = 0x38,
    SRAM_23LC1024_ACTION_RSTIO = 0xFF,
    SRAM_23LC1024_ACTION_RDMR = 0x05,
    SRAM_23LC1024_ACTION_WRMR = 0x01,
} SRAM_23LC1024_ACTION;

typedef enum _SRAM_23LC1024_MODE {
    SRAM_23LC1024_MODE_BYTE = 0x00,
    SRAM_23LC1024_MODE_SEQ = 0x01,
    SRAM_23LC1024_MODE_PAGE = 0x02,
    SRAM_23LC1024_MODE_RESV = 0x03,
} SRAM_23LC1024_MODE;

typedef enum _SRAM_23LC1024_STATUS_ERROR {
    SRAM_23LC1024_STATUS_ERROR_NORMAL = 0,
    SRAM_23LC1024_STATUS_ERROR_BAD_PARAM = 1,
    SRAM_23LC1024_STATUS_ERROR_GEN_FAIL = 2,
    SRAM_23LC1024_STATUS_ERROR_NO_MEM = 3,
} SRAM_23LC1024_STATUS_ERROR;

int8_t SRAM_23LC1024_Init(void);
int8_t SRAM_23LC1024_test(void);
int8_t SRAM_23LC1024_set_mode(uint8_t mode_register, uint8_t mode_value);
int8_t SRAM_23LC1024_write(uint32_t address, uint8_t *buffer, uint32_t length);
uint8_t *SRAM_23LC1024_read(uint32_t address, uint32_t length);

#endif /* _23LC1024_H_ */