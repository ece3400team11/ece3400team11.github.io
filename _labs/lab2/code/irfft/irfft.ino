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

#define ATMEG

void setup() {
  Serial.begin(9600); // use the serial port
  TIMSK0 = 0; // turn off timer0 for lower jitter
  #ifdef ATMEG
  ADCSRA = 0xe5; // set the adc to free running mode
  #else
  ADCSRA = 0xe6; // set the adc to free running mode
  #endif
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0

  pinMode(7, OUTPUT);
}

#ifdef ATMEG
int irBinNum = 81;
int irThresh = 60;
#else
int irBinNum = 84;
int irThresh = 60;
#endif

void loop() {
  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      #ifdef ATMEG
      ADCSRA = 0xf5; // restart adc (64 prescalar)
      #else
      ADCSRA = 0xf6; // restart adc (64 prescalar)
      #endif
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

    if (fft_log_out[irBinNum] > irThresh) {
      digitalWrite(7, HIGH);
    } else {
      digitalWrite(7, LOW);  
    }
  }
}
