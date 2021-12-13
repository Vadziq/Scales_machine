// Master

#include <TM1638.h>
#include <EEPROM.h>
#include <Wire.h>
#include <avr/wdt.h>
#include<SoftwareSerial.h>

#define SLAVES 8

const int screenPin3  = 2;
const int screenPin2  = 3;
const int screenPin1  = 4;
const int softRX      = 5;
const int softTX      = 6;
const int proxPin     = 7;
const int yellowPin   = 8;
const int setPin      = 9;
const int startPin    = 10;
const int dropPin     = 11;
const int packStart   = 12;
const int mainBelt    = 13;



boolean oneGate     = HIGH;
const boolean twoGates    = LOW;
const boolean threeGates  = LOW;
boolean fourGates   = LOW;
const boolean fiveGates   = LOW;
const boolean sixGates    = LOW;

float targetWeight        = 460; //122 для 125гр 
float maxResult           = 600;
byte basketsSet           = 160; //123 для 125гр
int packMoveTime          = 200; 
int delayBetweenReadings  = 20;
int delayBetweenDischarges = 20; //20 для 125гр
int delayForDrop          = 1400;
int delayAfterDischarge   = 0;

int loopCounter           = 0;
int machinePart           = 2;
boolean startFlag         = LOW;
boolean firstReady        = LOW;
boolean secondReady       = LOW;
boolean alternateSet         = LOW;
boolean noTareLoader      = LOW;
boolean cells[SLAVES];
float scales[SLAVES];
float results[134];
float cycleTime = 0.0F;
float oldTime = 0.0F;

SoftwareSerial mySerial(softRX, softTX);
TM1638 module(screenPin1, screenPin2, screenPin3);

void setup(){


  
  delay(1000);
  Serial.begin(115200);
  mySerial.begin(9600);
  mySerial.print("baud=38400");
  mySerial.write(0xff);
  mySerial.write(0xff);
  mySerial.write(0xff);
  mySerial.begin(38400);
  pinMode(yellowPin, INPUT);
  pinMode(setPin,    INPUT);
  pinMode(startPin,  INPUT);
  pinMode(proxPin,   INPUT);
  pinMode(mainBelt,  OUTPUT);
  pinMode(packStart, OUTPUT);
  pinMode(dropPin,    OUTPUT);
  module.setupDisplay(true,3);
  module.setDisplayToString("CELL");
  Serial.println("Multi cell main controller");
  module.setDisplayToString("    ");
  Wire.begin();
  Wire.setClock(100000);
  wdt_enable(WDTO_8S);  
  
}

void loop(){
  wdt_reset();
  buttons();
  loopCounter++;
  Serial.println(loopCounter);
  //delay(50);//speed
  if(machinePart == 1){
    machinePart = 2;
  }
  else{
    machinePart = 1;
  }

  if (startFlag){
      module.setDisplayToString("O__O");
     if(firstReady){ // && !secondReady){
      Serial.println("FIRST READY");
      machinePart = 2;
      module.setDisplayToString("___O");
    }
    if(secondReady){ //&& !firstReady){
      Serial.println("SECOND READY");
      machinePart = 1;
      module.setDisplayToString("O___");
    }
    
    
 //   digitalWrite(mainBelt, HIGH);
    
   // delay(1000);speed
    float timer = 0;
    timer = micros();
     
//    Serial.println();
//    Serial.println("clearing cell states...");
    
    for(int a = 0; a <= SLAVES - 1; a++){
      cells[a] = LOW;
    }


      
    interrogation(machinePart);
    recombination();
    discharge(machinePart);
    if(firstReady && secondReady){
      //delay(300);
      pack();
    }
   // pack();//for prox check
    timer = (micros() - timer)/1000000;
    Serial.println("");
    Serial.print(timer); Serial.println(" seconds");
    
   
    //delay(1000);//speed
  }
  else{
    module.setDisplayToString(" OFF");
    //digitalWrite(mainBelt, LOW);
    slavesOff(machinePart, basketsSet);
    interrogation(machinePart);
    delay(100);
    if (alternateSet == HIGH){
      targetWeight = 125;
      basketsSet = 125;
      delayBetweenDischarges = 20;
      oneGate = HIGH;
      fourGates = LOW;
      noTareLoader = LOW;
      delayForDrop = 1400;
    }
    else{
      targetWeight = 500;
      basketsSet = 200;
      delayBetweenDischarges = 20;
      oneGate = HIGH;
      fourGates = LOW;
      noTareLoader = LOW;//was High
      delayForDrop = 1400;
    }
  }
}

