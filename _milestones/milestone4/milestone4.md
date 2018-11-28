---
layout: blank
title:  "Milestone 4"
subtitle: "Vision"
status: "active" # disabled, active, complete
preview: "assets/images/project-1.jpg"
---

# Milestone 4

For this milestone, we had to do complete shape and color detection and communicate the data back to the arduino. The general overview of our FPGA code is to first detect edges, then record the color of that edge, and if we have enough edges that are of a certain color and in a certain direction, then we are able to determine a shape and color.

## Shape detection

Our shape detection starts with edge detection. In order to perform edge detection, we first convert the pixel values to grayscale by summing the red, green, and blue components into a pixel intensity value:

```verilog
grey = input_2[7:3] + {input_2[2:0], input_1[7:6]} + input_1[4:0];
```

Then, we treat the pixel values as a 1-D signal and store the previous 4 grayscale values:

```verilog
grey_pppp = grey_ppp;
grey_ppp = grey_pp;
grey_pp = grey_p;
grey_p  = grey;
```

In order to remove noise, we set the current grayscale value to be the average of the current and previous grayscale values:

```verilog
grey_blur = grey + grey_p;
grey_blur = grey_blur >> 1;
grey = grey_blur[7:0];
```

Note that this is equivalent to performing a convolution on the grayscale signal with the signal [1/2 1/2].
Next, we perform edge detection by convolving the pixel signal with an edge detection signal
[-1 -1 0 1 1]. Note that this convolution allows us to detect non-horizontal right edges.

```verilog
tmp_1 = grey - grey_pppp;
tmp_2 = grey_p - grey_ppp;
grey_eg = tmp_1 + tmp_2
```

Finally, every 12 lines we determine the x coordinate of the brightest edge pixel and compare it with the x coordinate of the brightest pixel from 12 lines above it. If the difference is positive, we record a negative edge. If the difference is negative, we record a positive edge. If the difference is around 0, we record a straight edge.

```verilog
if (Y_ADDR % 12 == 0 && X_ADDR >= 30 && X_ADDR < 150) begin
  if (grey_eg[7:3] > curBrightestVal) begin
    curBrightestVal = grey_eg[7:3];
    curXB = X_ADDR;
  end
end

if (Y_ADDR % 12 == 0 && X_ADDR == 150 ) begin
  if (curBrightestVal >= 1) begin
    xDiff = curXB - prevXB;
    if (xDiff > 3 && xDiff < 16) begin
      numNeg = numNeg + 1;
    end
    else if (xDiff < -3 && xDiff > -16) begin
      numPos = numPos + 1;
    end
    else if (xDiff <= 3 && xDiff >= -3) begin
      numStraight = numStraight + 1;
    end
  end
  prevXB = curXB;
  curBrightestVal = 0;
end
```

Later, we use the number of straight, positive, and negative edges to determine the shape.

## Color detection

In order to determine what color a shape is, we look at the color to the left of the brightest edge that we detect. This allows our edge detection to be more robust, since we avoid the case where we see many straight edges but the color is white, like at a break in the maze wall. The code first computes the L1 distance between the current pixel and the hard coded values for red and blue:

```verilog
redRDif = 5'b11111 - pixel_data_RGB565[15:11];
redGDif = 5'b01000 - pixel_data_RGB565[10:6];
redBDif = 5'b01000 - pixel_data_RGB565[4:0];
blueRDif = 5'b00001 - pixel_data_RGB565[15:11];
blueGDif = 5'b00001 - pixel_data_RGB565[10:6];
blueBDif = 5'b00101 - pixel_data_RGB565[4:0];
```

Next, once it comes across the pixel directly below and 10 to the left of the brightest edge pixel, it checks to see if the current pixel is close enough to red or blue to be considered red or blue:

```verilog
if (Y_ADDR % 12 == 1 && X_ADDR == prevXB - 20) begin
  if (redRDif <  `C_THRESH && redRDif >  -`C_THRESH &&
      redGDif <  `C_THRESH && redGDif >  -`C_THRESH &&
      redBDif <  `C_THRESH && redBDif >  -`C_THRESH    )
      numRed = numRed + 1;
  else if (blueRDif <  `B_THRESH && blueRDif >  -`B_THRESH &&
           blueGDif <  `B_THRESH && blueGDif >  -`B_THRESH &&
           blueBDif <  `B_THRESH && blueBDif >  -`B_THRESH    )
      numBlue = numBlue + 1;
end
```

## FPGA comunication with the Ardunio

It then sets 3 wires to the appropriate values to encode the 7 possible options according to the scheme envisioned in lab 4. The arduino periodically polls these signals and prints out the value to the serial monitor:

```verilog
res = 3b'111;
if (numRed >= 6) begin
  if      ( numNeg >= 6 ) begin
    res = 3b'000;
  end
  if      ( numStraight >= 6 ) begin
    res = 3b'001;
  end
  if      ( numNeg >= 3 && numPos >= 3 ) begin
    res = 3b'010;
  end
end
else if (numBlue >= 6) begin
  if      ( numNeg >= 6 ) begin
    res = 3b'100;
  end
  if      ( numStraight >= 6 ) begin
    res = 3b'101;
  end
  if      ( numNeg >= 3 && numPos >= 3 ) begin
    res = 3b'110;
  end
end
```

```cpp
int w1 = digitalRead(4);
int w2 = digitalRead(5);
int w3 = digitalRead(6);
if (w1 && w2) {
  Serial.println("Nothing");
} else if (!w1 && !w2 && !w3) {
  Serial.println("Blue triangle");
} else if (w1 && !w2 && !w3) {
  Serial.println("Blue square");
} else if (!w1 && w2 && !w3) {
  Serial.println("Blue diamond");
} else if (!w1 && !w2 && w3) {
  Serial.println("Red triangle");
} else if (w1 && !w2 && w3) {
  Serial.println("Red square");
} else if (!w1 && w2 && w3) {
  Serial.println("Red diamond");
}
```

## Demo:

Here is a demo of the full code working as expected:
