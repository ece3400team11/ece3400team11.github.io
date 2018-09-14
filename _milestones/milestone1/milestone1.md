---
layout: blank
title:  "Milestone 1"
subtitle: "Line Following"
status: "active" # disabled, active, complete
preview: "assets/images/milestone1.jpg"
---

# Milestone 1

For Milestone 1, we had to take the robot we built in the previous lab and give it the ability to maneuver a grid using line sensors. We did this by first programming our robot to follow a straight line, and then modifying our code to allow for the detection of intersections to drive the robot in a figure eight pattern on the grid provided to us in the lab space.

## Hardware Modifications:

After some trial and error, we decided to attach the line sensors to the front of our robot rather than the rear. We decided on using only 2 line sensors since we found that this was a sufficient number of line sensors to keep the robot relatively well oriented while also saving pins for later in the project. We first soldered male pin heads to the sensors so we could easily attach and remove wires between the sensors and the Arduino. Then we attached the sensors using a couple nuts and some extenders to the bottom of our base plate, making sure to position them pointing slightly inwards to keep them as close to the robot’s center of rotation as possible, as this makes the turning of the robot easier.

![Image](_milestones/milestone1/images/Line_Sensors.png)

We chose to space out our sensors a little wider than the line so the robot has some flexibility and does not need to correct itself too often.

## Task 1 - Setting up the Line Sensors:

The first thing we had to do was determine whether our line sensor was a digital or analog line sensor. We initially assumed that all the sensors were analog, however when we tried displaying the output of the sensor on the Serial Monitor while holding it over the white grid lines and the darker background colors, the numbers did not line up with that we were expecting. So we looked at the documentation for our sensor and discovered that it was actually a digital sensor. After setting up and testing the sensors using the digital pins on our Arduino, the values made more sense and we determined that the threshold value for whites was anything below 400.

## Task 2 - Following a Straight Line:


Demo:

## Task 3 - Traversing the Grid in a Figure Eight Pattern:


Demo:

## Bloopers :sweat_smile:

