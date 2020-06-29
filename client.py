import paho.mqtt.client as paho
import time
import matplotlib.pyplot as plt
import numpy as np

# MQTT broker hosted on local machine
mqttc = paho.Client()
# Settings for connection
host = "localhost"
topic = "Mbed"

op = []

# Callbacks
def on_connect(self, mosq, obj, rc):
      print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
      print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n");
      tmp = str(msg.payload)  
      l = len(msg.payload)
      if (tmp == "01"):
          print("forward\n")
      elif (tmp == "02"):
          print("backward\n")
      elif (tmp == "03"):
          print("right")
      elif (tmp == "04"):
          print("left")
      elif (tmp == "05"):
          print("stop")


def on_subscribe(mosq, obj, mid, granted_qos):
      print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
      print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)


for i in range(10):
       mqttc.loop()

# if(len(T) == 25):
#       print("X: " + str(X))
#       print("Y: " + str(Y))
#       print("Z: " + str(Z))
#       print("T: " + str(bool(T)))
