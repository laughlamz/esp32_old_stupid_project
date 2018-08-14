#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();
//--------------------------------------For internal temperature

#include <dht_esp32.h>
#include <WiFi.h>
#include <Wire.h>
#include <SSD1306.h>

WiFiClient client;
SSD1306 display(0x3c, 21, 22);
dht myDht;

void parseATText(char b);
void resetBuffer(void);
void control_device(void);
void Wifi_Login_Oled(void);
void Wifi_IP_Oled(void);
void Send_to_Oled(void);
void Read_internal_temperature(void);

const char* ssid     = "LaughZ";
const char* password = "pass9999";
const char* host = "laughlam.tk";
const int httpPort = 80;

enum _parseState {
  DETECT_TYPE,
  CONTENT_TYPE,
  NEW_LINE,
  CONTENT_LEN,
  CONTENT  
};
char buffer[80];
char device[10];
char message[50];
byte pos = 0;
byte parseState = DETECT_TYPE;
int8_t cnt;
int8_t staDevice1, staDevice2, staDevice3;
float sen1, sen2;
float espTemp;
int debug, start;
unsigned long ovftime1, ovftime2, timeout1, timeout2, timeout3;
String url = "blocks/scan_act_device.php";

void setup()
{
    Serial.begin(230400);
    pinMode(2,OUTPUT);
    pinMode(12,OUTPUT);
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    display.setContrast(255);
    delay(10);
    
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
    Wifi_Login_Oled(); delay(500);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Wifi_IP_Oled();  delay(500);
}

void loop()
{  
    myDht.read11(4);
    sen1 = myDht.temperature;
    sen2 = myDht.humidity;
    Read_internal_temperature();
    
    if( client.connect(host, httpPort) ){
      client.print("POST /" + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
      timeout1 = millis();           
      while(!client.available()){
        delay(1);
        if(millis() - timeout1 > 2000){
          Serial.println("Timeout1");
          break;
        }
      }
      while (client.available()) {
        char ch = client.read();
        //Serial.print(ch);   //DEBUG (useful)
        parseATText(ch);
      }
      
      client.stop();
    }
    
    control_device();
    Send_to_Oled();
    
    if(client.connect(host, httpPort)){
      client.print(String("GET /blocks/scan_sta_device.php?") + "statusD1=" + staDevice1 + "&statusD2=" + staDevice2 + "&statusD3=" + staDevice3 + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
      timeout2 = millis();           
      while(!client.available()){
        delay(1);
        if(millis() - timeout2 > 2000){
          Serial.println("Timeout2");
          break;
        }
      }             
      client.stop();
    }
    
    if(millis() - ovftime1 > 5000){
      ovftime1 = millis();
      if(client.connect(host, httpPort)){
        client.print(String("GET /blocks/scan_sen_device.php?") + "sensor1=" + sen1 + "&sensor2=" + sen2 + "&sensor3=" + String(espTemp,2) + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
        timeout3 = millis();           
        while(!client.available()){
          delay(1);
          if(millis() - timeout3 > 2000){
            Serial.println("Timeout3");
            break;
          }
        }                 
        client.stop();

      }
    }
}

void control_device(void){
  if(device[0] == '1'){
    digitalWrite(2,HIGH);
    staDevice1 = 1;
  }
  else{
    digitalWrite(2,LOW);
    staDevice1 = 0;
  }

  if(device[1] == '1'){
  digitalWrite(12,HIGH);
    staDevice2 = 1;
  }
  else{
  digitalWrite(12,LOW);
    staDevice2 = 0;
  }  

  if(device[2] == '1'){
//  digitalWrite(4,HIGH);
    staDevice3 = 1;
  }
  else{
//  digitalWrite(4,LOW);
    staDevice3 = 0;
  }  
}

void Wifi_Login_Oled(void)
{
  display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Connecting...");
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 26, "ID: LaughZ");
  display.display();
}
void Wifi_IP_Oled(void)
{
  display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Wifi is connected!");
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 26, "IP address:");
    display.drawString(0, 36, WiFi.localIP().toString());
  display.display();
}

void Send_to_Oled(void)
{
  display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    if(device[0] == '1')
      display.drawString(5, 12, String("1:ON"));
    else
      display.drawString(5, 12, String("1:OFF"));
    if(device[1] == '1')
      display.drawString(50, 12, String("2:ON"));
    else
      display.drawString(50, 12, String("2:OFF"));
    if(device[2] == '1')
      display.drawString(95, 12, String("3:ON"));
    else
      display.drawString(95, 12, String("3:OFF"));
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 25, String(message));
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 40, String("Temp.esp: "));
    display.drawString(50, 40, String(espTemp,2));
    display.drawString(0, 50, String("Temp: "));
    display.drawString(32, 50, String(sen1));
    display.drawString(70, 50, String("Humi: "));
    display.drawString(98, 50, String(sen2));
  display.display();
}



void resetBuffer(void) {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

void parseATText(char b) {
  
  buffer[pos++] = b;
  
  if(pos >= sizeof(buffer))
    resetBuffer(); // just to be safe
  switch(parseState)
  {
    case DETECT_TYPE:{
      if(b == '\n')
        resetBuffer();
      else if(b == ':'){
        if(strcmp(buffer, "Content-Type:") == 0){
          parseState = CONTENT_TYPE;
          //Serial.println("Detech x request id");
        }
      }
      break;
    }

    case CONTENT_TYPE:{
      if(b == '\n'){
        parseState = NEW_LINE;
        resetBuffer();
      }
      break;
    }
    
    case NEW_LINE:{
      if(b == '\n'){
        parseState = CONTENT_LEN;
        resetBuffer();
      }
      break;
    }

    case CONTENT_LEN:{
      if(b == '\n'){
        //Serial.println(buffer);
        parseState = CONTENT;
        resetBuffer();
      }
      break;
    }

    case CONTENT:{
      if(b == '\n'){     
        for(int i=0; i<3; i++)
          device[i] = buffer[i];
        for(int i=3; i<50; i++)
          message[i-3] = buffer[i];
        //Serial.println(buffer);
        Serial.println(device);
        Serial.println(message);  
        parseState = DETECT_TYPE;
        resetBuffer();
      }
      break;
    }
    
  }
}

void Read_internal_temperature(void){
  Serial.print("Temperature: ");
  espTemp = (temprature_sens_read() - 32) / 1.8;
  Serial.print(espTemp,2);
  Serial.println(" C");  
}
