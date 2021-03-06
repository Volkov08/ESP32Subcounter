

#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SevSeg.h>


//-------------------------------------------
//---------------Settings--------------------
unsigned int goal = 1000;
const char* ssid = "your WIFI name";
const char* password = "your WIFI pwd";
const String UID = "creator ID";
const String APIkey = "your YT API key";

const char thousand = "t" //will display 23456 as 23.4t
const char mil = "H" //will display 657891043 as 657H
//---------------Settings--------------------
//-------------------------------------------

const String URLs[] = {
                   "https://youtube.googleapis.com/youtube/v3/channels?part=statistics&id=" + UID + "&key=" + APIkey,
                   "https://youtube.googleapis.com/youtube/v3/search?part=snippet&channelId=" + UID + "&maxResults=1&order=date&type=video&key=" + APIkey,
                   "https://youtube.googleapis.com/youtube/v3/videos?part=statistics&key=" + APIke y+ "&id="
                 };

unsigned long last = 0;
unsigned int LEDtimer = 0;
unsigned int subs = 0;
unsigned int views = 0;
unsigned int likes = 0;

unsigned int touchDur = 0;

char text[8] = "____";

bool first = true;

SevSeg sevseg;
TaskHandle_t Upd;

void setup() {
  // put your setup code here, to run once:
  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  digitalWrite(21, 0);
  digitalWrite(22, 0);
  digitalWrite(23, 0);
  
  byte numDigits = 4;
  byte digitPins[] = {14,27,26,25};
  byte segmentPins[] = {2,0,4,16,17,5,18,19,21};
  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins, false, false);
  sevseg.setBrightness(20);
  Serial.begin(115200);
  initWiFi();
  sevseg.setChars(text);
  xTaskCreatePinnedToCore(
      loopC0,
      "loopC0",
      8000,
      NULL,
      0,
      &Upd,
      0);
}

void loopC0(void *params) {
  while(true){
  if (millis()-last > 30000 || first){
    getInfo(0);
    convert(subs);
    sevseg.setChars(text);
    first = false;
  }
  if (touchRead(33)<40){
    touchDur ++;
  } else {
    touchDur = 0;
  }
  if (touchDur > 500){
    sevseg.setChars("Load");
    digitalWrite(23, 1);
    getInfo(1);
    digitalWrite(23, 0);
    sevseg.setChars("vi");
    convert(views);
    delay(2000);
    sevseg.setChars(text);
    delay(3000);
    sevseg.setChars("li");
    convert(likes);
    delay(2000);
    sevseg.setChars(text);
    convert(subs);
    delay(3000);
    sevseg.setChars(text);
  }
  delay(1);
  }
}
void loop() {
  sevseg.refreshDisplay();
  if (millis() - LEDtimer > 1500){
    digitalWrite(21, 0);
  } else {
    digitalWrite(21, 1);
  }
}

//--------------------------------------------

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  digitalWrite(23, 1);
  while (WiFi.status() != WL_CONNECTED) {
    char* pos[] = {
         "conn",
         "c.onn",
         "c.o.nn",
         "c.o.n.n", 
         "c.o.n.n."
      };
    sevseg.setChars(pos[(millis()/500)%5]);
    sevseg.refreshDisplay();
  }
  Serial.println(WiFi.localIP());
  digitalWrite(23, 0);
}

void convert(int num){
  String ret;
  if (num < 1000){
    ret = String(num) ;
  } else if (num < 1000000) {
    ret = String((float)num/1000)  + " ";
    if (ret.charAt(3)=='.'){
      ret.setCharAt(3,thousand);
    } else {
      ret.setCharAt(4,thousand);
    }
  } else {
    ret = String((float)num/1000000) + " "; 
    if (ret.charAt(3)=='.'){
      ret.setCharAt(3,mil);
    } else {
      ret.setCharAt(4,mil);
    };
  }
  Serial.println(ret);
  ret.toCharArray(text,8);
}
void getInfo(int type) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(URLs[type].c_str());
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      StaticJsonDocument<1600> doc;
      DeserializationError error = deserializeJson(doc, payload);
      
      // Test if parsing succeeds.
      if (error) {
         Serial.print(F("deserializeJson() failed: "));
         Serial.println(error.f_str());
      return;
      }
      if (type == 0){
        const int updated = doc["items"][0]["statistics"]["subscriberCount"];
        Serial.println(updated);
        if (updated != subs) {
          if (updated > subs){
           subs = updated;
           Serial.println("new Sub");
            if (subs == goal){
              Serial.println("goal reached");
              for (int i = 0; i < 5; i ++){
               digitalWrite(22,1);
               delay(500);
               digitalWrite(22,0);
               delay(500);
              }
            } else {
             LEDtimer = millis();
            }
          }
        }
      } else {
        const String id = doc["items"][0]["id"]["videoId"].as<String>();
        Serial.println(id);
        http.begin(URLs[2].c_str() + id);
        int httpResponseCode2 = http.GET();
        String payload2 = http.getString();
        StaticJsonDocument<1600> doc2;
        DeserializationError error2 = deserializeJson(doc2, payload2);
      
        // Test if parsing succeeds.
        if (error2) {
         Serial.print(F("deserializeJson() failed: "));
         Serial.println(error2.f_str());
         return;
        }
        views = doc2["items"][0]["statistics"]["viewCount"].as<int>();
        likes = doc2["items"][0]["statistics"]["likeCount"].as<int>();
        Serial.println(views);
        Serial.println(likes);
      }
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
    initWiFi();
  }
  last = millis();
}
