---
layout: blank
title:  "Milestone 2"
subtitle: "Sensors"
status: "complete" # disabled, active, complete
preview: "assets/images/ms2.png"
---

# Milestone 2

## Goals:

For Milestone 2, we had to use the IR distance sensors to add wall-detection capability to the robot and successfully circle inside a set of walls using right-hand following. While in the maze the robot will have to avoid other robots so we had to make sure our robot was capable of detecting other robots, and then determining what it needed to do to avoid the robot. Finally, we had to prepare a demonstration that shows the what the robot was “thinking” while navigating the maze and detecting walls or other robots using various LEDs.


## Hardware Modifications

For Milestone 2, we had to add another level on top of our current robot so that we could add the circuits from Lab 2 as well as an IR sensor on each side and the front of the robot. The IR sensors were able to detect the presence and distance of a wall in front of or to the side of the robot. These IR wall sensors enabled us to use the right-hand wall following method to navigate the maze.

In order to add another level onto the robot, we first had to disassemble a significant portion of our first level in order to rearrange components for the second level as well as to replace some of our flimsy structures and connections with more solid ones. Once this was done, we attached another base plate on top of our original one using 3D printed beams. The final design has the battery pack strapped to the bottom of the first level base plate, the arduino mounted on top of the first level base plate, and two small breadboards on the second level base plate to handle all the connections.

![Image](milestones/milestone2/images/m2_4.jpg)

## Setting up Wall Sensors:

The IR wall sensors provided to us consisted of an IR-LED and an angle sensitive detector that simply used angle of reflection of the light from the object to determine its distance from the sensor. These wall sensors output analog values so they were quite simple to set up and read on the Arduino. The code for reading the wall sensors involves doing an analog read and checking the value against a threshold

```cpp
if (analogRead(FRONT_IR_SENSOR) > WALL_THRESH) {
  ...
}
```

We then had to decide whether to use the short-range or long-range IR sensors for the purposes of wall following. Using some trial and error, we determined that for the distances that were away from the wall the short-range IR sensor reading was much more accurate and stable than the long-range one, so we decided to use only the short-range IR sensors for the purposes of this milestone.

Once we determined an appropriate threshold value for the wall sensor to successfully detect a wall, we could begin working on our Wall Detection algorithm.

## Detecting other Robots:

For this milestone we decided to use a simple robot avoidance strategy. If our robot detects another robot, it simply stops and waits for the other robot to pass by. In order to maintain the accuracy and range of the IR Hat detection circuit, we decided that it would be best to continue using software FFT instead of trying to detect the frequency purely in hardware. However, we realized that the FFT library would likely interfere with the servo library and would slow down our robot’s navigation due to the extra processing time required to do FFT. Therefore, we decided to offload the FFT calculation onto a separate ATMega328P chip. We programmed this chip with the Arduino as ISP code and calibrated the FFT code to the ATMega328P’s internal clock using the MATLAB code from lab 2. We then connected a wire between the ATMega chip and our main arduino in order for the ATMega chip to let our main board know if it had detected another robot. We used the same code we used for Lab 2 except we changed the prescaler and the bin number based on the measurements we made from the ATMega.

```cpp
#ifdef ATMEG
int irBinNum = 81;
int irThresh = 60;
#else
int irBinNum = 84;
int irThresh = 60;
#endif

void setup() {
  #ifdef ATMEG
  ADCSRA = 0xe5; // set the adc to free running mode
  #else
  ADCSRA = 0xe6; // set the adc to free running mode
  ...
}

void loop() {
  ...
  // turn on LED and potentially send data back to main board
  if (fft_log_out[irBinNum] > irThresh) {
    digitalWrite(7, HIGH);
  } else {
    digitalWrite(7, LOW);  
  }
}
```

## Integrating Line Tracking and Robot Detection:

We started with our line following code from Milestone 2 and added the wall detection and IR Hat detection code to it in order to perform a right hand maze traversal. The wall detection code involved doing an analog read and checking the value against a threshold when the robot got to an intersection. If there was no wall to the right of the robot, it would turn right. Otherwise, if there was a wall in front of the robot and to the right, the robot would turn left.

```cpp
if (analogRead(RIGHT_IR_SENSOR) < WALL_THRESH) {
  // no wall to the right
  turnRight();
} else if (analogRead(FRONT_IR_SENSOR) > WALL_THRESH) {
  // wall to the right and wall in front
  turnLeft();
}
```

Since we offloaded the IR detection code to the ATMega chip, only a digital read of the pin connected to the ATMega was required on each pass through our main loop and it was stopped when high:

```cpp
if(digitalRead(FFT_PIN) == HIGH) {
  leftWheel.write(STOP_POS);
  rightWheel.write(STOP_POS);
}
```

One issue that we noticed when making the integrations was that whenever we wanted to stop our servos they would jitter around. We figured out that it had something to do with the ISR running for the line sensors. We ended up detaching the interrupts whenever we stopped and then reattached them when we started moving again and that seemed to calm the servos down. We are still unsure as to why the ISR were affecting the servos.

```cpp
void detachInterrupts() {
  detachInterrupt(digitalPinToInterrupt(SENSOR_LEFT_PIN));
  detachInterrupt(digitalPinToInterrupt(SENSOR_RIGHT_PIN));
}

void attachInterrupts() {
  // Tell the compiler which pin to associate with which ISR
  attachInterrupt(digitalPinToInterrupt(SENSOR_LEFT_PIN), SENSOR_LEFT_ISR, LOW);
  attachInterrupt(digitalPinToInterrupt(SENSOR_RIGHT_PIN), SENSOR_RIGHT_ISR, LOW);

  // Setup the sensors
  setup_sensor(SENSOR_LEFT_PIN, &SENSOR_LEFT_TIMER);
  setup_sensor(SENSOR_RIGHT_PIN, &SENSOR_RIGHT_TIMER);
}

...

void loop() {
  ...
  leftWheel.write(STOP_POS);
  rightWheel.write(STOP_POS);
  detachInterrupts();
  delay(1000);
  attachInterrupts();
  ...
}
```

After all the integrations, our system is laid out like this:

![Image](milestones/milestone2/images/block_dgrm.png)

## Demo:

<iframe width="560" height="315" src="https://www.youtube.com/embed/DNPShllTkOQ" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

Here is an image of the final circuit for milestone 2:

![Image](milestones/milestone2/images/m2_1.jpg)

![Image](milestones/milestone2/images/m2_2.jpg)

## Future Improvements:

One of the improvements we discussed for our final robot was to include multiple multi-directional IR sensors instead of just the singular one we have currently. This would allow us to not only detect another robot, but also detect which direction it was coming from, thus allowing our robot to avoid other robots even if the robot detection functions of the other robots are not working as it should.

On a similar note, we considered augmenting the short-range IR sensor currently on the front of our robot with another long-range IR sensor. This would enable us to detect walls not only right in front of our robot but also those a few squares away, thus allowing us to implement a less reactive and more predictive maze exploration algorithm for our final robot.
