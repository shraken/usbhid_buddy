#include <string.h>

#include <ADC.h>
#include <SPI.h>
#include <IntervalTimer.h>
#include <RingBuffer.h>

#define USBHID_TESTFIXTURE_MAJOR_VER 0
#define USBHID_TESTFIXTURE_MINOR_VER 1

#define SERIAL_BUFFER_LENGTH 8192
#define ADC_SBUFFER_SIZE 8192
#define DAC_SBUFFER_SIZE 8192

// general
#define DAC_PIN A21

#define LED0_PIN 39
#define LED1_PIN 38

#define SPI_DOUT_PIN 11
#define SPI_DIN_PIN 12
#define SPI_SCLK_PIN 13
#define SPI_CS1_PIN 31
#define SPI_CS0_PIN 10

#define PWM_0_PIN 2
#define PWM_1_PIN 3
#define PWM_2_PIN 4
#define PWM_3_PIN 5
#define PWM_4_PIN 6
#define PWM_5_PIN 7
#define PWM_6_PIN 8
#define PWM_7_PIN 9

#define ADC_0_PIN 14
#define ADC_1_PIN 15
#define ADC_2_PIN 16
#define ADC_3_PIN 17
#define ADC_4_PIN 18
#define ADC_5_PIN 19
#define ADC_6_PIN 20
#define ADC_7_PIN 21

// DAC
#define DAC_MUX_SEL_A_PIN 24
#define DAC_MUX_SEL_B_PIN 25
#define DAC_MUX_SEL_C_PIN 26

#define DAC_MUX_EN_PIN 27

#define DAC_MUX_SEL_A_MASK 0x04
#define DAC_MUX_SEL_B_MASK 0x02
#define DAC_MUX_SEL_C_MASK 0x01

#define DAC_MUX_SEL_A_SHIFT 2
#define DAC_MUX_SEL_B_SHIFT 1
#define DAC_MUX_SEL_C_SHIFT 0

#define USBHID_BUDDY_PWM_CHANNELS 5

// ADC
// PWM
// Counter

const int ledPin = 13;

typedef enum _USBHID_TESTFIXTURE_MODE {
  USBHID_TESTFIXTURE_MODE_NONE = 0,
  USBHID_TESTFIXTURE_MODE_ADC,
  USBHID_TESTFIXTURE_MODE_SADC,
  USBHID_TESTFIXTURE_MODE_DAC,
  USBHID_TESTFIXTURE_MODE_SDAC,
  USBHID_TESTFIXTURE_MODE_PWM,
  USBHID_TESTFIXTURE_MODE_PWM_DUTY,
  USBHID_TESTFIXTURE_MODE_PWM_FREQ,
  USBHID_TESTFIXTURE_MODE_COUNTER,
} USBHID_TESTFIXTURE_MODE;

// ABC values for the selector lines tied to MAX4617 multiplexor
typedef enum _USBHID_TESTFIXTURE_DAC_SEL {
  USBHID_TESTFIXTURE_DAC_SEL_FIO_0 = 0x00,
  USBHID_TESTFIXTURE_DAC_SEL_FIO_1 = 0x04,
  USBHID_TESTFIXTURE_DAC_SEL_FIO_2 = 0x02,
  USBHID_TESTFIXTURE_DAC_SEL_FIO_3 = 0x06,
  USBHID_TESTFIXTURE_DAC_SEL_FIO_4 = 0x01,
  USBHID_TESTFIXTURE_DAC_SEL_FIO_5 = 0x05,
  USBHID_TESTFIXTURE_DAC_SEL_FIO_6 = 0x03,
  USBHID_TESTFIXTURE_DAC_SEL_FIO_7 = 0x07,
} USBHID_TESTFIXTURE_DAC_SEL;

typedef enum _TEENSY_PWM {
  TEENSY_PWM_0,
  TEENSY_PWM_1,
  TEENSY_PWM_2,
  TEENSY_PWM_3,
  TEENSY_PWM_4,
  TEENSY_PWM_5,
  TEENSY_PWM_6,
  TEENSY_PWM_7,
} TEENSY_PWM;

typedef enum _USBHID_CHANNELS {
  USBHID_CHANNELS_FIO_0 = 0,
  USBHID_CHANNELS_FIO_1,
  USBHID_CHANNELS_FIO_2,
  USBHID_CHANNELS_FIO_3,
  USBHID_CHANNELS_FIO_4,
  USBHID_CHANNELS_FIO_5,
  USBHID_CHANNELS_FIO_6,
  USBHID_CHANNELS_FIO_7
} USBHID_CHANNELS;

typedef enum _USBHID_TESTFIXTURE_MAX335_CS {
  USBHID_TESTFIXTURE_MAX335_CS_0 = 0,
  USBHID_TESTFIXTURE_MAX335_CS_1 = 1,
} USBHID_TESTFIXTURE_MAX335_CS;

