void startup() {
  leftWheel.write(STOP_POS);
  rightWheel.write(STOP_POS);

  delay(300);

  leftWheel.write(FORWARD_LEFT);

  delay(300);

  leftWheel.write(BACKWARD_LEFT);

  delay(300);
  leftWheel.write(STOP_POS);

  rightWheel.write(FORWARD_RIGHT);

  delay(300);

  rightWheel.write(BACKWARD_RIGHT);

  delay(300);
  rightWheel.write(STOP_POS);

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  Serial.println(FRONT_WALL_READING);
  while(!is_wall_in_front()) {
    update_wall_sensor_values();
    Serial.println(FRONT_WALL_READING);
  }
  digitalWrite(4, HIGH);
  delay(1000);
  digitalWrite(4, LOW);

  while(!is_wall_on_left()) {
    update_wall_sensor_values();  
  }
  digitalWrite(4, HIGH);
  delay(1000);
  digitalWrite(4, LOW);

  while(!is_wall_on_right()) {
    update_wall_sensor_values();  
  }
  digitalWrite(4, HIGH);
  delay(1000);
  digitalWrite(4, LOW);

  
}
