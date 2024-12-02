// подключение библиотек
#define USE_SERIAL Serial
void setup() {
  USE_SERIAL.begin(115200);
  USE_SERIAL.setTimeout(500);
}
void loop() {
  uint8_t handleMessage = 0;
  if(USE_SERIAL.available() > 0)
  { 
    String UARTdat = USE_SERIAL.readString();
    if (UARTdat.indexOf("getDateTime") > -1){
        USE_SERIAL.print("2024-12-02 dt \"*  time 15:05:28 tm -#:\r\n");
    }
    else {
      USE_SERIAL.print("Good weather\r\n");
    }
  }
}