typedef enum _USBHID_SERIAL_CHANNEL {
  USBHID_SERIAL_CHANNEL_0 = 0,
  USBHID_SERIAL_CHANNEL_1,
  USBHID_SERIAL_CHANNEL_2,
  USBHID_SERIAL_CHANNEL_3,
  USBHID_SERIAL_CHANNEL_4,
  USBHID_SERIAL_CHANNEL_5,
  USBHID_SERIAL_CHANNEL_6,
  USBHID_SERIAL_CHANNEL_7,
} USBHID_SERIAL_CHANNEL;

typedef enum _USBHID_SERIAL_ACTION {
  USBHID_SERIAL_ACTION_DISABLE = 0x00,
  USBHID_SERIAL_ACTION_ENABLE = 0x01,
} USBHID_SERIAL_ACTION;

// MAX335 switch state
// _0 = PWM
// _1 = ADC
static uint8_t max335_state_0 = 0;
static uint8_t max335_state_1 = 0;

static bool firstRun = true;

static uint8_t rxSerialBuffer[SERIAL_BUFFER_LENGTH] = { 0 };
static int rxSerialBufferOffset = 0;

const uint8_t buffer_size = 128;
DMAMEM static volatile int16_t __attribute__((aligned(buffer_size+0))) buffer[buffer_size] = {};

int readPin = A0;

int period0 = 10000; // us
int readPin0 = A0;

int readPeriod = 10000; // us

const int pwmPin = A0;

uint16_t adc_latest = 0;

ADC *adc = new ADC();

RingBuffer *buffer_sadc = new RingBuffer;
RingBuffer *buffer_sdac = new RingBuffer;

int prevTimeFall[USBHID_BUDDY_PWM_CHANNELS];
int prevTimeRise[USBHID_BUDDY_PWM_CHANNELS];

int deltaHigh[USBHID_BUDDY_PWM_CHANNELS];
int deltaLow[USBHID_BUDDY_PWM_CHANNELS];

IntervalTimer timer0;
int startTimerValue0 = 0;

volatile int adcStreamCount = 0;
volatile int adcStreamCountStop = 0;
volatile bool adcStreamFinish = false;

int pwmTickCount[USBHID_BUDDY_PWM_CHANNELS] = { 0 };

int testfixture_mode = USBHID_TESTFIXTURE_MODE_NONE;
bool adc_active = false;

int stream_loop_mode = 0;

int count = 0;

void pwm_set_freq_value(uint8_t channel, uint32_t value);
void pwm_set_duty_value(uint8_t channel, uint16_t value);

void heartbeat_led() {
  static int lastMillis = millis();

  if (((millis() % 1000) == 0) && (lastMillis != millis())) {
    lastMillis = millis();

    if (digitalRead(LED0_PIN) == HIGH) {
      digitalWrite(LED0_PIN, LOW);
    } else {
      digitalWrite(LED0_PIN, HIGH);
    }
  }
}

void activity_led() {
  if (digitalRead(LED1_PIN) == HIGH) {
    digitalWrite(LED1_PIN, LOW);
  } else {
    digitalWrite(LED1_PIN, HIGH);
  }
}

void timer0_callback(void) {
  adc->startSingleRead(readPin0, ADC_0); // also: startSingleDifferential, analogSynchronizedRead, analogSynchronizedReadDifferential

  uint16_t val;

  if (testfixture_mode == USBHID_TESTFIXTURE_MODE_SDAC) {
    if (!buffer_sdac->isEmpty()) {
      val = buffer_sdac->read();
      analogWrite(DAC_PIN, val);

      if (stream_loop_mode) {
        buffer_sadc->write(val);
      }
    }
  }
}

void adc0_isr() {
    uint8_t pin = ADC::sc1a2channelADC0[ADC0_SC1A&ADC_SC1A_CHANNELS]; // the bits 0-4 of ADC0_SC1A have the channel
    adc_latest = adc->readSingle();
    
    // restore ADC config if it was in use before being interrupted by the analog timer
    if (adc->adc0->adcWasInUse) {
        adc->adc0->loadConfig(&adc->adc0->adc_config);
        adc->adc0->adcWasInUse = false;
    }

    if ((testfixture_mode == USBHID_TESTFIXTURE_MODE_SADC) && (adc_active) && (!buffer_sadc->isFull())) {
      //Serial.println("trigger adc conversion");
      
      buffer_sadc->write(adc_latest);
      adcStreamCount++;
      
      if (adcStreamCount >= adcStreamCountStop) {
        //Serial.println("trigger finished");

        adc_active = false;
        adcStreamFinish = true;
      }
    }
}

void max4617_set_sel(uint8_t channel) {
  uint8_t a_pin_value, b_pin_value, c_pin_value;

  a_pin_value = ((channel & DAC_MUX_SEL_A_MASK) >> DAC_MUX_SEL_A_SHIFT);
  b_pin_value = ((channel & DAC_MUX_SEL_B_MASK) >> DAC_MUX_SEL_B_SHIFT);
  c_pin_value = ((channel & DAC_MUX_SEL_C_MASK) >> DAC_MUX_SEL_C_SHIFT);
 
  digitalWrite(DAC_MUX_SEL_A_PIN, a_pin_value);
  digitalWrite(DAC_MUX_SEL_B_PIN, b_pin_value);
  digitalWrite(DAC_MUX_SEL_C_PIN, c_pin_value);
}

