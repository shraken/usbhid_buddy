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

def buddy_get_adc_value(hid_handle, channel, sample_rate=100, streaming=True):
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()
    packet = bt.general_packet_t()

    general_settings.function = bt.GENERAL_CTRL_ADC_ENABLE
    general_settings.mode = bt.MODE_CTRL_STREAM
    general_settings.channel_mask = (1 << int(channel))
    general_settings.resolution = bt.RESOLUTION_CTRL_HIGH

    timing_settings.period = bt.FREQUENCY_TO_NSEC(sample_rate)
    timing_settings.averaging = 1

    runtime_settings.adc_mode = bt.RUNTIME_ADC_MODE_SINGLE_ENDED
    runtime_settings.adc_ref = bt.RUNTIME_ADC_REF_VDD

    if (bt.buddy_configure(hid_handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        return -1

    time.sleep(0.1)

    adc_value = -1
    for i in range(0, 100):
        err_code = bt.buddy_read_adc(hid_handle, packet, streaming)

        if err_code == bt.BUDDY_ERROR_CODE_OK:
            adc_value = bt.int32_t_ptr_getitem(packet.channels, int(channel))

    return adc_value

def buddy_get_adc_value_old(hid_handle, channel, sample_rate=1, streaming=False):
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()
    packet = bt.general_packet_t()

    general_settings.function = bt.GENERAL_CTRL_ADC_ENABLE
    general_settings.mode = bt.MODE_CTRL_IMMEDIATE
    general_settings.channel_mask = (1 << int(channel))
    general_settings.resolution = bt.RESOLUTION_CTRL_HIGH

    timing_settings.period = bt.FREQUENCY_TO_NSEC(sample_rate)
    timing_settings.averaging = 1

    runtime_settings.adc_mode = bt.RUNTIME_ADC_MODE_SINGLE_ENDED
    runtime_settings.adc_ref = bt.RUNTIME_ADC_REF_VDD

    if (bt.buddy_configure(hid_handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        return -1

    # @todo: bug workaround, we have to read one sample out and toss it.  it sticks
    # to previous ADC value for some reason
    for index in range(0,5):
        err_code = bt.buddy_read_adc(hid_handle, packet, streaming)
        
    l = []
    for index in range(0,5):
        err_code = bt.buddy_read_adc(hid_handle, packet, streaming)
        adc_value = bt.int32_t_ptr_getitem(packet.channels, int(channel))
        l.append(adc_value)

    adc_mean = sum(l) / float(len(l))
    return adc_mean

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

    parser.add_argument('-c,--channel', nargs=1, dest="adc_channel",
                        help="ADC channel that the value is set on", required=True)
    args = parser.parse_args()

    if ((args.adc_channel is None)):
        print 'ERROR: must specify ADC channel'
        sys.exit()

    hid_info = bt.buddy_hid_info_t()
    fw_info = bt.firmware_info_t()

    hid_handle = bt.buddy_init(hid_info, fw_info)

    if hid_handle is None:
        print 'could not open Buddy HID device'
        sys.exit()

    display_usb_info(hid_info)
    display_fw_info(fw_info)

    adc_value = buddy_get_adc_value(hid_handle, int(args.adc_channel[0]))
    print('adc_value = {}'.format(adc_value))
    bt.buddy_cleanup(hid_handle, hid_info, False)