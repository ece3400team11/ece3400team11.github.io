enum RobotState {
  LISTENING,
  FORWARD,
  RIGHT_1,
  RIGHT_2,
  RIGHT_3,
  LEFT_1,
  LEFT_2,
  LEFT_3,
  GET_NEXT_ACTION,
  SAW_INTER,
};

RobotState state = LISTENING;

int forwardWait = 0;

int attempt_u_turn = 0;

unsigned long state_start_time = 0;
#define FORWARD_TIME_THRESH 3500
#define FORWARD_TIME_MIN 500
#define LEFT_TIME_THRESH 2000
#define RIGHT_TIME_THRESH 2000

#define SHAPE1_PIN A3
#define SHAPE2_PIN A4
#define COLOR_PIN A5

int num_states = 0;

void update_state_mach() {
  Serial.println(state);
//  Serial.println(state_start_time);
//  Serial.println(millis());
  if (state == LISTENING) {
    if (digitalRead(FFT_DATA_PIN) == HIGH) {
      delay(300);
      // debounce noise
      if (digitalRead(FFT_DATA_PIN) == HIGH) {
        delay(300);
        if (digitalRead(FFT_DATA_PIN) == HIGH) {
          state = GET_NEXT_ACTION;
          // high = IR detection
          digitalWrite(FFT_CTRL_PIN, HIGH); 
        }
      }
    }
  } else if (state == SAW_INTER) {
    if (forwardWait < 4) {
      forwardWait++;
    } else {
      forwardWait = 0;
      if (SENSOR_LEFT_READING < LEFT_SENSOR_THRESH && SENSOR_RIGHT_READING < RIGHT_SENSOR_THRESH) {
        // go forward and center over line
        leftWheel.write(FORWARD_LEFT);
        rightWheel.write(FORWARD_RIGHT);
        delay(150);

        state = GET_NEXT_ACTION;
      } else {
        state = FORWARD;
      }
    }
  } else if (state == FORWARD) {
    // wait a few cycles for line sensors to settle
    if (forwardWait < 4) {
      forwardWait++;
    } else if (millis() - state_start_time > FORWARD_TIME_THRESH) {
      Serial.println("Time up");
      state = GET_NEXT_ACTION;
    } else {
//      forwardWait = 0;
      //Centers and turns the robot in the appropriate direction if both line sensors
      //see a white line (come to an intersection)
      if (SENSOR_LEFT_READING < LEFT_SENSOR_THRESH && SENSOR_RIGHT_READING < RIGHT_SENSOR_THRESH) {
        if (millis() - state_start_time > FORWARD_TIME_MIN) {
          state = SAW_INTER; 
          Serial.println("Intersect");
          forwardWait = 0;
        }
      } else {
        //Stops left wheel to correct the robot if left line sensor sees the white line
        if (SENSOR_LEFT_READING < LEFT_SENSOR_THRESH) leftWheel.write(STOP_POS);
        else leftWheel.write(FORWARD_LEFT);
        
        //Stops right wheel to correct the robot if right line sensor sees the white line
        if (SENSOR_RIGHT_READING < RIGHT_SENSOR_THRESH) rightWheel.write(STOP_POS);
        else rightWheel.write(FORWARD_RIGHT);
      } 
    }

    // only look for robots if going forward
    if(num_states > 1 && digitalRead(FFT_DATA_PIN) == HIGH) {
      delay(30);
      if (digitalRead(FFT_DATA_PIN) == HIGH) {
       // if (analogRead(FRONT_IR_SENSOR) < FRONT_WALL_THRESH) {
        // turn around  
        Serial.println("IR detect");
        state = LEFT_1;
        attempt_u_turn = 1;
        updateCenterReading = 0;
        state_start_time = millis();
//        delay(500);
      //} 
      }
    }
  } else if (state == RIGHT_1) {
    // turn right step 1
    leftWheel.write(FORWARD_LEFT);
    rightWheel.write(BACKWARD_RIGHT);

    // turn until past center line (may skip this call)
    if (SENSOR_RIGHT_READING > RIGHT_SENSOR_THRESH) {
      state = RIGHT_2;
      // give a bit of time for measurement to settle
      delay(100);
    } else if (millis() - state_start_time > RIGHT_TIME_THRESH) {
      state = GET_NEXT_ACTION;
    }
  } else if (state == RIGHT_2) {
    // turn right step 2
    // turn until see line
    if (SENSOR_RIGHT_READING < RIGHT_SENSOR_THRESH) {
      state = RIGHT_3;
      // give a bit of time for measurement to settle
      delay(100); 
    } else if (millis() - state_start_time > RIGHT_TIME_THRESH) {
      state = GET_NEXT_ACTION;
    }
  } else if (state == RIGHT_3) {
    // turn right step 3
    // turn until past line
    if (SENSOR_RIGHT_READING > RIGHT_SENSOR_THRESH) {
      state = GET_NEXT_ACTION;
      delay(100);

      // start going forward again
      leftWheel.write(FORWARD_LEFT);
      rightWheel.write(FORWARD_RIGHT);
    } else if (millis() - state_start_time > RIGHT_TIME_THRESH) {
      state = GET_NEXT_ACTION;
    }
  }  else if (state == LEFT_1) {
    // turn left step 1
    leftWheel.write(BACKWARD_LEFT);
    rightWheel.write(FORWARD_RIGHT);

    // turn until past center line (may skip this call)
    if (SENSOR_LEFT_READING > LEFT_SENSOR_THRESH) {
      state = LEFT_2;
      // give a bit of time for measurement to settle
      delay(100);
    } else if (!attempt_u_turn && millis() - state_start_time > LEFT_TIME_THRESH) {
      state = GET_NEXT_ACTION;
    }
  } else if (state == LEFT_2) {
    // turn left step 2
    // turn until see line
    if (SENSOR_LEFT_READING < LEFT_SENSOR_THRESH) {
      state = LEFT_3;
      // give a bit of time for measurement to settle
      delay(100); 
    } else if (!attempt_u_turn && millis() - state_start_time > LEFT_TIME_THRESH) {
      state = GET_NEXT_ACTION;
    }
  } else if (state == LEFT_3) {
    // turn left step 3
    // turn until past line
    if (SENSOR_LEFT_READING > LEFT_SENSOR_THRESH) {
      state = GET_NEXT_ACTION;
      delay(100);
    } else if (!attempt_u_turn && millis() - state_start_time > LEFT_TIME_THRESH) {
      state = GET_NEXT_ACTION;
    }
  } else if (state == GET_NEXT_ACTION) {
    leftWheel.write(STOP_POS);
    rightWheel.write(STOP_POS);

    if (attempt_u_turn) {
      // reset next value
      maze[robotY][robotX] = 0;

      // move my position back
      robotDir = (robotDir + 2) % 4;
      adv(&robotX, &robotY, robotDir);
      Serial.println("moving forward");
      
      if (millis() - state_start_time > 1100) {
        // made u turn
        state = FORWARD;
        state_start_time = millis() - FORWARD_TIME_MIN;
        updateCenterReading = 1;
        forwardWait = 0;
        leftWheel.write(FORWARD_LEFT);
        rightWheel.write(FORWARD_RIGHT);
        attempt_u_turn = 0;
      } else {
        // only turned 90 degrees to the left, finish 180 degree turn
        state = LEFT_1;
        attempt_u_turn = 0;
        // update direction by turning right once
//        robotDir = (robotDir + 3) % 4;  
      }
    } else {
      update_wall_sensor_values();

      // get 3 point wall information
      int isFrontWall = is_wall_in_front();
      int isRightWall = is_wall_on_right();
      int isLeftWall = is_wall_on_left();
  
      // set the maze for the current position
      set_maze(isFrontWall, isLeftWall, isRightWall);
  
      // set the treasure data for current pos
      int shape1 = analogRead(SHAPE1_PIN) > 500;
      int shape2 = analogRead(SHAPE2_PIN) > 500;
      int color = analogRead(COLOR_PIN) > 500;
      set_treasure(0, 0, 0, isFrontWall, isLeftWall);
  
      // send the current maze cell data back to the base station
      sendData();
//      delay(500);
  
      // get the next action to do and advance the current maze state
      // by that action
      int nextAction = get_next_action();
      num_states++;
      if (nextAction == 0x0) {
        state = FORWARD;
        updateCenterReading = 1;
        forwardWait = 0;
        leftWheel.write(FORWARD_LEFT);
        rightWheel.write(FORWARD_RIGHT);
      } else if (nextAction == 0x1) {
        state = LEFT_1;
        updateCenterReading = 0;
      } else {
        state = RIGHT_1;
        updateCenterReading = 0;
      }
      state_start_time = millis(); 
    }
  }
}