void max335_enable_channel(uint8_t cs, uint8_t channel) {
  uint8_t temp_value;

  //Serial.println("max335_enable_channel A");

  temp_value = max335_state_0;
  temp_value |= (1 << channel);
  
  if (cs == USBHID_TESTFIXTURE_MAX335_CS_0) {
    temp_value = max335_state_0;
    temp_value |= (1 << channel);
    max335_state_0 = temp_value;

    //Serial.println("max335_enable_channel B");
    digitalWrite(SPI_CS0_PIN, LOW);
    //Serial.println("max335_enable_channel C");
    SPI.transfer(temp_value);
    //Serial.println("max335_enable_channel D");
    digitalWrite(SPI_CS0_PIN, HIGH);
    //Serial.println("max335_enable_channel E");
  } else if (cs == USBHID_TESTFIXTURE_MAX335_CS_1) {
    temp_value = max335_state_1;
    temp_value |= (1 << channel);
    max335_state_1 = temp_value;

    //Serial.println("max335_enable_channel F");
    digitalWrite(SPI_CS1_PIN, LOW);
    //Serial.println("max335_enable_channel G");
    SPI.transfer(temp_value);
    //Serial.println("max335_enable_channel H");
    digitalWrite(SPI_CS1_PIN, HIGH);
    //Serial.println("max335_enable_channel I");
  } else {
    //Serial.println("max335_enable_channel J");
    return;
  }
}

void max335_disable_channel(uint8_t cs, uint8_t channel) {
  uint8_t temp_value;

  if (cs == USBHID_TESTFIXTURE_MAX335_CS_0) {
    temp_value = max335_state_0;
    temp_value &= ~(1 << channel);
    max335_state_0 = temp_value;
    
    digitalWrite(SPI_CS0_PIN, LOW);
    SPI.transfer(temp_value);
    digitalWrite(SPI_CS0_PIN, HIGH);
  } else if (cs == USBHID_TESTFIXTURE_MAX335_CS_1) {
    temp_value = max335_state_1;
    temp_value &= ~(1 << channel);
    max335_state_1 = temp_value;
    
    digitalWrite(SPI_CS1_PIN, LOW);
    SPI.transfer(temp_value);
    digitalWrite(SPI_CS1_PIN, HIGH);
  } else {
    return;
  }
}

void set_mode(int mode) {
  switch (mode) {
    case USBHID_TESTFIXTURE_MODE_SADC:
    case USBHID_TESTFIXTURE_MODE_ADC:
      dac_mux_disable();
      pwm_mux_disable();
      
      adc_mux_enable();
      break;

    case USBHID_TESTFIXTURE_MODE_SDAC:
    case USBHID_TESTFIXTURE_MODE_DAC:
      adc_mux_disable();
      pwm_mux_disable();
      
      dac_mux_enable();   
      break;

    case USBHID_TESTFIXTURE_MODE_PWM_FREQ:
    case USBHID_TESTFIXTURE_MODE_PWM_DUTY:
      dac_mux_disable();
      adc_mux_disable();
      
      pwm_mux_enable();
      break;

    case USBHID_TESTFIXTURE_MODE_COUNTER:
      //dac_mux_disable();
      break;

     default:
      break;
  }
}

void pwm_set_duty_value(uint8_t channel, uint16_t value) {
  switch (channel) {
    case TEENSY_PWM_0:
      analogWrite(PWM_0_PIN, value);
      break;

    case TEENSY_PWM_1:
      analogWrite(PWM_1_PIN, value);
      break;

    case TEENSY_PWM_2:
      analogWrite(PWM_2_PIN, value);
      break;

    case TEENSY_PWM_3:
      analogWrite(PWM_3_PIN, value);
      break;

    case TEENSY_PWM_4:
      analogWrite(PWM_4_PIN, value);
      break;
      
    default:
      return;
  }
}

void pwm_set_freq_value(uint8_t channel, uint32_t value) {
  analogWriteFrequency(PWM_0_PIN, value);
  analogWriteFrequency(PWM_1_PIN, value);
}

void dac_mux_set_channel(uint8_t channel) {
  return max4617_set_sel(channel);
}

void pwm_mux_disable(void) {
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_0, USBHID_CHANNELS_FIO_0);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_0, USBHID_CHANNELS_FIO_1);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_0, USBHID_CHANNELS_FIO_2);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_0, USBHID_CHANNELS_FIO_3);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_0, USBHID_CHANNELS_FIO_4);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_0, USBHID_CHANNELS_FIO_5);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_0, USBHID_CHANNELS_FIO_6);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_0, USBHID_CHANNELS_FIO_7);
}

