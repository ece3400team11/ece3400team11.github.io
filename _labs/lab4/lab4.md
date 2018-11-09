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

First we define the register addresses:
```cpp
#define OV7670_I2C_ADDRESS 0x21
#define COM7 0x12
#define COM3 0x0C
#define CLKRC 0x11
#define COM17 0x42
#define MVFP 0x1E
#define COM15 0x40
#define COM9 0x14
```

Then we set the values as specified above:
```cpp
OV7670_write_register(COM7, 0x80);
   
set_color_matrix();

OV7670_write_register(COM7, 0x0E);
OV7670_write_register(COM3, 0x08);
OV7670_write_register(CLKRC, 0xC0);
OV7670_write_register(COM17, 0x08);
OV7670_write_register(COM15, 0xD0);
OV7670_write_register(COM9, 0x1A);
OV7670_write_register(MVFP, 0x30);
```

## Part 2 - Setting up the FPGA

The first thing we had to do to setup the FPGA was to generate all the clock signals that we would require to run our external devices (namely the camera and the VGA module). This was done by creating a PLL which would take the already provided 50 MHz clock signal as an input and use it to generate the 24 and 25 MHz clock signals. The 24 MHz clock signal was then assigned to a GPIO pin to allow it to be used by the camera. The 25 MHz clock signal was assigned to the clock for the VGA module and the read clock for the M9K module. The 50 MHz clock signal was assigned to the write clock for the M9K module. We used different clocks for the read and write ports of the memory module to make sure that we don’t read and write from the same address at the same time.

```cpp
///////* INSTANTIATE YOUR PLL HERE *///////
team11PLL	team11PLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( CLOCK_24_PLL ),
	.c1 ( CLOCK_25_PLL),
	.c2 ( CLOCK_50_PLL )
);

///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(CLOCK_50),
	.clk_R(CLOCK_25_PLL),
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(CLOCK_25_PLL),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(CLOCK_25_PLL),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.VSYNC(VSYNC),
	.HREF(HREF),
	.RESULT(RESULT)
);
```

Once we had the clocks all set up, we connected the VGA adapter to the specified GPIO pins on the FPGA (GPIO_0[5] -> GPIO_0[23]). The VGA adapter reads the pixel color information from the memory and outputs it as a VGA signal which can be displayed on the screen. 

## Part 3 - Displaying  a Test Pattern on the Screen

After setting up the FPGA and VGA Adapter, we had to make sure we had the ability to write to the M9K buffer so that we could store the current frame from the camera. Before integrating the camera, we manually wrote a test pattern into the M9K RAM to make sure that the VGA Driver was connected properly and that the correct image would be output to the monitor.

To write out test pattern to the buffer, we used two counter variables: one that incremented on every clock cycle, and one that incremented for every row. When the first counter reached our image width, we incremented the second counter and reset the first one back to zero. When the second counter reached our image height, it meant that we were done writing the entire test pattern into memory, so we reset both the counters. 

For our test pattern, we simply wrote a red line to the M9K buffer every 10 lines using a for loop. Here is our code to do this:

```cpp
///////* Buffer Writer *///////

integer i = 0;
integer j = 0;

always @(posedge CLOCK_24_PLL) begin
		W_EN = 1;
		
		X_ADDR = i;
		Y_ADDR = j;
		
		if (i < `SCREEN_WIDTH && (j % 10 == 0))
			pixel_data_RGB332 = RED;
		else
			pixel_data_RGB332 = BLACK;
			
		i = i+1;
			
		if (i == `SCREEN_WIDTH) begin
			i = 0;
			j = j+1;
			
			if (j == `SCREEN_HEIGHT) begin
				j = 0;
			end
		end
end
```

Here is an image of our result:

// Insert image of test pattern here

## Part 4 - Downsampling and Displaying Camera Output on the Screen

We had the camera set up to record pixel data in the RGB 565 format, which meant that each individual pixel consists of 16 bits. However, our M9K RAM could only allocate 8 bits per pixel, so it was necessary to downsample the input coming in from the camera to the RGB 332 format. We also had to take into account the fact the camera could only output 8 bits of a pixel at a time, so to completely read the 16 bit pixel data, we would need to read two consecutive inputs from the camera. For the most accurate color representations after downsampling, we took the most significant bits from each color and concatenated them together to form our RGB 332 value which we then wrote to the M9K RAM. 

// Insert camera datasheet picture with HREF anf VSYNC

Once we were happy with our downsampling, we had to figure out how and when to write to the buffer so that the image we see is properly synchronized with the camera. Initially we were only using the HREF signal output from the camera in order to tell when to change what line of the image we were writing. When we did this, we saw our image was skewed both horizontally and vertically. Also, our color bar would slowly shift to the side even though it should remain stationary. We were initially using the created 24 MHz clock on the FPGA to write to the buffer since that was the same clock we were sending to the camera, but when we compared the 24 MHz clock with the PCLOCK camera output using the oscilloscope, we realized that the PCLOCK was slightly slower than the 24 MHz it was receiving from the FPGA, so we decided to sync our buffer using this clock instead. Our image was now no longer shifting, but our colorbar was still vertically and horizontally skewed. After also incorporating the VSYNC camera output, which indicated when we were receiving a frame, with our memory buffer we finally got a steady image from the camera.

On every valid input from the camera, we flipped a bit we labeled k, which allowed us to tell whether the input we were receiving from the camera was the first or second half of the 16-bit pixel data. On receiving the second half of the pixel data, we downsampled it to 8 bits by concatenating together bits [7:5] and [2:0] of the first byte with bits [4:3] of the second byte, updated our x position and wrote the data to the buffer. When HREF went high, we paused as the data from the camera was not valid. We updated our y position and reset our x position during this time. When VSYNC went low, we also paused as the data from the camera was not valid.  We reset both our x and y positions during this time so we would be ready to write the next frame to the buffer. The code for the downsampler is shown below:

```cpp
///////* Downsampler *///////
reg[7:0] i = 0;
reg[7:0] j = 0;
reg      k = 0;

