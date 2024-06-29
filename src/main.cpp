#include <LoRa_E32.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <time.h>
#include "RTClib.h"

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define RXp2 16
#define TXp2 17
#define LED 2

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "PENELITIAN";
char pass[] = "fikri_123";

int keyIndex = 0;
WiFiClient client;

//RTC
RTC_DS3231 rtc;
char dataHari[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
String hari;
int tanggal, bulan, tahun, jam, menit, detik;
//float suhu;

//Array
String arrData[2];

// Connect ThingSpeak
unsigned long myChannelNumber = 1834824;
const char * myWriteAPIKey = "JNEDEGYEOC16QCCL";

uint16_t port = 8080;

unsigned long previousMillis = 0;
const long interval = 3000;

File myFile;

//void setTime(int timezone)
//{
//  Serial.print("[Time] : Setting time using SNTP \n");
//  
//  //UNIX TIMESTAMP UTC = Total seconds from 1970
//  configTime(timezone * 3600, 0, "id.pool.ntp.org");
//  time_t now = time(nullptr);
//  while (now < 10000)
//  {
//    delay(500);
//    Serial.print (".");
//    now = time(nullptr);
//  }
//  struct tm timeinfo;
//  gmtime_r(&now, &timeinfo);
//  Serial.printf("\n[TIME] : Current time : %s \n", asctime(&timeinfo));
//}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);
  pinMode (LED, OUTPUT);
  
  //Connect to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
    {
    Serial.print("Attempting to connect to SSID : ");
    Serial.println(ssid);
    delay(500);
    }
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());

//    setTime(7);
    
  rtc.begin();
  
  Serial.print("Initializing SD card...");
  SD.begin();

  if (!SD.begin(5)) {
    Serial.println("initialization failed!");
    while (1);
  }
  else{
  Serial.println("initialization done.");
  }

    if (! rtc.begin()) {
    Serial.println("RTC Tidak Ditemukan");
    Serial.flush();
    abort();
  }

//  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//  rtc.adjust(teTime(2022,5,01,13,57,0));

  WiFi.mode(WIFI_STA); 
  ThingSpeak.begin(client); 

  digitalWrite (LED, HIGH);
}

void loop() {

  DateTime now = rtc.now();
  hari    = dataHari[now.dayOfTheWeek()];
  tanggal = now.day();
  bulan   = now.month();
  tahun   = now.year();
  jam     = now.hour();
  menit   = now.minute();
  detik   = now.second();
//  suhu    = rtc.getTemperature();
  Serial.println(String() +hari+"," +tanggal+"-"+bulan+"-"+tahun);
  Serial.println(String() +jam+":" +menit+":"+detik);
//  Serial.println(String() +jam+":" +menit+":"+detik+" " +suhu+".C ");
//  Serial.println(String() + "Suhu : " + suhu + ".C");
  Serial.println();

   // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    digitalWrite (LED, LOW);
  }
  else{
    digitalWrite(LED, HIGH);
  }

//  time_t now = time(nullptr);
//  struct tm* p_timeinfo = localtime(&now);
//  Serial.printf("[NOW] : Time = %s \n", asctime(p_timeinfo));

  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval){
  previousMillis = currentMillis;

//  if (p_timeinfo->tm_min == 00)
  if (menit == 00)
{
  Serial2.print("Ya");
//  Serial.println("Ya");
}
  
  String paket = "";
  
  while(Serial2.available()>0)
  {
    paket += char(Serial2.read());
  }
    
  paket.trim();
  if(paket != "")
    {
      int index = 0;
      for(int i=0; i<= paket.length(); i++)
      {
        char delimiter = '#';
        if(paket [i] != delimiter)
          arrData[index] += paket[i];
        else 
          index++;
      }
      if (index == 1)
      {
      Serial.println("Temperature :  " + arrData[0]);
      Serial.println("Humidity : " + arrData[1]);
      Serial.println(String() +hari+"," +tanggal+"-"+bulan+"-"+tahun);
      Serial.println(String() +jam+":" +menit+":"+detik);
      Serial.println();
      }

      float t = arrData[0].toFloat();
      float h = arrData[1].toFloat();

      // set the fields with the values
      ThingSpeak.setField(1, t); //menampilkan data sensor di thingspeak di field 1
      ThingSpeak.setField(2, h);
      int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
   
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("/Data.txt", FILE_APPEND);

  // if the file opened okay, write to it:
  if (myFile) {
//    Serial.print("Writing to Data.txt...");
    myFile.println(String("Temperature : ") + t + String(".C "));
    myFile.println(String("Humidity : ") + h + String("%"));
    myFile.println(String() +hari+"," +tanggal+"-"+bulan+"-"+tahun);
    myFile.println(String() +jam+":" +menit+":"+detik);
    myFile.println();
    // close the file:
    myFile.close();
    Serial.println("done.");
  } 
  else {
//     if the file didn't open, print an error:
    Serial.println("error opening Data.txt");
  }    
      arrData[0] = "";
      arrData[1] = "";
    }
    }
  delay(1000); 
}
