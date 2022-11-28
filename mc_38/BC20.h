#include <SoftwareSerial.h> 

SoftwareSerial Serial_BC20(D5,D6);  //(Rx=14=D5, Tx=12=D6) to BC20(Tx,Rx)

// #include <ArduinoJson.h>

// void(* resetFunc) (void) = 0;

byte Rset_Count = 0;
int waitingTime = 30000;

String Check_Receive_Data()
{
  String data = "";
  char c;
  while (Serial_BC20.available())
  {
    delay(0.05*1000);
    c = Serial_BC20.read();       //Conduct a serial read
    data += c;                    //Shorthand for data = data + c
    if (c == '\n') break;
  }
  data.trim();
  return data;
}

byte Send_ATcommand(String msg, byte stepnum)
{
  String Showmsg, C_temp;
  Serial.print("Send: "); Serial.println(msg);
  Serial_BC20.println(msg);
  Showmsg = Check_Receive_Data();
  //Serial.println(Showmsg);
  long StartTime = millis();
  switch (stepnum)
  {
    case 0:                 //Reset BC20
      C_temp = "+IP:";
      break;
    case 1:                 //Other Data
      C_temp = "OK";
      break;
    case 2:                 //Check IPAddress
      C_temp = "+CGPADDR:";
      break;
    case 10:                //build MQTT Server
      C_temp = "+QMTOPEN: 0,0";
      break;
    case 11:                //Connect to MQTT server by username and password
      C_temp = "+QMTCONN: 0,0,0";
      break;
    case 12:                //Publisher MQTT Data
      C_temp = "+QMTPUB: 0,0,0";
      break;
    case 13:                //Sub MQTT Data
      C_temp = "+QMTSUB: 0,1,0,0";
      break;
    case 14:                //Close MQTT connection
      C_temp = "+QMTCLOSE: 0,0";
      break;      
  }
  while (!Showmsg.startsWith(C_temp))
  {
    Showmsg = Check_Receive_Data();
    if (Showmsg.startsWith("+")) Serial.println(Showmsg);
    if ((StartTime + waitingTime) < millis()) return stepnum;
  }
  return 99;
}

bool BC20_init() {             // initialization BC20
  Send_ATcommand("AT+QSCLK=0", 1);    delay(1*1000);
  Send_ATcommand("AT+CFUN=1", 1);     delay(1*1000);
  Send_ATcommand("AT+QBAND=1,8", 1);  delay(1*1000);
  Send_ATcommand("AT+QGACT=1,1,\"apn\",\"internet.iot\"", 1);   delay(1*1000);
  Send_ATcommand("AT+QCGDEFCONT=\"IP\",\"internet.iot\"", 1);   delay(1*1000);
  Send_ATcommand("AT+QRST=1", 0);     delay(1*1000);

  if (Send_ATcommand("ATE0", 1) == 99)
    if (Send_ATcommand("AT+CGPADDR=1", 2) == 99) return true;

  return false;
}

bool connect_MQTT(String Broker, String port, String clientID, String user, String pass) { //connect MQTT Broker
  String ATcmd_temp;
  
  ATcmd_temp = "AT+QMTOPEN=0,\"" + Broker + "\"," + port;
  if (Send_ATcommand(ATcmd_temp, 10) != 99)
    return false;

  ATcmd_temp = "AT+QMTCONN=0,\"" + clientID + "\",\"" + user + "\",\"" + pass + "\"";
  if (Send_ATcommand(ATcmd_temp, 11) != 99)
    return false;

  return true;
}


bool Publish_MQTT(String topic, String message) {
  String ATcmd_temp;

  ATcmd_temp = "AT+QMTPUB=0,0,0,0,\"" + topic + "\",\"" + message + "\"";
  if (Send_ATcommand(ATcmd_temp, 12) != 99)
    return false;

  return true;
}

bool Subscribe_MQTT(String topic) {         //Subscribe Topic
  String ATcmd_temp;

  ATcmd_temp = "AT+QMTSUB=0,1,\"" + topic + "\",0";
  if (Send_ATcommand(ATcmd_temp, 13) != 99)
    return false;

  return true;
}

bool Close_MQTT() {                         //Close MQTT connect
  String ATcmd_temp;

  ATcmd_temp = "AT+QMTCLOSE=0";
  if (Send_ATcommand(ATcmd_temp, 14) != 99)
    return false;

  return true;
}
