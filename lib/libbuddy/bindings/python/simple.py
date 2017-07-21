#!/usr/bin/env python

import buddy as bt
import sys
import datetime
import argparse
import time
import signal

BUDDY_TEST_ADC_FREQ = 1000      # 1 kHz
BUDDY_TEST_DAC_FREQ = 1000      # 5 Hz

hid_handle = None
hid_info = None

def test_seq_dac(handle, fw_info, sample_rate, streaming, oneshot):
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()

    general_settings.function = bt.GENERAL_CTRL_DAC_ENABLE
    general_settings.mode = \
        bt.MODE_CTRL_STREAM if streaming else bt.MODE_CTRL_IMMEDIATE
    general_settings.operation = \
        bt.OPER_CTRL_ONESHOT if oneshot else bt.OPER_CTRL_CONTINUOUS
    general_settings.queue = bt.QUEUE_CTRL_WAIT
    general_settings.channel_mask = bt.BUDDY_CHAN_ALL_MASK
    #general_settings.channel_mask = bt.BUDDY_CHAN_0_MASK
    #general_settings.channel_mask = bt.BUDDY_CHAN_0_MASK | bt.BUDDY_CHAN_1_MASK
    general_settings.resolution = bt.CODEC_BIT_WIDTH_12

    timing_settings.period = bt.FREQUENCY_TO_NSEC(sample_rate)

    runtime_settings.dac_power = bt.RUNTIME_DAC_POWER_ON
    runtime_settings.dac_ref = bt.RUNTIME_DAC_REF_EXT

    if (bt.buddy_configure(handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_OK):
        print 'test_seq_dac: could not configure Buddy device'
        return -1

    time.sleep(0.1)

    packet = bt.general_packet_t()
    test_seq_dac_count = 0

    for k in range(0, ((1 << general_settings.resolution) - 1)):
        for i in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7):
            bt.uint16_t_ptr_setitem(packet.channels, i, k)

        print 'test_seq_dac, sending %d packet with value %d' % \
                (test_seq_dac_count, k)
        test_seq_dac_count += 1

        if (bt.buddy_send_dac(handle,
                              packet,
                              streaming) != bt.BUDDY_ERROR_OK):
            print 'test_seq_dac: could not send DAC packet'
            return -1

        if not streaming:
            time.sleep(1.0 / sample_rate)

    if not bt.codec_buffer_empty():
        print 'test_seq_dac: flushing the buffer'
        bt.buddy_flush(handle)
    time.sleep(0.1)

    if oneshot:
        if bt.buddy_trigger(handle) != bt.BUDDY_ERROR_OK:
            return -1
        time.sleep(0.1)

    return 0

def test_seq_adc(handle, fw_info, sample_rate, streaming, oneshot):
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()
    packet = bt.general_packet_t()

    general_settings.function = bt.GENERAL_CTRL_ADC_ENABLE
    general_settings.mode = \
        bt.MODE_CTRL_STREAM if streaming else bt.MODE_CTRL_IMMEDIATE
    general_settings.operation = \
        bt.OPER_CTRL_ONESHOT if oneshot else bt.OPER_CTRL_CONTINUOUS
    general_settings.queue = bt.QUEUE_CTRL_SATURATE
    general_settings.channel_mask = bt.BUDDY_CHAN_ALL_MASK
    #general_settings.channel_mask = bt.BUDDY_CHAN_6_MASK
    general_settings.resolution = bt.CODEC_BIT_WIDTH_12

    timing_settings.period = bt.FREQUENCY_TO_NSEC(sample_rate)
    timing_settings.averaging = 1

    runtime_settings.adc_ref = bt.RUNTIME_ADC_REF_VDD

    print 'timing_settings.period = %d (0x%x)' % (timing_settings.period, timing_settings.period)

    if (bt.buddy_configure(handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_OK):
        print 'test_seq_adc: could not configure Buddy device'
        return -1

    time.sleep(0.1)

    if oneshot:
        if bt.buddy_trigger(handle) != bt.BUDDY_ERROR_OK:
            return -1
        time.sleep(0.1)

    recv_packets = 0;
    for i in range(0, 1200):
        err_code = bt.buddy_read_adc(handle, packet, streaming)

        if err_code == bt.BUDDY_ERROR_OK:
            print 'test_seq_adc: received packet %d' % recv_packets
            for j in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7 + 1):
                if (general_settings.channel_mask & (1 << j)):
                    print 'packet.channels[%d] = %d' % \
                        (j, bt.uint16_t_ptr_getitem(packet.channels, j))

            recv_packets += 1
        elif err_code == bt.BUDDY_ERROR_GENERAL:
            print 'test_seq_adc: could not read ADC packet'
            print 'err_code = %d' % err_code
            return -1
    
    return 0;

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

    if (fw_info.type_ext_memory < bt.FIRMWARE_INFO_MEM_TYPE_LENGTH):
        print 'Firmware External Memory type: %d - %s' % \
            (fw_info.type_ext_memory,
             bt.char_ptr_getitem(bt.cvar.fw_info_mem_type_names, fw_info.type_ext_memory))
    else:
        print 'Firmware External Memory type: %d' % fw_info.type_ext_memory

    print ''


def signal_handler(signal, frame):
    print('You pressed Ctrl+C!')
    bt.buddy_cleanup(hid_handle, hid_info)
    sys.exit(0)

if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_handler)

    parser = argparse.ArgumentParser(description='Test the DAC and ADC function'
                                                 'of the Buddy DAQ device')

    parser.add_argument('-d,--dac', action='store_true', dest='dac_mode',
                        help='enable DAC (data to analog) conversion')
    parser.add_argument('-a,--adc', action='store_true', dest='adc_mode',
                        help='enable ADC (analog to digital) conversion')
    parser.add_argument('-p,--dual', action='store_true', dest='dual_mode',
                        help='enable dual DAC and ADC conversion')
    parser.add_argument('-s,--stream', action='store_true', dest='stream_mode',
                        help='enable streaming for higher throughput')
    parser.add_argument('-o,--oneshot', action='store_true', dest='oneshot_mode',
                        help='enable oneshot mode for triggered conversion')
    args = parser.parse_args()

    if args.dac_mode and args.adc_mode:
        print 'ERROR: must select either DAC or ADC mode, not both'
        sys.exit()

    if (not args.dac_mode) and (not args.adc_mode) and (not args.dual_mode):
        print 'ERROR: must select DAC, ADC, or dual mode'
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

    if args.dac_mode:
        print 'running a DAC mode test'
        err_code = test_seq_dac(hid_handle,
                                fw_info,
                                BUDDY_TEST_DAC_FREQ,
                                args.stream_mode,
                                args.oneshot_mode)

    if args.adc_mode:
        print 'running a ADC mode test'
        err_code = test_seq_adc(hid_handle,
                                fw_info,
                                BUDDY_TEST_ADC_FREQ,
                                args.stream_mode,
                                args.oneshot_mode)

    time_end = time.time()
    time_diff = time_end - time_start
    print 'Test took (%f) seconds  to run' % time_diff

    bt.buddy_cleanup(hid_handle, hid_info)