import buddy as bt
import sys
import time
import threading
import numpy as np
from scipy import signal as scisig

BUDDY_TEST_DAC_FREQ = 50

hid_info = bt.buddy_hid_info_t()
fw_info = bt.firmware_info_t()

deviceOpened = False
_hid_handle = 0

def buddy_open():
    global deviceOpened
    global _hid_handle

    if deviceOpened is False:
        _hid_handle = bt.buddy_init(hid_info, fw_info)
        deviceOpened = True
    return _hid_handle

def buddy_close():
    global deviceOpened

    if deviceOpened:
        deviceOpened = False
        bt.buddy_cleanup(_hid_handle, hid_info, False)

def buddy_cleanup():
    global deviceOpened

    if deviceOpened:
        deviceOpened = False
        bt.buddy_cleanup(_hid_handle, hid_info, True)

def buddy_reset():
    hid_handle = buddy_open()
    bt.buddy_reset_device(hid_handle)
    return True

def buddy_get_app_version():
    hid_handle = buddy_open()
    return (fw_info.fw_rev_major, fw_info.fw_rev_minor, fw_info.fw_rev_tiny)

def buddy_get_bootl_version():
    hid_handle = buddy_open()
    return (fw_info.bootl_rev_major, fw_info.bootl_rev_minor, fw_info.bootl_rev_tiny)

def buddy_get_adc_value(channel, sample_rate=1):
    hid_handle = buddy_open()

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

    time.sleep(0.05)
    
    err_code = bt.buddy_clear(hid_handle)

    err_code = bt.buddy_read_adc(hid_handle, packet, False)
    adc_value = bt.int32_t_ptr_getitem(packet.channels, int(channel))
    
    return adc_value

def buddy_get_counter_value(channel, sample_rate=1000, streaming=False):
    hid_handle = buddy_open()

    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()
    packet = bt.general_packet_t()

    general_settings.function = bt.GENERAL_CTRL_COUNTER_ENABLE
    general_settings.mode = bt.MODE_CTRL_IMMEDIATE
    general_settings.channel_mask = (1 << int(channel))
    general_settings.resolution = bt.RESOLUTION_CTRL_SUPER

    timing_settings.period = bt.FREQUENCY_TO_NSEC(int(sample_rate))
    runtime_settings.counter_control = bt.RUNTIME_COUNTER_CONTROL_ACTIVE_LOW

    if (bt.buddy_configure(hid_handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        return -1

    test_time_start = time.time()
    err_code = bt.buddy_read_counter(hid_handle, packet, streaming)
    start_value = bt.int32_t_ptr_getitem(packet.channels, int(channel))

    time.sleep(1)

    err_code = bt.buddy_read_counter(hid_handle, packet, streaming)
    end_value = bt.int32_t_ptr_getitem(packet.channels, int(channel))
    test_time_end = time.time()

    diff_value = end_value - start_value
    return diff_value

def buddy_set_pwm_freq_value(channel, value, sample_rate=1000, streaming=False):
    hid_handle = buddy_open()

    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()

    general_settings.function = bt.GENERAL_CTRL_PWM_ENABLE
    general_settings.mode = bt.MODE_CTRL_IMMEDIATE
    general_settings.channel_mask = (1 << int(channel))
    general_settings.resolution = bt.RESOLUTION_CTRL_HIGH

    timing_settings.period = bt.FREQUENCY_TO_NSEC(int(sample_rate))

    runtime_settings.pwm_mode = bt.RUNTIME_PWM_MODE_FREQUENCY
    runtime_settings.pwm_timebase = bt.RUNTIME_PWM_TIMEBASE_SYSCLK_DIV_12

    if (bt.buddy_configure(hid_handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        return -1

    packet = bt.general_packet_t()
    
    for i in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7 + 1):
        bt.int32_t_ptr_setitem(packet.channels, i, int(value))

    if (bt.buddy_send_pwm(hid_handle, packet, streaming) != bt.BUDDY_ERROR_CODE_OK):
        return -1

    time.sleep(0.1)
    bt.buddy_flush(hid_handle)

def buddy_set_pwm_duty_value(channel, value, sample_rate=1000, streaming=False):
    hid_handle = buddy_open()
    
    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()

    general_settings.function = bt.GENERAL_CTRL_PWM_ENABLE
    general_settings.mode = bt.MODE_CTRL_IMMEDIATE
    general_settings.channel_mask = (1 << int(channel))
    general_settings.resolution = bt.RESOLUTION_CTRL_HIGH

    timing_settings.period = bt.FREQUENCY_TO_NSEC(int(sample_rate))

    runtime_settings.pwm_mode = bt.RUNTIME_PWM_MODE_DUTY_CYCLE
    runtime_settings.pwm_timebase = bt.RUNTIME_PWM_TIMEBASE_SYSCLK

    if (bt.buddy_configure(hid_handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        return -1

    packet = bt.general_packet_t()
    
    for i in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7 + 1):
        bt.int32_t_ptr_setitem(packet.channels, i, int(value))

    if (bt.buddy_send_pwm(hid_handle, packet, streaming) != bt.BUDDY_ERROR_CODE_OK):
        return -1

    time.sleep(0.1)
    bt.buddy_flush(hid_handle)

def buddy_set_dac_value(channel, value, sample_rate=1000, streaming=False):
    hid_handle = buddy_open()

    general_settings = bt.ctrl_general_t()
    timing_settings = bt.ctrl_timing_t()
    runtime_settings = bt.ctrl_runtime_t()

    general_settings.function = bt.GENERAL_CTRL_DAC_ENABLE
    general_settings.mode = \
        bt.MODE_CTRL_STREAM if streaming else bt.MODE_CTRL_IMMEDIATE
    general_settings.channel_mask = (1 << int(channel))
    general_settings.resolution = bt.RESOLUTION_CTRL_HIGH

    timing_settings.period = bt.FREQUENCY_TO_NSEC(int(sample_rate))

    runtime_settings.dac_power = bt.RUNTIME_DAC_POWER_ON
    runtime_settings.dac_ref = bt.RUNTIME_DAC_REF_EXT

    if (bt.buddy_configure(hid_handle,
                           general_settings,
                           runtime_settings,
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        return -1

    packet = bt.general_packet_t()
    
    for i in range(bt.BUDDY_CHAN_0, bt.BUDDY_CHAN_7 + 1):
        bt.int32_t_ptr_setitem(packet.channels, i, int(value))

    if (bt.buddy_send_dac(hid_handle, packet, streaming) != bt.BUDDY_ERROR_CODE_OK):
        return -1

    if not streaming:
        time.sleep(1.0 / int(sample_rate))

    time.sleep(0.5)
    bt.buddy_flush(hid_handle)

def buddy_get_app_version_major():
    return fw_info.fw_rev_major

def buddy_get_app_version_minor():
    return fw_info.fw_rev_minor

def buddy_get_app_version_tiny():
    return fw_info.fw_rev_tiny

def buddy_get_bootl_version_major():
    return fw_info.bootl_rev_major

def buddy_get_bootl_version_minor():
    return fw_info.bootl_rev_minor

def buddy_get_bootl_version_tiny():
    return fw_info.bootl_rev_tiny

def buddy_get_dac_type():
    return fw_info.type_dac

def buddy_get_datetime():
    return fw_info.flash_datetime

def buddy_get_serialnum():
    return int(fw_info.serial)

def buddy_get_usb_mfr():
    return hid_info.str_mfr

def buddy_get_usb_dev():
    return hid_info.str_product