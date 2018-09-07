---
layout: subpage
title:  "Lab1"
---

# Lab 1

In Lab 1, we had to construct several simple circuits using multiple external components and the Arduino Uno microcontroller. With the Arduino microcontroller and the Arduino IDE, we constructed several simple Arduino programs, which culminated with us building our first robot and having it drive autonomously in a figure eight pattern.

## Task 1 - Blinking the Internal LED:
The first task was to use the arduino to blink an internal LED. The on-chip LED is hard-wired to pin 13 which we set up as an ouput using the following code:

```cpp
#define PIN_NUMBER LED_BUILTIN

void setup() {
  pinMode(PIN_NUMBER, OUTPUT);
}
```

We then turned the LED on and off by setting it to high and low respectively:

```cpp
void loop() {
  digitalWrite(PIN_NUMBER, HIGH);   
  delay(1000);                       // waits for a second
  digitalWrite(PIN_NUMBER, LOW);    
  delay(1000);                       
}
```

Demo:
<iframe width="560" height="315" src="https://www.youtube.com/embed/FUZXOQbeATo" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

## Task 2 - Blinking an External LED:
Our next task was to modify the code for task one to blink an external LED. We did this by connecting an external LED to a digital pin through a resistor as shown in the diagram below:

![Image](images/blink.png)

All we did in the code for this task was to change our Pin Number from the on-chip LED to the pin that we connected to the external LED: 

```cpp
#define LED_PIN 0
```

The resistor was used to prevent sourcing too much current through the pin in case of a short circuit, which would burn out both the Arduino and the external LED.

Demo:
<iframe width="560" height="315" src="https://www.youtube.com/embed/RYbxh-uWJU4" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

## Task 3 - Reading an Analog Input:

Our third task was to detect changes in the resistance value of a potentiometer through an analog input pin and print the value to the serial monitor. This was accomplished by setting up a voltage divider circuit as shown below and connection the output of the circuit to an analog pin:

*Circuit Diagram needed*

We then used the following code to read the analog value of the of the pin and output it to the serial monitor every half second:

```cpp
{% include_relative code/PotRead/PotRead.ino %}
```

## Task 4 - Outputting an Analog Signal:

Combining the setups of Tasks 2 & 3, we designed the following circuit which allowed us to use the analog signal input from the potentiometer to control the brightness of the external LED:

*Circuit Diagram needed*

The LED could only be connected to one of the 7(?) digital pins which could output a PWN signal, which was required to simulate the effect of an analog voltage. The code we wrote for this task is shown below:

```cpp
{% include_relative code/PotLed/PotLed.ino %}
```

Demo:
<iframe width="560" height="315" src="https://www.youtube.com/embed/4hCbVb7QNAI" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

## Task 5 - Controlling Parallax Servos:

Our final task before assembling our robot was to use the PWM signal we generated in the previous task to drive and control a Parallax Continuous Rotation Servo. We did this using the Arduino Servo.h library and writing the following code to attach our servo to a PWM capable pin and output the analog voltage of the potentiometer setup through this pin to control the speed and direction of the servo:

```cpp
{% include_relative code/ServoPot/ServoPot.ino %}
```

Demo:
<iframe width="560" height="315" src="https://www.youtube.com/embed/89UeEeVpefA" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

## Task 6: Assembling the Robot

The parts we used to assemble our robot were:
- 2 servos
- 2 servo mounts
- 2 wheels
- 1 baseplate
- 11 screws
- 11 nuts
- 1 ball bearing wheel
- 1 arduino
- 1 rechargeable battery pack

The first thing we did was mount the servos to the servo mounts. As there was a shortage of screws and nuts in the lab, we only used two screws per servo instead of the four there are room for. The robot built in this lab will not be the same as the final robot, so we just needed enough screws to maintain structural integrity.

The servos were then mounted to the baseplate, using two screws per mount.

The wheels were then mounted to the servos. As one of our servos did not have a screw, we used wheels that fit snug on the servos. This is an issue that we will need to fix when designing later robots.

Next, we mounted the Arduino on top of the baseplate using two screws.

Finally, we attached the ball bearing wheel to the front of the robot.

## Task 7 - Driving the Robot Autonomously:

Once the robot was built, we wrote following code for the robot to autonomously drive in a figure eight pattern:

```cpp
{% include_relative code/Robot/Robot.ino %}
```

The first time we tested the robot, it became apparent that one of our servos was spinning much faster than the other; so much faster that the robot was practically impossible to control.

We performed a series of tasks in order to identify the problem. First, we swapped the pins for our two servos. This would check to see if the issue was in the software rather than in the hardware. After switching the pins had no effect, we decided to try a third servo. The third servo was much closer to the speed we were expecting, so we switched out the super fast servo. After this switch, we were able to get out robot to drive in straight lines and in a proper figure eight pattern.

Demo:
<iframe width="560" height="315" src="https://www.youtube.com/embed/GhzqjIfTxn0" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

## Bloopers :sweat_smile:

<iframe width="560" height="315" src="https://www.youtube.com/embed/d9uHi5ukqcA" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

