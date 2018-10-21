/*
fft_adc_serial.pde
guest openmusiclabs.com 7.7.14
example sketch for testing the fft library.
it takes in data on ADC0 (Analog0) and processes them
with the fft. the data is sent out over the serial
port at 115.2kb.
*/

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library

// 0 = 660 hz, 1 = 6.08 khz
int mode = 0;

#define DATA_PIN 13
#define MODE_PIN 12

void setup() {
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe6; // set the adc to free running mode
  ADMUX = 0x41; // use adc1 for audio
  DIDR0 = 0x01; // turn off the digital input for adc0

  pinMode(DATA_PIN, OUTPUT);
  pinMode(MODE_PIN, INPUT);
}

int irBinNum = 81;
int irThresh = 70;
int irSRA = 0xf5;
int micBinNum = 34;
int micThresh = 165;
int micSRA = 0xf7;

int window = 3;

void loop() {
  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      if (mode == 0) {
        // audio prescalar
        ADCSRA = micSRA; // restart adc (128 prescalar)
      }
      else {
        // ir prescalar
        ADCSRA = irSRA; // restart adc (32 prescalar)
      }
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();

    int sum = 0;
    int bin = 10;
    int thresh = 0;

    if (mode == 0) {
      bin = micBinNum;
      thresh = micThresh;
    } else {
      bin = irBinNum;
      thresh = irThresh;
    }

    for(int i = bin - window/2; i <= bin + window/2; i++) {
      sum += fft_log_out[i];
    }
    sum = sum / window;
    if (sum > thresh) {
      digitalWrite(DATA_PIN, HIGH);
    } else {
      digitalWrite(DATA_PIN, LOW);  
    }

    if (digitalRead(MODE_PIN) == LOW) {
      mode = 0;
      ADMUX = 0x41; // use adc1 for audio
    } else {
      mode = 1;
      ADMUX = 0x40; // use adc0 for IR
    }
  }
}
