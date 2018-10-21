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

#define FFT_DATA_PIN 13
#define FFT_CTRL_PIN 12

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

unsigned long SENSOR_LEFT_READING = 0;
unsigned long SENSOR_RIGHT_READING = 0;

int state = 1;

#define FRONT_WALL_THRESH 150
#define RIGHT_WALL_THRESH 200

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
    leftSenseBuf[i] = LEFT_SENSOR_THRESH+10;
    rightSenseBuf[i] = RIGHT_SENSOR_THRESH+10;
  }

  delay(2000);
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

      // start going forward again
      leftWheel.write(FORWARD_LEFT);
      rightWheel.write(FORWARD_RIGHT);
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
