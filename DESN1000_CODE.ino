#include <SPI.h>
#include <Pixy2.h>
Pixy2 pixy;
int checkOffset(int tracking_index);
void setup() {
  Serial.begin(9600);
  pixy.init();
}

void loop() {
  pixy.ccc.getBlocks();
  // If there are blocks, check if any are valid food objects
  if (pixy.ccc.numBlocks) {
    int lowest_y = -1;
    int lowest_tracking_index = -1;
    for (int i = 0; i < pixy.ccc.numBlocks; i++) {
      // If detecting red, go to next object
      if (pixy.ccc.blocks[i].m_signature == 3) {
        continue;
      }
      //Checking for lowest object on screen (closest to bot)
      if (pixy.ccc.blocks[i].m_y > lowest_y && pixy.ccc.blocks[i].m_age > 2) {
        lowest_tracking_index = pixy.ccc.blocks[i].m_index;
        lowest_y = pixy.ccc.blocks[i].m_y;
      }
    }
    Serial.println("Lowest index: " + lowest_tracking_index);
    if (lowest_tracking_index != -1) {
      centerObject(lowest_tracking_index);
      //Drive Forwards
    }
  }
}

//Center Object
// Scans vision for specific object and turns until the object is in the middle of the screen
// Parameters:
//  int tracking_index - index of closest "positive" object used by pixy
// Returns: None
void centerObject(int tracking_index) {
  int offset = 0;
  while ((offset = checkOffset(tracking_index)) && abs(offset) > 3) {
    if (offset > 0) {
      //stationary turn ______ (left/right)
    } else {
      //stationary turn ______ (left/right)
    }
    delay(50);
  }
}

//Check Offset
// Scans vision for object with specific tracking_index and returns the offset from the center of the bot's vision
// Parameters:
//  int tracking_index - index of closest "positive" object used by pixy
// Returns: offset from center of pixy screen
int checkOffset(int tracking_index) {
  pixy.ccc.getBlocks();
  for (int i = 0; i < pixy.ccc.numBlocks; i++) {
    if (pixy.ccc.blocks[i].m_index == tracking_index) {
      //Subtracts the object's x value from the middle of the frame
      return pixy.frameWidth/2 - pixy.ccc.blocks[i].m_x;
    }
  }
}
