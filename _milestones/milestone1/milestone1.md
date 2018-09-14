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

![Image](milestone1/images/Line_Sensors.png)

We chose to space out our sensors a little wider than the line so the robot has some flexibility and does not need to correct itself too often.

## Task 1 - Setting up the Line Sensors:

The first thing we had to do was determine whether our line sensor was a digital or analog line sensor. We initially assumed that all the sensors were analog, however when we tried displaying the output of the sensor on the Serial Monitor while holding it over the white grid lines and the darker background colors, the numbers did not line up with that we were expecting. So we looked at the documentation for our sensor and discovered that it was actually a digital sensor. After setting up and testing the sensors using the digital pins on our Arduino, the values made more sense and we determined that the threshold value for whites was anything below 400.

## Task 2 - Following a Straight Line:

As we did in the previous lab, we initialized the servos and defined the variables necessary for them. In addition, we also had to initialize the sensors and this was done by assigning each one a digital pin along with a read and timer variable.
 
```cpp
#define SENSOR_LEFT_PIN 2
#define SENSOR_RIGHT_PIN 3

// These variables are marked 'volatile' to inform the compiler that
// they can change at any time (as they are set by hardware interrupts).
volatile long SENSOR_LEFT_TIMER;
volatile long SENSOR_RIGHT_TIMER;

volatile int SENSOR_LEFT_READING; 
volatile int SENSOR_RIGHT_READING;
```

Because the sensors we used are digital, a digital write to the pin is used as a signal to trigger a sensor reading. This was done by calling the digitalWrite() function before reading any input, which set the appropriate pin as output, pulled the pin up to HIGH , and then set it back to an input.

```cpp
void setup_sensor(int pin, long *sensor_timer) {
  *sensor_timer = micros();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);
}
```

In the setup method, each sensor was assigned an interrupt handler since that was how a reading was taken from the sensor. The ISR for each sensor calculated the sensor reading and then reset sensor’s timer by setting it equal to the current time. Each ISR was attached to a sensor using attachInterrupt(). Finally, the sensors and servos were initialized as described earlier. 

```cpp
void SENSOR_LEFT_ISR() {
  // The sensor light reading is inversely proportional to the time taken
  // for the pin to fall from high to low. Lower values mean lighter colors.
  SENSOR_LEFT_READING = micros() - SENSOR_LEFT_TIMER;
  // Reset the sensor for another reading
  setup_sensor(SENSOR_LEFT_PIN, &SENSOR_LEFT_TIMER);
}

void setup() {
  Serial.begin(9600);

  // Tell the compiler which pin to associate with which ISR
  attachInterrupt(digitalPinToInterrupt(SENSOR_LEFT_PIN), SENSOR_LEFT_ISR, LOW);
  attachInterrupt(digitalPinToInterrupt(SENSOR_RIGHT_PIN), SENSOR_RIGHT_ISR, LOW);

  // Setup the sensors
  setup_sensor(SENSOR_LEFT_PIN, &SENSOR_LEFT_TIMER);
  setup_sensor(SENSOR_RIGHT_PIN, &SENSOR_RIGHT_TIMER);

  // Setup the servos
  leftWheel.attach(LEFT_WHEEL_PIN);
  rightWheel.attach(RIGHT_WHEEL_PIN);

  delay(2000);
}
```

Our algorithm to keep the robot following a straight line was as follows. As long as neither of the line sensors detected a white line, both the wheels of the robot would just keep going forward as usual. However, if one of the line sensors did detect a white line, it would stop that wheel until the it did not detect the white line anymore, at which time it would continue moving forward. This works because by stopping the wheel on one side but keeping the wheel on the other side moving forward, the robot naturally pivots around its center of rotation to correct its path. We had initially had the line sensors set up on the rear of the robot, but as we discovered, this did not work very well as the robots center of rotation was closer to its rear. Once we reversed the direction of our robot, our algorithm worked like a charm. Here is how it was implemented:

```cpp
void loop() {
  //Continues moving forwards if both line sensors see a white line
  if (SENSOR_LEFT_READING < 400 && SENSOR_RIGHT_READING < 400) {
    leftWheel.write(FORWARD_LEFT);
    rightWheel.write(FORWARD_RIGHT);
   
  } else {
    //Stops left wheel to correct the robot if left line sensor sees the white line
    if (SENSOR_LEFT_READING < 400) leftWheel.write(STOP_POS);
    else leftWheel.write(FORWARD)LEFT);
    
    //Stops right wheel to correct the robot if right line sensor sees the white line
    if (SENSOR_RIGHT_READING < 400) rightWheel.write(STOP_POS);
    else rightWheel.write(FORWARD_RIGHT);
  }
}
```

