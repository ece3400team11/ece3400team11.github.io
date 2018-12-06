---
layout: blank
title:  "Lab 3"
subtitle: "Radio and Integrations"
preview: "assets/images/lab3.png"
---

# Lab 3: Radio and Integrations

## Radio Team

The radio team worked on a simulated robot and interfacing with the GUI.

First, we had to download the radio library and set it up for the Arduino IDE. We then downloaded the GettingStarted sketch and replaced it with the one given to us in the RF24 library. The radios were then connected to the arduino and powered through the DC power supply as our arduinos could not supply enough current. Each radio was given a channel number based on the formula given and then we followed the code from the GettingStarted example. 

We had to decide on a data structure to store to store the contents of the maze in. For now, we have decided to implement an unsigned char array to store our data as this gives 8 bits to store information about every square. 4 bits are used to represent wall directions (North, East, South, and West, in that order), two bits for treasure shape (3 possible shapes plus no treasure), one bit for treasure color (red or blue), and one valid bit (has the robot explored this square yet?).

![Image](labs/lab3/images/array_data.png)

### Robot Simulation

First, we generated a maze similar to the 3x3 example given in the GUI release code using our unsigned char array data structure. Initially, we hard coded the movements of our simulated robot just to make sure the data from the array was being read properly. First, the robot put it’s current coordinates in a string. Based on the 4 wall bits, the robot then appended different messages to a string (ex: 1100 -> “North=true,East=true”) and printed it out via the serial port. The Python GUI code then reads the serial output and updates the GUI appropriately. Here is our code for taking the data received from the robot and generating the serial message:

```cpp
String convert(unsigned char x, unsigned char y, unsigned char curr){
  String s = String(y)+","+String(x)+",north=false";
  if(bitRead(curr,4)==1){s = s + ",west=true";}
  if(bitRead(curr,5)==1){s = s + ",south=true";}
  if(bitRead(curr,6)==1){s = s + ",east=true";}
  if(bitRead(curr,7)==1){s = s + ",north=true";}
  return s;
}
```

Once we verified that this was working, we added a right-hand wall following algorithm to our simulated robot so that it could autonomously run through the entire programmed maze. Here is the code we wrote to achieve this:

```cpp
// get the current position's maze data and perform Right hand following
void follow(){
  unsigned char curr = maze[y][x];
  if((bitRead(curr, dir+4)==0) && (bitRead(curr, (dir+1)%4 + 4)==1)){
    adv();
  }
  else if(bitRead(curr, (dir+1)%4 + 4)==0){
    dir = (dir+1)%4;
    adv();
  }
  else if((bitRead(curr, dir+4)==1) && (bitRead(curr, (dir+1)%4 + 4)==1) && (bitRead(curr, (dir+3)%4 + 4)==0)){
    dir = (dir+3)%4;
    adv();
  }
}
```

Based on the current walls in front of the simulated robot, the robot would either choose to turn left, right, or continue straight. To verify that the simulated robot was indeed right-hand wall following, we created a square maze and had the robot start on the edge. In theory, the robot should never explore the middle of the maze and that is what we observed.

After we had the simulated robot working on the same arduino as the base station communication, we tested sending the maze information between arduinos using the Nordic RF modules. We decided on a 2 byte data packet. The first byte contained the x and y coordinates of the robot, while the second byte is identical to the data stored in the unsigned char array. We added code to deconstruct and translate the data packet on the base station.

Here is a demo video of the simulation. The arduino connected to the computer is acting as the base station and the other arduino is simulation a robot in a maze:

<iframe width="560" height="315" src="https://www.youtube.com/embed/Y5K-v3xKlyE" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

## Robot Team

The robot team was in charge of integrating everything we had been working on up to this point. This included detecting the 660 Hz signal, detecting IR hats, mapping out a maze using the right hand rule, and transmitting the maze information back to the base station. 

### Hardware

Before we jumped into the code, we decided to clean up the circuitry on our robot. We had the IR detection on one breadboard and the servos, line sensors, and distance sensors on another breadboard. We decided to move everything to a single breadboard and add the microphone circuit. However, we realized that our microphone circuit had started behaving in unexpected ways and wasn’t giving a reliable signal anymore. 

When probing our old microphone circuit with the oscilloscope, we found that there was some underlying frequency being produced somewhere in the circuit. At first we thought this was due to all the new components added to the robot since the last time the microphone was used, but this issue persisted even after the microphone circuit was separated entirely.

Our original circuit used two op amps for signal amplification. Through probing we found that our first amplification stage was working fine, so we decided to ditch the second stage. To make up for this, we increased the gain of the first stage.

![Image](labs/lab3/images/Mic_circuit_diagram.JPG)

The final circuit was split into 2 sections. The first section contained the IR and microphone circuits, which were connected to the atmega on pins A0 and A1. The second section contained the connections for the 2 servos, 2 line sensors, and 2 short range IR distance sensors. The atmega chip was connected to the main arduino through 2 pins: atmega pin 13 to arduino pin 7 and atmega pin 12 to arduino pin 8.

![Image](labs/lab3/images/20181021_163810.jpg)

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

![Image](labs/lab3/images/state_mach.png)

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

Below is a demo of the robot going through a simple maze. The robot successfully starts on a 660 Hz tone, it maps out the 2 by 3 maze and stops whever it detects and IR hat to prevent robot collisions. We still had to power the radio with the lab bench power supply because we didn't have a voltage regulator on our board to provide the necessary current.

<iframe width="560" height="315" src="https://www.youtube.com/embed/3a3IrtT6TUA" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

Here is a top down view of the maze we were running through:

![Image](labs/lab3/images/20181021_155327.jpg)

And here is an image of the GUI after the robot had traversed the maze and transmitted the necessary data back to the base station:

![Image](labs/lab3/images/maze.jpg)

## Full lab code

You can find the full that we used for the lab [here](https://github.com/ece3400team11/ece3400team11.github.io/tree/master/_labs/lab3/code)

## Future Improvements

Currently, we have moved all of our circuits on to one breadboard for compactness, but it would be best if all of our circuits were on a single PCB so that we can make our robot as compact and robust as possible by avoiding silly errors due to issues such as wires falling out. This would also allow us to stack the PCB directly on the arduino and prevent use from having to add a second level on our robot, thus further increasing its stability and robustness.

As for the maze-mapping capabilities of the robot, we would like to add a third short-range wall sensor to the side of our robot and a long-range wall sensor on the front of our robot in addition to the short-range sensor already present there, as mentioned in our milestone 2 report. This would allow us to more accurately map out the maze and do so with the least amount of errors. On a similar note, we plan on getting multiple readings from the ATMega microcontroller so that we can differentiate useful signals from the background noise picked up by the sensors (possibly through a debouncing state machine) in order to further increase the accuracy and robustness of our robot. Finally, we plan on adding a manual start button for redundancy in the case the our robot doesn’t start on the 660 Hz tone in actual competition.
