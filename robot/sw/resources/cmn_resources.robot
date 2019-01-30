*** Settings ***

Resource   buddy_resources.robot
Resource   teensy_resources.robot
Library    DateTime
Library    String
Library    OperatingSystem
Library    common_library

*** Variables ***

${BUDDY_MIN_DAC}  0                           # 0 counts =    0 V
${BUDDY_MAX_DAC}  100                         # 2700 counts = 3.29 V
${BUDDY_THRESH_PWM_MODE_PLUSMINUS}  0.05      # 50 msec
${BUDDY_THRESH_PWM_FREQ_PLUSMINUS}  1000      # 1kHz
${BUDDY_THRESH_PWM_SET_FREQ_PLUSMINUS}  6000  # 1kHz

${BUDDY_THRESH_SINE_WAVE_THRESHOLD}  4

*** Keywords ***

Buddy Static Counter Check
    [Arguments]  ${channel}

    #@{counter_values}    Create List    1000  5000  10000  25000  50000  75000  100000
    @{counter_values}    Create List    100000  75000  50000  25000  10000  5000  1000

    # TODO: hack to for DAC mux onto unused channel
    Teensy Set DAC Channel Value  7  2048

    :FOR  ${value}    IN    @{counter_values}
    \   ${counter_count}=  Convert to Integer  ${value}
    \   Teensy Set PWM Frequency Channel Value  ${channel}  ${counter_count}
    \   Sleep  1.0s
    \   Teensy Set PWM Frequency Channel Value  ${channel}  ${counter_count}
    \   Sleep  0.s
    \   ${result}=  Buddy Get Counter Channel Value  ${channel}
    \   ${result_float}=  Convert to Number  ${result}
    \   Log  ${result}  console=yes
    #\   Log  ${result}  console=yes
    \   ${min} =      Evaluate    ${counter_count} - ${BUDDY_THRESH_PWM_SET_FREQ_PLUSMINUS}
    \   ${max} =      Evaluate    ${counter_count} + ${BUDDY_THRESH_PWM_SET_FREQ_PLUSMINUS}
    \   Should Be True  ${result} >= ${min}
    \   Should Be True  ${result} <= ${max}
    \   Sleep  0.5s

Buddy Static PWM Frequency Check
    [Arguments]  ${channel}

    @{pwm_freq_values}    Create List    10000  20000  30000  40000  50000  60000

    :FOR  ${value}    IN    @{pwm_freq_values}
    #\   Log  ${value}  console=yes
    \   ${pwm_count}=  Convert to Integer  ${value}
    \   ${pwm_count_float}=  Convert to Number  ${value}
    \   ${result}=  Buddy Set PWM Frequency Channel Value  ${channel}  ${pwm_count}
    \   Sleep  0.5s
    \   ${result}=  Teensy Get PWM Frequency Channel Value  ${channel}
    \   ${result_float}=  Convert to Number  ${result}
    #\   Log  ${pwm_count}  console=yes
    #\   Log  ${result}  console=yes
    \   ${min} =      Evaluate    ${pwm_count} - ${BUDDY_THRESH_PWM_FREQ_PLUSMINUS}
    \   ${max} =      Evaluate    ${pwm_count} + ${BUDDY_THRESH_PWM_FREQ_PLUSMINUS}
    \   Should Be True  ${result} >= ${min}
    \   Should Be True  ${result} <= ${max}

