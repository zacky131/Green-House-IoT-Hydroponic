import os
import time
import sys
import board
import adafruit_dht
import paho.mqtt.client as mqtt
import json
import RPi.GPIO as GPIO


THINGSBOARD_HOST = 'iotplatform.soltekno.com'
ACCESS_TOKEN = 'RASPI_GH_HYD_R001'
# Initial the dht device, with data pin connected to: (GPIO Number)
dhtDevice = adafruit_dht.DHT22(board.D17)

# you can pass DHT22 use_pulseio=False if you wouldn't like to use pulseio.
# This may be necessary on a Linux single board computer like the Raspberry Pi,
# but it will not work in CircuitPython.
# dhtDevice = adafruit_dht.DHT22(board.D18, use_pulseio=False)

 # GPIO Numbers instead of board numbers
GPIO.setmode(GPIO.BCM)

GPIO_RELAY = 16

GPIO.setwarnings(False)
GPIO.setup(GPIO_RELAY, GPIO.OUT) # GPIO Assign mode
GPIO.output(GPIO_RELAY, GPIO.LOW) # OFF

# Data capture and upload interval in seconds. Less interval will eventually hang the DHT22.
INTERVAL=120

sensor_data = {'Room Temperature': 0, 'Room Humidity': 0}

next_reading = time.time() 

client = mqtt.Client()

# Set access token
client.username_pw_set(ACCESS_TOKEN)

# Connect to ThingsBoard using default MQTT port and 60 seconds keepalive interval
client.connect(THINGSBOARD_HOST, 1883, 60)

client.loop_start()

while True:
    try:
        # Print the values to the serial port
        temperature_c = dhtDevice.temperature
        temperature_f = temperature_c * (9 / 5) + 32
        humidity = dhtDevice.humidity
        print(
            "Room Temp: {:.1f} F / {:.1f} C    Room Humidity: {}% ".format(
                temperature_f, temperature_c, humidity
            )
        )
        sensor_data['Room temperature'] = temperature_c
        sensor_data['Room humidity'] = humidity
        
        #To trigger GPIO
        if temperature_c > 35:
            GPIO.output(GPIO_RELAY, GPIO.HIGH) # ON
        else:
            GPIO.output(GPIO_RELAY, GPIO.LOW) # OFF

        # Sending humidity and temperature data to ThingsBoard
        client.publish('v1/devices/me/telemetry', json.dumps(sensor_data), 1)
        
        # Next reading sensor
        next_reading += INTERVAL
        sleep_time = next_reading-time.time()       
        
        if sleep_time > 0:
            time.sleep(sleep_time)
        
        

    except RuntimeError as error:
        # Errors happen fairly often, DHT's are hard to read, just keep going
        print(error.args[0])
        time.sleep(2.0)
        continue
    except Exception as error:
        dhtDevice.exit()
        raise error

    time.sleep(2.0)
    
client.loop_stop()
client.disconnect()