void pwm_mux_enable(void) {
  // do nothing
}

void adc_mux_disable(void) {
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_1, USBHID_CHANNELS_FIO_0);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_1, USBHID_CHANNELS_FIO_1);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_1, USBHID_CHANNELS_FIO_2);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_1, USBHID_CHANNELS_FIO_3);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_1, USBHID_CHANNELS_FIO_4);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_1, USBHID_CHANNELS_FIO_5);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_1, USBHID_CHANNELS_FIO_6);
  max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_1, USBHID_CHANNELS_FIO_7);
}

void adc_mux_enable(void) {
  // do nothing
}

void dac_mux_disable(void) {
  digitalWrite(DAC_MUX_EN_PIN, HIGH);
}

void dac_mux_enable(void) {
  digitalWrite(DAC_MUX_EN_PIN, LOW);
}

void pwm_enable_channel(uint8_t channel) {
  return max335_enable_channel(USBHID_TESTFIXTURE_MAX335_CS_0, channel);
}

void pwm_disable_channel(uint8_t channel) {
  return max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_0, channel);
}

void adc_enable_channel(uint8_t channel) {
  return max335_enable_channel(USBHID_TESTFIXTURE_MAX335_CS_1, channel);
}

void adc_disable_channel(uint8_t channel) {
  return max335_disable_channel(USBHID_TESTFIXTURE_MAX335_CS_1, channel);
}

uint16_t adc_get_value(uint8_t channel) {
  return adc->adc0->analogRead(channel);
}

void dac_init(void) {
  pinMode(DAC_MUX_SEL_A_PIN, OUTPUT);
  pinMode(DAC_MUX_SEL_B_PIN, OUTPUT);
  pinMode(DAC_MUX_SEL_C_PIN, OUTPUT);

  pinMode(DAC_MUX_EN_PIN, OUTPUT);
}

void adc_test(void) {
  // enable ADC0 through FIO_0 and sample ADC_0 value
  adc_enable_channel(USBHID_CHANNELS_FIO_0);

  for (int i = 0; i < 100; i++) {
    uint16_t value;
    value = adc_get_value(ADC_0_PIN);
    
    //Serial.print("Value: ");
    Serial.print(value, DEC);
    Serial.println();

    delay(1);
  }
}

void pwm_test(void) {
  // set PWM0 duty cycle 50% and enable PWM on FIO_0

  /*
  pwm_set_freq_value(TEENSY_PWM_0, 2048);
  pwm_set_freq_value(TEENSY_PWM_1, 32);
  pwm_set_freq_value(TEENSY_PWM_2, 64);
  pwm_set_freq_value(TEENSY_PWM_3, 128);
  pwm_set_freq_value(TEENSY_PWM_4, 256);
  pwm_set_freq_value(TEENSY_PWM_5, 512);
  pwm_set_freq_value(TEENSY_PWM_6, 1024);
  pwm_set_freq_value(TEENSY_PWM_7, 2048);
  */

  pwm_set_freq_value(TEENSY_PWM_0, 1000);
  pwm_set_freq_value(TEENSY_PWM_1, 100000);

  pwm_set_duty_value(TEENSY_PWM_0, 2048);
  pwm_set_duty_value(TEENSY_PWM_1, 1024);

  pwm_enable_channel(USBHID_CHANNELS_FIO_0);
  pwm_enable_channel(USBHID_CHANNELS_FIO_1);
}

void dac_test(void) { 
  // write sample value on the DAC
  analogWrite(DAC_PIN, 1024);

  // set DAC mode and route DAC to FIO_0
  set_mode(USBHID_TESTFIXTURE_MODE_DAC);
  dac_mux_set_channel(USBHID_TESTFIXTURE_DAC_SEL_FIO_0);
}

void print_header(void) {
  Serial.println("usbhid_testfixture_app foo\r\n");
  Serial.print("Version: ");
  Serial.print(USBHID_TESTFIXTURE_MAJOR_VER, DEC);
  Serial.print(".");
  Serial.print(USBHID_TESTFIXTURE_MINOR_VER, DEC);
  Serial.println();
  Serial.println();
}

void print_usage(void) {
  Serial.println("<command>,<opt1>,<opt2>,...,<optN>");
  Serial.println();
  Serial.println("<adc>,<0>");
}

void prompt(void) {
  Serial.print("> ");
}

void timer_init(void) {
  startTimerValue0 = timer0.begin(timer0_callback, period0);
}

void adc_init(void) {
  ///// ADC0 ////
    // reference can be ADC_REFERENCE::REF_3V3, ADC_REFERENCE::REF_1V2 (not for Teensy LC) or ADC_REFERENCE::REF_EXT.
    //adc->setReference(ADC_REFERENCE::REF_1V2, ADC_0); // change all 3.3 to 1.2 if you change the reference to 1V2

    adc->setAveraging(32); // set number of averages
    adc->setResolution(12); // set bits of resolution

    adc->setConversionSpeed(ADC_CONVERSION_SPEED::MED_SPEED); // change the conversion speed
    adc->setSamplingSpeed(ADC_SAMPLING_SPEED::MED_SPEED); // change the sampling speed
    
    Serial.println("Starting Timers");

    delayMicroseconds(25); // if we wait less than 36us the timer1 will interrupt the conversion
    adc->enableInterrupts(ADC_0);

    Serial.println("Timers started");
    delay(500);
}

