#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include <NewPing.h>
#include <Servo.h>

//L298n
#define ENA   12          // Enable/speed motors Right        GPIO12(D6)
#define ENB   5          // Enable/speed motors Left         GPIO5(D1)
#define IN_1  14          // L298N in1 motors Rightx          GPIO14(D5)
#define IN_2  2          // L298N in2 motors Right           GPIO12(D4)
#define IN_3  0           // L298N in3 motors Left            GPIO0(D3)
#define IN_4  4           // L298N in4 motors Left            GPIO4(D2)

//Ultrasonic Sensor
#define echoPin 15      //                                    GPIO15(D8)
#define trigPin 13      //                                    GPIO13(D7)

//SSID & PW WiFi
#ifndef APSSID
#define APSSID "ESParm"
#define APPSK  "qwerty123"
#endif

String command;             //String to store app command state.
int speedCar = 800;         // 400 - 1023.
int speed_Coeff = 3;
unsigned int distance, distance2, distance3, distance4;
char choice;
int Mode, pos, tick = 0;
bool stopped, turn_left = false;

const char *ssid = APSSID;
const char *password = APPSK;

Servo servo_Gripper, servo_Right;
NewPing sonar(trigPin, echoPin, 200);
ESP8266WebServer server(80);

void setup() {
 Serial.begin(115200);
 
 pinMode(ENA, OUTPUT);
 pinMode(ENB, OUTPUT);  
 pinMode(IN_1, OUTPUT);
 pinMode(IN_2, OUTPUT);
 pinMode(IN_3, OUTPUT);
 pinMode(IN_4, OUTPUT); 
 //sensor
 pinMode (echoPin, INPUT);
 pinMode (trigPin, OUTPUT);
 //servos
 servo_Gripper.attach(1);   //                      GPIO1(TX)
 servo_Right.attach(3);     //                      GPIO3(RX)
 
 servo_Gripper.write(0);
 servo_Right.write(90);
 
// Connecting WiFi
  WiFi.softAP(ssid, password);
  
  IPAddress myIP = WiFi.softAPIP();
 // Starting WEB-server 
 server.on ( "/", HTTP_handleRoot );
 server.onNotFound ( HTTP_handleRoot );
 
 Serial.print("Connected toIP address: ");
 Serial.println(myIP);
 Serial.println("HTTP started");
 server.begin();   
}

void goAhead(){ 
      digitalWrite(IN_1, LOW);
      digitalWrite(IN_2, HIGH);
      analogWrite(ENA, speedCar);

      digitalWrite(IN_3, LOW);
      digitalWrite(IN_4, HIGH);
      analogWrite(ENB, speedCar);
  }
void goBack(){ 

      digitalWrite(IN_1, HIGH);
      digitalWrite(IN_2, LOW);
      analogWrite(ENA, speedCar);

      digitalWrite(IN_3, HIGH);
      digitalWrite(IN_4, LOW);
      analogWrite(ENB, speedCar);
  }
//
void goRight(){ 
      digitalWrite(IN_1, LOW);
      digitalWrite(IN_2, HIGH);
      analogWrite(ENA, speedCar);

      digitalWrite(IN_3, HIGH);
      digitalWrite(IN_4, LOW);
      analogWrite(ENB, speedCar);
  }

void goLeft(){
      digitalWrite(IN_1, HIGH);
      digitalWrite(IN_2, LOW);
      analogWrite(ENA, speedCar);

      digitalWrite(IN_3, LOW);
      digitalWrite(IN_4, HIGH);
      analogWrite(ENB, speedCar);
}
void stopRobot(){  
      digitalWrite(IN_1, LOW);
      digitalWrite(IN_2, LOW);
      analogWrite(ENA, speedCar);

      digitalWrite(IN_3, LOW);
      digitalWrite(IN_4, LOW);
      analogWrite(ENB, speedCar);
}
void elevate(int duration){
        servo_Right.write(0); 
        delay(duration);
        servo_Right.write(90);
}
void drop(int duration){
        servo_Right.write(180); 
        delay(duration);
        servo_Right.write(90);
} 
void choose_pattern(){
  distance = sonar.ping_cm();
  if(distance >= 9  && distance <= 15){
    choice ='A';   
  }
  else{
    choice = 'B';
  }
  
}
void caseA(){
  servo_Gripper.write(40);
          delay(1000);
          drop(300);
          delay(1000);
          servo_Gripper.write(0);
          delay(1000);
          elevate(500);
          delay(1000);
          goRight();
          delay(600);
          stopRobot();
          delay(1000);
          
          distance2 = sonar.ping_cm();
          if(distance2 >= 0 && distance2 <= 10){
            goLeft();
            delay(1000);
            stopRobot();
            delay(500);
            
            distance3 = sonar.ping_cm();  
            if(distance3 >= 0 && distance3 <= 10){
              goRight();
              delay(500);
              stopRobot();
              delay(1000);
              
              goBack();
              delay(2000);
              stopRobot();
              delay(500);
  
              goRight();
              delay(500);
              stopRobot();
              delay(1000);
            }
          }
          
          drop(300);
          delay(1000);
          servo_Gripper.write(40);
          delay(1000);
          elevate(500);
          delay(1000);
          servo_Gripper.write(0);
          delay(1000);
          
          if(distance2 >= 0 && distance2 <= 10){
            goRight();
            delay(500);
            stopRobot();
            
          }else if (distance3 >= 0 && distance3 <= 10){
            goLeft();
            delay(500);
            stopRobot();  
          }
          else{
            goLeft();
            delay(500);
            stopRobot();  
          }
 }
void auto_mode(){
  if(Mode == 2){
    choose_pattern();
    
     switch(choice){
       case 'A':
          Serial.println("case A");
          caseA();
        break;
        
       case 'B':
          Serial.println("case B");
          goAhead();
          delay(1000);
          stopRobot();
          delay(1000);
          distance4 = sonar.ping_cm();
          if(distance4 >= 9 && distance4 <= 15){
            stopRobot();  
            caseA();
          }
          else{
            goLeft();
            delay(1000);  
          }
      }  
   }
}

void loop() {
    ArduinoOTA.handle();
    server.handleClient();
    
      command = server.arg("State");
      if (command == "F") goAhead();
      else if (command == "B") goBack();
      else if (command == "L") goLeft();
      else if (command == "R") goRight();
      
      else if (command == "FR") drop(10);
      else if (command == "BR") elevate(100);

      else if (command == "M") {Mode = 2; auto_mode();} //switch arm to Auto-mode
      else if (command == "S") {Mode =1; stopRobot();}
      
      else if (command == "0") {pos = 0; speedCar = 400;}
      else if (command == "1") {pos = 10; speedCar = 470;}
      else if (command == "2") {pos = 20; speedCar = 540;}
      else if (command == "3") {pos = 30; speedCar = 610;}
      else if (command == "4") {pos = 40; speedCar = 680;}
      else if (command == "5") {pos = 50; speedCar = 750;}
      else if (command == "6") {pos = 60; speedCar = 820;}
      else if (command == "7") {pos = 70; speedCar = 890;}
      else if (command == "8") {pos = 80; speedCar = 960;}
      else if (command == "9") {pos = 90; speedCar = 1023;}
      Serial.println(command);
      servo_Gripper.write(pos);
      delay(100);
      Serial.println(command);
}

void HTTP_handleRoot(void) {

if( server.hasArg("State") ){
       Serial.println(server.arg("State"));
  }
  server.send ( 200, "text/html", "" );
  delay(1);
}
