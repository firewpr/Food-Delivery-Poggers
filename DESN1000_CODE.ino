//////////////////////////////////////////////////////////////////
// This is the basic Sumobots scaffold code authored by the workshops and
// projects team for participants in MTRNSoc's 2024 sumobot competition.
////////////////////////////////////////////////////////////////////////////////

// TODO: Define ultrasonic and infrared sensor pins
// REFER TO ARDUINO BASICS WORKSHOP ON HOW TO CODE SENSORS
#include <SPI.h>
#include <Pixy2.h>

Pixy2 pixy;
int checkOffset(int tracking_index);
// Ultrasonic sensor pins
// CHANGE THIS
#define echoPin1 6
#define trigPin1 7
#include <Servo.h>
Servo Servo1;
Servo Servo2;
int servo1 = 11;
int servo2 = 12;
// TODO: Define constants/variables for motors (workshop 4)
int LEFT_IN1 = 3;  // Speed pin, ranges from 0 to 255 (PWMA)
int LEFT_IN2 = 2;      // Direction pin (DIRA)
int mode = 0;
int RIGHT_IN2 = 5;  // Speed pin, ranges from 0 to 255 (PWMB)
int RIGHT_IN1 = 4;      // Pin to move motor forwards (DIRB)
int RED_LED = 8;
int BLUE_LED = 9;
int GREEN_LED = 10;
int base_col = -1;

// TODO: Define other constants to be used in your sumobot
#define MAX_SPEED 255
#define PARTIAL_SPEED 30
#define SPEEDOFSENSOR 0.0340


void centerObject(int tracking_index);
int checkObjectDistance(int tracking_index);
int checkOffset(int tracking_index);
void detectObstacle(int trigPin, int echoPin);
double getDistance(int trigPin, int echoPin);
void driveForwards(int speed);
void driveBackwards(int speed);
void stationaryTurnLeft(int speed);
void turnLeft(int speed);
void stationaryTurnRight(int speed);
void turnRight(int speed);
void stop();
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Setup Function /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void setup() {
  int i = 0;
  pixy.init();
  pixy.getResolution();
  Servo1.attach(servo1);
  Servo2.attach(servo2);
  Servo1.write(125);
  Servo2.write(38);
  Serial.begin(9600);
  while (i < 3) {

    delay(1000);
    i++;
  }
  i = 0;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// Loop Function /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// stop 2s, drive back 10ms, continue

void loop() {
  
  delay(2000);
  //-1 Mode Searching for base colour
  if (mode == -1) {
    while (mode == -1) {
      pixy.ccc.getBlocks();
      for (int i = 0; i < pixy.ccc.numBlocks; i++) {
        //Signatures 4-7 are the colours of the 4 different base colours
        if (pixy.ccc.blocks[i].m_signature == 4) {
          base_col = 4;
          mode = 0;
          break;
        } else if (pixy.ccc.blocks[i].m_signature == 5) {
          base_col = 5;
          mode = 0;
          break;
        } else if (pixy.ccc.blocks[i].m_signature == 6) {
          base_col = 6;
          mode = 0;
          break;
        } else if (pixy.ccc.blocks[i].m_signature == 7) {
          base_col = 7;
          mode = 0;
          break;
        }
      }
    driveBackwards(180);
    delay(100);
    stationaryTurnRight(255);
    delay(200);
    stop();
    }
  //MODE 0 SEARCHING
  } else if (mode == 0) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(GREEN_LED, LOW); 
    pixy.ccc.getBlocks();
    // If there are blocks, check if any are valid food objects
    if (pixy.ccc.numBlocks) {
      int lowest_x = -1;
      int lowest_tracking_index = -1;
      for (int i = 0; i < pixy.ccc.numBlocks; i++) {
        // If detecting red, go to next object
        if (pixy.ccc.blocks[i].m_signature == 3) {
          continue;
        }
        //Checking for lowest object on screen (closest to bot) + is blue or green
        if (pixy.ccc.blocks[i].m_x > lowest_x && pixy.ccc.blocks[i].m_age > 2 && (pixy.ccc.blocks[i].m_signature == 2 || pixy.ccc.blocks[i].m_signature == 1)) {
          lowest_tracking_index = pixy.ccc.blocks[i].m_index;
          lowest_x = pixy.ccc.blocks[i].m_x;
        }
      }
      Serial.println("Lowest index: " + lowest_tracking_index);
      if (lowest_tracking_index != -1) {
        centerObject(lowest_tracking_index);
        //Drives until object is at a specific close spot on the screen
        while (checkObjectDistance(lowest_tracking_index) != 1) {
          driveForwards(80);
          delay(50);
        }
        driveBackwards(255);
        delay(20);
        stop();
        mode = 1;
      //BOTH stationaryTurnRight meant to change searching range to get new objects on screen
      } else {
        stationaryTurnRight(100);
        delay(100);
        stop();
      }
    } else {
      stationaryTurnRight(100);
      delay(100);
      stop();
    }
  //MODE 1 PICKING UP - RED LED ON
  } else if (mode == 1) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    //Servo opens up

    for (int j = 43; j > 15; j--) {
    Servo2.write(j);
    Servo1.write(175 - j);
    delay(20);
    }
    delay(300);
    //NEED TO CHANGE AMOUNT TO CENTER ON PINCERS
    driveForwards(50);
    delay(100);
    stop();
    //Close servos
    for (int i = 15; i <  43; i++) {
    Servo2.write(i);
    Servo1.write(175 - i);
    delay(20);
  }
    delay(300);

    mode = 2;
    //CARRYING MODE - BLUE LED
  } else if (mode == 2) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    delay(1000);
    //Turns until base is in the center of screen
    int base_detected = 0;
    while (base_detected == 0) {
      stationaryTurnLeft(100);
      pixy.ccc.getBlocks();
      for (int i = 0; i < pixy.ccc.numBlocks; i++) {
        if (pixy.ccc.blocks[i].m_signature == base_col && abs((pixy.frameHeight/2 - pixy.ccc.blocks[i].m_y)) < 5) {
          stop();
          //drive towards base until close
          while (getDistance(trigPin1, echoPin1) > 15) {
            driveForwards(200);
            delay(50);
          }
          //
          driveBackwards(200);
          delay(20);
          stop();
          mode = 3;
          break;
        }
      }
    } 
  //RELEASING MODE - GREEN LED
  } else if (mode == 3) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    for (int j = 43; j > 15; j--) {
    Servo2.write(j);
    Servo1.write(175 - j);
    delay(20);
    Serial.println(j);
    }
    delay(2000);
    for (int i = 15; i <  43; i++) {
    Servo2.write(i);
    Servo1.write(175 - i);
    delay(20);
    }
    driveBackwards(255);
    delay(200);
    stop();
    //Turn to face center of arena
    stationaryTurnRight(255);
    delay(240);
    stop();
    //searching mode
    mode = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// Sensor Functions ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//Center Object
