int nearSensor = A0;
int farSensor = A5;
int nearDist = 0;
int farDist = 0;

void setup() {
  // setup serial at 9600 baud
  Serial.begin(9600);   
}

// the loop function runs over and over again forever
void loop() {
  nearDist = analogRead(nearSensor);         
  farDist = analogRead(farSensor);
  Serial.print("Near: ");
  Serial.print(nearDist);
  Serial.print(", Far: ");
  Serial.println(farDist);           
  // wait half a second
  delay(500);
}
