#include <Servo.h>
#define LEFT_WHEEL_PIN 5
#define RIGHT_WHEEL_PIN 6

#define FFT_DATA_PIN 8
#define FFT_CTRL_PIN 7

#define FORWARD_LEFT 180
#define BACKWARD_LEFT 0
#define FORWARD_RIGHT 0
#define BACKWARD_RIGHT 180
#define STOP_POS 90

Servo leftWheel;
Servo rightWheel;

void setup() {
  Serial.begin(9600);
  
  // Setup the servos
  leftWheel.attach(LEFT_WHEEL_PIN);
  rightWheel.attach(RIGHT_WHEEL_PIN);
  leftWheel.write(STOP_POS);
  rightWheel.write(STOP_POS);

  pinMode(FFT_DATA_PIN, INPUT);
  pinMode(FFT_CTRL_PIN, OUTPUT);

  // low = 660 detection
  digitalWrite(FFT_CTRL_PIN, LOW);

  init_sensors();

  initRadio();

  init_maze();

//  startup();

  delay(2000);
}

void loop() {
  update_state_mach();

  update_line_sensor_values();
}
