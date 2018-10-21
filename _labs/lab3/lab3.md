---
layout: blank
title:  "Lab 3"
subtitle: "Radio and Integrations"
preview: "assets/images/lab1_img.png"
---

# Lab 3: Radio and Integrations

## Radio Team

The radio team worked on a simulated robot and interfacing with the GUI

Demo video:

<iframe width="560" height="315" src="https://www.youtube.com/embed/Y5K-v3xKlyE" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

## Robot Team

The robot team was in charge of integrating everything we had been working on up to this point. This included detecting the 660 Hz signal, detecting IR hats, mapping out a maze using the right hand rule, and transmitting the maze information back to the base station. 

### Hardware

TODO

### Software

We continued to use the ATMega328P chip for performing FFT to sense other robot’s IR hats. However, for this lab we also had to detect a 660 Hz signal. We came up with a simple 2 pin communication protocol between the main arduino and the atmega. The first pin would be used as a mode select pin and would be an output on the arduino and an input on the atmega. The second pin was the data pin, and it was an output on the atmega and an input on the arduino. If the mode select pin was low, the atmega would try and detect the 660 Hz signal on its analog pin 1. If the signal was high, the atmega would detect the 6.08 KHz signal on its analog pin 0.

```cpp
if (digitalRead(MODE_PIN) == LOW) {
  mode = 0;
  ADMUX = 0x41; // use adc1 for audio
} else {
  mode = 1;
  ADMUX = 0x40; // use adc0 for IR
}
```

For each of the 2 modes we had a threshold and a bin number that we would look at to determine if we had detected the signal. 

```cpp
if (mode == 0) {
  bin = micBinNum;
  thresh = micThresh;
} else {
  bin = irBinNum;
  thresh = irThresh;
}
```

If the atmega determined that it had seen the signal, it would set the data pin to high. Otherwise, the data pin was set to low. We used a small window over the primary bin to make the code more robust to small changes in frequency since every IR hat has a slightly different frequency

```cpp
for(int i = bin - window/2; i <= bin + window/2; i++) {
  sum += fft_log_out[i];
}
sum = sum / window;
if (sum > thresh) {
  digitalWrite(DATA_PIN, HIGH);
} else {
  digitalWrite(DATA_PIN, LOW);  
}
```

We determined the bin numbers and thresholds using the software serial and matlab code from previous labs and burned the FFT code into the atmega.


After we had the hardware ready for the robot, we started integrating all the code we had written up until now. One issue that we noticed early on while testing the code was that the interrupts for the digital line sensors seemed to be interfering with the servo motors, which caused them to jitter when we sent them the stop command. We are not completely sure why but we think it might be that the servo timer interrupt can’t be triggered when the arduino is inside of an ISR. Because of this, we decided to move the line sensing code outside of the ISR and put it in the main code loop. This required us to translate our code into a state machine. The state machine executes a small chunk of code each loop depending on which state its in and may make state transitions. At the end of the loop, we read the line sensors.

The state machine currently has 8 states. The 1st state is when the robot is waiting for the 660 Hz tone to be detected. In this state, the robot reads the data pin connected to the atmega and switches to the 2nd state if it reads a high value.

The 2nd state is the main line following code. If one of the line sensors is over the white line, then the robot pivots in the opposite direction to compensate. If both of the line sensors are over the white line, then the robot determines that it has reached an intersection. When this happens, the robot moves forwards a little bit to center itself over the intersection and then determines what to do next. It first reads its front and right distance sensors for walls, updates its maze data, and sends the updated data to the base station. Then, if there was no wall to the right, it turns right by going to the 3rd state. If there was a wall to the right and in front, it turns left by going to the 6th state. Otherwise, it keeps going straight by staying in the 2nd state.

The turn left and turn right states are almost the same, just in opposite directions. The first turn left state turns the robot to the left until the left line sensor is not on a white line. This state is usually skipped but is necessary in case the robot wasn’t completely centered on the intersection. The second turn left state turns the robot to the left until the left line sensor is back on a line. Then it goes to the third turn left state which turns the robot ot the left until the left line sensor is not on a while line. The robot has now turned 90 degrees and can continue going forward by transitioning back to the 2nd state. There are 3 turn left and 3 turn right states, which brings the total up to 8 states. A diagram of the state machine is given below:

// Insert diagram

We also made our line sensing code a bit more robust to noise by adding a small ring buffer to store the previous N values. We set N to 4 to get a decent averager. As stated above, the line sensing code comes after the state transition code, so we have to make sure that each transition only relies on a signal line sensor reading. The line sensing code is given below:

```cpp
pinMode(SENSOR_LEFT_PIN, OUTPUT);
pinMode(SENSOR_RIGHT_PIN, OUTPUT);
digitalWrite(SENSOR_LEFT_PIN, HIGH);
digitalWrite(SENSOR_RIGHT_PIN, HIGH);
delayMicroseconds(10);
pinMode(SENSOR_LEFT_PIN, INPUT);
pinMode(SENSOR_RIGHT_PIN, INPUT);

unsigned long startTime = micros();

int detectLeft = 0;
int detectRight = 0;
unsigned long t = micros();

unsigned long leftTime = SENSOR_LEFT_READING;
unsigned long rightTime = SENSOR_RIGHT_READING;

//time how long the input is HIGH, but quit after 3ms as nothing happens after that
while(t - startTime < 3000) {

  if (digitalRead(SENSOR_LEFT_PIN) == LOW && detectLeft == 0) {
    leftTime = t - startTime;
    detectLeft = 1;
  }
  if (digitalRead(SENSOR_RIGHT_PIN) == LOW && detectRight == 0) {
    rightTime = t - startTime;
    detectRight = 1;
  }

  if (detectLeft == 1 && detectRight == 1){
    break;
  }
  t = micros();
}

leftSenseBuf[leftSenseHead] = leftTime;
leftSenseHead = (leftSenseHead + 1) % SENSOR_AVE_SIZE;

rightSenseBuf[rightSenseHead] = rightTime;
rightSenseHead = (rightSenseHead + 1) % SENSOR_AVE_SIZE;

int sum = 0;
for(int i = 0; i < SENSOR_AVE_SIZE; i++) {
  sum += leftSenseBuf[i];
}
SENSOR_LEFT_READING = sum / SENSOR_AVE_SIZE;

sum = 0;
for(int i = 0; i < SENSOR_AVE_SIZE; i++) {
  sum += rightSenseBuf[i];  
}
SENSOR_RIGHT_READING = sum / SENSOR_AVE_SIZE;
```

## Demo

Here is a demo of the integration working on a simple maze:


## Full lab code

You can find the full that we used for the lab [here](https://github.com/ece3400team11/ece3400team11.github.io/tree/master/_labs/lab3/code)
