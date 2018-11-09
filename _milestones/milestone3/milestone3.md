---
layout: blank
title:  "Milestone 3"
subtitle: "Algorithms"
status: "disabled" # disabled, active, complete
preview: "assets/images/project-1.jpg"
---

# Milestone 3

## Maze exploration algorithm

The maze exploration algorithm that we used was DFS. However, the DFS algorithm was not explicitly coded into our robot. Instead, at each intersection the robot determines what its next action should be: go forward, turn left, or turn right. In order to determine this, the robot performs a BFS search on the currently explored parts of the maze and finds the closest unexplored frontier node. It then performs one of the 3 above actions in order to get it closer to that node. Since each of the edge weights in the BFS algorithm have a weight of 1, this is equivalent to performing Dijkstra's in order find the closest unexplored node. By iteratively performing the “find the closest unexplored node and move towards it”, our robot performs a DFS mapping of the maze.

The implementation of this algorithm requires 2 data structures. The first is an RxC array representing each of the intersections in the maze which tell us where the walls are and if we have explored the intersection. We actually already have this data structure from Lab 3. The additional data structure that we need to create is a BFS queue. At this point we make a distinction between explored and visited nodes. An explored node is a node that our robot has already physically been to. A visited node is a node that the BFS algorithm has visited in its search to find the closest unexplored node. If a node is put on the BFS queue, it must have been visited. A visited node cannot be put on the BFS queue. This means that a node will only be on the BFS queue once. We utilized this fact to build an implicit BFS queue in another RxC array. This RxC array was called the BFS matrix and it holds pointers to the next row and column to explore in the BFS algorithm. It also hold the visited information, the current direction going into that node, and the initial action that we took to get to that node.

Here is a description of the code we wrote.


This code sets up the initial BFS matrix state:

```cpp
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
```

We pop a node by setting the current x and y positions to the next pointer in the current BFS buffer cell:

```cpp
unsigned char nRC = maze_exp_info[curY][curX].nRC;
delayValue = bitRead(nRC, 7);
nRC = nRC & (~0x80);
// if next equals cur, then we know we are done (otherwise infinite loop)
if (numIters > 4*NUM_ROWS*NUM_COLS) {
  return 0x3;
}
curY = nRC / NUM_ROWS;
curX = nRC % NUM_ROWS;
```

We insert a delay value for turning left and turning right to account for the fact that going to the left or right cell takes 2 steps:

```cpp
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
```

If we have found an unexplored cell, this is where we want to go:

```cpp
if (!explored(curX, curY)) {
  return maze_exp_info[curY][curX].iD;
}
```

We then add the forward, left, and right cell to the implicit BFS queue:

```cpp
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
```

We use this helper function to determine if a node can be added to the queue:

```cpp
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
```
