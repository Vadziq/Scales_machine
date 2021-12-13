// Slave 

#include <Wire.h>
#include <HX711.h>
#include <EEPROM.h>
//SSSS    #include <FlexyStepper.h>
#include <Servo.h>
//#include <Servo_Hardware_PWM.h>
Servo myServo1;


int slaveAddress        = 14;
int calibrationFactor   = 2000;//
float readyLoad         = 0.0F;
float readyLoad2        = 125.0F;
float maxLoad           = 1000.0F;
int stabilisationDelay  = 200;
byte beltHighSpeed      = 255;
byte beltLowSpeed       = 255;
byte scaleReadingsNumber = 1;
unsigned int gateSpeed        = 50000;
unsigned int gateAcceleration = 40000;
unsigned int gateStayOpen     = 200;
int gateZero            = 9;
int gateOpen            = 55;

boolean startFlag = LOW;
/* Calibration factors
 * 1 - 1380 #14
 * 2 - 1330`#15
 * 3 - 1410 #15
 * 4 - 1360 #6
 * 5 - 1350 #6
 * 6 - 1370 #9
 * 7 - 1350 
 * 8 - 1390
 * 9 - 1385 #14
 * 10 - 1400 #12
 * 11 - 1355 #10
 * 12 - 1350 #10
 * 13 - 1350 #14
 * 14 - 1480 #9!!!
 * 15 - 1380 !!!
 * 16 - 1320
 * 
 */
int counter = 0;
float load = 0.0F;
char loadBuff[7];
const int ledPin            =13;
const int beltPin = 9;
const int SERVO_PIN = 5;
const int MOTOR_DIRECTION_PIN = 5;
const int solenoidPin = 10;
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;
const int DROP_PIN          = 7;
const int IND_PIN           = 8;

boolean slowFlag = LOW;
boolean readyFlag = LOW;
boolean stableFlag = LOW;
boolean dischargeFlag = LOW;
int timeBetweenReadings = 100;
long timer = 0;
long setStepper = 550;
long setStepperSetup = 600;
int beltSpeed = 0;
int servoPosition = 0;

HX711 scale;
//FlexyStepper stepper;
//Servo gateFlap;

void setup() {
  //noInterrupts();

 
  
  Serial.begin(115200);
  pinMode(ledPin,OUTPUT);
  pinMode(beltPin,OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(DROP_PIN, INPUT);
  pinMode(IND_PIN, OUTPUT);

  //delay(5000);
  calibrationFactorsFunction();

  
  
  pinMode(solenoidPin,OUTPUT);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  Serial.print("Slave address: \t\t\t");  Serial.println(slaveAddress);
  Serial.print("Calibration factor: \t\t");Serial.println(calibrationFactor);
  //readyLoad = EEPROM.read(0)+300;//1
  Serial.print("Set point: \t\t\t");   //Serial.println(readyLoad);
  
  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t\t");
  Serial.println(scale.read());      // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));   // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
            // by the SCALE parameter (not set yet)

  scale.set_scale(calibrationFactor);//(2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t\t");
  Serial.println(scale.read());                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));   // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);        // print the average of 5 readings from the ADC minus tare weight, divided
            // by the SCALE parameter set with set_scale

  Serial.println("Readings:");

    //interrupts();
   //STEPPER
  Serial.println("Servo start");
  //SSSS    stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
//  stepper.setSpeedInStepsPerSecond(1000);
//  stepper.setAccelerationInStepsPerSecondPerSecond(100);
//  stepper.setCurrentPositionInSteps(0);
//  stepper.setTargetPositionInSteps(setStepperPosition);
//  while(!stepper.motionComplete())
//  {
//    Serial.print(".");
//    stepper.processMovement();
//  }

//        stepper.setCurrentPositionInSteps(0);
//        stepper.setTargetPositionInSteps(setStepper);
//        stepper.setSpeedInStepsPerSecond(gateSpeed);
//        stepper.setAccelerationInStepsPerSecondPerSecond(gateAcceleration);
//        while(!stepper.motionComplete()){
//          stepper.processMovement();
//          if(setStepper == stepper.getCurrentPositionInSteps()){
//            delay(gateStayOpen);
//            Serial.print("Stepper position: ");
//            Serial.println(setStepper);
//            stepper.setTargetPositionInSteps(0); 
//            }
//        }
  myServo1.attach(SERVO_PIN);
  myServo1.write(gateZero);
  delay(100);
  myServo1.write(gateOpen);
  delay(500);  
  myServo1.write(gateZero);
  delay(500);
  myServo1.detach();
