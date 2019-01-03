/**
    Arduino_Bluetooth_Joystick_example_sketch.ino
    Description: Example sketch for working with Arduino Bluetooth Joystick app
    @author Danius Kalvaitis <danius.kalvaitis@gmail.com>
*/

#include <Servo.h> 

//Hardware pin definitions
#define A     2   //A pin of motor driver (L293D)
#define B     3   //B pin of motor driver (L293D)
#define EN    11  //EN pin of motor driver (L293D)
#define SERVO 10  //Servo control pin 

//Default parameter definitions
#define SERVO_LEFT_POSITION   0  //Value representing the left position of the servo (may require some tweeking)
#define SERVO_MID_POSITION    30 //Value representing the mid position of the servo (may require some tweeking)
#define SERVO_RIGHT_POSITION  60 //Value representing the right position of the servo (may require some tweeking)

Servo myservo; //Create a servo object 

int timeout_counter = 0; //Counter used for counting the time since the last received packet

//setup
void setup() {
  Serial.begin(9600);     //Begin serial for interfacing with HC-05
  setPwmFrequency(EN, 1); //Set PWM frequency of the EN pin to 32768Hz (above the frequency a human can hear, otherwise the motor is generating some noise). This function can be commented out or removed if this feature is not desired
  motorInitialize();      //Initialize the motor
  myservo.attach(SERVO);  //Attach a servo object
  myservo.write(30);      //Put servo to the middle position
  delay(1000);            //Wait for the servo to go to the middle
}

//main loop
void loop() {
  if(Serial.available()){ //Check if there is serial data ready to be read
    while(Serial.available()) handleSerial(Serial.read()); //If there is data ready to be read, pass the first character to the handleSerial function 
    timeout_counter = 0; //Reset the counter when a packet is received
  }
  else{  
    if(timeout_counter > 20){ //If no data was received in the last 500ms stop the motor, so that the car doesn't drive away when the connection is lost
      motorStop();
    }
    else{
      timeout_counter++; //Increment the counter
    }
  }  
  delay(25); //Wait for 25ms
}

void handleSerial(char c){
  switch(c){  //The first received character defines the packet type (speed, direction)
      case 'S':{  //'S' indicates that the speed packet was received
        int y = Serial.parseInt() - 250; //Parse the speed (the received speed is between 0 and 500, 250 being the middle point, 0 representing full reverse and 500 representing full forward)
        if(Serial.read() == 13){  //Check if the packet is complete, each packet ends with "\r\n" (13 and 10 in ANSII)
          if(Serial.read() == 10){
            motorDrive(y); //If the packet is valid, drive the motor
          }
          else while(Serial.available()) Serial.read(); //Clear the Serial buffer if the packet is invalid
        }
        else while(Serial.available()) Serial.read(); //Clear the Serial buffer if the packet is invalid
        break;
      }
      case 'D':{  //'D' indicates that the direction packet was received
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
  x = map(x, 0, 500, 0, 60); //Map the received data according to your servo parameters
  myservo.write(x); //Drive servo
}

void motorDrive(int y){
  if(y > 0) motorForward(y); //Positive value represents forward movement
  else if(y < 0) motorBackward((-1) * y); //Negative value represents backward movement
  else motorStop(); //If y equals 0 (Joystick is in the mid position), stop the motor  
}

void motorInitialize(){
  //Initialise pins for driving L293D IC  
  pinMode (A, OUTPUT);
  pinMode (B, OUTPUT);
  digitalWrite(A, 0);
  digitalWrite(B, 0);
  analogWrite(EN, 0);  
}

void motorForward(int velocity){
  //Check if the received velocity value is in range
  if (velocity < 0) velocity = 0;
  else if (velocity > 255) velocity = 255;

  analogWrite(EN, velocity); //Set speed
  digitalWrite(A, 1); //Set direction
  digitalWrite(B, 0);  
}

void motorBackward(int velocity){
  //Check if the received velocity value is in range
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

//This function does some register magic to set PWM to a custom frequency
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
