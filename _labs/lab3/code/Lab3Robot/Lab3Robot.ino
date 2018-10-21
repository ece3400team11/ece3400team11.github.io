/*
 * How to use the QRE1113 Digital Line Sensor by SparkFun with hardware interrupts
 * https://www.sparkfun.com/products/9454
 *
 * Note: The Arduino Uno is limited to two pins for digital state interrupts: pins 2 and 3.
 * This means you will be limited to two digital line sensors.
 *
 * Refer to this for information on attachInterrupt():
 * https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
  */

#include <Servo.h>
#define LEFT_WHEEL_PIN 5
#define RIGHT_WHEEL_PIN 6
#define SENSOR_LEFT_PIN 2
#define SENSOR_RIGHT_PIN 3
#define FRONT_IR_SENSOR A1
#define RIGHT_IR_SENSOR A0

#define FFT_DATA_PIN 7
#define FFT_CTRL_PIN 8

#define FORWARD_LEFT 180
#define BACKWARD_LEFT 0
#define FORWARD_RIGHT 0
#define BACKWARD_RIGHT 180
#define STOP_POS 90

#define RIGHT_TIME 1000
#define LEFT_TIME 1000

#define LEFT_SENSOR_THRESH 150
#define RIGHT_SENSOR_THRESH 150

#define SENSOR_AVE_SIZE 4

int leftSenseBuf[SENSOR_AVE_SIZE];
int leftSenseHead = 0;
int rightSenseBuf[SENSOR_AVE_SIZE];
int rightSenseHead = 0;

Servo leftWheel;
Servo rightWheel;

unsigned long SENSOR_LEFT_READING = 1000;
unsigned long SENSOR_RIGHT_READING = 1000;

int state = 0;

#define FRONT_WALL_THRESH 150
#define RIGHT_WALL_THRESH 200

#define MAZE_WIDTH 3
#define MAZE_LENGTH 3

unsigned char maze[MAZE_LENGTH][MAZE_WIDTH];

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9,10);

const int NORTH = 3;
const int EAST  = 2;
const int SOUTH = 1;
const int WEST  = 0;

unsigned char x=0;
unsigned char y=0;
int dir = EAST;

const uint64_t pipes[2] = { 0x000000001CLL, 0x000000001DLL };

void setup() {
  Serial.begin(9600);
  
  // Setup the servos
  leftWheel.attach(LEFT_WHEEL_PIN);
  rightWheel.attach(RIGHT_WHEEL_PIN);
  leftWheel.write(STOP_POS);
  rightWheel.write(STOP_POS);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  pinMode(FFT_DATA_PIN, INPUT);
  pinMode(FFT_CTRL_PIN, OUTPUT);

  // low = 660 detection
  digitalWrite(FFT_CTRL_PIN, LOW);

  for(int i = 0; i < SENSOR_AVE_SIZE; i++) {
    leftSenseBuf[i] = LEFT_SENSOR_THRESH+100;
    rightSenseBuf[i] = RIGHT_SENSOR_THRESH+100;
  }

  radio.begin();

  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(4);
  
  // put your setup code here, to run once:
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

  delay(2000);
}

// advance the robot's position based on its direction
void adv(){
  switch(dir){
      case NORTH:
        y=y-1;
        break;
      case EAST:
        x=x+1;
        break;
      case SOUTH:
        y=y+1;
        break;
      case WEST:
        x=x-1;
        break;
    }
}

void sendData() {
  radio.stopListening();

  unsigned char curr = maze[y][x];
  
  // pack data into payload
  unsigned long payload = 0;
  payload |= curr;
  payload = payload << 8;
  payload |= x;
  payload = payload << 8;
  payload |= y;
  
  bool ok = false;
  while(!ok) {
    ok = radio.write( &payload, sizeof(unsigned long) );
  }
}

