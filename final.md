# Final Robot Design

## Mechanical

The final mechanical design of 

## Hardware

We wanted to mill PCBS but they didn't turn out well so we hand soldered the schematic onto perf boards

We added an additional line sensor to have dynamic thresholds

## Software

We had several peices of code split over a few microcontrollers: FPGA code that did treasure detection, Base station code that received messages and updated the GUI, Robot code that ran the maze exploration and primary sensor code, and the atmega code which ran the FFT code and set up the camera.

### FPGA

The FPGA software was slightly modified from the milestone 4 code. We adjusted some thresholds and camera parameters to get a good image.

### Arduino

We split up the final arduino software between our atmega chip and our arduino. The atmega chip set up the camera and performed continuous FFT on either the IR sensor or the microphone, depending on the mode selection from the primary arduino. 

# Final Competion

## Videos

## Performance