// analogWrite(SERVO_PIN,0);
// delay(1000);
// analogWrite(SERVO_PIN,255);
// delay(1000); 
// analogWrite(SERVO_PIN,0);
//  delay(500);
  
 // for(int i = 0; i <= 20; i++){
//  gateFlap.write(-5);
//  delay(1000);
//  gateFlap.write(45);
//  delay(1000);
//  gateFlap.write(-5);
//  delay(1000);
  //}
//  analogWrite(5,0);
//  delay(1000);
//  analogWrite(5,25);
//  myServo1.write(45);
//  delay(1000);
//  myServo1.write(0);
  
  readyLoad = EEPROM.read(0);+300;//2
  if(readyLoad > 150){
    readyLoad = readyLoad + 300;
    }
  millisDelay(stabilisationDelay);
  scale.tare();
  Wire.begin(slaveAddress); // any 7-bit Slave address
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent); //Interrupt for handling incomming requests
  Serial.println("Wire begin...");
 
}

void loop() {
  if(millis() > timer + timeBetweenReadings){
    load = scale.get_units(scaleReadingsNumber);  
    if(startFlag){
      if(!readyFlag){
        Serial.print(" ! ");
        digitalWrite(ledPin,LOW);
        digitalWrite(IND_PIN,LOW);
        load = scale.get_units(scaleReadingsNumber);
        if(load < readyLoad && !dischargeFlag){
          if(!slowFlag){
            beltMotor(beltHighSpeed);
            Serial.println("HIGH MOTOR");
          }
          if(slowFlag){
            beltMotor(beltHighSpeed);
            millisDelay(100);
            beltMotor(0);
            millisDelay(800);
            Serial.println("SLOW MOTOR");
          }
          Serial.print(" ");
          Serial.print(load);
          if(load > maxLoad){
            Serial.println("BUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUGBUG");
            Serial.print(load);
            Serial.println(" overload");
            //readyLoad = readyLoad2;
          }
        }

        if(load >= readyLoad-40 && !dischargeFlag && load < maxLoad && !slowFlag){
          
          beltMotor(0);
          
          //analogWrite(beltPin, 0);
          Serial.println("");
          Serial.println("Stop loading, stabilisation");
          millisDelay(stabilisationDelay);
                  
         // delay(stabilisationDelay);
          Serial.println("Stable");
          load = scale.get_units(scaleReadingsNumber);
          Serial.println(load);
          slowFlag = HIGH;
          if(load >= readyLoad){
            Serial.println("READY");
            readyFlag = HIGH;
//          dischargeFlag = HIGH;
          }
        }
        if(load >= readyLoad && !dischargeFlag && load < maxLoad && slowFlag){
          beltMotor(0);
          
          //analogWrite(beltPin, 0);
          Serial.println("Slow");
          Serial.println("Stop loading, stabilisation");
          millisDelay(stabilisationDelay);
          //delay(stabilisationDelay);
          Serial.println("Stable");
          load = scale.get_units(scaleReadingsNumber);
          Serial.println(load);
          slowFlag = HIGH;
          if(load >= readyLoad){
            Serial.println("READY");
            readyFlag = HIGH;          
          }
        }
      }
      
      if(readyFlag && !dischargeFlag){
        //Serial.print("DischargeFlag: ");
        //Serial.println(dischargeFlag);
        //Serial.print("ReadyFlag: ");
        //Serial.println(readyFlag);
        digitalWrite(ledPin,HIGH);
        digitalWrite(IND_PIN,HIGH);
        load = scale.get_units(scaleReadingsNumber);
        //Serial.print("Ready weight: ");
        //Serial.println(load);
      }
      if(dischargeFlag){
        //Wire.end();
        //delay(65);
        //Serial.println("Discharging...");

        while(!digitalRead(DROP_PIN)){
          
        }
        readyFlag = LOW;
        slowFlag = LOW;
        digitalWrite(IND_PIN,HIGH);
     
//        stepper.setCurrentPositionInSteps(0);
//        stepper.setTargetPositionInSteps(setStepper);
//        stepper.setSpeedInStepsPerSecond(gateSpeed);
//        stepper.setAccelerationInStepsPerSecondPerSecond(gateAcceleration);
        
//        while(!stepper.motionComplete()){
//          stepper.processMovement();
//        }
//        Serial.print("Servo position: "); Serial.println(myServo1.read());
//        //if(myServo1.read() != 50){
//          Serial.println("Servo have to run now");
//          myServo1.write(50);
       // }

         myServo1.attach(SERVO_PIN);

          myServo1.write(gateOpen);
          delay(1000);  
          myServo1.write(gateZero);
          delay(500);
          myServo1.detach();
        delay(100);
        while(digitalRead(DROP_PIN)){
          
        }
        digitalWrite(IND_PIN,LOW);
    
        delay(100);
          

        dischargeFlag = LOW;
        load = 0;
        //beltSpeed = 0;
        delay(stabilisationDelay); //added for scale taring
        //Serial.println("Stable");
        beltMotor(beltHighSpeed);
       // readyLoad = EEPROM.read(0)+300;//3
        scale.tare(); //added for scale taring
        //Serial.println("Discharge finished");
        //Wire.begin(slaveAddress);
        //millisDelay(100);
      }
    }
    if(!startFlag){
      digitalWrite(ledPin, LOW);
      digitalWrite(IND_PIN,LOW);
      Serial.println(digitalRead(DROP_PIN));
      beltMotor(0);
      //beltSpeed = 0;
     // analogWrite(beltPin, beltSpeed);
      digitalWrite(solenoidPin, LOW);
      //delay(500);
    }
   // 
(50);
    timer = millis();
  }
}

