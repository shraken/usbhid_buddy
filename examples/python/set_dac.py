#!/usr/bin/env python

import sys
import datetime
import argparse
import time
import signal
import csv
import buddy as bt

BUDDY_TEST_DAC_FREQ = 50       # 5 Hz

hid_handle = None
hid_info = None

def reset_device(hid_handle):
    bt.buddy_reset_device(hid_handle)

def set_dac_value(hid_handle, channel, value):
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()

    general_settings.function = bt.GENERAL_CTRL_DAC_ENABLE
    general_settings.mode = bt.MODE_CTRL_STREAM
    general_settings.channel_mask = (1 << channel)
    general_settings.resolution = bt.RESOLUTION_CTRL_HIGH

    timing_settings.period = bt.FREQUENCY_TO_NSEC(BUDDY_TEST_DAC_FREQ)

    runtime_settings.dac_power = bt.RUNTIME_DAC_POWER_ON
    runtime_settings.dac_ref = bt.RUNTIME_DAC_REF_EXT

    if (bt.buddy_configure(hid_handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        print 'set_dac_value: could not configure Buddy device'
        return -1

    time.sleep(0.1)

    packet = bt.general_packet_t()
    
    for i in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7 + 1):
        bt.int32_t_ptr_setitem(packet.channels, i, value)

    print 'set_dac_value: setting DAC channel %d with value %d' %  (channel, value)

    if (bt.buddy_send_dac(hid_handle, packet, True) != bt.BUDDY_ERROR_CODE_OK):
            print 'test_seq_dac: could not send DAC packet'
            return -1

    time.sleep(1.0 / BUDDY_TEST_DAC_FREQ)

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

    parser.add_argument('-c,--channel', nargs=1, dest="dac_channel",
                        help="DAC channel that the value is set on", required=True)
    parser.add_argument('-v,--value', nargs=1, dest="dac_value",
                        help="DAC value to set on given channel", required=True)
    args = parser.parse_args()

    if ((args.dac_channel is None) or (args.dac_value is None)):
        print 'ERROR: must specify DAC channel and DAC value'
        sys.exit()

    hid_info = bt.buddy_hid_info_t()
    fw_info = bt.firmware_info_t()

    hid_handle = bt.buddy_init(hid_info, fw_info)

    if hid_handle is None:
        print 'could not open Buddy HID device'
        sys.exit()

    display_usb_info(hid_info)
    display_fw_info(fw_info)

    print 'Setting DAC value'
    err_code = set_dac_value(hid_handle, int(args.dac_channel[0]), int(args.dac_value[0]))
    bt.buddy_cleanup(hid_handle, hid_info, False)