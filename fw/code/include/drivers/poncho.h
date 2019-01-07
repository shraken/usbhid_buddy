/**
 * @file poncho.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2018-09-24
 * @brief Wiggle Labs Poncho Expander Board Driver
 *			
 */

#ifndef  _PONCHO_H_
#define  _PONCHO_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <compiler_defs.h>
#include <C8051F380_defs.h>
#include "globals.h"
#include "tca9555.h"
#include "init.h"

/**
 * @brief Poncho Expander driver error codes
 */
typedef enum _PONCHO_ERROR_CODE {
	PONCHO_ERROR_CODE_OK = 0,
	PONCHO_ERROR_CODE_GENERAL_ERROR = -1,
} PONCHO_ERROR_CODE;

/**
 * \struct poncho_pin_cfg_t
 * \brief defines the pin mapping from equivalent buddy simple pins from the
 *        BUDDY_CHANNELS enum to the poncho board CtrlA and CtrlB PCA9555
 *        pin mux.  
 */
typedef struct _poncho_pin_cfg_t {
	uint8_t buddy_pin;     /* enum of BUDDY_CHANNELS defining the channel */
    uint8_t ctrl_a_pin;    /* the PCA9555 CTRL_A numbered pin */
    uint8_t ctrl_b_pin;    /* the PCA9555 CTRL_B numbered pin */
} poncho_pin_cfg_t;

int8_t poncho_init(void);
int8_t poncho_configure(uint8_t mode, uint8_t pin_state);
int8_t poncho_set_mode(uint8_t pin, uint8_t pos);
int8_t poncho_set_out_mode(uint8_t pin);
int8_t poncho_set_in_mode(uint8_t pin);
int8_t poncho_default_config(void);

#endif /* _PONCHO_H_ */