#include <SoftwareSerial.h>
#include <Servo.h>

SoftwareSerial mySerial(3, 11); // RX, TX
Servo leftServo;
Servo rightServo;
Servo rotateServo;
Servo gripperServo;
char incomingByte = '\0';   // for incoming serial data

String inputString = "";
boolean stringComplete = false;
int pos1 = 90;
int pos2 = 90;
int pos3 = 90;

const int LEFT = A0; // analog
const int RIGHT = A1;
//const int ROTATE = A2;
const int GRIP = A2;

void setup()
{
  // replace with serial read
  pinMode(LEFT, INPUT);
  pinMode(RIGHT, INPUT);
//  pinMode(ROTATE, INPUT);
  pinMode(GRIP, INPUT);
  inputString.reserve(20);

  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  mySerial.begin(115200);
//  mySerial.println("Hello, world?");

  gripperServo.attach(5);
  leftServo.attach(6);
  rotateServo.attach(8);
  rightServo.attach(9);
}

void loop()
{
//  char c = mySerial.read();
        if (Serial.available() > 0) {
                // read the incoming byte:
                incomingByte = Serial.read();

                // say what you got:
                Serial.print("I received: ");
                Serial.println(incomingByte, DEC);
        }
//  Serial.print(c);
  int left, right, rotate;
  bool grip = false;

  left = analogRead(LEFT);
  right = analogRead(RIGHT);
//  rotate = analogRead(ROTATE);
  grip = digitalRead(GRIP);

  // close claw on button press
  if( grip | incomingByte == 'g' ){ // close the claw
     grip = 0;
  } else {  // open the claw
    grip = 0;
  }

  // position the left servo
  if( left > 900 || incomingByte == 'f' ) {
    pos1 = pos1 + 10;
  } else if ( right < 200 ) {
    pos1 = pos1 - 10;
  }
  if( pos1 >= 140 ) {
    pos1 = 140;
  }
  if( pos1 <= 60 ) {
    pos1 = 60;
  }
  // position the right servo
  if ( right > 900 || incomingByte == 'b' ) {
    pos2 = pos2 + 10;
  } else if ( right < 200 ) {
    pos2 = pos2 - 10;
  }
  if ( pos2 >= 90 ) {
    pos2 = 90;
  }
  if ( pos2 <= 35 ) {
    pos2 = 35;
  }
  pos3 = 0;

  // write the values out
  leftServo.write(pos1);
  rightServo.write(pos2);
  rotateServo.write(pos3);
  gripperServo.write(grip);


  delay(100);
}
