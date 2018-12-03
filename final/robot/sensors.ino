#define SENSOR_LEFT_PIN 2
#define SENSOR_RIGHT_PIN 3
#define SENSOR_CENTER_PIN 4
#define FRONT_IR_SENSOR A1
#define RIGHT_IR_SENSOR A2
#define LEFT_IR_SENSOR A0

int LEFT_SENSOR_THRESH = 700;
int RIGHT_SENSOR_THRESH = 700;

#define SENSOR_AVE_SIZE 4
#define LEFT_DIFF 150
#define RIGHT_DIFF 150
#define MAX_CENTER 700

int leftSenseBuf[SENSOR_AVE_SIZE];
int leftSenseHead = 0;
int rightSenseBuf[SENSOR_AVE_SIZE];
int rightSenseHead = 0;
int centerSenseBuf[SENSOR_AVE_SIZE];
int centerSenseHead = 0;
int updateCenterReading = 1;

unsigned long SENSOR_LEFT_READING = 1000;
unsigned long SENSOR_RIGHT_READING = 1000;
unsigned long SENSOR_CENTER_READING = 0;

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
  pinMode(SENSOR_CENTER_PIN, OUTPUT);
  digitalWrite(SENSOR_LEFT_PIN, HIGH);
  digitalWrite(SENSOR_RIGHT_PIN, HIGH);
  digitalWrite(SENSOR_CENTER_PIN, HIGH);
  delayMicroseconds(10);
  pinMode(SENSOR_LEFT_PIN, INPUT);
  pinMode(SENSOR_RIGHT_PIN, INPUT);
  pinMode(SENSOR_CENTER_PIN, INPUT);
  
  unsigned long startTime = micros();

  int detectLeft = 0;
  int detectRight = 0;
  int detectCenter = 0;
  unsigned long t = micros();

  unsigned long leftTime = SENSOR_LEFT_READING;
  unsigned long rightTime = SENSOR_RIGHT_READING;
  unsigned long centerTime = SENSOR_CENTER_READING;
  
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
    if (digitalRead(SENSOR_CENTER_PIN) == LOW && detectCenter == 0) {
      centerTime = t - startTime;
      detectCenter = 1;
    }
    if (detectLeft == 1 && detectRight == 1 && detectCenter == 1){
      break;
    }
    t = micros();
  }

  // unsigned long centerTime = analogRead(SENSOR_CENTER_PIN)/2;

  leftSenseBuf[leftSenseHead] = leftTime;
  leftSenseHead = (leftSenseHead + 1) % SENSOR_AVE_SIZE;

  rightSenseBuf[rightSenseHead] = rightTime;
  rightSenseHead = (rightSenseHead + 1) % SENSOR_AVE_SIZE;
  
//  centerSenseBuf[centerSenseHead] = centerTime;
//  centerSenseHead = (centerSenseHead + 1) % SENSOR_AVE_SIZE;

   if (updateCenterReading && centerTime < MAX_CENTER) {
     centerSenseBuf[centerSenseHead] = centerTime;
     centerSenseHead = (centerSenseHead + 1) % SENSOR_AVE_SIZE; 
   }

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

  sum = 0;
  for(int i = 0; i < SENSOR_AVE_SIZE; i++) {
    sum += centerSenseBuf[i];  
  }
  SENSOR_CENTER_READING = sum / SENSOR_AVE_SIZE;

  LEFT_SENSOR_THRESH = SENSOR_CENTER_READING+LEFT_DIFF;
  RIGHT_SENSOR_THRESH = SENSOR_CENTER_READING+RIGHT_DIFF;

//  Serial.print("Left: ");
//  Serial.println(SENSOR_LEFT_READING);
//  Serial.print("Right: ");
//  Serial.println(SENSOR_RIGHT_READING);
//  Serial.print("Center: ");
//  Serial.println(SENSOR_CENTER_READING);
//  delay(200);
}