void buttons(){
  if(digitalRead(setPin) == LOW){
    alternateSet = HIGH;
    Serial.println("HIGH");
  }
  else{
    alternateSet = LOW;
    Serial.println("LOW");
  }
  if(digitalRead(yellowPin)== LOW){
    positioning();
  }
  if(digitalRead(startPin) == LOW){
    
    if(startFlag == HIGH){
      //startFlag = LOW;
    }
    else{
      startFlag = HIGH;
      slavesOn(machinePart);
    }
    //Serial.println("button");
    //module.setDisplayToString("    ");
    //delay(1000);
  }
  else startFlag = LOW;
}

void slavesOn(int part){
  positioning();
  Serial.print("Slaves on ");
  Serial.println(part);
  for (int a = 1; a <= 16; a++){//for (int a = SLAVES*part-SLAVES+1; a <= SLAVES*part; a++){
//    Serial.print("Sent"); Serial.println(a);
    byte x = 2;
    Wire.beginTransmission(a);
    Wire.write(x);
    Wire.endTransmission();
    module.setDisplayToDecNumber(a*10000,0x00);
    Serial.print(a);Serial.print(" ");
    delay(70);
  }
  Serial.println("");
}

void slavesOff(int part, byte x){
  Serial.print("Slaves off ");
  Serial.println(part);
  for (int a = 1; a <= 16; a++){//for (int a = SLAVES*part-SLAVES+1; a <= SLAVES*part; a++){
//    Serial.print("Sending..."); Serial.println(a+1);
    //byte x = 59;
    Wire.beginTransmission(a);
//    Serial.print("Transmission... "); Serial.println(a+1);
    Wire.write(x);
//    Serial.print("Write... "); Serial.println(a+1);
    Wire.endTransmission();
//    Serial.print("Sent: "); Serial.println(a+1);
  }
   // Serial.print("Proximity sensor: ");
   // Serial.println(digitalRead(proxPin));
    SendData("t" +String(32,DEC)+".pco","63488");
    SendData("t" +String(32,DEC)+".txt","\"" + String(targetWeight,0) + "\"");
    SendData("t" +String(33,DEC)+".pco","63488");
    SendData("t" +String(33,DEC)+".txt","\"" + String(basketsSet) + "\"");
    
}

void interrogation(int part){
  int b = 0;
 // Serial.println();
  Serial.print("Reading cells... ");
  Serial.println(part);
  for (int a = SLAVES*part-SLAVES; a <= SLAVES*part-1; a++){
    String dataString = "";
    float weight = -999.0F;
    Wire.requestFrom(a+1, 7);
    
    while (Wire.available()) {
      char c = Wire.read();
      dataString = dataString + c;
    }
    
    if(part == 2){
      b = a - SLAVES;
    }
    else{
      b = a;
    }
    scales[b] = dataString.toFloat();
    if(scales[b] == 0.0F){
      scales[b] = -99999.0F;
    }
    //Serial.print(a+1); Serial.print("\t\t = "); Serial.println(scales[b]);
    String dataToScreen = "nan";
    if(scales[b] != -99999.0F){
      dataToScreen = String(scales[b]);
      SendData("t" +String(a,DEC)+".pco",String(65535)); 
    }
    else{
      SendData("t" +String(a,DEC)+".pco",String(63488));
      }
      
    SendData("t" +String(a,DEC)+".txt","\"" + dataToScreen + "\"");
   // SendData("t" +String(32,DEC)+".txt","\"" + String(123) + "\"");
    delay(delayBetweenReadings);//speed maybe no stops
  }
}

