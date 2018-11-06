const int NORTH = 3;
const int EAST  = 2;
const int SOUTH = 1;
const int WEST  = 0;
int robotX = 0;
int robotY = 0;
int robotDir = EAST;

#define NUM_ROWS 9
#define NUM_COLS 9

//typedef struct maze_elem {
//  unsigned char N : 1;
//  unsigned char E : 1;
//  unsigned char S : 1;
//  unsigned char W : 1;
//  unsigned char shape : 2;
//  unsigned char color : 1;
//  unsigned char explored : 1;
//};

// N | S | E | W | SHAPE | COLOR | Explored
//unsigned char maze[NUM_ROWS][NUM_COLS] = {
//  {0x90, 0x80, 0x80, 0xc0},
//  {0x10, 0x00, 0x00, 0x40},
//  {0x10, 0x00, 0x00, 0x40},
//  {0x30, 0x20, 0x20, 0x60}
//};
//unsigned char maze[NUM_ROWS][NUM_COLS] = {
//  {0x90, 0xa0, 0xa0, 0xc0},
//  {0x10, 0x80, 0xc0, 0x50},
//  {0x10, 0x00, 0x40, 0x70},
//  {0x30, 0x20, 0x20, 0xe0}
//};
unsigned char maze[NUM_ROWS][NUM_COLS] = {
  {0x90, 0xa0, 0xa0, 0xc0},
  {0x10, 0x80, 0xc0, 0x50},
  {0x10, 0x20, 0x60, 0x70},
  {0x30, 0xa0, 0xa0, 0xe0}
};
//unsigned char maze[NUM_ROWS][NUM_COLS] = {
//  {0x90, 0x80, 0x80, 0xc0},
//  {0x50, 0x10, 0x00, 0x40},
//  {0x50, 0x30, 0x20, 0x60},
//  {0x30, 0xa0, 0xa0, 0xe0}
//};

typedef struct maze_exp_info {
  unsigned char nRC : 8; // next row*num_rows + next col
  unsigned char dir : 2; // dir coming into this cell
  unsigned char iD : 2; // initial dirn (1 -> forward, 2 -> turn left, 3 -> right right)
  unsigned char v : 1; // visited this cell
};
struct maze_exp_info maze_exp_info[NUM_ROWS][NUM_COLS];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

int turnDirnLeft(int dirn) {
  return (dirn+1)%4;
}

int turnDirnRight(int dirn) {
  return (dirn+3)%4;
}

bool explored(int x, int y) {
  return maze[y][x] & 0x1 == 1 ? true : false;
}

void adv(int* x, int* y, int dirn){
  switch(dirn){
      case NORTH:
        *y=*y-1;
        break;
      case EAST:
        *x=*x+1;
        break;
      case SOUTH:
        *y=*y+1;
        break;
      case WEST:
        *x=*x-1;
        break;
    }
}

unsigned char getMazeWall(int x, int y, int dirn) {
  return (((unsigned char) maze[y][x]) >> (dirn + 4)) & 0x1;
}

bool canAdvance(int x, int y, int dirn) {
  int advX = x;
  int advY = y;
  adv(&advX, &advY, dirn);
  if (advX < 0 || advX >= NUM_COLS || advY < 0 || advY >= NUM_ROWS) {
    return false;
  }
  if (getMazeWall(x, y, dirn) == 1) {
    return false;
  }
  return true;
}

void queueAdd(int x, int y, int* fromX, int* fromY, int advDirn, int moved) {
  int advX = x;
  int advY = y;
  adv(&advX, &advY, advDirn);
  // visit if not visited
  if (maze_exp_info[advY][advX].v == 0x0) {
//    Serial.print("adv:");  
//    Serial.print(advX);
//    Serial.print(",");
//    Serial.print(advY);
//    Serial.print(",");
//    Serial.println(advDirn);
    // setup the initial direction for next node to 
    // the same as however we got here
    if (maze_exp_info[y][x].iD == 0x3) {
     maze_exp_info[advY][advX].iD = moved;
    } else {
     maze_exp_info[advY][advX].iD = maze_exp_info[y][x].iD; 
    }
    // set incoming dirn to advance dirn
    maze_exp_info[advY][advX].dir = advDirn;
    // mark the node we will visit as visited
    maze_exp_info[advY][advX].v = 0x1;

    // set the previous element on the queue to point
    // to this enqueued node
    maze_exp_info[*fromY][*fromX].nRC = advY*NUM_ROWS + advX;
    // set top bit to 1 if you want to delay it by 1
    if (moved != 0x0) {
      maze_exp_info[*fromY][*fromX].nRC |= 0x80;
    }

    // set next round of pointer chasing
    *fromY = advY;
    *fromX = advX; 
  }
}

