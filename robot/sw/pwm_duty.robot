*** Settings ***
Library    OperatingSystem
Library    String
Resource    resources/buddy_resources.robot
Resource    resources/teensy_resources.robot
Resource    resources/cmn_resources.robot
Documentation     Test suite for Buddy DAQ device

*** Variables ***
${SLEEP_INTERVAL}  1s

${BUDDY_TEST_DATETIME}  0
# 0x12345678 = 305419896
${BUDDY_TEST_SERIAL_NUMBER}  305419896
${BUDDY_TEST_USB_MFR}  WIGGLE
${BUDDY_TEST_USB_DEV}  BUDDY

${BUDDY_NUMBER_PWM_CHANNELS}  4
${BUDDY_NUMBER_COUNTER_CHANNELS}  2

${BUDDY_NUMBER_CHANNELS}  7
#${BUDDY_NUMBER_CHANNELS}  1

# stream test
${BUDDY_DYN_CHANNEL}  0
${BUDDY_DYN_FREQ}  1000
${BUDDY_DYN_WAVE_FREQ}  50
${BUDDY_THRESH_PLUSMINUS}  15

${BUDDY_DYN_WAVE_FREQ_ADC}  50

*** Test Cases ***

BUDDY_FW_VER_CHECK
    [Documentation]  Buddy firmware version is sane and bounded
    
    Buddy Reset Device
    Sleep  1.0s

    Check Firmware Version
    Check Bootloader Version

BUDDY_FW_SERIAL_NUM_CHECK
    [Documentation]  Buddy firmware serial number is correct
    
    Buddy Reset Device
    Sleep  1.0s

    ${result}=  Get Firmware Serial Number
    ${int_serialNum}    Convert To Integer    ${BUDDY_TEST_SERIAL_NUMBER}

    Should Be Equal    ${result}    ${int_serialNum}

BUDDY_USB_ID_CHECK
    [Documentation]  Buddy USB manufacturer and device strings are correct

    Buddy Reset Device
    Sleep  1.0s

    ${result}=  Get Firmware USB Manufacturer String
    Should Be Equal    ${result}    ${BUDDY_TEST_USB_MFR}

    ${result}=  Get Firmware USB Device String
    Should Be Equal    ${result}    ${BUDDY_TEST_USB_DEV}

BUDDY_TEENSY_ID_CHECK
    [Documentation]  External Teensy controller version are correct

    Teensy Reset Device
    Sleep  2.0s
    Check Teensy Firmware Version
    
BUDDY_PWM_DUTY_CHECK
    [Documentation]  Buddy PWM static duty cycle values check on all channels

    Buddy Reset Device
    Sleep  1.0s
    Teensy Reset Device
    Sleep  2.0s
    Teensy Default State
    Sleep  1.0s

    :FOR  ${channel}  IN RANGE  0  ${BUDDY_NUMBER_PWM_CHANNELS}
    \    Buddy Static PWM Duty Check  ${channel}