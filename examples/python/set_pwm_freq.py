#!/usr/bin/env python

import sys
import datetime
import argparse
import time
import signal
import csv
import buddy as bt

# base PWM frequency (in Hz)
BUDDY_TEST_PWM_FREQ = 10

hid_handle = None
hid_info = None

def reset_device(hid_handle):
    bt.buddy_reset_device(hid_handle)

def set_pwm_value(hid_handle, channel, value, poncho_mode):
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()

    general_settings.function = bt.GENERAL_CTRL_PWM_ENABLE
    general_settings.mode = bt.MODE_CTRL_IMMEDIATE
    general_settings.channel_mask = (1 << channel)
    general_settings.resolution = bt.RESOLUTION_CTRL_HIGH

    timing_settings.period = bt.FREQUENCY_TO_NSEC(BUDDY_TEST_PWM_FREQ)

    runtime_settings.pwm_mode = bt.RUNTIME_PWM_MODE_FREQUENCY
    runtime_settings.pwm_timebase = bt.RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_12
    
    if poncho_mode:
        general_settings.expander_type = bt.BUDDY_EXPANDER_TYPE_PONCHO
        general_settings.expander_mode = bt.BUDDY_EXPANDER_PONCHO_MODE_OUT
        general_settings.expander_pin_state = (1 << channel)
    else:
        general_settings.expander_type = bt.BUDDY_EXPANDER_TYPE_BASE

    if (bt.buddy_configure(hid_handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        print 'set_pwm_value: could not configure Buddy device'
        return -1

    time.sleep(0.1)

    packet = bt.general_packet_t()
    
    for i in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7 + 1):
        bt.int32_t_ptr_setitem(packet.channels, i, value)

    print 'set_pwm_value: setting PWM channel %d with value %d' %  (channel, value)

    if (bt.buddy_send_pwm(hid_handle, packet, False) != bt.BUDDY_ERROR_CODE_OK):
        print 'set_pwm_value: could not send PWM packet'
        return -1

    time.sleep(1.0 / BUDDY_TEST_PWM_FREQ)

    bt.buddy_flush(hid_handle)
    time.sleep(0.1)

    return 0

def display_usb_info(hid_info):
    print 'USB Manufacturer String: %s' % hid_info.str_mfr
    print 'USB Product String: %s' % hid_info.str_product
    print 'USB Serial Number String: %s' % hid_info.str_serial
    print 'USB Indexed String 1: %s' % hid_info.str_index_1
    print ''

def display_fw_info(fw_info):
    print 'Firmware Serial Number: %d (0x%x)' % \
        (fw_info.serial, fw_info.serial)
    print 'Firmware Revision: %d.%d.%d' % \
        (fw_info.fw_rev_major, fw_info.fw_rev_minor, fw_info.fw_rev_tiny)
    print 'Firmware Bootload Revision: %d.%d.%d' % \
        (fw_info.bootl_rev_major, fw_info.bootl_rev_minor, fw_info.bootl_rev_tiny)

    print 'Firmware Flash datetime: %s' % \
        datetime.datetime.fromtimestamp(fw_info.flash_datetime).strftime('%Y-%m-%d %H:%M:%S')

    if (fw_info.type_dac < bt.FIRMWARE_INFO_DAC_TYPE_LENGTH):
        print 'Firmware DAC type: %d - %s' % \
            (fw_info.type_dac,
             bt.char_ptr_getitem(bt.cvar.fw_info_dac_type_names, fw_info.type_dac))
    else:
        print 'Firmware DAC type: %d' % fw_info.type_dac

    print ''

def signal_handler(signal, frame):
    print('You pressed Ctrl+C!')
    bt.buddy_cleanup(hid_handle, hid_info)
    sys.exit(0)

if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_handler)

    parser = argparse.ArgumentParser(description='Quick set DAC channel values')

    parser.add_argument('-c,--channel', nargs=1, dest="pwm_channel",
                        help="PWM channel that the value is set on", required=True)
    parser.add_argument('-p,--poncho', action='store_true', dest='poncho_mode',
                        help='Enable the Poncho Expander board')
    parser.add_argument('-v,--value', nargs=1, dest="pwm_value",
                        help="PWM value to set on given channel", required=True)
    args = parser.parse_args()

    if ((args.pwm_channel is None) or (args.pwm_value is None)):
        print 'ERROR: must specify PWM channel and PWM value'
        sys.exit()

    hid_info = bt.buddy_hid_info_t()
    fw_info = bt.firmware_info_t()

    hid_handle = bt.buddy_init(hid_info, fw_info)

    if hid_handle is None:
        print 'could not open Buddy HID device'
        sys.exit()

    display_usb_info(hid_info)
    display_fw_info(fw_info)

    print 'Setting PWM value'
    err_code = set_pwm_value(hid_handle, int(args.pwm_channel[0]), int(args.pwm_value[0]), args.poncho_mode)
    # reset_device(hid_handle)

    bt.buddy_cleanup(hid_handle, hid_info, False)