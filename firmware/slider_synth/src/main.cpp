#include "MozziConfigValues.h"  // for named option values

#define MOZZI_AUDIO_MODE MOZZI_OUTPUT_EXTERNAL_TIMED
#define MOZZI_AUDIO_BITS 24
#define MOZZI_CONTROL_RATE 256 // Hz, powers of 2 are most reliable

#include <Mozzi.h>
#include <Oscil.h>
#include <tables/cos2048_int8.h> // table for Oscils to play
#include <SPI.h>
#include <DAC_MCP49xx.h>  // https://github.com/tomcombriat/DAC_MCP49XX 
// which is an adapted fork from https://github.com/exscape/electronics/tree/master/Arduino/Libraries/DAC_MCP49xx  (Thomas Backman)

// Synthesis part
Oscil<COS2048_NUM_CELLS, MOZZI_AUDIO_RATE> aCos1(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, MOZZI_AUDIO_RATE> aCos2(COS2048_DATA);
Oscil<COS2048_NUM_CELLS, MOZZI_CONTROL_RATE> kEnv1(COS2048_DATA);

#define SPI_SCK   12  // SCK (Clock)
#define SPI_MOSI  11  // SDI (MOSI)
#define SPI_MISO  9   // SDO (MISO) UNUSED, NOT ON THE MCP4922
#define SPI_CS    10  // Chip Select (CS)

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

// External audio output parameters and DAC declaration
#define SS_PIN 40  // if you are on AVR and using PortWrite you need still need to put the pin you are actually using: 7 on Uno, 38 on Mega
#define BITS_PER_CHANNEL 12  // each channel of the DAC is outputting 12 bits

DAC_MCP49xx dac(DAC_MCP49xx::MCP4922, SS_PIN);

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
  // Set custom SPI pins
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);
  
  aCos1.setFreq(440.f);
  aCos2.setFreq(220.f);
  kEnv1.setFreq(0.3f);

  dac.init();   // start SPI communications

  //dac.setPortWrite(true);  //comment this line if you do not want to use PortWrite (for non-AVR platforms)
  
  pinMode(1, INPUT_PULLUP);
  pinMode(5, OUTPUT);

  startMozzi();
  Serial.println("on!!!!!!!!");
}

// Carry enveloppes
int env1 = 0;

void updateControl() {
  env1 = kEnv1.next();
}

AudioOutput updateAudio() {
  return MonoOutput::fromNBit(24, (int32_t)aCos1.next() * aCos2.next() * env1) ; // specify that the audio we are sending here is 24 bits.
}

void loop() {
  audioHook();
}