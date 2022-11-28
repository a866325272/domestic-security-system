#include "BC20.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
//#include <WiFiClientSecureAxTLS.h>
//#include <TridentTD_LineNotify.h>
//#define WIFI_SSID "Jihoon"
//#define WIFI_PASSWORD "qj654vmp4"
//#define LINE_TOKEN "RE8hRxBY8E8aQ6hiqz1iCJtNnVawNp6ZmvaAPwZhvNB"

//int ledGreen=10;
//int ledRed=8;
//int switchReed=6;
#define switchReed D1
#define red D2
#define beeper D3

//#define ledRed D2
//#define ledGreen D3
//String msg;
//int magnet;

StaticJsonDocument<200> json_doc;
char json_output[100];
String on_off , red_light , sound; 

// for MQTT 
String mqttServer      = "140.128.99.71";
String mqttPort        = "1883";
String mqttClientID    = "client_s07352042";
String mqttUser        = "thulora";
String mqttPassword    = "hpclab99";
String mqttPubTopic_Sensor  = "NBIOT/pub_test";
//String mqttPubTopic_Temp    = "NBiotTeam10/Temp";
//String mqttPubTopic_Humi    = "NBiotTeam10/Humi";
String mqttMmessage_Sensor  = "";
//String mqttMmessage_Temp    = "";
//String mqttMmessage_Humi    = "";

// for MultiTasking
unsigned long previousMillis_t1 = 0;   //Task_1上一個時間點
unsigned long previousMillis_t2 = 0;   //Task_2上一個時間點
const long interval_t1 =  5000;        //Task_1的執行週期時間
const long interval_t2 = 60000;        //Task_2的執行週期時間

void(* resetFunc) (void) = 0;          //宣告系統重置參數

void setup(){
  Serial.begin(115200);
  Serial_BC20.begin(9600);

  if (!BC20_init()) {               //初始化BC20，若連線失敗等待3秒鐘再重新啟動。
    delay(3*1000);
    resetFunc();
  }else{
    Serial.println("Initialization OK ...");
  }
  //pinMode(ledGreen, OUTPUT);
  //pinMode(ledRed, OUTPUT);
  //pinMode(beeper, OUTPUT);  
  pinMode(switchReed, INPUT); 
  pinMode(red, INPUT); 
  pinMode(beeper, OUTPUT);
  
  delay(1*1000);
  /*WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("連線中");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }  
  Serial.print("\n 分配到 IP: ");
  Serial.println(WiFi.localIP());*/
}

void loop(){
  /*WiFiClientSecure client; 
  LINE.setToken(LINE_TOKEN);
  Serial.println(LINE.getVersion());*/
  
  // task 1: 判斷電磁開關是否接觸：是 ... start
  unsigned long currentTime_t1 = millis();                 //獲得自程式啟動以來，經過的毫秒數。

  if (currentTime_t1 - previousMillis_t1 >= interval_t1) { //如果到達了時間間隔，即Task的執行週期時間。
    previousMillis_t1 = currentTime_t1;

    get_temp_on_off();       //讀取電磁開關資料    
    get_human();             //讀取人體紅外線資料
    get_beeper();            //讀取蜂鳴器資料
  }
  else if (currentTime_t1 - previousMillis_t1 <= 0) {      //如果millis約50天會溢位並重置為零。
    previousMillis_t1 = currentTime_t1;
  }
// Task 1 ... end
  
  // task 2：NB-IoT MQTT Publish ... start
  unsigned long currentTime_t2 = millis();                 //獲得自程式啟動以來，經過的毫秒數。

  if (currentTime_t2 - previousMillis_t2 >= interval_t2) { //如果到達了時間間隔，即Task的執行週期時間。
    previousMillis_t2 = currentTime_t2;

    // 用 JSON格式打包感測資料
    //mqttMmessage_Sensor = "{\"DoorStatus\":"+String(op)+"}";
    //mqttMmessage_Temp   = String(t);
    //mqttMmessage_Humi   = String(h);
    json_doc["DoorStatus"] = on_off;
    json_doc["Something"] = red_light;
    json_doc["Sound"] = sound;
    
    serializeJson(json_doc, json_output);
    Serial.println(json_output);
    
    connect_MQTT(mqttServer, mqttPort, mqttClientID, mqttUser, mqttPassword);     //開啟MQTT連線
    delay(1*1000);
    //client.publish(mqttPubTopic_Sensor,json_output);
    Publish_MQTT(mqttPubTopic_Sensor, json_output);                       //MQTT傳出量測的值
    delay(1*1000);
    //Publish_MQTT(mqttPubTopic_Temp  , mqttMmessage_Temp);
    //delay(1*1000);
    //Publish_MQTT(mqttPubTopic_Humi  , mqttMmessage_Humi);    
    Close_MQTT();                                                                 //關閉MQTT連結
    delay(1*1000);
    //Send_ATcommand("AT+QSCLK=1", 1);    //BC20啟用休眠
    //Send_ATcommand("AT+CFUN=0", 1);     //讓BC20進入省電模式
  }
  else if (currentTime_t2 - previousMillis_t2 <= 0) {      //如果millis約50天會溢位並重置為零。
    previousMillis_t2 = currentTime_t2;
  }
// Task 2 ... end
  
   
   
  /*if (!client.connect("https://notify-api.line.me", 443)) {
    Serial.println("連線失敗!!!");
    return;   
  }*/
  /*String  req = "POST /api/notify HTTP/1.1\r\n";
        req += "Host: notify-api.line.me\r\n";
        req += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
        req += "Cache-Control: no-cache\r\n";
        req += "User-Agent: ESP8266\r\n";
        req += "Connection: close\r\n";
        req += "Content-Type: application/x-www-form-urlencoded\r\n";
        req += "Content-Length: " + String(String("message=" + msg).length()) + "\r\n";
        req += "\r\n";
        req += "message=" + msg + magnet;
  client.print(req);*/
  
  /*while(client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r")
      break;
  }*/
}

void get_temp_on_off(){
  // 判斷電磁開關是否接觸：是
  if (digitalRead(switchReed)==HIGH){
    //LINE.notify("正常\n目前門窗正常");
    //digitalWrite(ledRed, LOW);
    //digitalWrite(ledGreen, HIGH);
    //digitalWrite(beeper, LOW);  
    on_off = "usual";  
    Serial.println("DoorStatus" + on_off);
  }
  else {
    //LINE.notify("不正常\n警告 !!! \n門窗不正常");
    //digitalWrite(ledRed, HIGH);
    //digitalWrite(ledGreen, LOW);
    //digitalWrite(beeper, HIGH);
    on_off = "unusual";
    Serial.println("DoorStatus" + on_off);
  }
}
void get_human(){
  // 判斷人體紅外線是否偵測到：是
    if(digitalRead(red) == HIGH){
      red_light = "Moving";
      Serial.println("Something" + red_light);
    }else{
      red_light = "NotMoving";
      Serial.println("Something" + red_light);
    }
}

void get_beeper(){
  // 判斷人體紅外線後，蜂鳴器是否要響 : 是
  if(digitalRead(switchReed) == HIGH){
    sound = "Quiet";
    Serial.println("Sound" + sound);
    digitalWrite(beeper,HIGH);
    delay(1000);
  }else{
    sound = "Noisy";
    Serial.println("Sound" + sound);
    digitalWrite(beeper,LOW);
    delay(5000);
  }
}
