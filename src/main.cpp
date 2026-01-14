#include <Arduino.h>

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);

#include "RTClib.h"
char daysOfTheWeek[7][12] = { "MGG", "SEN", "SEL", "RBU", "KMS", "JMT", "SBT" };
RTC_DS3231 rtc;
int thn, bln, tgl, hr, jam, mnt, dtk;
String hari;
DateTime now;
bool jamKerja;

#include "PrayerTimes.h"
static const int DAY   = 13;
static const int MONTH = 1;
static const int YEAR  = 2026;

struct Alarm{int jam; int menit;int audio;}; // tambah message?
Alarm alarms[] = {
  {8,00,52}, //doa pagi
  {8,02,84}, //piket sesi 1
  {9,00,72}, //jam 9
  {10,00,73}, //jam 10
  {11,00,74}, //jam 11
  {12,00,75}, //istirahat
  {13,00,85}, //selesai istirahat // piket sesi 2a
  {14,00,46}, //persiapan kurir
  {15,00,78}, //po supply
  {15,25,79}, //menuju pulang
  {15,30,88}, //print kartu ucapan
  {15,20,86}, //piket sesi 2b
  {15,59,44} //pulang
  //{5,25,52},
};
const int jumlahAlarm = sizeof(alarms) / sizeof(alarms[0]);

#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"
SoftwareSerial FPSerial(D6, D5);
DFRobotDFPlayerMini myDFPlayer;
#define BUSY D3

#define LED D3
#define BUZZER D0
#define MIC A0

const int ledPin = LED_BUILTIN;
int ledState = LOW; 
unsigned long previousMillis = 0;
const long interval = 1000;

void action();
void debug();
void readRTC();
void tampil();
void playvoice(int);
void cekAlarm(DateTime);
void cekAlarmMin(DateTime);
void cekAlarmWeek(DateTime);
void printCity(const char* label,float lat,float lon,int tzMinutes);

void setup()
{
  pinMode(LED, OUTPUT); // LED
  pinMode(BUZZER, OUTPUT); // Buzzer
  pinMode(BUSY, INPUT); //BUSY DF
  pinMode(MIC, INPUT); // MIC
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);

  rtc.begin();
  delay(100);
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //rtc.adjust(DateTime(2026, 1, 14, 7, 36, 0));

  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(3,0);
  lcd.print("Hello, world!");

  printCity("Bandung",-6.973415,107.7545838,420);
  
  FPSerial.begin(9600);
  myDFPlayer.begin(FPSerial, /*isACK = */true, /*doReset = */false);
  delay(100);
  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  myDFPlayer.playMp3Folder(27);
  delay(100);
}

void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (ledState == LOW) {ledState = HIGH;} else {ledState = LOW;}
        digitalWrite(ledPin, ledState);
        action();
    }
}

void action()
{
  now = rtc.now(); // read RTC //cek hanya tiap menit (detik 1-3)
  //cek hanya saat perubahan jam kerja (menit 0)
  if (now.dayOfTheWeek() >= 0 && now.dayOfTheWeek() <= 5 && now.hour() >=7 && now.hour() <= 17){jamKerja=true;}
  else if (now.dayOfTheWeek() == 6 && now.hour() >= 7 && now.hour() <= 13){jamKerja=true;} else {jamKerja=false;}
  if (jamKerja){
    cekAlarm(now);
    cekAlarmMin(now);
     if(now.hour()==8 || now.hour()==13){cekAlarmWeek(now);} //hanya cek jam 8 dan jam 13 saja
  }
  tampil(); //lcd write
}

void cekAlarmWeek(DateTime x){
    if(x.dayOfTheWeek()==2 && x.hour()==8 && x.minute()==30 && x.second()<=3){playvoice(80);}
    else if(x.dayOfTheWeek()==3 && x.hour()==13 && x.minute()==1 && x.second()<=3){playvoice(81);}
    else if(x.dayOfTheWeek()==4 && x.hour()==13 && x.minute()==1 && x.second()<=3){playvoice(82);}
}

