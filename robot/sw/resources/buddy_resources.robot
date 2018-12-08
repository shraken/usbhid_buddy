*** Settings ***

Library  buddy_library
Library  String

*** Variables ***

# Application Firmware Version
${BUDDY_APP_VERSION_MAJOR_MIN}  0
${BUDDY_APP_VERSION_MAJOR_MAX}  1
${BUDDY_APP_VERSION_MINOR_MIN}  0
${BUDDY_APP_VERSION_MINOR_MAX}  10
${BUDDY_APP_VERSION_TINY_MIN}   0
${BUDDY_APP_VERSION_TINY_MAX}   100

# Bootloader Version
${BUDDY_BOOTL_VERSION_MAJOR_MIN}  0
${BUDDY_BOOTL_VERSION_MAJOR_MAX}  1
${BUDDY_BOOTL_VERSION_MINOR_MIN}  0
${BUDDY_BOOTL_VERSION_MINOR_MAX}  10
${BUDDY_BOOTL_VERSION_TINY_MIN}   0
${BUDDY_BOOTL_VERSION_TINY_MAX}   100

${BUDDY_PWM_MODEDUTY_FREQ}  1000
${BUDDY_PWM_MODEFREQ_FREQ}  1000

*** Keywords ***

Check Firmware Version
    buddy_open
    ${app_major}=  buddy_get_app_version_major
    ${app_minor}=  buddy_get_app_version_minor
    ${app_tiny}=   buddy_get_app_version_tiny

    Should Be True	${app_major} >= ${BUDDY_APP_VERSION_MAJOR_MIN}
    Should Be True	${app_major} <= ${BUDDY_APP_VERSION_MAJOR_MAX}

    Should Be True	${app_minor} >= ${BUDDY_APP_VERSION_MINOR_MIN}
    Should Be True	${app_minor} <= ${BUDDY_APP_VERSION_MINOR_MAX}

    Should Be True	${app_tiny} >= ${BUDDY_APP_VERSION_TINY_MIN}
    Should Be True	${app_tiny} <= ${BUDDY_APP_VERSION_TINY_MAX}

Check Bootloader Version
    buddy_open
    ${bootl_major}=  buddy_get_bootl_version_major
    ${bootl_minor}=  buddy_get_bootl_version_minor
    ${bootl_tiny}=   buddy_get_bootl_version_tiny

    Should Be True	${bootl_major} >= ${BUDDY_BOOTL_VERSION_MAJOR_MIN}
    Should Be True	${bootl_major} <= ${BUDDY_BOOTL_VERSION_MAJOR_MAX}

    Should Be True	${bootl_minor} >= ${BUDDY_BOOTL_VERSION_MINOR_MIN}
    Should Be True	${bootl_minor} <= ${BUDDY_BOOTL_VERSION_MINOR_MAX}

    Should Be True	${bootl_tiny} >= ${BUDDY_BOOTL_VERSION_TINY_MIN}
    Should Be True	${bootl_tiny} <= ${BUDDY_BOOTL_VERSION_TINY_MAX}

Get Firmware Serial Number
    buddy_open
    ${result}=  buddy_get_serialnum
    [Return]  ${result}

Get Firmware Datetime
    buddy_open
    ${result}=  buddy_get_datetime
    [Return]  ${result}

Get Firmware USB Manufacturer String
    buddy_open
    ${result}=  buddy_get_usb_mfr
    [Return]  ${result}

Get Firmware USB Device String
    buddy_open
    ${result}=  buddy_get_usb_dev
    [Return]  ${result}

Buddy Reset Device
    buddy_open
    buddy_reset
    buddy_close

Buddy Open Device
    buddy_open

Buddy Close Device
    buddy_close

Buddy Get Counter Channel Value
    [Arguments]  ${channel}
    ${adc_value}=  buddy_get_counter_value  ${channel}
    #Log  ${adc_value}  console=yes

    [Return]  ${adc_value}

Buddy Get ADC Channel Value
    [Arguments]  ${channel}
    ${adc_value}=  buddy_get_adc_value  ${channel}

    [Return]  ${adc_value}

Buddy Set DAC Channel Value
    [Arguments]  ${channel}  ${value}
    ${result}=  buddy_set_dac_value  ${channel}  ${value}  1
    [Return]  ${result}

Buddy Set PWM Duty Channel Value
    [Arguments]  ${channel}  ${value}
    ${result}=  buddy_set_pwm_duty_value  ${channel}  ${value}  ${BUDDY_PWM_MODEDUTY_FREQ}
    [Return]  ${result}

Buddy Set PWM Frequency Channel Value
    [Arguments]  ${channel}  ${value}
    ${result}=  buddy_set_pwm_freq_value  ${channel}  ${value}  ${BUDDY_PWM_MODEFREQ_FREQ}
    [Return]  ${result}