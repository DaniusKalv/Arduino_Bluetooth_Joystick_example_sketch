#include <Servo.h> 

//Hardware pin defines
#define A 2   //Pin A of motor driver
#define B 3     //Pin B of motor driver
#define EN 11
#define SERVO 10

Servo myservo; 

int counter = 0; //Counter used for counting the time since the last received packet

void setup() {

  Serial.begin(9600); //Begin serial for interfacing with HC-05
  setPwmFrequency(EN, 1); //Set PWM frequency of the EN pin on the L293D
  motorInitialize(); //Initialize the motor
//  pinMode(MEASUREMENT, OUTPUT);
//  digitalWrite(MEASUREMENT, 0);
  myservo.attach(SERVO); 
  myservo.write(30); //Put servo to the middle position
  delay(1000); //ČWait for the servo to go to the middle
}

void loop() {

  if(Serial.available()){ //Check if serial is available
    while(Serial.available()) handleSerial(Serial.read()); //If there is data to be read, pass the first character to the handleSerial function 
    counter = 0; //Reset the counter
  }
  else{ 
    if(counter > 20) motorStop(); //If there no data was received in the last 500ms stop the motor, so that the car doesn't drive away when the connection is lost
    counter++; //Increment the counter
  }  
  delay(25);
}

void handleSerial(char c){
  switch(c){ //According to the first character a proper function is executed
      case 'S':{ //'S' indicates that the speed data is presented next
        int y = Serial.parseInt() - 250; //Read the speed (the received speed is between 0 and 500, 250 being the middle point, 0 representing full reverse and 500 representing full forward)
        if(Serial.read() == 13){ //Check if the packet is complete, each packet ends with "\r\n" (13 and 10 in ANSII)
          if(Serial.read() == 10){
            driveMotor(y); //If the packet is valid, drive motor
          }
          else while(Serial.available()) Serial.read(); //Clear the Serial buffer if the packet is invalid
        }
        else while(Serial.available()) Serial.read(); //Clear the Serial buffer if the packet is invalid
        break;
      }
      case 'D':{ //'D' indicates that the direction data is presented next
        int x = Serial.parseInt(); //Read the direction (the received direction value is between 0 and 500)
        if(Serial.read() == 13){ //Check if the packet is complete, each packet ends with "\r\n" (13 and 10 in ANSII)
          if(Serial.read() == 10){
            driveServo(x); //If the packet is valid, drive servo
          }
          else while(Serial.available()) Serial.read(); //Clear the Serial buffer if the packet is invalid
        }
        else while(Serial.available()) Serial.read(); //Clear the Serial buffer if the packet is invalid
        break;
      }
  }
}  
  

void driveServo(int x){
  x = map(x, 0, 500, 0, 60); //the received data according to your servo parameters
  myservo.write(x); //Drive servo
}

void driveMotor(int y){
  if(y > 0) motorForward(y);
  else if(y < 0) motorBackward((-1) * y);
  else motorStop();  
}

void motorInitialize(){  
  pinMode (A, OUTPUT); //Initialise pins for driving L293D IC
  pinMode (B, OUTPUT);
  digitalWrite(A, 0);
  digitalWrite(B, 0);
  analogWrite(EN, 0);  
}

void motorForward(int velocity){
  if (velocity < 0) velocity = 0;
  else if (velocity > 255) velocity = 255;
  analogWrite(EN, velocity); //Set speed
  digitalWrite(A, 1); //Set direction
  digitalWrite(B, 0);  
}

void motorBackward(int velocity){
  if (velocity < 0) velocity = 0;
  else if (velocity > 255) velocity = 255;
  analogWrite(EN, velocity); //Set speed
  digitalWrite(A, 0); /Set direction
  digitalWrite(B, 1);    
}

void motorStop(){
  analogWrite(EN, 255); //Stop the motor by shorting its both connections to GND
  digitalWrite(A, 1);
  digitalWrite(B, 1);   
}

void motorOff(){
  analogWrite(EN, 0); //Shut down the L293D
  digitalWrite(A, 0);
  digitalWrite(B, 0); 
}

void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

//int batteryPercentage(){
//
//  float current, finalVoltage;
//  digitalWrite(MEASUREMENT, 1);
//  delay(1);
//  current = ((float(analogRead(BATTERY)) / 1023) * 5) / float(R2);
//  digitalWrite(MEASUREMENT, 0);
//  finalVoltage = current * float(R1 + R2);
//  int percent = ((finalVoltage - 10.8) / 3.3) * 100;
//  if(percent > 100) percent = 100;
//  else if(percent < 0) percent = 0;
//  return percent;
//}
//
//float batteryVoltage(){
//
//  float current, finalVoltage;
//  digitalWrite(MEASUREMENT, 1);
//  delay(1);
//  current = ((float(analogRead(BATTERY)) / 1023) * 5) / float(R2);
//  digitalWrite(MEASUREMENT, 0);
//  finalVoltage = current * float(R1 + R2);
//  return finalVoltage;
//}