void cekAlarmMin(DateTime x){
if(x.minute()==15 && x.second()<=3){playvoice(70);} //istirahat sejenak
if(x.minute()==45 && x.second()<=3){playvoice(71);} //cek chat resi
//if(x.minute()==26 && x.second()<=3){playvoice(71);} //cek chat resi
}

void cekAlarm(DateTime x){
  for (int i = 0; i<jumlahAlarm;i++){
    if (x.hour()==alarms[i].jam && x.minute()==alarms[i].menit && x.second()<=3)
        {playvoice(alarms[i].audio);}
  }
}

void tampil(){
  char dateBuffer[12];
  char timeBuffer[20];
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  sprintf(dateBuffer, " %02u/%02u ", now.month(), now.day());
  sprintf(timeBuffer, "%02d:%02d:%02d ", now.hour(), now.minute(), now.second());
  lcd.print(dateBuffer);
  lcd.print(timeBuffer);
  //if (jamKerja==true){lcd.print("Y");}else{lcd.print("N");}
  lcd.print(jamKerja?"Y":"N");
  //lcd.setCursor(17, 0);
  
}

void debug() {
  //DateTime now = rtc.now();
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
  Serial.print(now.minute(),DEC);
  Serial.print(':');
  Serial.print(now.second());
  Serial.println();
  //Serial.println("loop");
}

void playvoice(int x) {  //y1 = tamu y0 = alarm
  myDFPlayer.playMp3Folder(27); //intro
  delay(200);
  while (digitalRead(BUSY) == LOW) {delay(100);}
  myDFPlayer.playMp3Folder(x);  //prog
  delay(200);
  while (digitalRead(BUSY) == LOW) {delay(100);}
  myDFPlayer.play(28); //outro
}

void readRTC() {
  //DateTime now = rtc.now();
  thn = now.year();
  bln = now.month();
  tgl = now.day();
  hr = now.dayOfTheWeek();
  hari = (daysOfTheWeek[now.dayOfTheWeek()]);
  jam = now.hour();
  mnt = now.minute();
  dtk = now.second();
}

void printCity(const char* label,float lat,float lon,int tzMinutes) {
  PrayerTimes pt(lat, lon, tzMinutes);
  // For high latitude locations, use adjustment
  if (pt.isHighLatitude()) {
    pt.setHighLatitudeRule(ANGLE_BASED);
  }
  pt.setAsrMethod(SHAFII);
  pt.setCalculationMethod(CalculationMethods::INDONESIA);
  pt.setAdjustments(2, -2, 0, 0, 2, 0);

  int fajrH, fajrM, sunriseH, sunriseM;
  int dhuhrH, dhuhrM, asrH, asrM;
  int maghribH, maghribM, ishaH, ishaM;

  pt.calculate(
    DAY, MONTH, YEAR,
    fajrH, fajrM,
    sunriseH, sunriseM,
    dhuhrH, dhuhrM,
    asrH, asrM,
    maghribH, maghribM,
    ishaH, ishaM
  );

  Serial.println();
  Serial.print("=== "); Serial.print(label);
  if (pt.isHighLatitude()) {
    Serial.print(" (High Latitude: ");
    Serial.print(lat, 1);
    Serial.print("°)");
  }
  Serial.println(" ===");
  Serial.print("Fajr:    "); Serial.println(pt.formatTime12(fajrH, fajrM));
  Serial.print("Sunrise: "); Serial.println(pt.formatTime12(sunriseH, sunriseM));
  Serial.print("Dhuhr:   "); Serial.println(pt.formatTime12(dhuhrH, dhuhrM));
  Serial.print("Asr:     "); Serial.println(pt.formatTime12(asrH, asrM));
  Serial.print("Maghrib: "); Serial.println(pt.formatTime12(maghribH, maghribM));
  Serial.print("Isha:    "); Serial.println(pt.formatTime12(ishaH, ishaM));
  
  if (pt.isHighLatitude()) {
    Serial.println("* High latitude adjustments applied");
  }
}