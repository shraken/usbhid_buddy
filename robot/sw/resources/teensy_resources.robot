*** Settings ***

Library  teensy_library
Library  String

*** Variables ***

# Application Firmware Version
${TEENSY_VERSION_MAJOR_MIN}  0
${TEENSY_VERSION_MAJOR_MAX}  1
${TEENSY_VERSION_MINOR_MIN}  0
${TEENSY_VERSION_MINOR_MAX}  10

*** Keywords ***

Check Teensy Firmware Version
    teensy_open
    ${teensy_major}=  teensy_get_major_version
    ${teensy_minor}=  teensy_get_minor_version

    Should Be True	${teensy_major} >= ${TEENSY_VERSION_MAJOR_MIN}
    Should Be True	${teensy_major} <= ${TEENSY_VERSION_MAJOR_MAX}

    Should Be True	${teensy_minor} >= ${TEENSY_VERSION_MINOR_MIN}
    Should Be True	${teensy_minor} <= ${TEENSY_VERSION_MINOR_MAX}

Teensy Default State
    teensy_open
    teensy_defaults

Teensy Reset Device
    teensy_open
    teensy_reset
    teensy_close

Teensy Get PWM Frequency Channel Value
    [Arguments]  ${channel}

    teensy_open
    ${pwm_value}=  teensy_get_pwm_freq_value  ${channel}
    [Return]  ${pwm_value}

Teensy Get PWM Duty Channel Value
    [Arguments]  ${channel}

    teensy_open
    ${pwm_value}=  teensy_get_pwm_duty_value  ${channel}
    [Return]  ${pwm_value}

Teensy Get ADC Channel Value
    [Arguments]  ${channel}

    teensy_open
    ${adc_value}=  teensy_get_adc_value  ${channel}
    [Return]  ${adc_value}

Teensy Set DAC Channel Value
    [Arguments]  ${channel}  ${value}

    teensy_open
    ${result}=  teensy_set_dac_value  ${channel}  ${value}
    [Return]  ${result}

Teensy Set PWM Frequency Channel Value
    [Arguments]  ${channel}  ${value}

    teensy_open
    ${result}=  teensy_set_pwm_freq_value  ${channel}  ${value}
    [Return]  ${result}