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
#define LEFT_IR_SENSOR A2

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
    leftSenseBuf[i] = LEFT_SENSOR_THRESH+100;
    rightSenseBuf[i] = RIGHT_SENSOR_THRESH+100;
  }

  initRadio();

  delay(2000);
}

void loop() {
  update_state_mach();

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
