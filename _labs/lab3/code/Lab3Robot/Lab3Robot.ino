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
#define LEFT_WHEEL_PIN 6
#define RIGHT_WHEEL_PIN 5
#define SENSOR_LEFT_PIN 3
#define SENSOR_RIGHT_PIN 2
#define FRONT_IR_SENSOR A0

#define FFT_PIN 13
#define FFT_CTRL_PIN 12

#define FORWARD_LEFT 180
#define BACKWARD_LEFT 0
#define FORWARD_RIGHT 0
#define BACKWARD_RIGHT 180
#define STOP_POS 90

#define RIGHT_TIME 1000
#define LEFT_TIME 1000

#define LEFT_SENSOR_THRESH 100
#define RIGHT_SENSOR_THRESH 100


// These variables are marked 'volatile' to inform the compiler that they can change
// at any time (as they are set by hardware interrupts).
volatile long SENSOR_LEFT_TIMER;
volatile long SENSOR_RIGHT_TIMER;

Servo leftWheel;
Servo rightWheel;
int counter = 0;

int started = 0;

volatile long SENSOR_LEFT_READING; 
volatile long SENSOR_RIGHT_READING;

#define WALL_THRESH 300

// A digital write is required to trigger a sensor reading.
void setup_sensor(int pin, long *sensor_timer) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
  delayMicroseconds(10);
  pinMode(pin, INPUT);
  *sensor_timer = micros();
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
  leftWheel.write(STOP_POS);
  rightWheel.write(STOP_POS);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  pinMode(FFT_PIN, INPUT);
  pinMode(FFT_CTRL_PIN, OUTPUT);

  // low = 660 detection
  digitalWrite(FFT_CTRL_PIN, LOW);

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
  //delay(LEFT_TIME);
  while(SENSOR_LEFT_READING < LEFT_SENSOR_THRESH) {
    // turn until past center line (may skip this call)
  }
  delay(100);
  while(SENSOR_LEFT_READING > LEFT_SENSOR_THRESH) {
    // turn until see line
  }
  delay(100);
  while(SENSOR_LEFT_READING < LEFT_SENSOR_THRESH) {
    // turn until past line
  }
}

void loop() {
  if (started == 0) {
    if (digitalRead(FFT_PIN) == HIGH) {
      started = 1;  
      // high = IR detection
      digitalWrite(FFT_CTRL_PIN, HIGH);
    }
  } else {
    //Centers and turns the robot in the appropriate direction if both line sensors
    //see a white line (come to an intersection)
    if (SENSOR_LEFT_READING < LEFT_SENSOR_THRESH && SENSOR_RIGHT_READING < RIGHT_SENSOR_THRESH) {
      leftWheel.write(FORWARD_LEFT);
      rightWheel.write(FORWARD_RIGHT);
      delay(200);
  
      // at intersection, check if wall to the right.
      // if not, turn right
      leftWheel.write(STOP_POS);
      rightWheel.write(STOP_POS);
      delay(1000); //For the robot to center itself on the intersection
      if (analogRead(FRONT_IR_SENSOR) > WALL_THRESH) {
         turnLeft();
      }
      leftWheel.write(FORWARD_LEFT);
      rightWheel.write(FORWARD_RIGHT);
     
    } else {
      //Stops left wheel to correct the robot if left line sensor sees the white line
      if (SENSOR_LEFT_READING < LEFT_SENSOR_THRESH) leftWheel.write(STOP_POS);
      else leftWheel.write(FORWARD_LEFT);
      
      //Stops right wheel to correct the robot if right line sensor sees the white line
      if (SENSOR_RIGHT_READING < RIGHT_SENSOR_THRESH) rightWheel.write(STOP_POS);
      else rightWheel.write(FORWARD_RIGHT);
    }
    // stop while we detect another robot
    while(digitalRead(FFT_PIN) == HIGH) {
      leftWheel.write(STOP_POS);
      rightWheel.write(STOP_POS);
    } 
  }
}
