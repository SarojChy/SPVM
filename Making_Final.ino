#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <MFRC522.h>
#include <RTClib.h> // for the RTC

#define SS_PIN 10
#define RST_PIN 9
#define BUZZ 8 //define pin buzzer pin
#define MOTDEBUG
File myFile;
String data;
String  DataTemp;
String  UID;
bool Valid_UID = false;
#define motorEN1 4 //enbalbe pin for driver 
#define motorEN2 4 //enbalbe pin for driver 

int Nema17Motor_Steps = 200;
int Nema14Motor_Steps = 100;

#define stepPin  3
#define dirPin  2
#define stepPin2  6
#define dirPin2 5
const int PadNo = 60;   //Total number of PAD
int NpadLim = PadNo / 2;
int address = 0;
int address2 = 1;
int pinState;
#define Resetpin A0
int NPadNo;
#define CS_SD 7

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);


RTC_DS1307 rtc;
const int checkInHour = 15;
const int checkInMinute = 40;

int userCheckInHour;
int userCheckInMinute;

void setup()
{
  Buzzer();
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

#ifdef DEBUG
  //  Serial.print("Initializing SD card...");
#endif
  if (!SD.begin(CS_SD))
  {
    Serial.println("SD initialization failed!");
    displayMsg("SD InIt failed!");
    while (1)
      ;
  }

  // Setup for the RTC
  if (!rtc.begin()) {
    //Serial.println("Couldn't find RTC");
    displayMsg("RTC init failed!");
    while (1);
  }
  //  else {
  //    // following line sets the RTC to the date & time this sketch was compiled
  //    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //  }
  if (!rtc.isrunning()) {
    //    Serial.println("RTC is NOT running!");
  }

  ///Serial.println("initialization done.");

  pinMode(BUZZ, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(motorEN1, OUTPUT);

  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(motorEN2, OUTPUT);

  digitalWrite(motorEN1, HIGH);
  digitalWrite(motorEN2, HIGH);

  pinMode(Resetpin, INPUT_PULLUP);
  EEPROM.update(address, PadNo);
  NPadNo = EEPROM.read(address2);

  lcd.clear();
  displayMsg("    Namaste");
  delay(2000);
  displayMsg("** Nepatronix **");
  for (int a = 0; a <= 15; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(150);
  }
  delay(1000);
  displayMsg(" Tap Your Card");
  lcd.setCursor(1, 1);
  lcd.print("Available: ");
  lcd.setCursor(12, 1);
  lcd.print(NPadNo);
}

void loop()
{
  Reset();
  NPadNo = EEPROM.read(address2);
  Serial.println(NPadNo);
  delay(1000);
  if (NPadNo > 0) {
    if (!mfrc522.PICC_IsNewCardPresent())
    {
      //    Serial.println("Not found");
      //    delay(100);
      return;
    }
    if (!mfrc522.PICC_ReadCardSerial())
    {
      //    Serial.println("Not read yet");
      //    delay(100);

      return;
    }
    //Serial.println("Card found");
    String content = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
      // Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      // Serial.print(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    data = content.substring(1);
    //Serial.print("Data : ");
    //Serial.println(data);

    // Opening SD card to validate UID
    myFile = SD.open("TEST.txt");
    if (myFile) {
      while (myFile.available()) {

        char c = myFile.read(); // Get the next character
        if (isPrintable(c)) {
          DataTemp.concat(c);
        }
        if (c == '\n') {
          UID = DataTemp;
          DataTemp = "";
          if (UID == data)
            Valid_UID =  true;
        }
      }
      myFile.close();
    }
    verify();   //checking either UIDs presein SD or not
  }
  else {
    nopad();
  }
}

void verify() {
  if (Valid_UID) {
    Buzzer();
    checkForStack();
    logDataOnSD();
    count();
  }
  if (!Valid_UID) {
    tone(BUZZ, 300);
    delay(1500);
    noTone(BUZZ);
    invalidCard();
  }
  Valid_UID = false;
}
