#include "MozziConfigValues.h"  // for named option values

#define MOZZI_AUDIO_MODE MOZZI_OUTPUT_EXTERNAL_TIMED
#define MOZZI_AUDIO_BITS 24
#define MOZZI_CONTROL_RATE 256 // Hz, powers of 2 are most reliable

#include <Mozzi.h>
#include <Oscil.h>
#include <tables/sin2048_int8.h> // table for Oscils to play

#include <SPI.h>
#include <DAC_MCP49xx.h>  // https://github.com/tomcombriat/DAC_MCP49XX 
// which is an adapted fork from https://github.com/exscape/electronics/tree/master/Arduino/Libraries/DAC_MCP49xx  (Thomas Backman)
#include <ESP32Encoder.h>
#include <Ticker.h>

#define Linear_Pot_Pin 13 // Linear Potentiometer
#define FSR_Pin 4 // Force Sensitive Resistor

#define LED0_Pin 39
#define LED1_Pin 40
#define LED2_Pin 41
#define LED3_Pin 42

#define TX_Pin 43
#define RX_Pin 44

#define Rotary_A_Pin 21
#define Rotary_B_Pin 47
#define Rotary_Button_Pin 48

#define SPI_SCK   12  // SCK (Clock)
#define SPI_MOSI  11  // SDI (MOSI)
#define SPI_MISO  9   // SDO (MISO) UNUSED, NOT ON THE MCP4922
// External audio output parameters and DAC declaration
#define SS_PIN 10  // if you are on AVR and using PortWrite you need still need to put the pin you are actually using: 7 on Uno, 38 on Mega
#define BITS_PER_CHANNEL 12  // each channel of the DAC is outputting 12 bits

const int fsr_vel_start = 18000;
const int fsr_velocity_limit = 56688;

int starting_octave = 3; //cant be less than 0
const int scale = 3; //scale of octaves playable

int pitch; //27.5hz-4186hz
int velocity; //0-127

const float base_freq = 32.703; // C1
float hz_map_low;
float hz_map_high;

float depth = 0.02;

//not me
const float depth_min = 0.0;
const float depth_max = 0.2;
const float depth_step = 0.01;

float vibrato_speed = 6.5;
const float vibrato_speed_min = 1.0;
const float vibrato_speed_max = 15.0;
const float vibrato_speed_step = 0.5;

float filter_cutoff = 1.0;
const float filter_cutoff_min = 0.1;
const float filter_cutoff_max = 2.0;
const float filter_cutoff_step = 0.1;
//not me above

int mode = 0;
int changing_to = 0;
bool changing_mode = false;

// Synthesis part
Oscil <SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <2048, MOZZI_CONTROL_RATE> kVib(SIN2048_DATA);

DAC_MCP49xx dac(DAC_MCP49xx::MCP4922, SS_PIN);

ESP32Encoder encoder;

int linear_val;
int force_val;
int encoder_count = 0;
bool slow_down = false;

void ledModeFlash();

Ticker ledModeFlashTimer(ledModeFlash, 100);

void ledModeFlash() {

  digitalWrite(LED0_Pin, LOW);
  digitalWrite(LED1_Pin, LOW);
  digitalWrite(LED2_Pin, LOW);
  digitalWrite(LED3_Pin, LOW);

  if (changing_mode) {
    switch (changing_to) {
      case 0:
        digitalWrite(LED0_Pin, !digitalRead(LED0_Pin));
        break;
      case 1:
        digitalWrite(LED1_Pin, !digitalRead(LED1_Pin));
        break;
      case 2:
        digitalWrite(LED2_Pin, !digitalRead(LED2_Pin));
        break;
      case 3:
        digitalWrite(LED3_Pin, !digitalRead(LED3_Pin));
        break;
    }
  } else {
    switch (mode) {
      case 0:
        digitalWrite(LED0_Pin, HIGH);
        break;
      case 1:
        digitalWrite(LED1_Pin, HIGH);
        break;
      case 2:
        digitalWrite(LED2_Pin, HIGH);
        break;
      case 3:
        digitalWrite(LED3_Pin, HIGH);
        break;
    }
  }
}

void buttonSlowDown();

Ticker slowDownTimer(buttonSlowDown, 1000);

void buttonSlowDown() {
  slow_down = false;
}

void generateHz() {
  hz_map_low = base_freq * pow(2, starting_octave - 1);
  hz_map_high = hz_map_low * pow(2, scale);
}

void octaveUp() {
  if (starting_octave >= 8) return;

  starting_octave++;
  generateHz();
}

void octaveDown() {
  if (starting_octave <= -2) return;

  starting_octave--;
  generateHz();
}

void vibratoUp() {
  if (depth >= depth_max) return;
  
  depth += depth_step;
  if (depth > depth_max) depth = depth_max;
}

void vibratoDown() {
  if (depth <= depth_min) return;
  
  depth -= depth_step;
  if (depth < depth_min) depth = depth_min;
}

void vibratoSpeedUp() {
  if (vibrato_speed >= vibrato_speed_max) return;
  
  vibrato_speed += vibrato_speed_step;
  if (vibrato_speed > vibrato_speed_max) vibrato_speed = vibrato_speed_max;
  
  kVib.setFreq(vibrato_speed);
}