void recombination(){
  Serial.print("R ");
//// PRINTOUT SCALES  
//    for(int a = 0; a <= SLAVES-1; a++){
//    Serial.print(a+1); Serial.print(" = "); Serial.println(scales[a]);
//  }
  
  int stepCounter = 0;
 // delay(1000);
  if(oneGate){
    for (int a = 0; a <= SLAVES-1; a++){
      results[stepCounter] = scales[a];
//      Serial.print(stepCounter); Serial.print(". ");
      //Serial.print(" "); Serial.print(a); Serial.print(" sum = "); Serial.println(results[stepCounter]);
      stepCounter++;
    }
    Serial.print("1 ");
  }


  if(twoGates){
    for (int a = 0; a <= SLAVES-2; a++){
      for (int b = a+1; b <= SLAVES-1; b++){
      results[stepCounter] = scales[a] + scales[b];
   //   Serial.print(stepCounter); Serial.print(". ");
  //    Serial.print(" "); Serial.print(a); Serial.print(" "); Serial.print(b);
  //    Serial.print(" sum = "); Serial.println(results[stepCounter]);
      stepCounter++;
      }
    }
    Serial.print("2 ");
  }

  if(threeGates){
    for (int a = 0; a <= SLAVES-3; a++){
      for (int b = a+1; b <= SLAVES-2; b++){
        for (int c = b+1; c <= SLAVES-1; c++){
          results[stepCounter] = scales[a] + scales[b] + scales[c];
   //     Serial.print(stepCounter); Serial.print(". ");
  //        Serial.print(" "); Serial.print(a); Serial.print(" "); Serial.print(b);
  //        Serial.print(" "); Serial.print(c);
  //        Serial.print(" sum = "); Serial.println(results[stepCounter]);
          stepCounter++;
        }
      }
    }
    Serial.print("3 ");
  }



  if(fourGates){
    for (int a = 0; a <= SLAVES-4; a++){
      for (int b = a+1; b <= SLAVES-3; b++){
        for (int c = b+1; c <= SLAVES-2; c++){
          for (int d = c+1; d <= SLAVES-1; d++){
          results[stepCounter] = scales[a] + scales[b] + scales[c] + scales[d];
//          Serial.print(stepCounter); Serial.print(". ");
//          Serial.print(" "); Serial.print(a); Serial.print(" "); Serial.print(b);
//          Serial.print(" "); Serial.print(c); Serial.print(" "); Serial.print(d);
//          Serial.print(" sum = "); Serial.println(results[stepCounter]);
          stepCounter++;
          }
        }
      }
    }
    Serial.print("by four ");
  }



 
//  if(fiveGates){
//    for (int a = 0; a <= 1; a++){
//      for (int b = a+1; b <= 2; b++){
//        for (int c = b+1; c <= 3; c++){
//          for (int d = c+1; d <= 4; d++){
//            for (int e = d+1; e <= 5; e++){
//              results[stepCounter] = scales[a] + scales[b] + scales[c] + scales[d] + scales[e];
//              Serial.print(stepCounter); Serial.print(". ");
//  //            Serial.print(" "); Serial.print(a); Serial.print(" "); Serial.print(b);
//  //            Serial.print(" "); Serial.print(c); Serial.print(" "); Serial.print(d);
//  //            Serial.print(" "); Serial.print(e);
//  //            Serial.print(" sum = "); Serial.println(results[stepCounter]);
//              stepCounter++;
//            }
//          }
//        }
//      }
//    }
//    Serial.println("");
//  }
//
//
//  
//  if(sixGates){
//  results[stepCounter] = scales[0] + scales[1] + scales[2] + scales[3] + scales[4] + scales[5];
////  Serial.print(stepCounter); Serial.print(". ");
////  Serial.print(" 0 1 2 3 4 5"); 
////  Serial.print(" sum = "); Serial.println(results[stepCounter]);
//  }
  Serial.print("|");
  analysis();

}

void analysis(){
 // Serial.println();
  Serial.print(" A ");

  
  float bestResult = maxResult;
  int a = 0;
  int bestPosition = -1;
  for(a = 0; a <= 133; a++){
    if(results[a] >= targetWeight && results[a] < bestResult){// <= changed to <
      bestResult = results[a];
      //Serial.print("Good result is at position "); Serial.print(a);
      //Serial.print(" and weight is: \t"); Serial.print(bestResult); Serial.println(" grams");
      bestPosition = a;
    }
  }
  //Serial.print("Best position is: ");
  //Serial.println(bestPosition);
  solution(bestPosition);
//  return bestPosition;
}

