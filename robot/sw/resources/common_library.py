import numpy as np
import scipy as sp
import buddy_library as bl
import teensy_library as tl
import sys
import time
import csv
import threading
import numpy as np
from scipy import signal as scisig
from pylab import plot, show, title, xlabel, ylabel, subplot, savefig, grid, semilogy, figure
import matplotlib.pyplot as plt

WAVEFORM_DAC_TIME = 5
WAVEFORM_ADC_TIME = 2
TEENSY_WAVEFORM_ADC_TIME = 1.0

WAVEFORM_TIME = 5             # 20 seconds

threads = []
adc_text = ''
enableDynamic = False
dynamicFinished = False

def buddy_dynamic_thread_adc(channel, sample_rate, waveform_frequency):
    hid_handle = bl.buddy_open()

    general_settings = bl.bt.ctrl_general_t()
    timing_settings = bl.bt.ctrl_timing_t()
    runtime_settings = bl.bt.ctrl_runtime_t()
    packet = bl.bt.general_packet_t()

    general_settings.function = bl.bt.GENERAL_CTRL_ADC_ENABLE
    general_settings.mode = bl.bt.MODE_CTRL_STREAM
    general_settings.channel_mask = (1 << int(channel))
    general_settings.resolution = bl.bt.RESOLUTION_CTRL_HIGH

    timing_settings.period = bl.bt.FREQUENCY_TO_NSEC(int(sample_rate))
    timing_settings.averaging = 1

    runtime_settings.adc_mode = bl.bt.RUNTIME_ADC_MODE_SINGLE_ENDED
    runtime_settings.adc_ref = bl.bt.RUNTIME_ADC_REF_VDD

    t = np.linspace(0, WAVEFORM_ADC_TIME, int(sample_rate) * WAVEFORM_ADC_TIME, endpoint=False)
    
    entry = []
    if (bl.bt.buddy_configure(hid_handle,
                              general_settings,
                              runtime_settings,
                              timing_settings) != bl.bt.BUDDY_ERROR_CODE_OK):
        return -1

    time.sleep(0.1)

    for i in range(0, len(t)):
        err_code = bl.bt.buddy_read_adc(hid_handle, packet, True)

        if err_code == bl.bt.BUDDY_ERROR_CODE_OK:
            value = bl.bt.int32_t_ptr_getitem(packet.channels, int(channel))
            entry.append(int(value))
        elif err_code == bl.bt.BUDDY_ERROR_CODE_GENERAL:
            print 'buddy_dynamic_thread_adc: could not read ADC packet'
            print 'err_code = %d' % err_code
            return -1
        else:
            print 'unknown error code, err_code = %d' % err_code
    
    bl.buddy_cleanup()
    return entry

def buddy_dynamic_thread_dac(channel, sample_rate, waveform_frequency):
    hid_handle = bl.buddy_open()

    general_settings = bl.bt.ctrl_general_t()
    timing_settings = bl.bt.ctrl_timing_t()
    runtime_settings = bl.bt.ctrl_runtime_t()

    general_settings.function = bl.bt.GENERAL_CTRL_DAC_ENABLE
    general_settings.mode = bl.bt.MODE_CTRL_STREAM
    general_settings.channel_mask = (1 << int(channel))
    general_settings.resolution = bl.bt.RESOLUTION_CTRL_HIGH

    timing_settings.period = bl.bt.FREQUENCY_TO_NSEC(int(sample_rate))

    runtime_settings.dac_power = bl.bt.RUNTIME_DAC_POWER_ON
    runtime_settings.dac_ref = bl.bt.RUNTIME_DAC_REF_EXT

    if (bl.bt.buddy_configure(hid_handle,
                              general_settings,
                              runtime_settings,
                              timing_settings) != bl.bt.BUDDY_ERROR_CODE_OK):
        return -1

    time.sleep(0.1)
    
    y_mag = 2700
    t = np.linspace(0, WAVEFORM_TIME, int(sample_rate) * WAVEFORM_TIME, endpoint=False)

    y = ((np.sin(np.pi * 2 * int(waveform_frequency) * t) + 1) / 2) * y_mag
    
    test_seq_dac_count = 0
    packet = bl.bt.general_packet_t()

    for k in y:
        for i in range(bl.bt.BUDDY_CHAN_0, bl.bt.BUDDY_CHAN_7 + 1):
            bl.bt.int32_t_ptr_setitem(packet.channels, i, int(k))

        test_seq_dac_count += 1

        if (bl.bt.buddy_send_dac(hid_handle,
                              packet,
                              True) != bl.bt.BUDDY_ERROR_CODE_OK):
            return -1

    bl.bt.buddy_flush(hid_handle)
    time.sleep(0.1)

def buddy_dynamic_start_adc(channel, sample_rate=1000, waveform_frequency=100):
    t_buddy_adc = threading.Thread(target=buddy_dynamic_thread_adc, args=(channel, sample_rate, waveform_frequency))
    t_buddy_adc.start()
    t_buddy_adc.join()

