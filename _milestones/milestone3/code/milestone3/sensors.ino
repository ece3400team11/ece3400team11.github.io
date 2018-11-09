#define SENSOR_LEFT_PIN 2
#define SENSOR_RIGHT_PIN 3
#define FRONT_IR_SENSOR A1
#define RIGHT_IR_SENSOR A0
#define LEFT_IR_SENSOR A2

#define LEFT_SENSOR_THRESH 145
#define RIGHT_SENSOR_THRESH 145

#define SENSOR_AVE_SIZE 4

int leftSenseBuf[SENSOR_AVE_SIZE];
int leftSenseHead = 0;
int rightSenseBuf[SENSOR_AVE_SIZE];
int rightSenseHead = 0;

unsigned long SENSOR_LEFT_READING = 1000;
unsigned long SENSOR_RIGHT_READING = 1000;

unsigned long FRONT_WALL_READING = 0;
unsigned long LEFT_WALL_READING = 0;
unsigned long RIGHT_WALL_READING = 0;
#define NUM_WALL_READINGS 4

// larger values mean the object is closer
#define FRONT_WALL_THRESH 200
#define RIGHT_WALL_THRESH 200
#define LEFT_WALL_THRESH 200

void init_sensors() {
  pinMode(FRONT_IR_SENSOR, INPUT);
  pinMode(RIGHT_IR_SENSOR, INPUT);
  pinMode(LEFT_IR_SENSOR, INPUT);
  
  for(int i = 0; i < SENSOR_AVE_SIZE; i++) {
    leftSenseBuf[i] = LEFT_SENSOR_THRESH+100;
    rightSenseBuf[i] = RIGHT_SENSOR_THRESH+100;
  }
}

void update_wall_sensor(unsigned long* reading, int sensor_num) {
  *reading = 0;
  for(int i = 0; i < NUM_WALL_READINGS; i++) {
    *reading += analogRead(sensor_num);
  }
  *reading = *reading / NUM_WALL_READINGS;
}

void update_wall_sensor_values() {
  update_wall_sensor(&FRONT_WALL_READING, FRONT_IR_SENSOR);
  update_wall_sensor(&LEFT_WALL_READING, LEFT_IR_SENSOR);
  update_wall_sensor(&RIGHT_WALL_READING, RIGHT_IR_SENSOR);
}

boolean is_wall_in_front() {
  return FRONT_WALL_READING > FRONT_WALL_THRESH;
}

boolean is_wall_on_left() {
  return LEFT_WALL_READING > LEFT_WALL_THRESH;
}

boolean is_wall_on_right() {
  return RIGHT_WALL_READING > RIGHT_WALL_THRESH;
}

void update_line_sensor_values() {
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

