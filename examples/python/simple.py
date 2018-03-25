#!/usr/bin/env python

import sys
import datetime
import argparse
import time
import signal
import csv
import buddy as bt

BUDDY_TEST_ADC_FREQ = 1000      # 1 kHz
BUDDY_TEST_DAC_FREQ = 1000       # 5 Hz
BUDDY_TEST_PWM_FREQ = 10       # 1 kHz
BUDDY_TEST_COUNTER_FREQ = 1000     # 10 Hz
hid_handle = None
hid_info = None

'''
    configures buddy device for PWM frequency operation.  The pwm_timebase field
    of the ctrl_runtime_t defines the base tick frequency.  The resolution is
    specifies if an 8, 16, or 32-bit value will be sent. 
'''
def test_seq_pwm_freq(handle, sample_rate, streaming):
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()

    general_settings.function = bt.GENERAL_CTRL_PWM_ENABLE
    general_settings.mode = \
        bt.MODE_CTRL_STREAM if streaming else bt.MODE_CTRL_IMMEDIATE
    general_settings.channel_mask = bt.BUDDY_CHAN_0_MASK
    #general_settings.resolution = bt.RESOLUTION_CTRL_LOW
    general_settings.resolution = bt.RESOLUTION_CTRL_HIGH
    #general_settings.resolution = bt.RESOLUTION_CTRL_SUPER

    timing_settings.period = bt.FREQUENCY_TO_NSEC(sample_rate)

    runtime_settings.pwm_mode = bt.RUNTIME_PWM_MODE_FREQUENCY
    runtime_settings.pwm_timebase = bt.RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_12

    if (bt.buddy_configure(handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        print 'test_seq_pwm_freq: could not configure Buddy device'
        return -1

    time.sleep(0.1)

    packet = bt.general_packet_t()
    test_seq_pwm_count = 0

    #for k in range(50000, (50000 + 1)):
    #for k in range(25000, (25000 + 1)):
    #for k in range(25000, (25000 + 1)):
    #for k in range(1000, 50000):
    #for k in range(100000, (100000 + 1)):
    for k in range(45000, (45000 +1)):
        for i in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7 + 1):
            bt.int32_t_ptr_setitem(packet.channels, i, k)

        print 'test_seq_pwm_freq: sending %d packet with value %d' % \
                (test_seq_pwm_count, k)
        test_seq_pwm_count += 1

        if (bt.buddy_send_pwm(handle,
                              packet,
                              streaming) != bt.BUDDY_ERROR_CODE_OK):
            print 'test_seq_pwm_freq: could not send PWM packet'
            return -1

        if not streaming:
            time.sleep(1.0 / sample_rate)

    bt.buddy_flush(handle)
    time.sleep(0.1)

    return 0

'''
    configures buddy device for PWM duty cycle operation.  A 16-bit or 8-bit
    value is sent to control the duty cycle of a fixed frequency controlled
    by the pwm_timebase field of the ctrl_runtime_t structure.
'''
def test_seq_pwm_duty(handle, sample_rate, streaming):
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()

    general_settings.function = bt.GENERAL_CTRL_PWM_ENABLE
    general_settings.mode = \
        bt.MODE_CTRL_STREAM if streaming else bt.MODE_CTRL_IMMEDIATE
    general_settings.channel_mask = bt.BUDDY_CHAN_0_MASK
    general_settings.resolution = bt.RESOLUTION_CTRL_HIGH
    #general_settings.resolution = bt.RESOLUTION_CTRL_LOW

    timing_settings.period = bt.FREQUENCY_TO_NSEC(sample_rate)

    runtime_settings.pwm_mode = bt.RUNTIME_PWM_MODE_DUTY_CYCLE
    runtime_settings.pwm_timebase = bt.RUNTIME_PWM_TIMEBASE_SYSCLK

    if (bt.buddy_configure(handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        print 'test_seq_pwm_duty: could not configure Buddy device'
        return -1

    time.sleep(0.1)

    packet = bt.general_packet_t()
    test_seq_pwm_count = 0

    #for k in range(1, 65535, 10):
    #for k in range(128, 129):
    #for k in range(63,64):
    #for k in range(191, 192):
    for k in range(32767, 32768):
    #for k in range(16383, 16384):
        for i in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7 + 1):
            bt.int32_t_ptr_setitem(packet.channels, i, k)

        print 'test_seq_pwm_duty: sending %d packet with value %d' % \
                (test_seq_pwm_count, k)
        test_seq_pwm_count += 1

        if (bt.buddy_send_pwm(handle,
                              packet,
                              streaming) != bt.BUDDY_ERROR_CODE_OK):
            print 'test_seq_pwm_duty: could not send PWM packet'
            return -1

        if not streaming:
            time.sleep(1.0 / sample_rate)

    bt.buddy_flush(handle)
    time.sleep(0.1)

    return 0

'''
    configures buddy device for DAC operation.  An iterative loop over all
    the codes for a 12-bit DAC (0 - 4095) are sent.
'''
def test_seq_dac(handle, sample_rate, streaming):
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()

    general_settings.function = bt.GENERAL_CTRL_DAC_ENABLE
    general_settings.mode = \
        bt.MODE_CTRL_STREAM if streaming else bt.MODE_CTRL_IMMEDIATE
    #general_settings.channel_mask = bt.BUDDY_CHAN_ALL_MASK
    #general_settings.channel_mask = bt.BUDDY_CHAN_7_MASK
    #general_settings.channel_mask = bt.BUDDY_CHAN_0_MASK | bt.BUDDY_CHAN_1_MASK
    general_settings.channel_mask = bt.BUDDY_CHAN_0_MASK
    general_settings.resolution = bt.RESOLUTION_CTRL_HIGH

    timing_settings.period = bt.FREQUENCY_TO_NSEC(sample_rate)

    print 'timing_settings.period = '
    print timing_settings.period

    runtime_settings.dac_power = bt.RUNTIME_DAC_POWER_ON
    runtime_settings.dac_ref = bt.RUNTIME_DAC_REF_EXT

    if (bt.buddy_configure(handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        print 'test_seq_dac: could not configure Buddy device'
        return -1

    time.sleep(0.1)

    packet = bt.general_packet_t()
    test_seq_dac_count = 0

    for k in range(0, 4095 + 1):
        for i in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7 + 1):
            bt.int32_t_ptr_setitem(packet.channels, i, k)

        print 'test_seq_dac: sending %d packet with value %d' % \
                (test_seq_dac_count, k)
        test_seq_dac_count += 1

        if (bt.buddy_send_dac(handle,
                              packet,
                              streaming) != bt.BUDDY_ERROR_CODE_OK):
            print 'test_seq_dac: could not send DAC packet'
            return -1

        if not streaming:
            time.sleep(1.0 / sample_rate)

    bt.buddy_flush(handle)
    time.sleep(0.1)

    return 0

'''
    configures buddy device for counter operation where pos/neg edge ticks
    are counted on IN0 and/or IN1 pins.  A total of 1,000 tick counts are
    collected before the test is terminated. 
'''
def test_seq_counter(handle, sample_rate, streaming, log_file):
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()
    packet = bt.general_packet_t()

    general_settings.function = bt.GENERAL_CTRL_COUNTER_ENABLE
    general_settings.mode = \
        bt.MODE_CTRL_STREAM if streaming else bt.MODE_CTRL_IMMEDIATE
    #general_settings.channel_mask = bt.BUDDY_CHAN_ALL_MASK
    #general_settings.channel_mask = bt.BUDDY_CHAN_0_MASK | bt.BUDDY_CHAN_1_MASK
    general_settings.channel_mask = bt.BUDDY_CHAN_1_MASK
    
    general_settings.resolution = bt.RESOLUTION_CTRL_SUPER

    timing_settings.period = bt.FREQUENCY_TO_NSEC(sample_rate)
    runtime_settings.counter_control = bt.RUNTIME_COUNTER_CONTROL_ACTIVE_LOW

    # write CSV header
    if log_file:
        header = []
        header.append('time')
        header.append('index')

        # counter operation can only occur on channel0 and channel1
        for j in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_1 + 1):
            if (general_settings.channel_mask & (1 << j)):
                header.append('sensor %d' % j)

        try:
            log_file.writerow(header)
        except csv.Error as e:
            raise NameError('Could not write into CSV writer object')
            return False

    if (bt.buddy_configure(handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        print 'test_seq_counter: could not configure Buddy device'
        return -1

    time.sleep(0.1)

    recv_packets = 0;
    first_packet = True
    for i in range(0, 1000):
        err_code = bt.buddy_read_counter(handle, packet, streaming)

        if err_code == bt.BUDDY_ERROR_CODE_OK:
            if first_packet:
                test_time_start = time.time()
                first_packet = False

            entry = []
            entry.append('%.10f' % (time.time() - test_time_start))
            entry.append('%d' % recv_packets)

            for j in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_1 + 1):
                if (general_settings.channel_mask & (1 << j)):
                    value = bt.int32_t_ptr_getitem(packet.channels, j)
                    entry.append('%d' % value)
            recv_packets += 1

            if log_file:
                try:
                    log_file.writerow(entry)
                except csv.Error as e:
                    raise NameError('Could not write into CSV writer object')
                    return False

        elif err_code == bt.BUDDY_ERROR_CODE_GENERAL:
            print 'test_seq_counter: could not read counter packet'
            print 'err_code = %d' % err_code
            return -1
        else:
            print 'unknown error code, err_code = %d' % err_code
    
    return 0;

'''
    configures buddy device for ADC operation.  A low/high resolution field specifies
    if 8-bit or 16-bit ADC samples are to be returned.  Additionally, single ended
    or differential ADC mode is specified.   A total of 1,000 tick counts are
    collected before the test is terminated. 
'''
def test_seq_adc(handle, sample_rate, streaming, log_file):
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()
    packet = bt.general_packet_t()

    general_settings.function = bt.GENERAL_CTRL_ADC_ENABLE
    general_settings.mode = \
        bt.MODE_CTRL_STREAM if streaming else bt.MODE_CTRL_IMMEDIATE
    #general_settings.channel_mask = bt.BUDDY_CHAN_ALL_MASK
    general_settings.channel_mask = bt.BUDDY_CHAN_2_MASK
    general_settings.resolution = bt.RESOLUTION_CTRL_HIGH
    #general_settings.resolution = bt.RESOLUTION_CTRL_LOW

    timing_settings.period = bt.FREQUENCY_TO_NSEC(sample_rate)
    timing_settings.averaging = 1

    runtime_settings.adc_mode = bt.RUNTIME_ADC_MODE_SINGLE_ENDED
    #runtime_settings.adc_mode = bt.RUNTIME_ADC_MODE_DIFFERENTIAL
    runtime_settings.adc_ref = bt.RUNTIME_ADC_REF_VDD

    print 'timing_settings.period = %d (0x%x)' % (timing_settings.period, timing_settings.period)

    # write CSV header
    if log_file:
        header = []
        header.append('time')
        header.append('index')

        if runtime_settings.adc_mode == bt.RUNTIME_ADC_MODE_SINGLE_ENDED:
            for j in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7 + 1):
                if (general_settings.channel_mask & (1 << j)):
                    header.append('sensor %d' % j)
        elif runtime_settings.adc_mode == bt.RUNTIME_ADC_MODE_DIFFERENTIAL:
            for j in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_3 + 1):
                if (general_settings.channel_mask & (1 << j)):
                    header.append('sensor %d' % j)

        try:
            log_file.writerow(header)
        except csv.Error as e:
            raise NameError('Could not write into CSV writer object')
            return False

    if (bt.buddy_configure(handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        print 'test_seq_adc: could not configure Buddy device'
        return -1

    time.sleep(0.1)

    recv_packets = 0;
    first_packet = True
    for i in range(0, 1000):
        err_code = bt.buddy_read_adc(handle, packet, streaming)

        if err_code == bt.BUDDY_ERROR_CODE_OK:
            if first_packet:
                test_time_start = time.time()
                first_packet = False

            entry = []
            entry.append('%.10f' % (time.time() - test_time_start))
            entry.append('%d' % recv_packets)

            if runtime_settings.adc_mode == bt.RUNTIME_ADC_MODE_SINGLE_ENDED:
                for j in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7 + 1):
                    if (general_settings.channel_mask & (1 << j)):
                        value = bt.int32_t_ptr_getitem(packet.channels, j)
                        entry.append('%d' % value)
                        #print 'ADC chan = %d with value = %d' % (j, value)
            elif runtime_settings.adc_mode == bt.RUNTIME_ADC_MODE_DIFFERENTIAL:
                for j in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_3 + 1):
                    if (general_settings.channel_mask & (1 << j)):
                        value = bt.int32_t_ptr_getitem(packet.channels, j)
                        entry.append('%d' % value)
                        #print 'ADC chan = %d with value = %d' % (j, value)
            recv_packets += 1

            if log_file:
                try:
                    log_file.writerow(entry)
                except csv.Error as e:
                    raise NameError('Could not write into CSV writer object')
                    return False

        elif err_code == bt.BUDDY_ERROR_CODE_GENERAL:
            print 'test_seq_adc: could not read ADC packet'
            print 'err_code = %d' % err_code
            return -1
        else:
            print 'unknown error code, err_code = %d' % err_code
    
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

    print ''


def signal_handler(signal, frame):
    print('You pressed Ctrl+C!')
    bt.buddy_cleanup(hid_handle, hid_info, False)
    sys.exit(0)

if __name__ == '__main__':
    signal.signal(signal.SIGINT, signal_handler)

    parser = argparse.ArgumentParser(description='Test the DAC, ADC, and PWM function of the Buddy DAQ device')

    parser.add_argument('-d,--dac', action='store_true', dest='dac_mode',
                        help='enable DAC (data to analog) mode')
    parser.add_argument('-a,--adc', action='store_true', dest='adc_mode',
                        help='enable ADC (analog to digital) mode')
    parser.add_argument('-c,--counter', action='store_true', dest='counter_mode',
                        help='enable counter mode')
    parser.add_argument('-p,--pwm_duty', action='store_true', dest='pwm_mode_duty',
                        help='enable PWM (pulse width modulation) duty cycle mode')
    parser.add_argument('-t,--pwm_freq', action='store_true', dest='pwm_mode_freq',
                        help='enable PWM (pulse width modulation) frequency mode')
    parser.add_argument('-s,--stream', action='store_true', dest='stream_mode',
                        help='enable streaming for higher throughput')
    parser.add_argument('-l,--latch', action='store_true', dest='hold_mode',
                        help='hold the last mode and value on exit')
    parser.add_argument('-i,--info', action='store_true', dest='info_mode',
                        help='request device info and exit')
    parser.add_argument('-f,--file', nargs=1, dest="output_file",
                        help="CSV log file to be created", required=False)
    args = parser.parse_args()

    '''
    if ((args.dac_mode and args.adc_mode) or
        (args.dac_mode and args.pwm_mode_duty) or
        (args.dac_mode and )
        print 'ERROR: must select either DAC, ADC, or PWM mode!'
        parser.print_help()
        sys.exit()
    '''

    if args.output_file and (args.adc_mode or args.counter_mode):
        try:
            log_file = open(args.output_file[0], 'wb')
        except (OSError, IOError) as e:
            raise NameError('Could not open CSV file for writing')
            sys.exit()

        try:
            log_file_writer = csv.writer(log_file,
                                         quoting=csv.QUOTE_NONE,
                                         delimiter=',')
        except csv.Error as e:
            raise NameError('Could not open CSV writer object')
            sys.exit()
    else:
        log_file = None
        log_file_writer = None

    hid_info = bt.buddy_hid_info_t()
    fw_info = bt.firmware_info_t()

    hid_handle = bt.buddy_init(hid_info, fw_info)

    if hid_handle is None:
        print 'could not open Buddy HID device'
        sys.exit()

    display_usb_info(hid_info)
    display_fw_info(fw_info)

    if args.info_mode:
        sys.exit()

    time_start = time.time()

    if args.dac_mode:
        print 'running a DAC mode test'
        err_code = test_seq_dac(hid_handle,
                                BUDDY_TEST_DAC_FREQ,
                                args.stream_mode)
    
    if args.adc_mode:
        print 'running a ADC mode test'
        err_code = test_seq_adc(hid_handle,
                                BUDDY_TEST_ADC_FREQ,
                                args.stream_mode,
                                log_file_writer)
    
    if args.counter_mode:
        print 'running a counter mode test'
        err_code = test_seq_counter(hid_handle,
                                   BUDDY_TEST_COUNTER_FREQ,
                                   args.stream_mode,
                                   log_file_writer)

    if args.pwm_mode_duty:
        print 'running a PWM duty cycle mode test'
        err_code = test_seq_pwm_duty(hid_handle,
                                     BUDDY_TEST_PWM_FREQ,
                                     args.stream_mode)
    
    if args.pwm_mode_freq:
        print 'running a PWM frequency mode test'
        err_code = test_seq_pwm_freq(hid_handle,
                                     BUDDY_TEST_PWM_FREQ,
                                     args.stream_mode)

    time_end = time.time()
    time_diff = time_end - time_start
    print 'Test took (%f) seconds  to run' % time_diff
    print 'hold_mode = %d' % args.hold_mode

    if args.hold_mode:
        bt.buddy_cleanup(hid_handle, hid_info, False)
    else:
        bt.buddy_cleanup(hid_handle, hid_info, True)

    if log_file:
        log_file.close()