
#include <Servo.h>

Servo myservo1;
Servo myservo2;
Servo gripper;

int pos1 = 90;
int pos2 = 90;
int loopVar = 0;

const int VERT1 = A0; // analog
const int VERT2 = A1;
const int SELECT = 11;

void setup()
{
  pinMode(VERT1, INPUT);
  pinMode(VERT2, INPUT);
  pinMode(SELECT, INPUT);
  Serial.begin(9600);
  myservo1.attach(6);
  myservo2.attach(9);
  gripper.attach(5);
}

void loop()
{
  loopVar++;
  int vert1, vert2,grip;
  bool select = false;

  vert1 = analogRead(VERT1);
  vert2 = analogRead(VERT2);
  select = digitalRead(SELECT);  
  if(!select){
//    if(grip != 0){
     grip+=15; 
//    }
//    else{
//      grip -= 15;
//    }
  }
  // POS 2 RIGHT SERVO
  if ( vert1 > 900 ) {
    pos1 = pos1 + 15;
  } else if ( vert1 < 200 ) {
    pos1 = pos1 - 15;
  }
  if ( pos1 >= 140 ) {
    pos1 = 140;
  }
  if ( pos1 <= 60 ) {
    pos1 = 60;
  }
  if ( vert2 > 900 ) {
    pos2 = pos2 + 15;
  } else if ( vert2 < 200 ) {
    pos2 = pos2 - 15;
  }
  if ( pos2 >= 90 ) {
    pos2 = 90;
  }
  // toward ard
  if ( pos2 <= 35 ) {
    pos2 = 35;
  }
  myservo1.write(pos1);
  myservo2.write(pos2);
//  gripper.write(grip);
  delay(100);

  // print out the values
  Serial.print("pos1: ");
  Serial.print(pos1, DEC);
  Serial.print(" pos2: ");
  Serial.print(pos2, DEC);
  Serial.print(" vert1: ");
  Serial.print(vert1, DEC);
  Serial.print(" vert2: ");
  Serial.print(vert2, DEC);
  Serial.println("...");
}
