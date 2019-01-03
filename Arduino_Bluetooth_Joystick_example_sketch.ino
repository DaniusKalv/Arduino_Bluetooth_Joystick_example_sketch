/**
    Arduino_Bluetooth_Joystick_example_sketch.ino
    Description: Example sketch for working with Arduino Bluetooth Joystick app
    @author Danius Kalvaitis <danius.kalvaitis@gmail.com>
*/

#include <Servo.h> 

//Hardware pin definitions
#define L293D_A       2   //A pin of motor driver (L293D)
#define L293D_B       3   //B pin of motor driver (L293D)
#define L293D_EN      11  //EN pin of motor driver (L293D)
#define L293D_EN_MAX  255 //Maximum PWM value on L293D EN pin. It can be decreased if a maximum spped limit is desired.
#define SERVO         10  //Servo control pin 

//Default parameter definitions
#define SERVO_LEFT_POSITION   0     //Value representing the left position of the servo (may require some tweeking)
#define SERVO_MIDDLE_POSITION 30    //Value representing the mid position of the servo (may require some tweeking)
#define SERVO_RIGHT_POSITION  60    //Value representing the right position of the servo (may require some tweeking)
#define RANGE_FROM            0     //Set to the value selected in app settings (Settings->Joystick Value Range) 
#define RANGE_TO              1024  //Set to the value selected in app settings (Settings->Joystick Value Range)

Servo myservo; //Create a servo object 

int timeout_counter = 0; //Counter used for counting the time since the last received packet

//setup
void setup() {
  Serial.begin(9600);     //Begin serial for interfacing with HC-05
  setPwmFrequency(L293D_EN, 1); //Set PWM frequency of the EN pin to 32768Hz (above the frequency a human can hear, otherwise the motor is generating some noise). This function can be commented out or removed if this feature is not desired
  motorInitialize();      //Initialize the motor
  myservo.attach(SERVO);  //Attach a servo object
  myservo.write(SERVO_MIDDLE_POSITION); //Put servo to the middle position
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
        int y = Serial.parseInt(); //Parse the speed
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
            servoDrive(x); //If the packet is valid, drive servo
          }
          else while(Serial.available()) Serial.read(); //Clear the Serial buffer if the packet is invalid
        }
        else while(Serial.available()) Serial.read(); //Clear the Serial buffer if the packet is invalid
        break;
      }
  }
}  
  
void servoDrive(int x){
  x = map(x, RANGE_FROM, RANGE_TO, SERVO_LEFT_POSITION, SERVO_RIGHT_POSITION); //Map the received data according to your servo parameters
  myservo.write(x); //Drive servo
}

//Initialise pins for driving L293D IC  
void motorInitialize(){
  pinMode (L293D_A, OUTPUT);
  pinMode (L293D_B, OUTPUT);
  digitalWrite(L293D_A, 0);
  digitalWrite(L293D_B, 0);
  analogWrite(L293D_EN, 0);  
}

void motorDrive(int y){
  y = y - RANGE_TO/2; //Subtract the mid point value to determine if we are going forward or backward

  if(y > 0){ //Positive value represents forward movement 
    y = map(y, RANGE_FROM, RANGE_TO, 0, L293D_EN_MAX); //Map the speed value to L293D_EN_MAX limit
    motorForward(y);    
  } 
  else if(y < 0){ //Negative value represents backward movement
    y = (-1) * y; //We know that we are going backward, let's make y positive again
    y = map(y, RANGE_FROM, RANGE_TO, 0, L293D_EN_MAX); //Map the speed value to L293D_EN_MAX limit
    motorBackward(y);
  } 
  else motorStop(); //If y equals 0 (Joystick is in the mid position), stop the motor  
}

void motorForward(int velocity){
  //Check if the received velocity value is in range
  if (velocity < 0) velocity = 0;
  else if (velocity > 255) velocity = 255;

  analogWrite(L293D_EN, velocity); //Set speed
  digitalWrite(L293D_A, 1); //Set direction
  digitalWrite(L293D_B, 0);  
}

void motorBackward(int velocity){
  //Check if the received velocity value is in range
  if (velocity < 0) velocity = 0;
  else if (velocity > 255) velocity = 255;

  analogWrite(L293D_EN, velocity); //Set speed
  digitalWrite(L293D_A, 0); //Set direction
  digitalWrite(L293D_B, 1);    
}

void motorStop(){
  analogWrite(L293D_EN, 255); //Stop the motor by shorting its both connections to GND
  digitalWrite(L293D_A, 1);
  digitalWrite(L293D_B, 1);   
}

void motorOff(){
  analogWrite(L293D_EN, 0); //Shut down the L293D
  digitalWrite(L293D_A, 0);
  digitalWrite(L293D_B, 0); 
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
