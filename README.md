Connect MAX7219, DHT11.
Show time, weather, temperature, huminidy. 
Get time and weather from a local servre
ip local server 192.168.0.104

# General info about project
I broke the project for STM32F103... while updating it to work with new STM Cube IDE.
I also wrote the "server_stub" project for the ESP8266. It intended to illuminate request to the web server and run old project on the STM.
I did dump file from the STM.

# How to connect

## DHT 7319
- DHT 7319 - blue pill
- CLK - P13
- CS - A1
- DIN - P15

## DHT 22
- DHT22 - blue pill
- OUT - A4

## ST-link v2
(link)[https://stm32-base.org/guides/connecting-your-debugger.html]
- ST-link v2 - blue pill
- SWDIO (2) - SWO 
- SWCLK (6) - SWCLK
- GND (4) - GND
- 3v3 (8) - 3v3

## ESP8266
- ESP8266 - blue pill
- TX - A10
- RX - A9