void solution(int stepNumber){
  
//  Serial.println();
//  Serial.println("Scales weights:");
//  for(int a = 0; a <= SLAVES-1; a++){
//    if (scales[a] < maxResult){
//    Serial.print(a+1); Serial.print(" = ");
//    Serial.print(scales[a]); Serial.println("gr.");
//    }
//    else{
//    Serial.print(a+1); Serial.print(" = ");
//    Serial.println("overload");
//    }
//  }
//  Serial.println();
  Serial.print(" S ");
  
  int stepCounter = 0;
  
  if(oneGate){
    for (int a = 0; a <= SLAVES-1; a++){
      if(stepNumber == stepCounter){
        cells[a] = HIGH;
//        Serial.println("Solution");
        //Serial.print(stepCounter); Serial.print(". ");
        //Serial.print(" "); Serial.print(a);
        Serial.print(" sum = \t\t"); Serial.println(results[stepCounter]);
        module.setDisplayToDecNumber(results[stepCounter]*100000,0x20);
        sendReady(results[stepCounter]);
        
      }
      stepCounter++;
    }
  }

  if(twoGates){
    for (int a = 0; a <= SLAVES-2; a++){
      for (int b = a+1; b <= SLAVES-1; b++){
        if(stepNumber == stepCounter){
          cells[a] = HIGH;
          cells[b] = HIGH;
          Serial.print(stepCounter); Serial.print(". ");
          Serial.print(" "); Serial.print(a); Serial.print(" "); Serial.print(b);
          Serial.print(" sum = "); Serial.println(results[stepCounter]);
          module.setDisplayToDecNumber(results[stepCounter]*100000,0x20);
        }
        stepCounter++;
      }
    }
  }

  if(threeGates){
    for (int a = 0; a <= SLAVES-3; a++){
      for (int b = a+1; b <= SLAVES-2; b++){
        for (int c = b+1; c <= SLAVES-1; c++){
          if(stepNumber == stepCounter){
            cells[a] = HIGH;
            cells[b] = HIGH;
            cells[c] = HIGH;
            Serial.print(stepCounter); Serial.print(". ");
            Serial.print(" "); Serial.print(a); Serial.print(" "); Serial.print(b);
            Serial.print(" "); Serial.print(c);
            Serial.print(" sum = "); Serial.println(results[stepCounter]);
            module.setDisplayToDecNumber(results[stepCounter]*100000,0x20);
          }
          stepCounter++;
        }
      }
    }
  }

  if(fourGates){
    for (int a = 0; a <= SLAVES-4; a++){
      for (int b = a+1; b <= SLAVES-3; b++){
        for (int c = b+1; c <= SLAVES-2; c++){
          for (int d = c+1; d <= SLAVES-1; d++){
            if(stepNumber == stepCounter){
              cells[a] = HIGH;
              cells[b] = HIGH;
              cells[c] = HIGH;
              cells[d] = HIGH;
              Serial.print(stepCounter); Serial.print(". ");
              Serial.print(" "); Serial.print(a); Serial.print(" "); Serial.print(b);
              Serial.print(" "); Serial.print(c); Serial.print(" "); Serial.print(d);
              Serial.print(" sum = "); Serial.println(results[stepCounter]);
              module.setDisplayToDecNumber(results[stepCounter]*100000,0x20);
              sendReady(results[stepCounter]);
            }
            stepCounter++;
          }
        }
      }
    }
  }

  if(fiveGates){
    for (int a = 0; a <= 1; a++){
      for (int b = a+1; b <= 2; b++){
        for (int c = b+1; c <= 3; c++){
          for (int d = c+1; d <= 4; d++){
            for (int e = d+1; e <= 5; e++){
              if(stepNumber == stepCounter){
                cells[a] = HIGH;
                cells[b] = HIGH;
                cells[c] = HIGH;
                cells[d] = HIGH;
                cells[e] = HIGH;
                Serial.print(stepCounter); Serial.print(". ");
                Serial.print(" "); Serial.print(a); Serial.print(" "); Serial.print(b);
                Serial.print(" "); Serial.print(c); Serial.print(" "); Serial.print(d);
                Serial.print(" "); Serial.print(e);
                Serial.print(" sum = "); Serial.println(results[stepCounter]);
                module.setDisplayToDecNumber(results[stepCounter]*100000,0x20);
              }
              stepCounter++;
            }
          }
        }
      }
    }
  }
  
  if(sixGates){
    if(stepNumber == stepCounter){
      cells[0] = HIGH;
      cells[1] = HIGH;
      cells[2] = HIGH;
      cells[3] = HIGH;
      cells[4] = HIGH;
      cells[5] = HIGH;
      Serial.print(stepCounter); Serial.print(". ");
      Serial.print("0 1 2 3 4 5"); 
      Serial.print(" sum = "); Serial.println(results[stepCounter]);
      module.setDisplayToDecNumber(results[stepCounter]*100000,0x20);
      sendReady(results[stepCounter]);
      
    }
  }
  Serial.println("");
}