Buddy Static PWM Duty Check
    [Arguments]  ${channel}

    @{pwm_count_values}    Create List    128  256  512  1024  2048  4096  8192  16384  32768  50000

    ${result}=  Buddy Set PWM Duty Channel Value  ${channel}  128
    Sleep  1s

    :FOR  ${value}    IN    @{pwm_count_values}
    #\   Log  ${value}  console=yes
    \   ${pwm_count}=  Convert to Integer  ${value}
    \   ${pwm_count_float}=  Convert to Number  ${value}
    \   ${result}=  Buddy Set PWM Duty Channel Value  ${channel}  ${pwm_count}
    \   Sleep  0.5s
    \   ${result}=  Teensy Get PWM Duty Channel Value  ${channel}
    \   ${result_float}=  Convert to Number  ${result}
    \   ${pwm_duty_src_ratio}=  Evaluate  (${pwm_count_float} / 65535)
    \   ${pwm_duty_src_ratio}=  Evaluate  (1 - ${pwm_duty_src_ratio})
    #\   Log  ${pwm_count}  console=yes
    #\   Log  ${result}  console=yes
    #\   Log  ${pwm_duty_src_ratio}  console=yes
    \   ${min} =      Evaluate    ${pwm_duty_src_ratio} - ${BUDDY_THRESH_PWM_MODE_PLUSMINUS}
    \   ${max} =      Evaluate    ${pwm_duty_src_ratio} + ${BUDDY_THRESH_PWM_MODE_PLUSMINUS}
    \   Should Be True  ${result} >= ${min}
    \   Should Be True  ${result} <= ${max}

Buddy Static DAC Check
    [Arguments]  ${channel}

    ${volt_thres}=  Convert To Number  0.1  # +- 100 mV
    #@{dac_count_values}    Create List    32  64  128  256  512  1024  2048  4095
    @{dac_count_values}    Create List    4095  2048  1024  512  256  128  64  32
    #@{dac_count_values}    Create List    4095

    # TODO: hack to for DAC mux onto unused channel
    ${temo_channel}=  Evaluate  (${channel} + 1) % 7
    Teensy Set DAC Channel Value  ${temo_channel}  2048

    :FOR  ${value}    IN    @{dac_count_values}
    #\   Log  ${value}  console=yes
    \   ${dac_count}=  Convert to Integer  ${value}
    \   ${dac_count_float}=  Convert to Number  ${value}
    #\   Log  ${channel}  console=yes
    #\   Log  ${dac_count}  console=yes
    #\   Log  Setting DAC channel to zero
    \   Teensy Set DAC Channel Value   ${channel}   ${dac_count}
    \   Sleep  0.5s
    \   ${result}=  Buddy Get ADC Channel Value  ${channel}
    \   ${result_float}=  Convert to Number  ${result}
    #\   Log  ${result_float}  console=yes
    \   ${adc_volt_value}=  Evaluate  (${result_float} / 1024) * 3.3
    \   ${dac_volt_value}=  Evaluate  (${dac_count_float} / 4095) * 3.3
    #\   Log  ${adc_volt_value}  console=yes
    #\   Log  ${dac_volt_value}  console=yes
    \   ${diff_volt_value}=  Evaluate  (${adc_volt_value} - ${dac_volt_value})
    \   ${diff_volt_value_abs} =  Evaluate  abs(${diff_volt_value})
    \   Should Be True  ${diff_volt_value_abs} <= ${volt_thres}
    \   Teensy Set DAC Channel Value   ${channel}   0

Buddy Static ADC Check
    [Arguments]  ${channel}

    ${volt_thres}=  Convert To Number  0.15  # +- 150 mV
    #@{dac_count_values}    Create List    32  64  128  256  512  1024  2048
    @{dac_count_values}    Create List    2048  1024  512  256  128  64  32

    :FOR  ${value}    IN    @{dac_count_values}
    \   ${t_channel}=  Evaluate  (${channel} + 1) % 8
    #\   Log  ${channel}  console=yes
    #\   Log  TCHANNEL
    #\   Log  ${t_channel}  console=yes
    # TODO: look at removing below entry
    #\   Sleep  0.5s
    \   Teensy Set DAC Channel Value  ${t_channel}  0
    \   ${dac_count}=  Convert to Integer  ${value}
    \   ${dac_count_float}=  Convert to Number  ${value}
    \   Buddy Set DAC Channel Value   ${channel}   ${dac_count}
    \   Sleep  0.5s
    #\   Log  ${channel}  console=yes
    #\   Log  ${dac_count}  console=yes
    \   ${result}=  Teensy Get ADC Channel Value  ${channel}
    \   ${result_float}=  Convert to Number  ${result}
    #\   Log  ${result_float}  console=yes
    \   ${adc_volt_value}=  Evaluate  (${result_float} / 4095) * 5.0
    \   ${dac_volt_value}=  Evaluate  (${dac_count_float} / 4095) * 5.0
    #\   Log  ${dac_volt_value}  console=yes
    #\   Log  ${adc_volt_value}  console=yes
    \   ${diff_volt_value}=  Evaluate  (${adc_volt_value} - ${dac_volt_value})
    \   ${diff_volt_value_abs} =  Evaluate  abs(${diff_volt_value})
    \   Should Be True  ${diff_volt_value_abs} <= ${volt_thres}
    \   Buddy Set DAC Channel Value   ${channel}   0

