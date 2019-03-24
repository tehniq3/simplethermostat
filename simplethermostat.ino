/*
termostat cu senzor DS18B20, afisaj 1602 cu interfata i2c, 
reglaj temperatura dorita din butoane

program scris de Nicu FLORICA (niq_ro) pt George DACIN
ver.1  - 28.02.2018
*/

#include <EEPROM.h>  // http://tronixstuff.com/2011/03/16/tutorial-your-arduinos-inbuilt-eeprom/
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F, 16, 4);  // scan adress with i2c scaner sketch https://playground.arduino.cc/Main/I2cScanner

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

float te; // temperature from DS18B20 sensor
float tes; // desired temperature 
float dete = 0.5; // histeresys
byte tset1, tset2; // value for read from EEPROM
byte tset01, tset02; // value for record in EEPROM
int tes0; // value use for record in EEPROM

#define insus 4
#define injos 5 
#define releu 6

// http://arduino.cc/en/Reference/LiquidCrystalCreateChar
byte grad[8] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
};

unsigned long ultimacitire; // last read of DS sensor
unsigned long timpasteptare = 20000; // time between reading of DS sensor


void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Thermostat for heating");

// define outputs and inputs
 pinMode(releu, OUTPUT);  
 pinMode(injos, INPUT);
 pinMode(insus, INPUT);

// set the default state for outputs and inputs
  digitalWrite(releu, LOW);
  digitalWrite(injos, HIGH);
  digitalWrite(insus, HIGH);

// initialize the LCD
  lcd.begin();
// create custom symbol
lcd.createChar(0, grad);

  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.setCursor(0,0); 
  lcd.print("Termostat v.1.0");
  lcd.setCursor(0,1);  
  lcd.print("  by niq_ro  ");

  // Start up the library
  sensors.begin();
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  
  Serial.print("Temperature for the device 1 (index 0) is: ");
  te = sensors.getTempCByIndex(0);
  Serial.println(te);  

  delay(2000);
  lcd.clear();

tset1 = EEPROM.read(201);
tset2 = EEPROM.read(202);
tes = 256 * tset1 + tset2;  // recover the number
tes = tes/10;
ultimacitire = millis();  

lcd.setCursor(0,0);  
lcd.print("Actual: ");
lcd.setCursor(3,1);  
lcd.print("Set: ");
lcd.setCursor(8,0);  
if (te < 10.) lcd.print(" ");
lcd.print(te,1);
//lcd.write(0b11011111);
lcd.write(byte(0));
lcd.print("C ");
lcd.setCursor(8,1);  
if (tes < 10.) lcd.print(" ");
lcd.print(tes,1);
//lcd.write(0b11011111);
lcd.write(byte(0));
lcd.print("C ");

}  // end setup

void loop(void)
{ 


  
if ((millis() - ultimacitire) > timpasteptare)
{ 
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  Serial.print("Temperature for the device 1 (index 0) is: ");
  te = sensors.getTempCByIndex(0);
  Serial.println(te);  
  ultimacitire = millis();
if (te < 10.) lcd.print(" ");
lcd.setCursor(8,0); 
lcd.print(te,1);
//lcd.write(0b11011111);
lcd.write(byte(0));
lcd.print("C ");
}
  
// change desired temperature
   if (digitalRead(injos) == LOW)
  { tes = tes - 0.1; 
  if (tes < 5.0) tes = 5.0;  // limit for minimum set value
  delay(250);
  }
   if (digitalRead(insus) == LOW)
  { tes = tes + 0.1;
   if (tes > 27.0) tes = 27.0;  // limit for maximum set value
  delay(250);
  }

tes0 = tes*10;  
tset01 = tes0 / 256;
tset02 = tes0 - tset01 * 256;
if ((tset1 != tset01) || (tset2 != tset02))
{
EEPROM.write(201, tset01);  // partea intreaga
EEPROM.write(202, tset02);   // rest
Serial.println("store in EEPROM");
lcd.setCursor(8,1);  
if (tes < 10.) lcd.print(" ");
lcd.print(tes,1);
//lcd.write(0b11011111);
lcd.write(byte(0));
lcd.print("C ");
}  

if (te < tes)  // temperature is bellow
{
  digitalWrite(releu, HIGH); 
  lcd.setCursor(15,1);
  lcd.print("*");  
}
if (te > tes + dete)  // temperature is over
{
  digitalWrite(releu, LOW); 
  lcd.setCursor(15,1);  
  lcd.print(" ");
}

} // end main loop