void discharge(int part){
  int b = 0;
  Serial.print("D ");
  Serial.print(" open gates: ");
  for(int a = 0; a <= SLAVES-1; a++){
    Serial.print(cells[a]);
  }
  Serial.println("");
  for (int a = SLAVES*part-SLAVES; a <= SLAVES*part-1; a++){
     if(part == 2){
      b = a - SLAVES;
    }
    else{
      b = a;
    }
    if(cells[b]){
        Serial.print("Sent"); Serial.println(a+1);
        byte x = 1;
        Wire.beginTransmission(a+1);
        Wire.write(x);
        Wire.endTransmission();
        delay(delayBetweenDischarges);
        wdt_reset();
    }
    
  }
  if (cells[0] || cells[1] || cells[2] || cells[3] || cells[4] || cells[5] || cells[6] || cells[7]){
//    pack();
    if(part == 1){
      firstReady = HIGH;
      Serial.println("First ready");
    }
    if(part == 2){
      secondReady = HIGH;
      Serial.println("Second ready");
    }
  }
  Serial.println("Discharge done");
  delay(delayAfterDischarge);
}

void pack(){
  digitalWrite(dropPin, HIGH);
  digitalWrite(mainBelt, HIGH);
  delay(delayForDrop);
  digitalWrite(dropPin, LOW);
  digitalWrite(mainBelt, LOW);
  //delay(200);
  //noInterrupts();
  Serial.println("Packing...");
  cycleTime = millis()- oldTime;
  module.setDisplayToString("PACk");
  Serial.println(cycleTime);
  module.setDisplayToDecNumber(cycleTime*1000,0x00);
  
  oldTime = millis();
  
  delay(500);
  digitalWrite(packStart, HIGH);
  
  delay(packMoveTime);
  while(digitalRead(proxPin)){
    //delay(50);
    
    }
  digitalWrite(packStart, LOW); //unlimited belt
  
  module.setDisplayToString("    ");
  //delay(500);
  firstReady = LOW;
  secondReady = LOW;
//  if(!noTareLoader){
//    byte x = 2;
//    Wire.beginTransmission(17);
//    Wire.write(x);
//    Wire.endTransmission();
//  }
//  delay(100);
    SendData("t" +String(32,DEC)+".pco","33488");
  //  SendData("t" +String(32,DEC)+".txt","\"" + String(targetWeight) + "\"");
    SendData("t" +String(33,DEC)+".pco","33488");
  //  SendData("t" +String(33,DEC)+".txt","\"" + String(basketsSet) + "\"");
}

void positioning(){
  //noInterrupts();
  Serial.println("Positioning...");
  module.setDisplayToString(" POS");
  digitalWrite(packStart, HIGH);
  //digitalWrite(dropPin, HIGH);
  delay(packMoveTime);
  while(digitalRead(proxPin)){
    wdt_reset();
    //delay(50);
    }
  digitalWrite(packStart, LOW); //unlimited belt
  //digitalWrite(dropPin, LOW);
  module.setDisplayToString("done");

//   if(!noTareLoader){
//    byte x = 2;
//    Wire.beginTransmission(17);
//    Wire.write(x);
//    Wire.endTransmission();
//   }
//  delay(100); 
  //interrupts();
  //delay(500); 
}

void SendData(String dev, String data){
  mySerial.print(dev);
  mySerial.print("=");
  mySerial.print(data);
  mySerial.write(0xff);
  mySerial.write(0xff);
  mySerial.write(0xff);
}

void sendReady(float weight){
        if(machinePart == 1){
        SendData("t" +String(32,DEC)+".pco","8066");
        SendData("t" +String(32,DEC)+".txt","\"" + String(weight) + "\"");
      }
      if(machinePart == 2){
        SendData("t" +String(33,DEC)+".pco","8066");
        SendData("t" +String(33,DEC)+".txt","\"" + String(weight) + "\"");
      }
}
