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
#define LEFT_WHEEL_PIN 4
#define RIGHT_WHEEL_PIN 5
#define SENSOR0_PIN 2
#define SENSOR1_PIN 3
#define SPEED_LEFT 1
#define SPEED_RIGHT 179

#define STOP_POS 90
// WRT left wheel
#define FORWARD_POS 180
#define BACKWARD_POS 0

int rightTime = 1000;
int leftTime = 1000;

// These variables are marked 'volatile' to inform the compiler that they can change
// at any time (as they are set by hardware interrupts).
volatile long SENSOR0_TIMER;
volatile long SENSOR1_TIMER;

Servo leftWheel;
Servo rightWheel;
int counter = 0;


// Consider smoothing this value with your favorite smoothing technique (exponential moving average?)
volatile int SENSOR0_READING;
volatile int SENSOR1_READING;

// A digital write is required to trigger a sensor reading.
void setup_sensor(int pin, long *sensor_timer) {
  *sensor_timer = micros();
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);
}

void SENSOR0_ISR() {
  // The sensor light reading is inversely proportional to the time taken
  // for the pin to fall from high to low. Lower values mean lighter colors.
  SENSOR0_READING = micros() - SENSOR0_TIMER;
  // Reset the sensor for another reading
  setup_sensor(SENSOR0_PIN, &SENSOR0_TIMER);
}

void SENSOR1_ISR() {
  SENSOR1_READING = micros() - SENSOR1_TIMER;
  setup_sensor(SENSOR1_PIN, &SENSOR1_TIMER);
}

void setup() {
  Serial.begin(9600);

  // Tell the compiler which pin to associate with which ISR
  attachInterrupt(digitalPinToInterrupt(SENSOR0_PIN), SENSOR0_ISR, LOW);
  attachInterrupt(digitalPinToInterrupt(SENSOR1_PIN), SENSOR1_ISR, LOW);

  // Setup the sensors
  setup_sensor(SENSOR0_PIN, &SENSOR0_TIMER);
  setup_sensor(SENSOR1_PIN, &SENSOR1_TIMER);

  // Setup the servos
  leftWheel.attach(LEFT_WHEEL_PIN);
  rightWheel.attach(RIGHT_WHEEL_PIN);

  delay(2000);
}

void moveRight() {
  // turn right
  leftWheel.write(FORWARD_POS);
  rightWheel.write(FORWARD_POS);
  delay(rightTime);
}

void moveLeft() {
  // turn left
  leftWheel.write(BACKWARD_POS);
  rightWheel.write(BACKWARD_POS);
  delay(leftTime);
}

void fig8() {
  if (counter%8 == 0) moveLeft();
  if (counter%8 == 1) moveLeft();
  if (counter%8 == 2) moveLeft();
  if (counter%8 == 3) moveLeft();
  if (counter%8 == 4) moveRight();
  if (counter%8 == 5) moveRight();
  if (counter%8 == 6) moveRight();
  if (counter%8 == 7) moveRight(); 
}

void loop() {
  // These delays are purely for ease of reading.
  /*Serial.println("Sensor 0");
  Serial.println(SENSOR0_READING);
  delay(500);
  Serial.println("Sensor 1");
  Serial.println(SENSOR1_READING);
  delay(500);*/
  
  if (SENSOR0_READING < 400 && SENSOR1_READING < 400) {
    leftWheel.write(SPEED_LEFT);
    rightWheel.write(SPEED_RIGHT);
  } else {
    if (SENSOR0_READING < 400) leftWheel.write(90);
    else leftWheel.write(SPEED_LEFT);
  
    if (SENSOR1_READING < 400) rightWheel.write(90);
    else rightWheel.write(SPEED_RIGHT);
  }
}
