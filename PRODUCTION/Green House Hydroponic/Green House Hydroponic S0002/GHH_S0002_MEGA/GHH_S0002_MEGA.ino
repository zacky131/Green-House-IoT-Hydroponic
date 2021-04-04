//Written by: Muhammad Zacky Asy'ari. auftechnique.com December 2020

#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include <GravityTDS.h>
#include <DFRobot_PH.h>


// FOR TEMPERATURE SENSOR
// Data wire is conntec to the Arduino digital pin 4
#define ONE_WIRE_BUS 4
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
float temperature;


// FOR Water level
#define pinSensor A1  // mendefinisikan pin A1 sebagai pin yang berhubungan dengan sensor


//FOR pH SENSOR
#define PH_PIN A3
float voltage,phValue,temperature_pH = 25;
DFRobot_PH ph;


//FOR TDS SENSOR
#define TdsSensorPin A0
GravityTDS gravityTds;
float temperatureRef = 25,tdsValue = 0;


String kirim = "";


void setup(){
  //Begin both serial communication
  Serial.begin(57600);
  Serial3.begin(57600);
  //Begin Temperature sensor
  sensors.begin();

  
  // For TDS Sensor
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();  //initialization  

    // For pH Sensor
  ph.begin();
}

  
void loop(){
  //TEMPERATURE MEASUREMENT
    // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  Serial.print("temperature Reading ==> "); 
  Serial.print("Celsius temperature: ");
  // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  temperature = sensors.getTempCByIndex(0);
  Serial.println(temperature);
  delay(500);
   

  // SEND WATER LEVEL DATA
  float sensorValue = analogRead(pinSensor);
  float tinggiAir = sensorValue/100;

  // print data ke serial monitor
  Serial.print("Sensor Value = ");
  Serial.print(sensorValue);
  Serial.print(" ==> ");
  Serial.print("Tinggi Air = ");
  Serial.print(tinggiAir);
  Serial.println("cm");
  delay(500);
  

  //SEND pH DATA
    static unsigned long timepoint = millis();
    if(millis()-timepoint>1000U)  //time interval: 1s
    {
      timepoint = millis( 
        );
      voltage = analogRead(PH_PIN)/1024.0*5000;  // read the voltage
      //temperature = readTemperature();  // read your temperature sensor to execute temperature compensation
      phValue = ph.readPH(voltage,temperature_pH);  // convert voltage to pH with temperature compensation
      Serial.print("pH Reading ==> ");
      Serial.print("temperature pH:");
      Serial.print(temperature_pH,1);
      Serial.print("^C  pH:");
      Serial.println(phValue,2);
    }
    ph.calibration(voltage,temperature_pH);  // calibration process by Serail CMD
    

  //SEND TDS DATA
  gravityTds.setTemperature(temperatureRef);  // set the temperature and execute temperature compensation
  gravityTds.update();  //sample and calculate
  tdsValue = gravityTds.getTdsValue();  // then get the value
  Serial.print("TDS Reading==> ");
  Serial.print(tdsValue,0);
  Serial.println(" ppm");
  delay(500);


  
  //Send Data to ESP
  kirim  = "";
  kirim += temperature;
  kirim += ";";
  kirim += tinggiAir;
  kirim += ";";
  kirim += phValue;
  kirim += ";";
  kirim += tdsValue;
//  Serial.println(kirim);  
  Serial3.println(kirim);
  
  if(Serial3.available()){
    String msg = "";
    while(Serial3.available()){
      msg += char(Serial3.read());
      delay(500);
    }
    Serial.println(msg);
  }  
  delay(120000);// This is important, need longer time if you send longer data 
}
