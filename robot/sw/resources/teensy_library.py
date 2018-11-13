import serial
import io
import re
import threading

DEFAULT_TEESNY_SERIAL = "COM3"

deviceOpened = False
_ser = 0
_ser_io = 0

def teensy_open():
    global deviceOpened
    global _ser
    global _ser_io

    if deviceOpened is False:
        _ser = serial.Serial(port = DEFAULT_TEESNY_SERIAL,
                             baudrate = 115200,
                             timeout = 10)

        _ser_io = io.TextIOWrapper(io.BufferedRWPair(_ser, _ser, 1),  
                                   newline = '\r',
                                   line_buffering = True)

        deviceOpened = True
    return _ser

def teensy_close():
    global deviceOpened

    if deviceOpened is True:
        deviceOpened = False
        _ser.close()

def teensy_sendrecv(input_msg):
    teensy_open()

    if deviceOpened is True:
        #_ser.reset_output_buffer()
        #_ser.reset_input_buffer()

        _ser_io.write(input_msg)
        text_in = _ser_io.readline()
        return text_in

def teensy_reset():
    reset_recv = teensy_sendrecv(u'reset,0,0\r\n')
    return reset_recv

def teensy_defaults():
    stop_recv = teensy_sendrecv(u'stop,0,0\r\n')
    return stop_recv

def teensy_get_version():
    version_recv = teensy_sendrecv(u'info,0,0\r\n')

    teensy_version = [x.strip() for x in version_recv.split(',')]
    teensy_major_ver = int(teensy_version[1])
    teensy_minor_ver = int(teensy_version[2])

    return (teensy_major_ver, teensy_minor_ver)

def teensy_get_adc_value(channel):
    adc_response = teensy_sendrecv(u'adc,{},1\r'.format(channel))

    adc_response_tok = [x.strip() for x in adc_response.split(',')]
   
    adc_value = int(adc_response_tok[2])
    adc_channel = int(adc_response_tok[1])

    # disable ADC channel
    teensy_sendrecv(u'adc,{},0\r'.format(channel))
    return adc_value

def teensy_get_pwm_freq_value(channel):
    pwm_response = teensy_sendrecv(u'pwm_freq,{},1\r'.format(channel))
    pwm_response_tok = [x.strip() for x in pwm_response.split(',')]

    pwm_freq_value = float(pwm_response_tok[1])

    return pwm_freq_value

def teensy_get_pwm_duty_value(channel):
    pwm_response = teensy_sendrecv(u'pwm_duty,{},1\r'.format(channel))
    pwm_response_tok = [x.strip() for x in pwm_response.split(',')]

    pwm_duty_value = float(pwm_response_tok[1])

    return pwm_duty_value

def teensy_set_dac_value(channel, value):
    dac_response = teensy_sendrecv(u'dac,{},{}\r'.format(channel,value))
    dac_response_tok = [x.strip() for x in dac_response.split(',')]

    if (dac_response_tok[0] != 'AOK'):
        return False
    return True

def teensy_set_pwm_freq_value(channel, value):
    pwm_freq_response = teensy_sendrecv(u'pwm_set,{},{}\r'.format(channel,value))
    pwm_freq_response_tok = [x.strip() for x in pwm_freq_response.split(',')]

    if (pwm_freq_response_tok[0] != 'AOK'):
        return False
    return True

def teensy_get_major_version():
    version_text = teensy_get_version()
    return version_text[0]

def teensy_get_minor_version():
    version_text = teensy_get_version()
    return version_text[1]