#include <SPI.h>

#include <nRF24L01.h>
#include <RF24.h>
RF24 radio (7,8);

const int buzzer =2;

const byte address[6] ="00001";

void setup(){

Serial.begin(9600);

// put your setup code here, to run once:

pinMode (buzzer, OUTPUT);

radio.begin();

radio.openReadingPipe (0, address);

radio.setPALevel (RF24_PA_MAX);

radio.setDataRate (RF24_250KBPS);

radio.startListening ();}
float humidity;
float temperature;
float temperature2;
float heatindex;
float heatindex2;
float pressure;
float  pressure2;
float altitude;

bool newData=false;

void getData() {

if (radio.available()){
  float datatoreceive[8];
  radio.read(&datatoreceive,sizeof(datatoreceive));
  humidity=datatoreceive[0];
  temperature=datatoreceive[1];
  temperature2=datatoreceive[2];
  heatindex=datatoreceive[3];
  heatindex2=datatoreceive[4];
  pressure=datatoreceive[5];
  pressure2=datatoreceive[6];
  altitude=datatoreceive[7];
 
  newData=true;}
}
 void showData(){
  if (newData==true){
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%\tTemperature: ");
    Serial.print(temperature);
    Serial.print("°C ");
    Serial.print(temperature2);
    Serial.print("°F\t");
    Serial.print("Heatindex: ");
    Serial.print(heatindex);
    Serial.print("°C ");
    Serial.print(heatindex2);
    Serial.print("°F ");
    Serial.print("\tAbsolute Pressure: ");
    Serial.print(pressure);
    Serial.print(" mb ");
    Serial.print(pressure2);
    Serial.print(" Hg ");
    Serial.print("\t Altitude: ");
    Serial.print(altitude);
    Serial.print(" meters ");
    Serial.print("\n");
    
    
    newData=false;
  }
  
}
void loop(){
  getData();
  showData();
}

  
