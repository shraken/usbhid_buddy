/**
 ******************************************************************************
 * @file  23lc1024.c
 * @author  Nicholas Shrake
 * @date  5 May 2017
 * @brief
 *   Driver library Microchip 1MBit SPI SRAM
 *
 ******************************************************************************
 */

#include <spi.h>
#include <stdint.h>
#include <stdio.h>
#include <23lc1024.h>
#include <utility.h>

#define TEST_BUFFER_SIZE 4

extern unsigned char SPI_Data_Rx_Array[];
extern unsigned char SPI_Data_Tx_Array[];
extern unsigned char bytes_trans;

static uint8_t test_out_buf[TEST_BUFFER_SIZE] = {0x01, 0x02, 0x03, 0x04};
static uint8_t test_in_buff[TEST_BUFFER_SIZE];

int8_t SRAM_23LC1024_test(void)
{
    int i;
    uint8_t *test_read_p;

    SRAM_23LC1024_write(0, (uint8_t *) &test_out_buf, TEST_BUFFER_SIZE);
    test_read_p = SRAM_23LC1024_read(0x00, TEST_BUFFER_SIZE);
    memcpy((uint8_t *) &test_in_buff, test_read_p, TEST_BUFFER_SIZE);

    printf("test_out_buf = ");
    for (i = 0; i < TEST_BUFFER_SIZE; i++) {
        printf("%02bx,", test_out_buf[i]);
    }
    printf("\r\n");

    printf("test_in_buff = ");
    for (i = 0; i < TEST_BUFFER_SIZE; i++) {
        printf("%02bx,", test_in_buff[i]);
    }
    printf("\r\n");

    return SRAM_23LC1024_STATUS_ERROR_NORMAL;
}

int8_t SRAM_23LC1024_Init(void)
{
    int8_t err_code_read, err_code_write;
    uint8_t mode;

    mode = SRAM_23LC1024_MODE_SEQ;
    err_code_read = SRAM_23LC1024_set_mode(SRAM_23LC1024_ACTION_RDMR, mode);
    err_code_write = SRAM_23LC1024_set_mode(SRAM_23LC1024_ACTION_WRMR, mode);

    if ((err_code_read != SRAM_23LC1024_STATUS_ERROR_NORMAL) ||
        (err_code_write != SRAM_23LC1024_STATUS_ERROR_NORMAL)) {
        return SRAM_23LC1024_STATUS_ERROR_GEN_FAIL;
    } else {
        return SRAM_23LC1024_STATUS_ERROR_NORMAL;
    }
}

int8_t SRAM_23LC1024_set_mode(uint8_t mode_register, uint8_t mode_value)
{
    int8_t err_code;

    if (mode_register) {
		SPI_Select(SPI_DEVICE_TYPE_23LC1024);
		
		SPI_Data_Tx_Array[SRAM_23LC1024_INSTR_OFFSET] = mode_register;
		SPI_Data_Tx_Array[SRAM_23LC1024_INSTR_OFFSET + 1] = (mode_value << 6);
		
		bytes_trans = SRAM_23LC1024_WMRM_LENGTH;
		SPI_Array_ReadWrite();
		
		err_code = SRAM_23LC1024_STATUS_ERROR_NORMAL;
    } else {
        err_code = SRAM_23LC1024_STATUS_ERROR_BAD_PARAM;
    }

    return err_code;
}

int8_t SRAM_23LC1024_write(uint32_t address, uint8_t *buffer, uint32_t length)
{
    int8_t err_code;
    int i;

    //printf("SRAM_23LC1024_write entered\r\n");
    //printf("addr = %08x, buffer = %p, length = %d\r\n", address, buffer, length);
	
	SPI_Data_Tx_Array[SRAM_23LC1024_INSTR_OFFSET] = SRAM_23LC1024_ACTION_WRITE;
	SPI_Data_Tx_Array[SRAM_23LC1024_INSTR_OFFSET + 1] = ((address & 0xFF0000) >> 16);
	SPI_Data_Tx_Array[SRAM_23LC1024_INSTR_OFFSET + 2] = ((address & 0x00FF00) >> 8);
	SPI_Data_Tx_Array[SRAM_23LC1024_INSTR_OFFSET + 3] = ((address & 0x0000FF));
	
    if ((length + SRAM_23LC1024_HEADER_LENGTH) < SPI_MAX_BUFFER_SIZE) {
		SPI_Select(SPI_DEVICE_TYPE_23LC1024);
	
		memcpy(&SPI_Data_Tx_Array[SRAM_23LC1024_HEADER_LENGTH], buffer, length);
		bytes_trans = length + SRAM_23LC1024_HEADER_LENGTH;
		SPI_Array_ReadWrite();

        err_code = SRAM_23LC1024_STATUS_ERROR_NORMAL;
    } else {
        err_code = SRAM_23LC1024_STATUS_ERROR_NO_MEM;
    }

    return err_code;
}

uint8_t *SRAM_23LC1024_read(uint32_t address, uint32_t length)
{
    int8_t err_code;
    int i;

    //printf("SRAM_23LC1024_read entered\r\n");
    //printf("addr = %08x, length = %d\r\n", address, length);
	
	SPI_Data_Tx_Array[SRAM_23LC1024_INSTR_OFFSET] = SRAM_23LC1024_ACTION_READ;
	SPI_Data_Tx_Array[SRAM_23LC1024_INSTR_OFFSET + 1] = ((address & 0xFF0000) >> 16);
	SPI_Data_Tx_Array[SRAM_23LC1024_INSTR_OFFSET + 2] = ((address & 0x00FF00) >> 8);
	SPI_Data_Tx_Array[SRAM_23LC1024_INSTR_OFFSET + 3] = ((address & 0x0000FF));
	
    if ((length + SRAM_23LC1024_HEADER_LENGTH) < SPI_MAX_BUFFER_SIZE) {
		SPI_Select(SPI_DEVICE_TYPE_23LC1024);
		
		bytes_trans = length + SRAM_23LC1024_HEADER_LENGTH;
		SPI_Array_ReadWrite();
        return &SPI_Data_Tx_Array[SRAM_23LC1024_HEADER_LENGTH];
    }

	// otherwise read length exceeds SPI buffer size
    return NULL;
}