int getNextDir() {
  // invariant x,y should be explored
  int curX = robotX;
  int curY = robotY;
  unsigned char delayValue = 0;
  // reset visited
  for (int i = 0; i < NUM_ROWS; i++) {
    for (int j = 0; j < NUM_ROWS; j++) {
       maze_exp_info[i][j].v = 0;
       maze_exp_info[i][j].nRC = 0xf;
       maze_exp_info[i][j].dir = robotDir;
       maze_exp_info[i][j].iD = 0x3;
    }
  }

  int prevX = curX;
  int prevY = curY;
  maze_exp_info[curY][curX].v = 0x1;
  int numIters = 0;

  // invariant: if a node gets popped, it must be visited
  while(curX != 0xf && curY != 0xf) {
    numIters++;
//    Serial.print(curX);
//    Serial.print(",");
//    Serial.println(curY);
    if (delayValue == 1) {
      // remove node and reinsert at end of queue
      maze_exp_info[prevY][prevX].nRC = curY*NUM_ROWS + curX;
      prevY = curY;
      prevX = curX;

      // get the next element
      unsigned char nRC = maze_exp_info[curY][curX].nRC;
      delayValue = bitRead(nRC, 7);
      nRC = nRC & (~0x80);
      curY = nRC / NUM_ROWS;
      curX = nRC % NUM_ROWS;
      continue;
    }
    
    if (!explored(curX, curY)) {
//      Serial.println("unex");
      return maze_exp_info[curY][curX].iD;
    }

    int curDir = maze_exp_info[curY][curX].dir;
//    Serial.print("dir:");  
//    Serial.println(curDir);
    // check if can go forward
    if (canAdvance(curX, curY, curDir)) {
      queueAdd(curX, curY, &prevX, &prevY, curDir, 0x0);
    }
    
    int leftDir = turnDirnLeft(curDir);
    // check if can go left
    if (canAdvance(curX, curY, leftDir)) {
      queueAdd(curX, curY, &prevX, &prevY, leftDir, 0x1);
    }

    int rightDir = turnDirnRight(curDir);
    // check if can go right
    if (canAdvance(curX, curY, rightDir)) {
      queueAdd(curX, curY, &prevX, &prevY, rightDir, 0x2);
    }

    // get the next element
    unsigned char nRC = maze_exp_info[curY][curX].nRC;
    delayValue = bitRead(nRC, 7);
    nRC = nRC & (~0x80);
    // if next equals cur, then we know we are done (otherwise infinite loop)
    if (numIters > 4*NUM_ROWS*NUM_COLS) {
//      Serial.println("infinite loop");
      return 0x3;
    }
    curY = nRC / NUM_ROWS;
    curX = nRC % NUM_ROWS;
  }
  return 0x3;
}

String convert(int x, int y){
  String s = String(y)+","+String(x)+",";
  unsigned char curr = maze[y][x];
  if(bitRead(curr,4)==1){s = s + "west=true,";}
  if(bitRead(curr,5)==1){s = s + "south=true,";}
  if(bitRead(curr,6)==1){s = s + "east=true,";}
  if(bitRead(curr,7)==1){s = s + "north=true,";}
  return s;
}

void loop() {
  Serial.println("reset");
  delay(1000);
  robotX=0;
  robotY=0;
  robotDir=EAST;

  Serial.println(convert(robotX,robotY));
  delay(1000);
  for(int i = 0; i<100; i++){
    // mark as explored
    maze[robotY][robotX] |= 0x1;
    int n = getNextDir();
    if (n == 0x0) {
//      Serial.println("fwd");
      adv(&robotX, &robotY, robotDir);
    } else if (n == 0x1) {
//      Serial.println("turn left");
      robotDir = turnDirnLeft(robotDir);
    } else {
//      Serial.println("turn right");
      robotDir = turnDirnRight(robotDir);
    }
    Serial.println(convert(robotX,robotY));
    delay(1000);
  }
  
}
