#define NUM_COLS 9
#define NUM_ROWS 9

unsigned char maze[NUM_ROWS][NUM_COLS];

const int NORTH = 3;
const int EAST  = 2;
const int SOUTH = 1;
const int WEST  = 0;

// initial position and direction
int robotX = 0;
int robotY = 0;
int robotDir = SOUTH;

// BFs "queue" node
typedef struct maze_exp_info {
  unsigned char nRC : 8; // next row*num_rows + next col
  unsigned char dir : 2; // dir coming into this cell
  unsigned char iD : 2; // initial dirn (1 -> forward, 2 -> turn left, 3 -> right right)
  unsigned char v : 1; // visited this cell
};
// hold all of the BFS information for all of the nodes
struct maze_exp_info maze_exp_info[NUM_ROWS][NUM_COLS];

// return the turned left direction of the
// given direction
int turnDirnLeft(int dirn) {
  return (dirn+1)%4;
}

// return the turned right direction of the
// given direction
int turnDirnRight(int dirn) {
  return (dirn+3)%4;
}

// return true if our robot has visited maze position
// (x,y)
bool explored(int x, int y) {
  return maze[y][x] & 0x1 == 1 ? true : false;
}

// advance the values pointed to by x and y according
// to the direction
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
      return maze_exp_info[curY][curX].iD;
    }

    int curDir = maze_exp_info[curY][curX].dir;
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
      return 0x3;
    }
    curY = nRC / NUM_ROWS;
    curX = nRC % NUM_ROWS;
  }
  return 0x3;
}

void init_maze() {
  // assume starting with back to wall
  maze[robotY][robotX] |= 1 << (robotDir + 2);
  // assert that left sensor detects a wall
}

void set_maze(int front, int left, int right) {
  maze[robotY][robotX] |= front << (robotDir + 4);
  maze[robotY][robotX] |= left << (((robotDir+1)%4) + 4);
  maze[robotY][robotX] |= right << (((robotDir+3)%4) + 4);
}

// figure out what to do next, update the maze state for that
// and update the robot state to that new state
// return how to get to that new state
// 0 -> forward, 1 -> turn left, else -> turn right
int get_next_action() {
  // mark as explored
  maze[robotY][robotX] |= 0x1;
  int n = getNextDir();
  if (n == 0x0) {
    adv(&robotX, &robotY, robotDir);
  } else if (n == 0x1) {
    robotDir = turnDirnLeft(robotDir);
  } else {
    robotDir = turnDirnRight(robotDir);
  }
  return n;
}

unsigned long get_maze_data_payload() { 
  unsigned char curr = maze[robotY][robotX];
  
  // pack data into payload
  unsigned long payload = 0;
  payload |= curr;
  payload = payload << 8;
  payload |= robotX;
  payload = payload << 8;
  payload |= robotY;
  return payload;
} 

