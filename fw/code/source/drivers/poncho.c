#include "drivers/poncho.h"

/* This is a map specific to a hardware revision of the poncho expander
 * board.  The first field `buddy_pin` is the key specifying the Buddy DAQ
 * output and the `ctrl_a_pin` and `ctrl_b_pin` are a tuple of the values
 * that need to be stored in CTRLA and CTRLB registers on the TCA9555 chip.
 */
static poncho_pin_cfg_t poncho_cfg[] = {
    { BUDDY_CHAN_0, TCA9555_PIN_7, TCA9555_PIN_2 },
    { BUDDY_CHAN_1, TCA9555_PIN_6, TCA9555_PIN_3 },
    { BUDDY_CHAN_2, TCA9555_PIN_4, TCA9555_PIN_1 },
    { BUDDY_CHAN_3, TCA9555_PIN_5, TCA9555_PIN_0 },
    { BUDDY_CHAN_4, TCA9555_PIN_0, TCA9555_PIN_4 },
    { BUDDY_CHAN_5, TCA9555_PIN_1, TCA9555_PIN_5 },
    { BUDDY_CHAN_6, TCA9555_PIN_3, TCA9555_PIN_6 },
    { BUDDY_CHAN_7, TCA9555_PIN_2, TCA9555_PIN_7 },
};

/// set to true when device initialization has run otherwise false.  Initialization 
/// must be run before the driver can be used.
static bool poncho_device_init = false;

/**
 * @brief Set the operating mode (IN or OUT) of the Poncho expander board.  The IN mode is
 *			  used for input signals (ADC & Counter).  The OUT mode is used for DAC and PWM output
 *			  applications.
 * @param pin enum of type BUDDY_CHANNELS specifying the channel index
 * @param pos enum of type TCA9555_PIN_VALUE specifying if a high or low pin state is requested
 * @return PONCHO_ERROR_CODE_OK on success, otherwise see PONCHO_ERROR_CODE.
 */
int8_t poncho_set_mode(uint8_t pin, uint8_t pos) {
    int i;
    
    for (i = 0; i < (sizeof(poncho_cfg) / sizeof(poncho_cfg[0])); i++) {
        if (poncho_cfg[i].buddy_pin == pin) {
            if (tca9555_set_port_pin(TCA9555_PORT_0, poncho_cfg[i].ctrl_a_pin, pos) != TCA9555_ERROR_CODE_OK) {
							return PONCHO_ERROR_CODE_GENERAL_ERROR;
						}
						
            Delay();
            
            if (tca9555_set_port_pin(TCA9555_PORT_1, poncho_cfg[i].ctrl_b_pin, pos) != TCA9555_ERROR_CODE_OK) {
							return PONCHO_ERROR_CODE_GENERAL_ERROR;
						}
						
            Delay();
        }
    }
    
    return PONCHO_ERROR_CODE_OK;
}

/**
 * @brief Set the requested buddy pin to an output mode.
 * @param pin enum of type BUDDY_CHANNELS specifying the channel index
 * @return PONCHO_ERROR_CODE_OK on success, otherwise see PONCHO_ERROR_CODE.
 */
int8_t poncho_set_out_mode(uint8_t pin) {
    return poncho_set_mode(pin, TCA9555_PIN_VALUE_LOW);
}

/**
 * @brief Set the requested buddy pin to an output mode.
 * @param pin enum of type BUDDY_CHANNELS specifying the channel index
 * @return PONCHO_ERROR_CODE_OK on success, otherwise see PONCHO_ERROR_CODE.
 */
int8_t poncho_set_in_mode(uint8_t pin) {
    return poncho_set_mode(pin, TCA9555_PIN_VALUE_HIGH);
}

/**
 * @brief configure the poncho expander board
 * 
 * @param mode enum of type BUDDY_EXPANDER_PONCHO_MODE specifying if poncho
 *  expander pins are to be treated as out or inputs.
 * @param pin_state bitmask describing channels to activate for the given
 *  functionality specified in the mode output parameter.
 * @return int8_t PONCHO_ERROR_CODE_OK on success, otherwise error.
 */
int8_t poncho_configure(uint8_t mode, uint8_t pin_state) {
		int i;

		for (i = BUDDY_CHAN_0; i < BUDDY_CHAN_7; i++) {
			if (!(pin_state & (1 << (uint8_t) i))) {
				continue;
			}
			
			if (mode == BUDDY_EXPANDER_PONCHO_MODE_OUT) {
				poncho_set_out_mode(i);
			} else if (mode == BUDDY_EXPANDER_PONCHO_MODE_IN) {
				poncho_set_in_mode(i);
			} else {
                return PONCHO_ERROR_CODE_GENERAL_ERROR;
			}
		}
		
		return PONCHO_ERROR_CODE_OK;
}

/**
 * @brief Put the poncho board in a default pin state.  The default pin state should
 *        make the IN mode active so that we never have big rail voltages accidentally
 *			  hanging off the terminal jack connectors.
 * @return PONCHO_ERROR_CODE_OK on success, otherwise see PONCHO_ERROR_CODE.
 */
int8_t poncho_init(void) {
	uint8_t pins[] = {
		BUDDY_CHAN_0,
		BUDDY_CHAN_1,
		BUDDY_CHAN_2,
		BUDDY_CHAN_3,
		BUDDY_CHAN_4,
		BUDDY_CHAN_5,
		BUDDY_CHAN_6,
		BUDDY_CHAN_7
	};
	int i;
	
	if (poncho_device_init) {
		return PONCHO_ERROR_CODE_OK;
	}
	
	printf("poncho_init: init tca9555\n");
	if (tca9555_init() != TCA9555_ERROR_CODE_OK) {
		return PONCHO_ERROR_CODE_GENERAL_ERROR;
	}
	
	for (i = 0; i < (sizeof(pins) / sizeof(pins[0])); i++) {
		if (poncho_set_in_mode(pins[i]) != PONCHO_ERROR_CODE_OK) {
		  return PONCHO_ERROR_CODE_GENERAL_ERROR;
		}			
	}
	
	printf("poncho_init: expander initialized\n");
	poncho_device_init = true;
	return PONCHO_ERROR_CODE_OK;
}