// Scans vision for specific object and turns until the object is in the middle of the screen
// Parameters:
//  int tracking_index - index of closest "positive" object used by pixy
// Returns: None
void centerObject(int tracking_index) {
  int offset = 0;
  while ((offset = checkOffset(tracking_index)) && abs(offset) > 3) {
    if (offset > 0) {
      stationaryTurnLeft(80);
    } else {
      stationaryTurnRight(80);
    }
    delay(50);
  }
}

int checkObjectDistance(int tracking_index) {
  pixy.ccc.getBlocks();
  for (int i = 0; i < pixy.ccc.numBlocks; i++) {
    if (pixy.ccc.blocks[i].m_index == tracking_index) {
      if (pixy.ccc.blocks[i].m_x > 50) {
        return 0;
      } else {
        return 1;
      }
    }

  }
  return -1;
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
      return pixy.frameHeight/2 - pixy.ccc.blocks[i].m_y;
    }
  }
  return 999;
}

void detectObstacle(int trigPin, int echoPin) {
  driveBackwards(100);
  delay(25);
  while (getDistance(trigPin, echoPin) < 15 && getDistance(trigPin, echoPin) < 15) {
    stop();
    delay(1000);
  }
  
  driveBackwards(100);
  delay(10);
  stop();
  delay(100);
}
/*  Function: Get Distance
/   parameters: trigPin, EchoPin
/   returns: distance
/   summary: uses ultrasonic sensor to get distance of object in front
*/
double getDistance(int trigPin, int echoPin) {
  double distance, duration;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration * SPEEDOFSENSOR) / 2;  // distance's unit is cm and duration is in ms
  Serial.println("Distance detected: " + String(distance) + " cm");
  return distance;
}

// IMPORTANT NOTE:
// How will you prevent your sumobot from detecting objects outside of the ring?


// What if you have more than 1 ultrasonic or infrared sensor? Do you need more
// reading functions or can the same ones be used?

void driveForwards(int speed) {
    Serial.println("Driving forward");
    digitalWrite(RIGHT_IN2, LOW);
    analogWrite(RIGHT_IN1, speed+20);
    digitalWrite(LEFT_IN2, LOW);
    analogWrite(LEFT_IN1, speed);
}
// ADDITIONAL: How can we change the above function to all the sumobot to move
// forward at a variable speed? HINT: Modify the analogWrite functions

/*  Function: Drive backwards
/   parameters: none
/   returns: none
/   summary: this function drives sumobot backwards
*/
void driveBackwards(int speed) {
  Serial.println("Driving backwards");
    digitalWrite(RIGHT_IN1, LOW);
    analogWrite(RIGHT_IN2, speed+20);
    digitalWrite(LEFT_IN1, LOW);
    analogWrite(LEFT_IN2, speed);
}

/*  Function: Turn left
/   parameters: none
/   returns: none
/   summary: this function turns sumobot to the left
*/

//Turn left in place
void stationaryTurnLeft(int speed) {
  Serial.println("Turning left");
  digitalWrite(RIGHT_IN2, LOW);
  analogWrite(RIGHT_IN1, speed+20);
  digitalWrite(LEFT_IN1, LOW);
  analogWrite(LEFT_IN2, speed);

}

//Turn left with an arc
void turnLeft(int speed) {
  Serial.println("Moving left");
  digitalWrite(RIGHT_IN2, LOW);
  analogWrite(RIGHT_IN1, speed+20);
  digitalWrite(LEFT_IN2, LOW);
  digitalWrite(LEFT_IN1, LOW);
}


// What other movement functions might we need?
// TODO: Create some of your own movement functions.
//TURN RIGHT Function
void stationaryTurnRight(int speed) {
  Serial.println("Turning right");
  digitalWrite(RIGHT_IN1, LOW);
  analogWrite(RIGHT_IN2, speed+20);
  digitalWrite(LEFT_IN2, LOW);
  analogWrite(LEFT_IN1, speed);
}

void turnRight(int speed) {
  Serial.println("Moving right");

  digitalWrite(LEFT_IN2, LOW);
  analogWrite(LEFT_IN1, speed+20);
  digitalWrite(RIGHT_IN2, LOW);
  digitalWrite(RIGHT_IN1, LOW);
}


//Stop function
void stop() {
  Serial.println("STOP");
  digitalWrite(LEFT_IN2, HIGH);
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(RIGHT_IN2, HIGH);
  digitalWrite(RIGHT_IN1, HIGH);
}
