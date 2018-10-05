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

We first built the simple series circuit shown in the online lab writeup. We then confirmed that the simple circuit was able to detect other robots by hooking up the IR hat to the power supply and powering it with 9V as specified in the lab writeup.
<Oscilloscope screen grab of FFT and signal>

After we accomplished this, we decided to add a simple RC low pass filter to our circuit in order to filter out frequencies higher than our sampling rate (which could “look” like the 6.08 KHz signal after performing FFT). We decided to try and set the filter cutoff frequency at about 7 KHz which should let the IR hat signal through but exponentially dampen every frequency higher than that. We ended up using a 1 nF capacitor and a 22 KOhm resistor for the low pass filter. We then verified the filter by analyzing the waveform and FFT response on the oscilloscope.
<Oscilloscope screen grab of FFT and signal>

We then noticed that the voltage in the IR signal could be quite low when the IR hat was more than a few inches away. Therefore, we decided to build a simple non-inverting op amp with a target gain of around 2. We initially had some difficulties getting the non-inverting op amp to work correctly. We initially used the LF353 in an non inverting configuration but we never saw that amplified signal on the op amp; only a constant voltage at around 4.3 volts (not quite the voltage on the Vcc rail). After trying a few different 353 ICs, we tried switching to the LM358 op amp (which has the same pinout as the 353) and our amplification circuit immediately worked. We are still unsure of why the 353s weren’t working. We decided to use an 12 KOhm resistor as R1 and a 18 KOhm resistor as R2, giving a gain of about 1+18/12 = 2.5. As before, we verified that the op amp was behaving correctly by viewing the output waveform on the oscilloscope.
<Oscilloscope screen grab of signal and FFT>

### Software

Finally, we used the FFT library for the Arduino to measure the IR signal from port A0 and generate the amplitudes in the frequency bins. 
<Code snippet>

We then used some matlab code to read the fft bin data from the arduino and plot the bin number vs. bin amplitude. This allowed us to visually inspect the result of the FFT analysis and figure out the bin number that corresponded with the 6.08K Hz signal and what a reasonable threshold should be to distinguish a signal from background noise. 
<Code snippet>

![Image](labs/lab2/images/6khztriangle.png)

## Full lab code

You can find the full that we used for the lab [here](https://github.com/ece3400team11/ece3400team11.github.io/tree/master/_labs/lab2/code)
