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
#define SENSOR_LEFT_PIN 3
#define SENSOR_RIGHT_PIN 2

#define FORWARD_LEFT 0
#define BACKWARD_LEFT 180
#define FORWARD_RIGHT 180
#define BACKWARD_RIGHT 0
#define STOP_POS 90

#define RIGHT_TIME 1000
#define LEFT_TIME 1000


// These variables are marked 'volatile' to inform the compiler that they can change
// at any time (as they are set by hardware interrupts).
volatile long SENSOR_LEFT_TIMER;
volatile long SENSOR_RIGHT_TIMER;

Servo leftWheel;
Servo rightWheel;
int counter = 0;

volatile int SENSOR_LEFT_READING; 
volatile int SENSOR_RIGHT_READING;

// A digital write is required to trigger a sensor reading.
void setup_sensor(int pin, long *sensor_timer) {
  *sensor_timer = micros();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);
}

void SENSOR_LEFT_ISR() {
  // The sensor light reading is inversely proportional to the time taken
  // for the pin to fall from high to low. Lower values mean lighter colors.
  SENSOR_LEFT_READING = micros() - SENSOR_LEFT_TIMER;
  // Reset the sensor for another reading
  setup_sensor(SENSOR_LEFT_PIN, &SENSOR_LEFT_TIMER);
}

void SENSOR_RIGHT_ISR() {
  // The sensor light reading is inversely proportional to the time taken
  // for the pin to fall from high to low. Lower values mean lighter colors.
  SENSOR_RIGHT_READING = micros() - SENSOR_RIGHT_TIMER;
  // Reset the sensor for another reading
  setup_sensor(SENSOR_RIGHT_PIN, &SENSOR_RIGHT_TIMER);
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

void turnRight() {
  //Moves both wheels in opposite directions for an empirically determined amount of time
  leftWheel.write(FORWARD_LEFT);
  rightWheel.write(BACKWARD_RIGHT);
  delay(RIGHT_TIME);
}

void turnLeft() {
  //Moves both wheels in opposite directions for  an empirically determined amount of time
  leftWheel.write(BACKWARD_LEFT);
  rightWheel.write(FORWARD_RIGHT);
  delay(LEFT_TIME);
}

//Hardcoded turns required for a figure eight pattern
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
  //Centers and turns the robot in the appropriate direction if both line sensors
  //see a white line (come to an intersection)
  if (SENSOR_LEFT_READING < 400 && SENSOR_RIGHT_READING < 400) {
    leftWheel.write(FORWARD_LEFT);
    rightWheel.write(FORWARD_RIGHT);
    delay(400); //For the robot to center itself on the intersection
    fig8();
    counter++;
   
  } else {
    //Stops left wheel to correct the robot if left line sensor sees the white line
    if (SENSOR_LEFT_READING < 400) leftWheel.write(STOP_POS);
    else leftWheel.write(FORWARD_LEFT);
    
    //Stops right wheel to correct the robot if right line sensor sees the white line
    if (SENSOR_RIGHT_READING < 400) rightWheel.write(STOP_POS);
    else rightWheel.write(FORWARD_RIGHT);
  }
}
