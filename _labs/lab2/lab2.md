---
layout: blank
title:  "Lab 2"
subtitle: "Analog Circuitry and FFTs"
preview: "assets/images/lab1_img.png"
---

# Lab 2: Analog Circuitry and FFTs

## Acoustic Team

The acoustic team...

### Materials Used

TODO

### Procedure

![Image](labs/lab2/images/660hz.png)

## Optical Team

The optical team...

### Harware Construction

Here is a picture of the materials we were given:
![Image](labs/lab2/images/ir_parts_list.jpg)

We first built the simple series circuit shown in the online lab writeup. We then confirmed that the simple circuit was able to detect other robots by hooking up the IR hat to the power supply and powering it with 9V as specified in the lab writeup. Below is an image from the oscilloscope of the waveform emitted by the IR receiver.
![Image](labs/lab2/images/hatwaveform1.jpg)

After we accomplished this, we decided to add a simple RC low pass filter to our circuit in order to filter out frequencies higher than our sampling rate (which could “look” like the 6.08 KHz signal after performing FFT). We decided to try and set the filter cutoff frequency at about 7 KHz which should let the IR hat signal through but exponentially dampen every frequency higher than that. We ended up using a 1 nF capacitor and a 22 KOhm resistor for the low pass filter. We then verified the filter by analyzing the waveform and FFT response on the oscilloscope.
<Oscilloscope screen grab of FFT and signal>

We then noticed that the voltage in the IR signal could be quite low when the IR hat was more than a few inches away. Therefore, we decided to build a simple non-inverting op amp with a target gain of around 2. We initially had some difficulties getting the non-inverting op amp to work correctly. We initially used the LF353 in an non inverting configuration but we never saw that amplified signal on the op amp; only a constant voltage at around 4.3 volts (not quite the voltage on the Vcc rail). After trying a few different 353 ICs, we tried switching to the LM358 op amp (which has the same pinout as the 353) and our amplification circuit immediately worked. We are still unsure of why the 353s weren’t working. We decided to use an 12 KOhm resistor as R1 and a 18 KOhm resistor as R2, giving a gain of about 1+18/12 = 2.5. As before, we verified that the op amp was behaving correctly by viewing the output waveform on the oscilloscope.
<Oscilloscope screen grab of signal and FFT>

### Software

Finally, we used the FFT library for the Arduino to measure the IR signal from port A0 and generate the amplitudes in the frequency bins using the following code:
```cpp
for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
  while(!(ADCSRA & 0x10)); // wait for adc to be ready
  ADCSRA = 0xf7; // restart adc
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
Serial.println("start");
for (byte i = 0 ; i < FFT_N/2 ; i++) { 
  Serial.println(fft_log_out[i]); // send out the data
}
```
We also added some additional setup code to have more control over how fast the ADC sampled data:
```cpp
TIMSK0 = 0; // turn off timer0 for lower jitter
ADCSRA = 0xe7; // set the adc to free running mode
ADMUX = 0x40; // use adc0
DIDR0 = 0x01; // turn off the digital input for adc0
```
The 7 at the end of the `ADCSRA` corresponds to the ADC clock prescalar, which determines how fast it samples data. This is important to tune so that you get proper frequency resolution for the frequencies that you are interested in.

We then used some matlab code to read the fft bin data from the arduino and plot the bin number vs. bin amplitude. This allowed us to visually inspect the result of the FFT analysis and figure out the bin number that corresponded with the 6.08K Hz signal and what a reasonable threshold should be to distinguish a signal from background noise. 
```matlab
myserialport = serial("/dev/cu.wchusbserial1410", "BaudRate", 9600)
fopen(myserialport)
binNums = [0:127];
bins = zeros(1,128);
try
    while 1
    start = fscanf(myserialport,"%s");
    if start == "start"
       break 
    end
    end
    for i = 1:128
        bins(i) = fscanf(myserialport,"%d")
    end
catch ME
    warning('error occured');
end
fclose(myserialport)
plot(binNums, bins)
```

Here is a video where the red LED shows if the arduino has detected another robot:
<iframe width="560" height="315" src="https://www.youtube.com/embed/cwhYxnZrcJQ" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

## Full lab code

You can find the full that we used for the lab [here](https://github.com/ece3400team11/ece3400team11.github.io/tree/master/_labs/lab2/code)