void pwm_pin_check(int pin, int index) {
  if (digitalRead(pin)) {
    // high state
    // compute lowtime
    int curTime = micros();
    
    deltaLow[index] = curTime - prevTimeFall[index];
    prevTimeRise[index] = curTime;
  } else {
    // low state
    // compute hightime

    int curTime = micros();

    // compute PWM duty cycle params
    deltaHigh[index] = curTime - prevTimeRise[index];
    prevTimeFall[index] = curTime;
  }

  pwmTickCount[index]++;
}

void pwm0_pin_change(void) {
  pwm_pin_check(PWM_0_PIN, TEENSY_PWM_0);
}

void pwm1_pin_change(void) {
  pwm_pin_check(PWM_1_PIN, TEENSY_PWM_1);
}

void pwm2_pin_change(void) {
  pwm_pin_check(PWM_2_PIN, TEENSY_PWM_2);
}

void pwm3_pin_change(void) {
  pwm_pin_check(PWM_3_PIN, TEENSY_PWM_3);
}

void pwm4_pin_change(void) {
  pwm_pin_check(PWM_4_PIN, TEENSY_PWM_4);
}

void pwm_pin_mode_input(void) {
  pinMode(PWM_0_PIN, INPUT_PULLUP);
  pinMode(PWM_1_PIN, INPUT_PULLUP);
  pinMode(PWM_2_PIN, INPUT_PULLUP);
  pinMode(PWM_3_PIN, INPUT_PULLUP);
  pinMode(PWM_4_PIN, INPUT_PULLUP);
}

