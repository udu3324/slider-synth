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

#define Linear_Pot_Pin 13 // Linear Potentiometer
#define FSR_Pin 4 // Force Sensitive Resistor

#define LED1_Pin 39
#define LED2_Pin 40
#define LED3_Pin 41
#define LED4_Pin 42

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

const int starting_octave = 3; //cant be less than 0
const int scale = 3; //scale of octaves playable

int pitch; //27.5hz-4186hz
int velocity; //0-127

const float base_freq = 32.703; // C1
float hz_map_low;
float hz_map_high;

float depth = 0.05;

// Synthesis part
Oscil <SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <2048, MOZZI_CONTROL_RATE> kVib(SIN2048_DATA);

DAC_MCP49xx dac(DAC_MCP49xx::MCP4922, SS_PIN);

ESP32Encoder encoder;

int linear_val;
int force_val;

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
  hz_map_low = base_freq * pow(2, starting_octave - 1);
  hz_map_high = hz_map_low * pow(2, scale);

  //vibrato setup
  kVib.setFreq(6.5f);

  pinMode(LED1_Pin, OUTPUT);
  pinMode(LED2_Pin, OUTPUT);
  pinMode(LED3_Pin, OUTPUT);
  pinMode(LED4_Pin, OUTPUT);

  pinMode(Rotary_B_Pin, INPUT);

  encoder.attachHalfQuad(Rotary_A_Pin, Rotary_A_Pin);
  encoder.setCount(0);

  startMozzi();
  Serial.println("on!!!!!!!!");
}

// Carry enveloppes
int env1 = 0;

void updateControl() {
  Serial.println("it is looping!!!");

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

  float vibrato = (velocity * 0.02) * kVib.next();

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
  return MonoOutput::fromNBit(24, (int32_t)aSin.next()) ; // specify that the audio we are sending here is 24 bits.
}

void loop() {  
  audioHook();
}