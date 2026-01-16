//belum readrfid -> write DB via API

#include <Arduino.h>

#include <ESP8266WiFi.h>
const char* ssid = "37L3"; const char* password = "74737970";

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);

#include "RTClib.h"
char daysOfTheWeek[7][12] = { "MGG", "SEN", "SEL", "RAB", "KAM", "JUM", "SAB"};
RTC_DS3231 rtc;
int thn, bln, tgl, hr, jam, mnt, dtk;
String hari;
DateTime now;
bool jamKerja;

#include "PrayerTimes.h"
//static const int DAY   = 13; static const int MONTH = 1; static const int YEAR  = 2026;
int DAY,MONTH,YEAR;
int JDz,MAs,JAs,MDz;
int IqmJDz,IqmJAs,IqmMDz,IqmMAs,Iqm=-3;
String JDzuhur, JAshar;

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
  {15,25,79}, //kesehatan toko
  {15,30,88}, //print kartu ucapan
  {15,55,86}, //piket sesi 2b
  {15,59,44}, //pulang
  {0,0,87},//reserve utk iqmdz
  {0,0,87}//reserve utk iqmas
  //{5,25,52},
  //IqmDz
  //IqmAs
};
const int jumlahAlarm = sizeof(alarms) / sizeof(alarms[0]);

//jadwal piket
//jadwal mhd
//piket mingguan

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
  lcd.setCursor(0,0);
  lcd.print("EFST Announcer V2");

  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi"); while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());

  //blm autocalculate when day change
  now = rtc.now();
  DAY=now.day();MONTH=now.month();YEAR=now.year();
  printCity("Bandung",-6.973415,107.7545838,420);
  alarms[13]={IqmJDz,IqmMDz,87}; //write
  alarms[14]={IqmJAs,IqmMAs,87};
  //IqmDz = 

  //Serial.println(JDz);
  //Serial.println(MDz);
  //char buffer[8];
    //Serial.println(sprintf(buffer, "%02d:%02d", JDz, MDz));
    //Serial.println(sprintf(buffer, "%02d:%02d", JAs, MAs));
  
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
  if (now.dayOfTheWeek() > 0 && now.dayOfTheWeek() <= 5 && now.hour() >7 && now.hour() < 16){jamKerja=true;}
  else if (now.dayOfTheWeek() == 6 && now.hour() > 7 && now.hour() < 13){jamKerja=true;} else {jamKerja=false;}
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
  lcd.setCursor(0, 1);
  lcd.print("D:"+JDzuhur);
  lcd.setCursor(8, 1);
  lcd.print("A:"+JAshar);
  lcd.setCursor(16, 1);
  lcd.print("I:");
  //lcd.setCursor(19, 1);
  lcd.print(Iqm);

  lcd.setCursor(0, 3);
  lcd.print(WiFi.localIP());
  lcd.setCursor(14, 3);
  lcd.print(WiFi.RSSI());
  lcd.print("dBm");
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
  myDFPlayer.playMp3Folder(27); delay(200); //intro
  while (digitalRead(BUSY) == LOW) {delay(100);}
  myDFPlayer.playMp3Folder(x);  delay(200); //prog
  while (digitalRead(BUSY) == LOW) {delay(100);}
  myDFPlayer.playMp3Folder(28); //outro
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
  pt.setAdjustments(3, 1, 4, 3, 4, 3);

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

  JDzuhur=pt.formatTime24(dhuhrH, dhuhrM);
  JAshar=pt.formatTime24(asrH, asrM);
  JDz=dhuhrH; MDz=dhuhrM;
  JAs=asrH; MAs=asrM;
  
  int totalmin;
  totalmin = (dhuhrH*60+dhuhrM)+Iqm;
  IqmJDz=(totalmin/60)%24;
  IqmMDz=totalmin%60;

  totalmin = (asrH*60+asrM)+Iqm;
  IqmJAs=(totalmin/60%24);
  IqmMAs=totalmin%60;

  Serial.println();
  Serial.print("=== "); Serial.print(label);
  if (pt.isHighLatitude()) {
    Serial.print(" (High Latitude: ");
    Serial.print(lat, 1);
    Serial.print("°)");
  }
  Serial.println(" ===");
  Serial.print("Fajr:    "); Serial.println(pt.formatTime24(fajrH, fajrM));
  Serial.print("Sunrise: "); Serial.println(pt.formatTime24(sunriseH, sunriseM));
  Serial.print("Dhuhr:   "); Serial.println(pt.formatTime24(dhuhrH, dhuhrM));
  Serial.print("Asr:     "); Serial.println(pt.formatTime24(asrH, asrM));
  Serial.print("Maghrib: "); Serial.println(pt.formatTime24(maghribH, maghribM));
  Serial.print("Isha:    "); Serial.println(pt.formatTime24(ishaH, ishaM));
  Serial.println(" === Iqm : "+Iqm);
  Serial.print("Iqm Dhz: "); Serial.println(pt.formatTime24(IqmJDz,IqmMDz));
  Serial.print("Iqm Ash: "); Serial.println(pt.formatTime24(IqmJAs,IqmMAs));
  
  if (pt.isHighLatitude()) {
    Serial.println("* High latitude adjustments applied");
  }
}