void pwm_pin_mode_output(void) {
  pinMode(PWM_0_PIN, OUTPUT);
  pinMode(PWM_1_PIN, OUTPUT);
  pinMode(PWM_2_PIN, OUTPUT);
  pinMode(PWM_3_PIN, OUTPUT);
  pinMode(PWM_4_PIN, OUTPUT);

  analogWrite(PWM_0_PIN, 2048);
  analogWrite(PWM_1_PIN, 2048);
  analogWrite(PWM_2_PIN, 2048);
  analogWrite(PWM_3_PIN, 2048);
  analogWrite(PWM_4_PIN, 2048);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // setup LED lines
  // LED0 = off (heartbeat)
  // LED1 = on (activity)
  pinMode(LED0_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  digitalWrite(LED0_PIN, HIGH);
  digitalWrite(LED1_PIN, LOW);
  
  analogWriteResolution(12);

  // enable SPI for MAX335 SPDT switches for ADC and PWM
  pinMode(SPI_CS1_PIN, OUTPUT);
  pinMode(SPI_CS0_PIN, OUTPUT);

  digitalWrite(SPI_CS1_PIN, HIGH);
  digitalWrite(SPI_CS0_PIN, HIGH);
  
  SPI.begin(); 

  SPI.setClockDivider(SPI_CLOCK_DIV64);
  SPI.setBitOrder(MSBFIRST); 
  SPI.setDataMode(SPI_MODE0);

  timer_init();
  dac_init();
  adc_init();

  dac_mux_disable();
  pwm_mux_disable();
  adc_mux_disable();
}

int dac_input_get(int channel, uint8_t *channelTemp) {
  if (!channelTemp) {
    return -1;
  }
  
  switch (channel) {
    case USBHID_SERIAL_CHANNEL_0:
      *channelTemp = USBHID_TESTFIXTURE_DAC_SEL_FIO_0;
      break;

    case USBHID_SERIAL_CHANNEL_1:
      *channelTemp = USBHID_TESTFIXTURE_DAC_SEL_FIO_1;
      break;

    case USBHID_SERIAL_CHANNEL_2:
      *channelTemp = USBHID_TESTFIXTURE_DAC_SEL_FIO_2;
      break;

    case USBHID_SERIAL_CHANNEL_3:
      *channelTemp = USBHID_TESTFIXTURE_DAC_SEL_FIO_3;
      break;

    case USBHID_SERIAL_CHANNEL_4:
      *channelTemp = USBHID_TESTFIXTURE_DAC_SEL_FIO_4;
      break;

    case USBHID_SERIAL_CHANNEL_5:
      *channelTemp = USBHID_TESTFIXTURE_DAC_SEL_FIO_5;
      break;

    case USBHID_SERIAL_CHANNEL_6:
      *channelTemp = USBHID_TESTFIXTURE_DAC_SEL_FIO_6;
      break;

    case USBHID_SERIAL_CHANNEL_7:
      *channelTemp = USBHID_TESTFIXTURE_DAC_SEL_FIO_7;
      break;

    default:
      *channelTemp = 0xFF;
      break;
  }
  
  return 0;
}

int input_get_channel_pin(int channel, uint8_t *channelTemp, uint8_t *adcPin) {
  if ((!channelTemp) || (!adcPin)) {
    return -1;
  }
  
  switch (channel) {
    case USBHID_SERIAL_CHANNEL_0:
      *channelTemp = USBHID_CHANNELS_FIO_0;
      *adcPin = ADC_0_PIN;
      break;

    case USBHID_SERIAL_CHANNEL_1:
      *channelTemp = USBHID_CHANNELS_FIO_1;
      *adcPin = ADC_1_PIN;
      break;

    case USBHID_SERIAL_CHANNEL_2:
      *channelTemp = USBHID_CHANNELS_FIO_2;
      *adcPin = ADC_2_PIN;
      break;

    case USBHID_SERIAL_CHANNEL_3:
      *channelTemp = USBHID_CHANNELS_FIO_3;
      *adcPin = ADC_3_PIN;
      break;

    case USBHID_SERIAL_CHANNEL_4:
      *channelTemp = USBHID_CHANNELS_FIO_4;
      *adcPin = ADC_4_PIN;
      break;

    case USBHID_SERIAL_CHANNEL_5:
      *channelTemp = USBHID_CHANNELS_FIO_5;
      *adcPin = ADC_5_PIN;
      break;

    case USBHID_SERIAL_CHANNEL_6:
      *channelTemp = USBHID_CHANNELS_FIO_6;
      *adcPin = ADC_6_PIN;
      break;

    case USBHID_SERIAL_CHANNEL_7:
      *channelTemp = USBHID_CHANNELS_FIO_7;
      *adcPin = ADC_7_PIN;
      break;

    default:
      *channelTemp = 0xFF;
      *adcPin = 0xFF;
      break;
  }
  
  return 0;
}

int dumpStreamADCBuffer(void) {
  uint16_t val;

  while (!buffer_sadc->isEmpty()) {
    val = buffer_sadc->read();
    Serial.print(val, DEC);

    if (!buffer_sadc->isEmpty()) {
      Serial.print(",");
    }
  }
  Serial.println();

  return 0;
}

int sdac_action(int channel, int action, int freq, int nsamples) {
  if ((nsamples * sizeof(uint16_t)) >= DAC_SBUFFER_SIZE) {
    Serial.println("NOK,INVALID_SAMP_LEN");
    return -1;
  }

  Serial.print("AOK,0,");
  Serial.println(action, DEC);
  
  return 0;
}

int sadc_action(int channel, int action, int freq, int nsamples) {
  uint16_t value;
  uint8_t channelTemp;
  uint8_t adcPin;

  adcStreamCountStop = nsamples;
  readPin = adcPin;
  
  if ((nsamples * sizeof(uint16_t)) >= ADC_SBUFFER_SIZE) {
    Serial.println("NOK,INVALID_SAMP_LEN");
    return -1;
  }

  if (input_get_channel_pin(channel, &channelTemp, &adcPin) != 0) {
    return -1;
  }

  readPin0 = channelTemp;
  adc_enable_channel(channelTemp);

  float foo = 1.0f;
  foo = foo / (float) freq;
  foo = foo * 1e6;
  period0 = (int) foo;

  // in microseconds
  timer0.update(period0);
  adc_active = true;
  
  while (adcStreamFinish == false) {
    //Serial.println("waiting..");
    delay(1);
  }

  //Serial.println("hold condition over");
  dumpStreamADCBuffer();

  adcStreamCount = 0;
  adcStreamFinish = false;
  return 0;
}

int adc_action(int channel, int action) {
  uint16_t value;
  uint8_t channelTemp;
  uint8_t adcPin;

  //Serial.println("adc_action A");
  if (input_get_channel_pin(channel, &channelTemp, &adcPin) != 0) {
    return -1;
  }

  //Serial.println("adc_action B");
  readPin0 = channelTemp;

  if (action == USBHID_SERIAL_ACTION_ENABLE) {
    //Serial.println("adc_action C");
    adc_enable_channel(channelTemp);
    delay(10);
    value = adc_latest;

    //Serial.println("adc_action D");

    Serial.print("AOK,");
    Serial.print(channel, DEC);
    Serial.print(",");
    Serial.println(value);
  } else if (action == USBHID_SERIAL_ACTION_DISABLE) {
    //Serial.println("adc_action D");
    adc_disable_channel(channelTemp);
    Serial.println("AOK,0,0");
    //Serial.println("adc_action E");
  }

  //Serial.println("adc_action F");
  
  return 0;
}

// taken from:
// https://bigdanzblog.wordpress.com/2017/10/27/watch-dog-timer-wdt-for-teensy-3-1-and-3-2/
void reset_action(void) {
  noInterrupts();                                         // don't allow interrupts while setting up WDOG
  WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;                         // unlock access to WDOG registers
  WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
  delayMicroseconds(1);                                   // Need to wait a bit..

  // for this demo, we will use 1 second WDT timeout (e.g. you must reset it in < 1 sec or a boot occurs)
  WDOG_TOVALH = 0x006d;
  WDOG_TOVALL = 0xdd00;

  // This sets prescale clock so that the watchdog timer ticks at 7.2MHz
  WDOG_PRESC  = 0x400;

  // Set options to enable WDT. You must always do this as a SINGLE write to WDOG_CTRLH
  WDOG_STCTRLH |= WDOG_STCTRLH_ALLOWUPDATE |
      WDOG_STCTRLH_WDOGEN | WDOG_STCTRLH_WAITEN |
      WDOG_STCTRLH_STOPEN | WDOG_STCTRLH_CLKSRC;
  interrupts();

  // delay 10 sec forcing a reset
  delay(10000UL);
}

int info_action(int channel, int action) {
  Serial.print("AOK,");
  Serial.print(USBHID_TESTFIXTURE_MAJOR_VER, DEC);
  Serial.print(",");
  Serial.println(USBHID_TESTFIXTURE_MINOR_VER, DEC);
  
  return 0;
}

void pwm_detach_interrupts(void) {
  detachInterrupt(PWM_0_PIN);
  detachInterrupt(PWM_1_PIN);
  detachInterrupt(PWM_2_PIN);
  detachInterrupt(PWM_3_PIN);
  detachInterrupt(PWM_4_PIN);
}

void pwm_attach_interrupt(int channel) {
  switch (channel) {
    case USBHID_CHANNELS_FIO_0:
      attachInterrupt(PWM_0_PIN, pwm0_pin_change, CHANGE);
      break;
      
    case USBHID_CHANNELS_FIO_1:
      attachInterrupt(PWM_1_PIN, pwm1_pin_change, CHANGE);
      break;

    case USBHID_CHANNELS_FIO_2:
      attachInterrupt(PWM_2_PIN, pwm2_pin_change, CHANGE);
      break;

    case USBHID_CHANNELS_FIO_3:
      attachInterrupt(PWM_3_PIN, pwm3_pin_change, CHANGE);
      break;

    case USBHID_CHANNELS_FIO_4:
      attachInterrupt(PWM_4_PIN, pwm4_pin_change, CHANGE);
      break;
  }
}

int pwm_duty_action(int channel, int action) {
  pwm_detach_interrupts();
  pwm_mux_disable();
  
  pwm_attach_interrupt(channel);
  pwm_enable_channel(channel);
  delay(5); // settle
  
  // computer floating point duty cycle
  float duty_cycle;
  
  if ((deltaHigh[channel] == 0) && (deltaLow[channel] == 0)) {
    duty_cycle = 0.0;
  } else {
     duty_cycle = ((float) deltaHigh[channel] / ( (float) (deltaHigh[channel] + deltaLow[channel])));
  }
  
  Serial.print("AOK,");
  Serial.print(duty_cycle);
  Serial.println(",");
  
  return 0;
}

int pwm_freq_action(int channel, int action) {
  pwm_detach_interrupts();
  pwm_mux_disable();
  
  pwm_attach_interrupt(channel);
  pwm_enable_channel(channel);
  delay(5); // settle

  float freq;
  float timePeriod;
  float deltaTime;
  int startTick, endTick;

  startTick = pwmTickCount[channel];
  delay(100);
  endTick = pwmTickCount[channel];
  
  // computer floating point duty cycle
  deltaTime = (float) (endTick - startTick);
  deltaTime = deltaTime / 2.0;

  timePeriod = (float) (1.0 / deltaTime);
  freq = (float) 1.0 / timePeriod;
  freq = freq * 10.0;
  
  Serial.print("AOK,");
  Serial.print(freq);
  Serial.println(",");
  
  return 0;
}

int pwm_set_action(int channel, int action) {
  pwm_mux_disable();
  pwm_set_freq_value(channel, action);

  pwm_enable_channel(channel);
  delay(5); // settle
  
  Serial.print("AOK,0,");
  Serial.println(action, DEC);
  
  return 0;
}

int dac_action(int channel, int action) {
  uint8_t channelTemp;
  uint8_t adcPin;

  if (dac_input_get(channel, &channelTemp) != 0) {
    return -1;  
  }

  // write sample value on the DAC
  analogWrite(DAC_PIN, action);

  // set DAC mode and route DAC to FIO_0
  set_mode(USBHID_TESTFIXTURE_MODE_DAC);
  dac_mux_set_channel(channelTemp);

  Serial.print("AOK,0,");
  Serial.println(action, DEC);
  
  return 0;
}

void process() {
  uint8_t incomingByte;
  
  if (Serial.available() > 0) {
    incomingByte = Serial.read();

    if ((incomingByte == '\r') || (incomingByte == '\n')) {
      rxSerialBuffer[rxSerialBufferOffset] = 0x00;

      char* token = strtok( (char *) rxSerialBuffer, ",");
      int tokenIndex = 0;

      if (token) {
        char *tokenChannel;
        char *tokenAction;
        int channel;
        int action;
        
        tokenChannel = strtok(NULL, ",");
        tokenAction = strtok(NULL, ",");

        if ((tokenChannel) && (tokenAction)) {
          channel = atoi(tokenChannel);
          action = atoi(tokenAction);

          if ((channel < USBHID_SERIAL_CHANNEL_0) || (channel > USBHID_SERIAL_CHANNEL_7)) {
            Serial.println("NOK,0,0");
            goto _cleanup;
          }

          activity_led();
          
          if (strcmp(token, "adc") == 0) {
            //Serial.println("adc mode active");
            testfixture_mode = USBHID_TESTFIXTURE_MODE_ADC;
            adc_action(channel, action);
          } else if (strcmp(token, "sadc") == 0) {
            char *tokenFreq;
            char *tokenNsamples;
            int freq;
            int nsamples;

            tokenFreq = strtok(NULL, ",");
            tokenNsamples = strtok(NULL, ",");
            
            if ((!tokenFreq) || (!tokenNsamples)) {
              Serial.println("NOK,0,0");
              goto _cleanup;
            }

            freq = atoi(tokenFreq);
            nsamples = atoi(tokenNsamples);

            // action = auto stream loop mode
            stream_loop_mode = action;

            // Streaming ADC samples 
            testfixture_mode = USBHID_TESTFIXTURE_MODE_SADC;
            //Serial.println("starting sadc action");
            sadc_action(channel, action, freq, nsamples);
          } else if (strcmp(token, "dac") == 0) {
            testfixture_mode = USBHID_TESTFIXTURE_MODE_DAC;
            dac_action(channel, action);
          } else if (strcmp(token, "sdac") == 0) {
            pwm_mux_disable();
            adc_mux_disable();

            pwm_pin_mode_input();
            
            char *tokenFreq;
            char *tokenNsamples;
            char *token;
            int freq;
            int nsamples;
  
            tokenFreq = strtok(NULL, ",");
            tokenNsamples = strtok(NULL, ",");
            
            if ((!tokenFreq) || (!tokenNsamples)) {
              Serial.println("NOK,0,0");
              goto _cleanup;
            }

            freq = atoi(tokenFreq);
            nsamples = atoi(tokenNsamples);

            float foo = 1.0f;
            foo = foo / (float) freq;
            foo = foo * 1e6;
            period0 = (int) foo;

            // in microseconds
            timer0.update(period0);
            
            // pull off sample DAC values specified in comma delimited list
            token = strtok(NULL, ",");
            while (token != NULL) {
              buffer_sdac->write(atoi(token));
              token = strtok(NULL, ",");
            }

            uint8_t channelTemp;
            dac_input_get(channel, &channelTemp);
            
            // set DAC mode and route DAC to FIO_0
            set_mode(USBHID_TESTFIXTURE_MODE_SDAC);
            dac_mux_set_channel(channelTemp);
  
            // Streaming DAC samples 
            testfixture_mode = USBHID_TESTFIXTURE_MODE_SDAC;
            sdac_action(channel, action, freq, nsamples);
          } else if (strcmp(token, "pwm_set") == 0) {
            pwm_pin_mode_output();

            testfixture_mode = USBHID_TESTFIXTURE_MODE_PWM;
            pwm_set_action(channel, action);
          } else if (strcmp(token, "pwm_duty") == 0) {
            pwm_pin_mode_input();
            
            testfixture_mode = USBHID_TESTFIXTURE_MODE_PWM_DUTY;
            pwm_duty_action(channel, action);
          } else if (strcmp(token, "pwm_freq") == 0) {
            pwm_pin_mode_input();
            
            testfixture_mode = USBHID_TESTFIXTURE_MODE_PWM_FREQ;
            pwm_freq_action(channel, action);
          } else if (strcmp(token, "info") == 0) {
            testfixture_mode = USBHID_TESTFIXTURE_MODE_NONE;
            info_action(channel, action);
          } else if (strcmp(token, "stop") == 0) {
            dac_mux_disable();
            pwm_mux_disable();
            adc_mux_disable();

            pwm_pin_mode_input();
            
            Serial.println("AOK,0,0");
          } else if (strcmp(token, "reset") == 0) {
            testfixture_mode = USBHID_TESTFIXTURE_MODE_NONE;
            reset_action(); 
          } else {
            testfixture_mode = USBHID_TESTFIXTURE_MODE_NONE;
            Serial.println("unknown request");
          }
        }
      }

      _cleanup:
      rxSerialBufferOffset = 0;
      memset(rxSerialBuffer, 0x00, SERIAL_BUFFER_LENGTH);
    } else {
      rxSerialBuffer[rxSerialBufferOffset++] = incomingByte;
    }
  }
}

void loop() {
  uint8_t incomingByte;

  heartbeat_led();
  
  if (firstRun && (millis() > 2000)) {
    firstRun = false;
    
    print_header();
    prompt();
  }

  process();
}