As you can see in the code above, if both of the line sensors see the white line at the same time, the robot has arrived at an intersection. For now, we want to ignore the intersections and thus the robot is programmed to just continue moving forward, but they become more important in the next section.

Demo:


## Task 3 - Traversing the Grid in a Figure Eight Pattern:

Now that the robot was able to follow a straight line, we set about programming it to autonomously traverse a grid in a figure eight pattern.

The algorithm for this movement was generally the same as the one for following a straight line, with the only difference being the behaviour of the robot when it encountered an intersection. On reaching an intersection, the robot would first need to move forward a little bit to center itself over the intersection (since the line sensors are placed a little ahead of the wheels). The amount of time we needed the robot to move forward to center itself perfectly was determined by trial and error. We then needed the robot to turn left or right, which we did using the code we had written for a similar task in Lab 1. However, since we were now powering more stuff off the Arduino, each individual component received less power so turn took a lot longer than it had done previously.

```cpp
void turnLeft() {
  //Moves both wheels in opposite directions for  an empirically determined amount of time
  leftWheel.write(BACKWARD_LEFT);
  rightWheel.write(FORWARD_RIGHT);
  delay(LEFT_TIME);
}

void loop() {

  if (SENSOR_LEFT_READING < 400 && SENSOR_RIGHT_READING < 400) {
    leftWheel.write(FORWARD_LEFT);
    rightWheel.write(FORWARD_RIGHT);
    delay(400); //For the robot to center itself on the intersection
    turnLeft();
  } else {
 ...
```

Once we got the turns working well, all that remained was to string together the correct sequence of turns for the robot to perform a figure eight. This was done quite simply by creating a function which utilised a counter to chain together the turns required.

```cpp
void fig8() {
  if (counter%8 == 0) turnLeft();
  if (counter%8 == 1) turnLeft();
  if (counter%8 == 2) turnLeft();
  if (counter%8 == 3) turnLeft();
  if (counter%8 == 4) turnRight();
  if (counter%8 == 5) turnRight();
  if (counter%8 == 6) turnRight();
  if (counter%8 == 7) turnRight(); 
}

void loop() {

  if (SENSOR_LEFT_READING < 400 && SENSOR_RIGHT_READING < 400) {
    leftWheel.write(FORWARD_LEFT);
    rightWheel.write(FORWARD_RIGHT);
    delay(400); //For the robot to center itself on the intersection
    fig8();
    counter++;
  } else {
 ...
```

Demo:

## Future Improvements:

Our algorithm works fine right now, but we have several ideas for how to improve the robot’s ability to traverse the grid. 

First, we need to improve the speed of the robot. As we continue to add components to the robot, the robot will require more power. Up to this point we have been powering all the components directly from the Arduino, but we are likely reaching the limit to the amount of current the board can supply. Our plan for the next lab is to begin to design a power distribution system so that the board and all the components can be powered directly from the battery pack.

Second, we need to improve the robustness of our turning algorithm. As of now, we have our turning algorithm hard coded. When the robot is signaled to turn, it turns for a predetermined amount of time before following the line again. This presents some problems moving forward. We implemented the same turning sequence we used in lab 1, but the robot was no longer turning as much. We believe this is because with the additional hardware added in milestone 1, the Arduino could no longer supply as much current to the servos, causing them to run slower. This part of the turning can be fixed with a power distribution system as described above. But since the turns may not be entirely consistent through just using time, we may consider implementing the line sensors to tell the robot when to stop turning.

Third, we may explore the option of altering the structural design of the robot. Possible alterations include different chassis designs and different wheel sizes. These changes could allow us to place our sensors in a more ideal position and allow us to drive faster.

Fourth, we may spend some time testing various servos in the lab until we find 2 that are almost exactly the same speed. This will help the robot maintain a straight trajectory from the beginning so it won’t have to correct itself as much. If finding better servos in the lab proves to difficult, we may entertain the option of buying our own.

Fifth, we may consider implementing a moving average function for our sensors. During the lab, we observed that the sensors occasionally spiked up or down in value; if we told the robot to stop on a white line, it would occasionally jitter around. We didn’t observe this problem in the opposite scenario (driving straight and jittering to a stop), but this may be a problem that arises later. For this reason, this change is not the highest priority at the moment.