void loop() {
  Serial.println(state);
//  state = 1;
  if (state == 0) {
    if (digitalRead(FFT_DATA_PIN) == HIGH) {
      state = 1;  
      // high = IR detection
      digitalWrite(FFT_CTRL_PIN, HIGH);
    }
  } else if (state == 1) {
    //Centers and turns the robot in the appropriate direction if both line sensors
    //see a white line (come to an intersection)
    if (SENSOR_LEFT_READING < LEFT_SENSOR_THRESH && SENSOR_RIGHT_READING < RIGHT_SENSOR_THRESH) {
      // go forward and center over line
      leftWheel.write(FORWARD_LEFT);
      rightWheel.write(FORWARD_RIGHT);
      delay(200);

      // TODO maybe remove
      leftWheel.write(STOP_POS);
      rightWheel.write(STOP_POS);

      adv();

      int isFrontWall = analogRead(FRONT_IR_SENSOR) > FRONT_WALL_THRESH;
      int isRightWall = analogRead(RIGHT_IR_SENSOR) > RIGHT_WALL_THRESH;

      maze[y][x] |= isFrontWall << dir + 4;
      maze[y][x] |= isRightWall << ((dir+3)%4) + 4;

      sendData();
  
      if (analogRead(RIGHT_IR_SENSOR) < RIGHT_WALL_THRESH) {
        // no wall to the right
        // turn right
        state = 2;
      } else if (analogRead(FRONT_IR_SENSOR) > FRONT_WALL_THRESH) {
        // wall to the right and wall in front
        // turn left
        state = 5;
      }
    } else {
      //Stops left wheel to correct the robot if left line sensor sees the white line
      if (SENSOR_LEFT_READING < LEFT_SENSOR_THRESH) leftWheel.write(STOP_POS);
      else leftWheel.write(FORWARD_LEFT);
      
      //Stops right wheel to correct the robot if right line sensor sees the white line
      if (SENSOR_RIGHT_READING < RIGHT_SENSOR_THRESH) rightWheel.write(STOP_POS);
      else rightWheel.write(FORWARD_RIGHT);
    }
    // stop while we detect another robot
    if(digitalRead(FFT_DATA_PIN) == HIGH) {
//      if (analogRead(FRONT_IR_SENSOR) < FRONT_WALL_THRESH) {
//        // turn around  
//        //state = 3;
//      }
      leftWheel.write(STOP_POS);
      rightWheel.write(STOP_POS);
      delay(1000);
    }
  } else if (state == 2) {
    // turn right step 1
    leftWheel.write(FORWARD_LEFT);
    rightWheel.write(BACKWARD_RIGHT);

    // turn until past center line (may skip this call)
    if (SENSOR_RIGHT_READING > RIGHT_SENSOR_THRESH) {
      state = 3;
      // give a bit of time for measurement to settle
      delay(100);
    }
  } else if (state == 3) {
    // turn right step 2
    // turn until see line
    if (SENSOR_RIGHT_READING < RIGHT_SENSOR_THRESH) {
      state = 4;
      // give a bit of time for measurement to settle
      delay(100); 
    }
  } else if (state == 4) {
    // turn right step 3
    // turn until past line
    if (SENSOR_RIGHT_READING > RIGHT_SENSOR_THRESH) {
      state = 1;
      delay(100);

      // start going forward again
      leftWheel.write(FORWARD_LEFT);
      rightWheel.write(FORWARD_RIGHT);

      // assume only turn 90 degrees
      dir = (dir+3)%4;
    }
  }  else if (state == 5) {
    // turn left step 1
    leftWheel.write(BACKWARD_LEFT);
    rightWheel.write(FORWARD_RIGHT);

    // turn until past center line (may skip this call)
    if (SENSOR_LEFT_READING > LEFT_SENSOR_THRESH) {
      state = 6;
      // give a bit of time for measurement to settle
      delay(100);
    }    
  } else if (state == 6) {
    // turn left step 2
    // turn until see line
    if (SENSOR_LEFT_READING < LEFT_SENSOR_THRESH) {
      state = 7;
      // give a bit of time for measurement to settle
      delay(100); 
    }
  } else if (state == 7) {
    // turn left step 3
    // turn until past line
    if (SENSOR_LEFT_READING > LEFT_SENSOR_THRESH) {
      state = 1;
      delay(100);

      // assume only turn 90 degrees
      dir = (dir+1)%4;

      if (analogRead(FRONT_IR_SENSOR) > FRONT_WALL_THRESH) {
        // still wall in front after turning left, dead end, complete 180
        state = 5;
      } else {
        // start going forward again
        leftWheel.write(FORWARD_LEFT);
        rightWheel.write(FORWARD_RIGHT);
      }
    }
  }

  // do line sensing
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

//  Serial.print("Left: ");
//  Serial.println(SENSOR_LEFT_READING);
//  Serial.print("Right: ");
//  Serial.println(SENSOR_RIGHT_READING);
//  delay(200);
}
