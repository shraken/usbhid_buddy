/**
 * @file process.h
 * @author Nicholas Shrake <shraken@gmail.com>
 *
 * @date 2017-12-10
 * @brief Handles the incoming configuration and data messages
 *				from the host.  Prepares the configuration response
 *				and sends the data messages back to the host.  
 *			
 */

#ifndef  _PROCESS_H_
#define  _PROCESS_H_

#include <stdint.h>

/** @brief Routine that is called an infinite loop by the firmware.  It executes the proess_out and process_in messages.
 *  @return 0 on sucess, -1 on error.
 */
void process();

#endif