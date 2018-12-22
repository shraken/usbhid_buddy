#ifndef  _IO_H_
#define  _IO_H_

#include <stdint.h>
#include <string.h>
#include <adc.h>
#include <counter.h>
#include <pwm.h>
#include <tlv563x.h>

#include <F3xx_USB0_ReportHandler.h>
#include <F3xx_USB0_InterruptServiceRoutine.h>
#include <c8051f3xx.h>

#include <globals.h>
#include "buddy_common.h"

extern unsigned char xdata OUT_PACKET[];
extern unsigned char xdata IN_PACKET[];

extern unsigned char xdata *P_IN_PACKET_SEND;

extern buddy_ctx_t buddy_ctx;

extern int16_t data adc_results[MAX_ANALOG_INPUTS];

extern uint8_t data in_packet_ready;

extern uint8_t new_dac_packet;
extern uint8_t new_pwm_packet;
extern uint8_t flag_usb_out;

extern code firmware_info_t fw_info;

extern unsigned char xdata *P_IN_PACKET_RECORD;
extern unsigned char xdata in_packet_record_cycle;

void io_init(void);
void usb_buffer_clear(void);
void respond_data(uint8_t *buffer, uint8_t length);
void respond_status(int8_t error_code);
void build_adc_packet(void);
void build_counter_packet(void);
void execute_out_stream(void);
void execute_out(void);

#endif