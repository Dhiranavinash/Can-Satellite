  #include "DHT.h"
#include <Wire.h>
#include <stdio.h>
#include <math.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h> 

#define DHTPIN A0  
#define DHTTYPE DHT11  
#define BMP180_ADDR 0x77 // 7-bit address
#define BMP180_REG_CONTROL 0xF4
#define BMP180_REG_RESULT 0xF6
#define BMP180_COMMAND_TEMPERATURE 0x2E
#define BMP180_COMMAND_PRESSURE 0x34

DHT dht(DHTPIN, DHTTYPE);

RF24 radio (7,8);

const byte address [6] = "00001";


void setup() {
  Serial.begin(9600);
  

  Serial.println(F("DHTxx test!"));
  dht.begin();
  Serial.begin(9600); 
  Serial.println("REBOOT");
  if (begin())
    Serial.println("BMP180 init success");
  else{
    Serial.println("BMP180 init fail\n\n");
    while(1);
  }
  radio.begin();
  radio.openWritingPipe (address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.stopListening();
}

  float datatosend[8];
  
void loop() {
  
  delay(2000);



  
  float h = dht.readHumidity();
  
  float t = dht.readTemperature();
  
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  String g;
  float hif = dht.computeHeatIndex(f, h);
  
  float hic = dht.computeHeatIndex(t, h, false);
  

  Serial.print(("Humidity: "));
  datatosend[0]=h;
  Serial.print(datatosend[0]);
  
  Serial.print(("%\tTemperature: "));
  datatosend[1]=t;
  Serial.print(datatosend[1]);
  
  Serial.print(("°C "));
  datatosend[2]=f;
  Serial.print(datatosend[2]);
  
  Serial.print(("°F\tHeat index: "));
  datatosend[3]=hic;
  Serial.print(datatosend[3]);
  
  Serial.print(("°C "));
  datatosend[4]=hif;
  Serial.print(datatosend[4]);
  
  Serial.print(("°F "));
  
  char status;
  double T,P,p0,a;
  
    
    if (measureParameters(P,T) != 0){
    
      Serial.print("\t Absolute pressure: ");
      Serial.print(P);
      datatosend[5]=P;
      Serial.print(" mb, ");
      Serial.print(P*0.0295333727);
      datatosend[6]=(P*0.0295333727);
      Serial.print("in Hg");
      
      a = 44330.0*(1-pow(P/1013.25,1/5.255));
      Serial.print("\tAltitude: ");
      Serial.print(a);
      datatosend[7]=a;
      Serial.print("meters, ");
      
    }
    else 
      Serial.println("error retrieving pressure measurement\n");
   
  delay(2000);
  radio.write(&datatosend,sizeof(datatosend));
}



uint16_t unSignedIntTempVar,AC4,AC5,AC6;
int16_t signedIntTempVar,AC1,AC2,AC3,VB1,VB2,MB,MC,MD;

double c5,c6,mc,md,x0,x1,x2,y0,y1,y2,p0,p1,p2;
char _error;



char begin(){
  double c3,c4,b1;
  Wire.begin();

  if(readCompData(0xAA))
    AC1=signedIntTempVar;
  if(readCompData(0xAC))
    AC2=signedIntTempVar;
  if(readCompData(0xAE))
    AC3=signedIntTempVar;
  if(readCompData(0xB0))
    AC4=unSignedIntTempVar;
  if(readCompData(0xB2))
    AC5=unSignedIntTempVar;
  if(readCompData(0xB4))
    AC6=unSignedIntTempVar;
  if(readCompData(0xB6))
    VB1=signedIntTempVar;
  if(readCompData(0xB8))
    VB2=signedIntTempVar;
  if(readCompData(0xBA))
    MB=signedIntTempVar;
  if(readCompData(0xBC))
    MC=signedIntTempVar;
  if(readCompData(0xBE))
    MD=signedIntTempVar;

  c3 = 160.0 * pow(2,-15) * AC3;
  c4 = pow(10,-3) * pow(2,-15) * AC4;
  b1 = pow(160,2) * pow(2,-30) * VB1;
  c5 = (pow(2,-15) / 160) * AC5;
  c6 = AC6;
  mc = (pow(2,11) / pow(160,2)) * MC;
  md = MD / 160.0;
  x0 = AC1;
  x1 = 160.0 * pow(2,-13) * AC2;
  x2 = pow(160,2) * pow(2,-25) * VB2;
  y0 = c4 * pow(2,15);
  y1 = c4 * c3;
  y2 = c4 * b1;
  p0 = (3791.0 - 8.0) / 1600.0;
  p1 = 1.0 - 7357.0 * pow(2,-20);
  p2 = 3038.0 * 100.0 * pow(2,-36);
  
  return(1);
}


char readCompData(char address){
  unsigned char data[2];
  char x;

  Wire.beginTransmission(BMP180_ADDR);
  Wire.write(address);
  _error = Wire.endTransmission();
  if (_error == 0){
    Wire.requestFrom(BMP180_ADDR,2);
    while(Wire.available() != 2) ;
    for(x=0;x<2;x++){
      data[x] = Wire.read();
    }
    signedIntTempVar = (int16_t)((data[0]<<8)|data[1]);
    unSignedIntTempVar = (((uint16_t)data[0]<<8)|(uint16_t)data[1]);
    return(1);
  }
  signedIntTempVar= unSignedIntTempVar= 0;
  return(0);
}


char writeBytes(unsigned char *values, char length){
  char x;

  Wire.beginTransmission(BMP180_ADDR);
  Wire.write(values,length);
  _error = Wire.endTransmission();
  if (_error == 0)
    return(1);
  else
    return(0);
} 


char measureParameters(double &P, double &T){
  unsigned char data[3],delay1,x1;
  char result;
  double up,ut;

  data[0] = BMP180_REG_CONTROL;
  data[1] = BMP180_COMMAND_TEMPERATURE;
  writeBytes(data, 2);
  delay(5);

  Wire.beginTransmission(BMP180_ADDR);
  Wire.write(BMP180_REG_RESULT);

  if (Wire.endTransmission() == 0){
    Wire.requestFrom(BMP180_ADDR,2);
    while(Wire.available() != 2) ;
    for(x1=0;x1<2;x1++){
      data[x1] = Wire.read();
    }
    ut = (data[0] * 256.0) + data[1];
    T= calculateTemperature(ut);
  }
  else
    return(0);
 
  data[0] = BMP180_REG_CONTROL;
  data[1] = BMP180_COMMAND_PRESSURE;
  delay1 = 26;//5,8,14,26
  result = writeBytes(data, 2);
  delay(delay1);
  
  Wire.beginTransmission(BMP180_ADDR);
  Wire.write(BMP180_REG_RESULT);
  
  if (Wire.endTransmission() == 0){
    Wire.requestFrom(BMP180_ADDR,3);
    while(Wire.available() != 3) ;
    for(x1=0;x1<3;x1++){
      data[x1] = Wire.read();
    }
    
    up = (data[0] * 256.0) + data[1] + (data[2]/256.0);
    P=calculatePressure(up,T);
    return(1);
  }
  return(0);
}
double calculateTemperature(double ut){
  double T,a;

  a = c5 * (ut - c6);
  T = a + (mc / (a + md));
  
  return T;
}
double calculatePressure(double up,double T){
  double P;
  double s,x,y,z;
  
  s = T - 25.0;
  x = (x2 * pow(s,2)) + (x1 * s) + x0;
  y = (y2 * pow(s,2)) + (y1 * s) + y0;
  z = (up - x) / y;
  P = (p2 * pow(z,2)) + (p1 * z) + p0;
  
  return P;
  delay(1000);
  
}
