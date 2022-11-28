import json
import paho.mqtt.client as mqtt
import datetime as datetimeLibrary
import time
import mysql.connector
from mysql.connector import Error
import requests

LORA_NODE_MAC = '00000000aa110030'

# 當地端程式連線伺服器得到回應時，要做的動作
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # 將訂閱主題寫在on_connet中
    # 如果我們失去連線或重新連線時
    # 地端程式將會重新訂閱
    client.subscribe("NBIOT/pub_test")

# 當接收到從伺服器發送的訊息時要進行的動作
def on_message(client, userdata, msg):
	# print(msg.topic+" "+msg.payload.decode('utf-8'))   # 轉換編碼utf-8才看得懂中文
    
    print(msg.topic+" "+str(msg.payload))
    
    #TIME_FORMAT = '%Y-%m-%d %H:%M:%S'
    
    d = json.loads(msg.payload)
    
    status = d['DoorStatus']
    something = d['Something']
    sound = d['Sound']

    # Request MQTT
    requests.post('https://notify-api.line.me/api/notify', data = {'message':"門窗狀態 : " + status + "物體狀態 : " + something + "蜂鳴器狀態 : " + sound}, headers={'Authorization': 'Bearer 7njT5F76LuqxgV7xBfYImBMAND1xIFkOXwmJe12wTBt'})




# 連線設定
client = mqtt.Client()            # 初始化地端程式 
client.on_connect = on_connect    # 設定連線的動作
client.on_message = on_message    # 設定接收訊息的動作

client.username_pw_set("course","iot999")      # 設定登入帳號密碼 
client.connect("140.128.99.71", 1883, 60) # 設定連線資訊(IP, Port, 連線時間)

# 開始連線，執行設定的動作和處理重新連線問題
# 也可以手動使用其他loop函式來進行連接
client.loop_forever()