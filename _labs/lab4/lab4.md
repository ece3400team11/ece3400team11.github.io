---
layout: blank
title:  "Lab 4"
subtitle: "FPGA and Shape Detection"
preview: "assets/images/lab1_img.png"
---

# Lab 4: FPGA and Shape Detection

In this lab, we were required to develop a FPGA module capable of detecting basic shapes from a camera input and pass the information to the Arduino. This would then be mounted to the robot to be able to detect treasures within the maze. In order for this to be done, we have to ensure that the Arduino, camera and FPGA are able to communicate among each other. The end goal is to be able to display a test image from the camera to the screen and be able to detect color, specifically be able to differentiate between red and blue images.

## Pre-Lab

In the prelab we made a few simple calculations in order to determine what our camera settings should be and how we should set up the FPGA code. We first determined how much memory we had on the chip in terms of M9K blocks. This was directly from the data sheet: 594 kbits, which is around 74 Kbytes. Next, we figured out that the maximum color information we could output from the camera was RGB 565. After downsampling to RGB 332 (i.e. one byte per pixel), we need to figure out the maximum resolution that we can store the image at. Looking at the OV7670 datasheet, we noticed that we had a few options available to us: QCIF, QVGA, and CIF. CIF is 352 by 240, which is around 84 Kbytes, too big to fit in our memory. QVGA is 320 by 240, which is around 76 KBytes, just barely too big to fit into memory. QCIF is 176 by 144, which is 25 KBytes, so that will definitely fit into memory. Therefore, we need to set the output resolution to  the QCIF format.

## Part 1 - Setting up the Camera

Setting up the camera required inspecting the data sheet to figure out which of the camera registers we needed to write in order to output the test bar image from the camera. After we defined all of the registers addresses, we had to reset all of the registers to their default values, which we did by writing the msb of COM7. Then, we enabled the external clock by setting CLKRC. We then set the output format to QCIF RGB by writing COM7. We then wrote to COM15 to set the RGB output mode to RGB565 at full range. When enabling the color bar test we enabled the DSP color bar on COM17 and the color bar on COM7. The final code looks something like this:

```cpp

```

## Part 2 - Setting up the FPGA

The first thing we had to do to setup the FPGA was to generate all the clock signals that we would require to run our external devices (namely the camera and the VGA module). This was done by creating a PLL which would take the already provided 50 MHz clock signal as an input and use it to generate the 24 and 25 MHz clock signals. The 24 MHz clock signal was then assigned to a GPIO pin to allow it to be used by the camera. The 25 MHz clock signal was assigned to the clock for the VGA module and the read clock for the M9K module. The 50 MHz clock signal was assigned to the write clock for the M9K module. We used different clocks for the read and write ports of the memory module to make sure that we don’t read and write from the same address at the same time.

// Include code with all the module assignments

## Part 3 - Displaying  a Test Pattern on the Screen

Once we had the FPGA set up, we had to make sure we had the ability to write to the M9K buffer so that we could store the current frame from the camera. Before integrating the camera, we manually wrote a test pattern into the buffer to see if the correct image would be output to the monitor. We needed to write to the buffer at a faster rate than we read, so we decided to use our 50 MHz clock to write the data.

To test our buffer, we kept two counter variables: one that incremented on every clock cycle, and one that incremented for every line. When the first counter reached our image width, it reset to zero and we incremented our other counter. When this counter reached our image height, we reset both counters. We wrote a red line to the M9K buffer every 10 lines using a for loop. Here is our code to do this:

INSERT BUFFER WRITER CODE HERE

Here is an image of our result:

INSERT IMAGE HERE.

## Part 4 - Downsampling and Displaying Camera Output on the Screen

The camera data was output in RGB 565 format, which meant that it took 2 PCLK cycles to get the data for a single pixel. 

## Part 5 - Color Detection

Blah Blah

## Part 6 - Communication with the Arduino

For ease of use and because we have enough pins left, we will go with a parallel data transmission protocol. We will use 3 wires that are each either low or high to indicate one on the 7 states of the treasure detector and an additional invalid state. The 7 states are:

- 000: Blue triangle
- 001: Blue square
- 010: Blue diamond
- 100: Red triangle
- 101: Red square
- 110: Red diamond
- x11: Nothing