reg      h_flag = 0;
reg      v_flag = 0;

reg  [7:0]	input_1 = 8'd0;
reg  [7:0]	input_2 = 8'd0;
wire [7:0]	CAMERA_INPUT;
assign 		CAMERA_INPUT = {GPIO_1_D[23],GPIO_1_D[21],GPIO_1_D[19],GPIO_1_D[17],
				GPIO_1_D[15],GPIO_1_D[13],GPIO_1_D[11],GPIO_1_D[9]};
wire			P_CLOCK;
assign		P_CLOCK = GPIO_1_D[7];
wire			HREF;
assign		HREF = GPIO_1_D[5];
wire			VSYNC;
assign		VSYNC = GPIO_1_D[3];

always @(posedge P_CLOCK) begin

	if (VSYNC == 1'b1 && v_flag == 0) begin
		i = 0;
		j = 0;
		v_flag = 1;
	end
	
	else if (VSYNC == 1'b0) begin
	
		v_flag = 0;
		
		if (HREF == 1'b0 && h_flag == 0) begin
			k = 0;
			i = 0;
			j = j+1;
			h_flag = 1;
		end
		
		else if (HREF == 1'b1) begin
			h_flag = 0;
			
			X_ADDR = i;
			Y_ADDR = j;
			
			if (k == 0) begin
				input_1 = CAMERA_INPUT;
				k = 1;
				W_EN = 0;
			end
			
			else begin
				input_2 = CAMERA_INPUT;
				pixel_data_RGB332 = {input_2[7:5], input_2[2:0], input_1[4:3]};
				i = i+1;
				k = 0;
				W_EN = 1;
			end
		end
	end		
end
```

At this point our images were very clear, but our colors were entirely wrong. Instead of reading color, it appeared that our camera was only measuring light intensity. After comparing our register values with the TAs, we realized that we had addressed our COM15 register incorrectly. After we fixed this we finally received a color bar that was close to what we were expecting.

// Insert color bar image

We switched back to displaying our camera output and finally saw images that were properly colored. We messed with the automatic gain of the camera for a bit until we achieved a brightness level we were happy with.

// Insert camera output image

## Part 5 - Color Detection

To test color detection, we decided to use three internal LEDs on the FPGA that would light up to show whether red, blue or neither color was detected. The actual color detection was performed in the Image Processor module.

For every valid pixel, the image processor would first determine its RGB values, and then by comparing them against a certain threshold value, it would decide if the pixel was red, blue, or neither and increment the counter for the same. These threshold values were determined by trial and error and are dependent on the camera and how it is setup. This same procedure was repeated for every pixel in the frame.

At the end of a frame, we compared the counter values for each color to a threshold value which determined whether the frame was majority red, majority blue, or neither. These threshold values were once again determined by trial and error and are dependent on the camera and how it is setup. The register res was assigned a value based on what color was seen. If there were enough blue pixels to detect blue, the res would be set to 0’b111. Likewise, res would be set to 0’b110 if red was detected. If neither color was detected, res would be set to 0’b000. The value of res was assigned to the output RESULT. The counters were then reset for the next frame.

```cpp
Include image processor code here
```

In the Deo Nano module, we took RESULT from the image processor and turned on the respective LEDs to indicate what the module had found. If the image processor determined that the frame was majority red, we turned on LED 7 on the FPGA.  If the image processor determined that the frame was majority blue, we turned on LED 6 on the FPGA.  Finally, if the image processor determined that the frame neither majority red nor majority blue, we turned on LED 0 on the FPGA.

```cpp
///////* Color Detection *///////
reg     LED_7;
reg     LED_6;
reg     LED_0;

assign   LED[7] = LED_7;
assign   LED[6] = LED_6;
assign   LED[0] = LED_0;

always @(*) begin
		if (RESULT == 3'b111) begin //turn on LED 7
			LED_7 = 1; 
			LED_6 = 1;
			LED_0 = 1;
		end
		else if (RESULT == 3'b111) begin //turn on LED 6
			LED_7 = 0;
			LED_6 = 1;
			LED_0 = 0;
		end
		else begin
			LED_7 = 0;
			LED_6 = 0;
			LED_0 = 1;
		end
end
```

## Part 6 - Communication with the Arduino

For ease of use and because we have enough pins left, we will go with a parallel data transmission protocol. We will use 3 wires that are each either low or high to indicate one on the 7 states of the treasure detector and an additional invalid state. The 7 states are:

- 000: Blue triangle
- 001: Blue square
- 010: Blue diamond
- 100: Red triangle
- 101: Red square
- 110: Red diamond
- x11: Nothing

## Part 7 - Future Improvements