void calibrationFactorsFunction(){
  switch(slaveAddress){
    case 1:
    calibrationFactor = 1385;
    gateZero          = 14;
    break;
    case 2:
    calibrationFactor = 1350;
    gateZero          = 15;
    break;
    case 3:
    calibrationFactor = 1395;
    gateZero          = 15;
    break;
    case 4:
    calibrationFactor = 1375;
    gateZero          = 6;
    break;
    case 5:
    calibrationFactor = 1350;
    gateZero          = 6;
    break;
    case 6:
    calibrationFactor = 1500;
    gateZero      = 9;
    break;
    case 7:
    calibrationFactor = 1350;
    break;
    case 8:
    calibrationFactor = 1390;
    break;
    case 9:
    calibrationFactor = 1350;
    gateZero          = 14;
    break;
    case 10:
    calibrationFactor = 1395;
    gateZero      = 12;
    break;
    case 11:
    calibrationFactor = 1400;
    gateZero      = 10;
    break;
    case 12:
    calibrationFactor = 1350;
    gateZero      = 14;
    break;
    case 13:
    calibrationFactor = 1350;
    gateZero      = 14;
    break;
    case 14:
    calibrationFactor = 1370;
    gateZero      = 9;
    break;
    case 15:
    calibrationFactor = 1350;
    gateZero      = 14;
    break;
    case 16:
    calibrationFactor = 1320;
    break;
  }
}

void millisDelay(int inDelay){
  Serial.print("Delay: \t\t"); Serial.print(inDelay);          
  float timer = 0;
  timer = millis();
  while(millis() < timer + inDelay){

  }
  Serial.println(" OK");     
}  

void beltMotor(int speedSet){
  Serial.print(" M");
  Serial.print(speedSet);
  analogWrite(beltPin, speedSet);
//  if(beltSpeed < speedSet){
//    for(beltSpeed; beltSpeed <= speedSet; beltSpeed++){
//      analogWrite(beltPin, speedSet);
//      delay(1);
//    }
//  }
//  if(beltSpeed > speedSet){
//    for(beltSpeed; beltSpeed >= speedSet; beltSpeed--){
//      //Serial.println(beltSpeed);
//      analogWrite(beltPin, speedSet);
//      //delay(1);      
//    }
//  }
}

void requestEvent() {
  if(readyFlag && !dischargeFlag){
    Serial.println("Sending weight....");
    dtostrf(load,7,2, loadBuff);
    Serial.println(loadBuff);
    Wire.write(loadBuff);
  }
  if(!startFlag){
    Serial.print("Sending weight.... ");
    dtostrf(load,7,2, loadBuff);
    Serial.println(loadBuff);
    Wire.write(loadBuff);
  }

}

void receiveEvent(int howMany) {
    //Serial.print("Command arrived: ");
    //beltMotor(0);
//    byte c = Wire.read(); // receive byte as a character
//    Serial.println(c);         // print the character
    int rule = 0;
    String dataString = "";
//    byte c = 3;
    
  while (Wire.available()) { // loop through all but the last
    byte c = Wire.read(); // receive byte as a character
    dataString = dataString + c;
  } 
    rule = dataString.toInt();
    Serial.println(rule);
  if (rule == 1){
    dischargeFlag = HIGH;
    Serial.println("Discharge");
  }
  if (rule == 2){
    Serial.print(counter);
    Serial.print(" on");
    counter++;
    startFlag = HIGH;
  }
  if (rule >= 3){
    Serial.println("off");
    startFlag = LOW;
    EEPROM.update(0,rule);
  }  
 }