Buddy Dynamic DAC Check
    [Arguments]  ${channel}  ${sample_rate}  ${waveform_frequency}

    ${CurrentDate} =    Get Current Date    result_format=%Y%m%d-%H%M%S
    Log    ${CurrentDate}

    ${filename} =   Catenate    reports/images/buddy_as_adc_${CurrentDate}_${channel}
    ${filename} =   Catenate    ${filename}.png
    
    ${csv_filename} =   Catenate    reports/csv/buddy_as_adc_${CurrentDate}_${channel}
    ${csv_filename} =   Catenate    ${csv_filename}.csv

    # make sure DAC mux not enabled on Buddy ADC sample channel
    ${t_channel}=  Evaluate  (${channel} + 1) % 8
    Teensy Set DAC Channel Value  ${t_channel}  0

    #Log  ${channel}  console=yes
    teensy_dynamic_thread_dac  ${channel}  ${sample_rate}  ${waveform_frequency}
    Sleep  0.1s

    #${result}=  run_dynamic_buddy_adc  ${channel}  ${sample_rate}  ${waveform_frequency}
    ${result}=  buddy_dynamic_thread_adc  ${channel}  ${sample_rate}  ${waveform_frequency}

    plotSpectrum  ${result}  ${sample_rate}  ${filename}

    ${peak_freq}    ${mag}     freq_mag_find  ${result}  ${sample_rate}  ${csv_filename} 
    #Log  ${peak_freq}  console=yes

    ${min} =  Evaluate    ${waveform_frequency} - ${BUDDY_THRESH_SINE_WAVE_THRESHOLD}
    ${max} =  Evaluate    ${waveform_frequency} + ${BUDDY_THRESH_SINE_WAVE_THRESHOLD}
    Should Be True  ${peak_freq} >= ${min}
    Should Be True  ${peak_freq} <= ${max}

Buddy Dynamic ADC Check
    [Arguments]  ${channel}  ${sample_rate}  ${waveform_frequency}

    ${CurrentDate} =    Get Current Date    result_format=%Y%m%d-%H%M%S
    Log    ${CurrentDate}

    ${filename} =   Catenate    reports/images/buddy_as_dac_${CurrentDate}_${channel}
    ${filename} =   Catenate    ${filename}.png

    ${csv_filename} =   Catenate    reports/csv/buddy_as_dac_${CurrentDate}_${channel}
    ${csv_filename} =   Catenate    ${csv_filename}.csv

    run_dynamic_buddy_dac  ${channel}  ${sample_rate}  ${waveform_frequency}
    #Log  ${csv_filename}  console=yes
    ${result}=  teensy_dynamic_thread_adc  ${channel}
    ${peak_freq}    ${mag}     plotSpectrum  ${result}  ${sample_rate}  ${filename}

    #Log  ${csv_filename}  console=yes
    #Log  ${peak_freq}  console=yes
    #Log  ${mag}  console=yes

    ${peak_freq}    ${mag}     freq_mag_find  ${result}  ${sample_rate}  ${csv_filename}
    #Log  ${peak_freq}  console=yes
    #Log  ${mag}  console=yes

    [Return]  ${peak_freq}