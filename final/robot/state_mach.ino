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
};

RobotState state = LISTENING;

int forwardWait = 0;

void update_state_mach() {
  Serial.println(state);
  if (state == LISTENING) {
    if (digitalRead(FFT_DATA_PIN) == HIGH) {
      delay(300);
      // debounce noise
      if (digitalRead(FFT_DATA_PIN) == HIGH) {
        state = GET_NEXT_ACTION;  
        // high = IR detection
        digitalWrite(FFT_CTRL_PIN, HIGH); 
      }
    }
  } else if (state == FORWARD) {
    // wait a few cycles for line sensors to settle
    if (forwardWait < 4) {
      forwardWait++;
    } else {
      forwardWait = 0;
      //Centers and turns the robot in the appropriate direction if both line sensors
      //see a white line (come to an intersection)
      if (SENSOR_LEFT_READING < LEFT_SENSOR_THRESH && SENSOR_RIGHT_READING < RIGHT_SENSOR_THRESH) {
        // go forward and center over line
        leftWheel.write(FORWARD_LEFT);
        rightWheel.write(FORWARD_RIGHT);
        delay(200);
  
        state = GET_NEXT_ACTION;
      } else {
        //Stops left wheel to correct the robot if left line sensor sees the white line
        if (SENSOR_LEFT_READING < LEFT_SENSOR_THRESH) leftWheel.write(STOP_POS);
        else leftWheel.write(FORWARD_LEFT);
        
        //Stops right wheel to correct the robot if right line sensor sees the white line
        if (SENSOR_RIGHT_READING < RIGHT_SENSOR_THRESH) rightWheel.write(STOP_POS);
        else rightWheel.write(FORWARD_RIGHT);
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
    }
  } else if (state == RIGHT_2) {
    // turn right step 2
    // turn until see line
    if (SENSOR_RIGHT_READING < RIGHT_SENSOR_THRESH) {
      state = RIGHT_3;
      // give a bit of time for measurement to settle
      delay(100); 
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
    }    
  } else if (state == LEFT_2) {
    // turn left step 2
    // turn until see line
    if (SENSOR_LEFT_READING < LEFT_SENSOR_THRESH) {
      state = LEFT_3;
      // give a bit of time for measurement to settle
      delay(100); 
    }
  } else if (state == LEFT_3) {
    // turn left step 3
    // turn until past line
    if (SENSOR_LEFT_READING > LEFT_SENSOR_THRESH) {
      state = GET_NEXT_ACTION;
      delay(100);
    }
  } else if (state == GET_NEXT_ACTION) {
    leftWheel.write(STOP_POS);
    rightWheel.write(STOP_POS);

    update_wall_sensor_values();

    // get 3 point wall information
    int isFrontWall = is_wall_in_front();
    int isRightWall = is_wall_on_right();
    int isLeftWall = is_wall_on_left();

    // set the maze for the current position
    set_maze(isFrontWall, isLeftWall, isRightWall);

    // send the current maze cell data back to the base station
//    sendData();

    // get the next action to do and advance the current maze state
    // by that action
    int nextAction = get_next_action();
    if (nextAction == 0x0) {
      state = FORWARD;
      updateCenterReading = 1;
    } else if (nextAction == 0x1) {
      state = LEFT_1;
      updateCenterReading = 0;
    } else {
      state = RIGHT_1;
      updateCenterReading = 0;
    }
  }

  
  if (state != LISTENING) {
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
  }
}

