import buddy_library
from scipy import signal as scisig
import numpy as np
import time
import sys

def buddy_dynamic_thread():
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
                           timing_settings) != bt.BUDDY_ERROR_CODE_OK):
        print 'test_waveform_dac: could not configure Buddy device'
        return -1

    time.sleep(0.1)

    packet = bt.general_packet_t()
    test_seq_dac_count = 0
    
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
            bt.uint32_t_ptr_setitem(packet.channels, i, int(k))

        print 'test_waveform_dac: sending %d packet with value %d' % (test_seq_dac_count, k)
        test_seq_dac_count += 1

        if (bt.buddy_send_dac(handle,
                              packet,
                              streaming) != bt.BUDDY_ERROR_CODE_OK):
            print 'test_waveform_dac: could not send DAC packet'
            return -1

        if not streaming:
            time.sleep(1.0 / sample_rate)

    bt.buddy_flush(handle)
    time.sleep(0.1)