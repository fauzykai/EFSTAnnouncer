#include <Arduino.h>

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);

#include "RTClib.h"
char daysOfTheWeek[7][12] = { "MGG", "SEN", "SEL", "RBU", "KMS", "JMT", "SBT" };
RTC_DS3231 rtc;
int thn, bln, tgl, hr, jam, mnt, dtk;
String hari;

#include <SoftwareSerial.h>

#include "DFRobotDFPlayerMini.h"
SoftwareSerial FPSerial(D6, D5);
//#define FPSerial softSerial
DFRobotDFPlayerMini myDFPlayer;

#define BUSY D3

void debug();
void readRTC();

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  FPSerial.begin(9600);
  myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */false);

  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("Hello, world!");
  rtc.begin();

  myDFPlayer.volume(10);  //Set volume value. From 0 to 30
  myDFPlayer.play(10);  //Play the first mp3
}

void loop()
{
  readRTC();
  debug();
  delay(1000);
}

void tampil(){
  
}

void debug() {
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  //Serial.print(now.dayOfTheWeek());
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(mnt);
  Serial.print(':');
  Serial.print(now.second());
  Serial.println();
  //Serial.println("loop");
}

void readRTC() {
  DateTime now = rtc.now();
  thn = now.year();
  bln = now.month();
  tgl = now.day();
  hr = now.dayOfTheWeek();
  hari = (daysOfTheWeek[now.dayOfTheWeek()]);
  jam = now.hour();
  mnt = now.minute();
  dtk = now.second();
}