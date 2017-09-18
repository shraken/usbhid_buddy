#!/usr/bin/env python

import buddy as bt
import sys
import datetime
import argparse
import time
import signal
import numpy as np
from scipy import signal as scisig

BUDDY_TEST_DAC_FREQ = 10000     # 1000 Hz
WAVEFORM_TIME = 20             # 20 seconds
WAVEFORM_FREQUENCY = 10         # 1 Hz

hid_handle = None
hid_info = None

def test_waveform_dac(handle, fw_info, sample_rate, wave_type, streaming):
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()

    general_settings.function = bt.GENERAL_CTRL_DAC_ENABLE
    general_settings.mode = \
        bt.MODE_CTRL_STREAM if streaming else bt.MODE_CTRL_IMMEDIATE
    #general_settings.channel_mask = bt.BUDDY_CHAN_ALL_MASK
    general_settings.channel_mask = bt.BUDDY_CHAN_0_MASK
    general_settings.resolution = bt.RESOLUTION_CTRL_HIGH

    timing_settings.period = bt.FREQUENCY_TO_NSEC(sample_rate)

    runtime_settings.dac_power = bt.RUNTIME_DAC_POWER_ON
    runtime_settings.dac_ref = bt.RUNTIME_DAC_REF_EXT

    if (bt.buddy_configure(handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_OK):
        print 'test_waveform_dac: could not configure Buddy device'
        return -1

    time.sleep(0.1)

    packet = bt.general_packet_t()
    test_seq_dac_count = 0
    
    #y_mag = ((1 << general_settings.resolution) - 1)
    #y_mag = 255
    y_mag = 4095
    t = np.linspace(0, WAVEFORM_TIME, sample_rate * WAVEFORM_TIME, endpoint=False)

    if wave_type == 'square':    
        y = ((scisig.square(np.pi * 2 * WAVEFORM_FREQUENCY * t) + 1) / 2) * y_mag
    elif wave_type == 'sine':
        y = ((np.sin(np.pi * 2 * WAVEFORM_FREQUENCY * t) + 1) / 2) * y_mag
    elif wave_type == 'sawtooth':
        y = ((scisig.sawtooth(np.pi * 2 * WAVEFORM_FREQUENCY * t) + 1) / 2) * y_mag
    else:
        return -1

    for k in y:
        for i in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7):
            bt.uint16_t_ptr_setitem(packet.channels, i, int(k))

        print 'test_waveform_dac: sending %d packet with value %d' % (test_seq_dac_count, k)
        test_seq_dac_count += 1

        if (bt.buddy_send_dac(handle,
                              packet,
                              streaming) != bt.BUDDY_ERROR_OK):
            print 'test_waveform_dac: could not send DAC packet'
            return -1

        if not streaming:
            time.sleep(1.0 / sample_rate)

    bt.buddy_flush(handle)
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

    parser = argparse.ArgumentParser(description='Test the DAC and ADC function'
                                                 'of the Buddy DAQ device')

    parser.add_argument('-s,--stream', action='store_true', dest='stream_mode',
                        help='enable streaming for higher throughput')
    parser.add_argument('-t,--type', action='store', dest='wave_type',
                        help='specify type of waveform to generate (sine, square, triangle, noise, etc.)')
    args = parser.parse_args()

    if args.wave_type is None:
        print 'ERROR: must select a wave type (sine, square, sawtooth, noise, etc.)'
        sys.exit()

    hid_info = bt.buddy_hid_info_t()
    fw_info = bt.firmware_info_t()

    hid_handle = bt.buddy_init(hid_info, fw_info)

    if hid_handle is None:
        print 'could not open Buddy HID device'
        sys.exit()

    display_usb_info(hid_info)
    display_fw_info(fw_info)

    time_start = time.time()

    err_code = test_waveform_dac(hid_handle,
                                fw_info,
                                BUDDY_TEST_DAC_FREQ,
                                args.wave_type,
                                args.stream_mode)
    
    time_end = time.time()
    time_diff = time_end - time_start
    print 'Test took (%f) seconds  to run' % time_diff

    bt.buddy_cleanup(hid_handle, hid_info)