void vibratoSpeedDown() {
  if (vibrato_speed <= vibrato_speed_min) return;
  
  vibrato_speed -= vibrato_speed_step;
  if (vibrato_speed < vibrato_speed_min) vibrato_speed = vibrato_speed_min;
  
  kVib.setFreq(vibrato_speed);
}

void filterUp() {
  if (filter_cutoff >= filter_cutoff_max) return;
  
  filter_cutoff += filter_cutoff_step;
  if (filter_cutoff > filter_cutoff_max) filter_cutoff = filter_cutoff_max;
}

void filterDown() {
  if (filter_cutoff <= filter_cutoff_min) return;
  
  filter_cutoff -= filter_cutoff_step;
  if (filter_cutoff < filter_cutoff_min) filter_cutoff = filter_cutoff_min;
}

void runEncoderModeUp() {
  switch (mode) {
    case 0:
      octaveUp();
      break;
    case 1:
      vibratoUp();
      break;
    case 2:
      vibratoSpeedUp();
      break;
    case 3:
      filterUp();
      break;
  }
}

void runEncoderModeDown() {
  switch (mode) {
    case 0:
      octaveDown();
      break;
    case 1:
      vibratoDown();
      break;
    case 2:
      vibratoSpeedDown();
      break;
    case 3:
      filterDown();
      break;
  }
}

void audioOutput(const AudioOutput f) // f is a structure containing both channels
{

  int out = MOZZI_AUDIO_BIAS + f.l();

  unsigned short lowBits = (unsigned short) out;  //
  unsigned short highBits =  out >> BITS_PER_CHANNEL;

  dac.output2(highBits, lowBits);  // outputs the two channels in one call.
}

void setup() {
  // serial for uart
  Serial.begin(115200, SERIAL_8N1, RX, TX);

  Serial.println("udu3324 was here!!");

  // Set custom SPI pins
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  dac.init();   // start SPI communications
  //dac.setPortWrite(true);  //comment this line if you do not want to use PortWrite (for non-AVR platforms)
  
  //calculate the starting hz
  generateHz();

  //vibrato setup
  kVib.setFreq(vibrato_speed);

  pinMode(LED0_Pin, OUTPUT);
  pinMode(LED1_Pin, OUTPUT);
  pinMode(LED2_Pin, OUTPUT);
  pinMode(LED3_Pin, OUTPUT);

  pinMode(Rotary_A_Pin, INPUT_PULLUP);
  pinMode(Rotary_B_Pin, INPUT_PULLUP);

  pinMode(Rotary_Button_Pin, INPUT_PULLUP);

  encoder.attachSingleEdge(Rotary_A_Pin, Rotary_B_Pin);
  encoder.setCount(0);

  startMozzi();
  Serial.println("on!!!!!!!!");

  ledModeFlashTimer.start();
  slowDownTimer.start();

}

// Carry enveloppes
float filter_memory = 0.0; // For simple low-pass filter

void updateControl() {
  Serial.println("it is looping!!!");

  if (digitalRead(Rotary_Button_Pin) == LOW && !slow_down) {
    changing_mode = !changing_mode;

    if (!changing_mode) {
      mode = changing_to;
    }

    slow_down = true;
  }

  if (encoder.getCount() > encoder_count) {
    if (changing_mode && changing_to < 3 ) {
      changing_to = changing_to + 1;
    } else {
      runEncoderModeUp();
    }
  } else if (encoder.getCount() < encoder_count) {
    if (changing_mode && changing_to > 0 ) {
      changing_to = changing_to - 1;
    } else {
      runEncoderModeDown();
    }
  }

  encoder_count = encoder.getCount();

  ledModeFlashTimer.update();
  slowDownTimer.update();

  linear_val = mozziAnalogRead16(Linear_Pot_Pin);
  force_val = mozziAnalogRead16(FSR_Pin);

  //map to freq and vel
  pitch = map(linear_val, 0, 65536, hz_map_low, hz_map_high);
  velocity = map(force_val, fsr_vel_start, fsr_velocity_limit, 0, 20);

  //hard limits
  if ((linear_val == 0)) pitch = 0; //stop mapping of zero
  if (velocity > 127) velocity = 127; //stop higher mappings

  //stop negative numbers for some reason
  if (velocity < 0) velocity = 0;

  float vibrato = (velocity * depth) * kVib.next();

  Serial.print("p = \t");
  Serial.print(pitch);
  Serial.print("\t v = ");
  Serial.println(velocity);

  if (linear_val > 0) {
    aSin.setFreq(pitch + vibrato);
  } else {
    aSin.setFreq(0);
  }
}

AudioOutput updateAudio() {
  int32_t sample = (int32_t)aSin.next();
  
  // Apply simple low-pass filter (cutoff controlled by filter_cutoff)
  float alpha = filter_cutoff / (filter_cutoff + 1.0);
  filter_memory = alpha * sample + (1.0 - alpha) * filter_memory;
  
  int32_t filtered_sample = (int32_t)filter_memory;
  
  return MonoOutput::fromNBit(24, filtered_sample); // specify that the audio we are sending here is 24 bits.
}

void loop() {  
  audioHook();
}