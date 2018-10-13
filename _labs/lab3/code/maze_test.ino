const int NORTH = 3;
const int EAST  = 2;
const int SOUTH = 1;
const int WEST  = 0;
int x;
int y;
int dir = SOUTH;


unsigned char maze[3][3] = {
  {0x90, 0x80, 0xc0},
  {0x10, 0x00, 0x40},
  {0x30, 0x20, 0x60}
};

void setup() {
  // put your setup code here, to run once:
  x = 0;
  y = 0;
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void adv(){
  switch(dir){
      case NORTH:
        y=y-1;
        break;
      case EAST:
        x=x+1;
        break;
      case SOUTH:
        y=y+1;
        break;
      case WEST:
        x=x-1;
        break;
    }
}

void follow(){
  unsigned char curr = maze[y][x];
  if((bitRead(curr, dir+4)==0) && (bitRead(curr, (dir+1)%4 + 4)==1)){
    adv();
  }
  else if(bitRead(curr, (dir+1)%4 + 4)==0){
    dir = (dir+1)%4;
    adv();
  }
  else if((bitRead(curr, dir+4)==1) && (bitRead(curr, (dir+1)%4 + 4)==1) && (bitRead(curr, (dir+3)%4 + 4)==0)){
    dir = (dir+3)%4;
    adv();
  }
}

String convert(int x, int y){
  String s = String(y)+","+String(x);
  unsigned char curr = maze[y][x];
  if(bitRead(curr,4)==1){s = s + ",west=true";}
  if(bitRead(curr,5)==1){s = s + ",south=true";}
  if(bitRead(curr,6)==1){s = s + ",east=true";}
  if(bitRead(curr,7)==1){s = s + ",north=true";}
  return s;
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("reset");
  delay(1000);
  x=0;
  y=0;
  Serial.println(convert(x,y));
  delay(1000);
  for(int i = 0; i<100; i++){
    follow();
    Serial.println(convert(x,y));
    delay(1000);
  }

}