def buddy_dynamic_start_dac(channel, sample_rate=1000, waveform_frequency=100):
    # get waveform running from Buddy as DAC channel
    t_buddy_dac = threading.Thread(target=buddy_dynamic_thread_dac, args=(channel, sample_rate, waveform_frequency))
    threads.append(t_buddy_dac)
    t_buddy_dac.start()
    #t_buddy_dac.join()

'''
    buddy DUT running as ADC mode.  Stream mode DAC requested from external teensy
    with a waveform provided to teensy controller.
'''
def teensy_dynamic_thread_dac(channel, sample_rate, waveform_frequency):
    y_mag = 4095
    t = np.linspace(0, TEENSY_WAVEFORM_ADC_TIME, float(sample_rate) * TEENSY_WAVEFORM_ADC_TIME, endpoint=False)
    nsamples = len(t)

    y = ((np.sin(np.pi * 2 * float(waveform_frequency) * t) + 1) / 2) * y_mag

    #sdac_send = u'sdac,0,1,1,10,4095,2048,1024,512,256,4095,2048,1024,512,256,\r'
    sdac_send = u'sdac,{},1,{},{},'.format(channel, sample_rate, nsamples)

    for y_item in y:
        sitem_temp = u'{:d},'.format(int(y_item))
        sdac_send += sitem_temp

    sdac_send += u'\r'

    tl.teensy_open()
    sdac_recv = tl.teensy_sendrecv(sdac_send)

    return 0

'''
    buddy DUT running as DAC mode.  Stream mode ADC requested from external teensy
'''
def teensy_dynamic_thread_adc(channel):
    # enable stream ADC channel, wait for result
    freq = 1000
    nsamples = 2000

    global adc_text

    tl.teensy_open()
    adc_recv = tl.teensy_sendrecv(u'sadc,{},1,{},{}\r'.format(channel, freq, nsamples))
    adc_text = [int(x.strip()) for x in adc_recv.split(',')]

    global dynamicFinished
    dynamicFinished = True
    return adc_text

'''
    runs buddy DUT as DAC mode with streaming sine wave.  Teensy external controller
    processes the input.
'''
def run_dynamic_buddy_dac(channel, sample_rate, waveform_frequency):
    buddy_dynamic_start_dac(channel, sample_rate, waveform_frequency)
    time.sleep(0.5)
    return 0

'''
    runs buddy DUT as ADC mode with streaming.  Teensy external controller provides
    DAC sine wave input.  
'''
def run_dynamic_buddy_adc(channel, sample_rate, waveform_frequency):
    return buddy_dynamic_thread_adc(channel, sample_rate, waveform_frequency)

def plotSpectrum(y, Fs, filename):
    n = len(y) # length of the signal
    k = sp.arange(n)
    T = n / int(Fs)
    frq = k / T # two sides frequency range
    frq = frq[range(n/2)] # one side frequency range

    Y = sp.fft(y)/n # fft computing and normalization
    Y = Y[range(n/2)]
 
    # DC Null
    Y[0] = 0

    plt.clf()
    figure(num=None, figsize=(16, 12), dpi=120, facecolor='w', edgecolor='k')

    timeplot = subplot(2, 1, 1)
    Ts = 1.0 / int(Fs)
    t = sp.arange(0, 2, Ts)
    plot(t, y)
    xlabel('Time')
    ylabel('Amplitude')
    timeplot.set_xlim([0, 0.1])
    grid()

    subplot(2, 1, 2)
    #plot(frq, abs(Y), 'r')
    semilogy(frq, abs(Y), 'r-')
    xlabel('Freq (Hz)')
    ylabel('|Y(freq)|')
    grid()

    savefig(filename)
    #return Y
    maxvalue = np.amax(abs(Y), axis=0)
    index = Y.argmax(axis=0)

    return (index / 2, maxvalue)

def freq_mag_find(y, Fs, csv_filename):
    n = len(y)
    k = sp.arange(n)
    T = n / int(Fs)
    frq = k / T
    frq = frq[range(n/2)]
    
    Y = sp.fft(y) / n
    Y = Y[range(n/2)]
    
    # DC Null
    Y[0] = 0
    
    maxvalue = np.amax(abs(Y), axis=0)
    index = Y.argmax(axis=0)

    f = open(csv_filename, 'wb')
    writer = csv.writer(f, delimiter=',', quotechar='"', quoting=csv.QUOTE_ALL)

    freqs = []
    mags = []
    for freq, mag in zip(frq, abs(Y)):
        freqs.append(freq)
        mags.append(mag)

        writer.writerow( [freq, mag] )

    f.close()    
    index_largest = mags.index(max(mags))
    
    value_largest = mags[index_largest]
    freq_largest = freqs[index_largest]

    return freq_largest, value_largest
    #return (index / 2, maxvalue)