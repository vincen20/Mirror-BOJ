#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "OLEDDisplayUi.h"
#include "images.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "SR04.h"
#include <Ticker.h>
Ticker ticker;
Ticker ticker2;
Ticker ticker3;


const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
//ADC_MODE(ADC_VCC);
//----------------------  HC-SR04
#define MODEM_SLEEP_T 150
#define TRIG_PIN D1
#define ECHO_PIN D2
SR04 sr04 = SR04(ECHO_PIN, TRIG_PIN);
// Display Settings OLED
const char *v;//voli grg
#define OLED_SDA D3
#define OLED_SCl D4
SSD1306  display(0x3c, OLED_SDA, OLED_SCl);
OLEDDisplayUi ui     ( &display );
void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
 // display->drawString(128, 0, "50%");
}
FrameCallback frames[] = {drawFrame1, drawFrame2, drawFrame3};
int frameCount = 3;
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;
//--------------------DS18B20
#define ONE_WIRE_BUS D5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
//----------------------------
#define DEMO_DURATION 50
typedef void (*Demo)(void);
int demoMode = 0;
int counter = 1;
const char* server = "apis.baidu.com";
// ---------------------------------------const 
unsigned long lastcont = 0;          // last time you connected to the server, in milliseconds
const unsigned long timesout = 8 * 1000; // delay between 2 datapoints, 30s
const unsigned long timesout2 = 50 * 1000; // delay between 2 datapoints, 30s
const int UPDATE_INTERVAL_SECS = 20 * 60; // Update every 10 minutes 10*60
const int READ_DIST_SECS= 2;
const int COLSE_DISP_SECS =10;
const float DIST_CM=60.0; // 90cm
const char* WIFI_SSID = "wiffssid";
const char* WIFI_PWD = "passwodd";
const char* W_APIKEY = "baidu.api????/";
String sdata,webdatas,tmp,tmp3, hum, codes;
bool beginrecord=false;
bool tag1 = false;
bool tag3 = false;
bool isdisp=true;
bool readyForUpdate = false;
float dist3=0.0;
const char *p;//wearh drig
WiFiClient client;
/////////////////////////////////////////////////
  ///////// im2

  void dx4() {
  display.drawXbm(0, 0, 128, 64, x4);
}
void dx3() {
  display.drawXbm(0, 0, 128, 64, x3);
}
void dx32() {
  display.drawXbm(0, 0, 128, 64, x32);
}
void dx31() {
  display.drawXbm(0, 0, 128, 64, x31);
}
void dx2() {
  display.drawXbm(0, 0, 128, 64, x2);
}
void dx1() {
  display.drawXbm(0, 0, 128, 64, x1);
}
Demo demos[] = {dx2, dx3, dx31, dx32, dx4,dx2, dx3, dx31, dx32, dx4,dx2, dx3, dx31, dx32, dx4,dx2, dx3, dx31, dx32, dx4,dx2, dx3, dx31, dx32, dx4,dx2, dx3, dx31, dx32};
//-------------------------------------
int demoLength = (sizeof(demos) / sizeof(Demo));
long timeSinceLastModeSwitch = 0;
////////////////////////////////////////////////
void setup() {
 // Serial.begin(115200);
 // Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);
  //-------------------------------
  ui.setTargetFPS(60);
  ui.setActiveSymbol(activeSymbole);
  ui.setInactiveSymbol(inactiveSymbole);
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, frameCount);
  ui.setOverlays(overlays, overlaysCount);
  ui.init();
  //display.flipScreenVertically();

  //------------------------ initialize dispaly
  display.init();
  display.clear();
  display.display();
  //display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.setContrast(255);
  //============================
    display.drawString(0, 0, "OLED OK.");
    display.display();
  //----------------------- DS18B20
    sensors.begin();
    display.drawString(0, 10, "DS18B20 INIT..");
    display.display();
  //--------------------------WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
   while (WiFi.status() != WL_CONNECTED) {
      display.drawString(0, 20, "Connecting WIFI ...");
      display.display();
    }
   display.drawString(0, 30, "IP:"+(String)WiFi.localIP()); 
   display.display();
  if (!client.connect(server, 80)) {
    display.drawString(0, 40, "Connection failed");
    display.drawString(0, 50, "System failed");
    display.display();
    return;
  }
  else
  {
   display.drawString(0, 40, "Server connect OK....");
   display.display();
   }
 ticker.attach(UPDATE_INTERVAL_SECS, setReadyForWeatherUpdate);
 //ticker2.attach(COLSE_DISP_SECS, setDispClose);
 //ticker3.attach(READ_DIST_SECS, readDist);
 display.drawString(0, 50, "Ticker set OK.....");
  display.display();
// Serial.println("setup end");
 delay(1000);
for(int i=80;i>0;i--)
{
  im2();
}
    display.clear();
   ///////////////////////////////////////
   postData("CN101270302");
   tmp3=gettemp3();
//   dist3=getdist();
 readvdrg();
ticker2.attach(COLSE_DISP_SECS, setDispClose);
ticker3.attach(READ_DIST_SECS, readDist);
}
void loop() {
  if (client.available()) {
    char c = client.read();
    if(c=='{'){
       beginrecord=true;
      }
      if(beginrecord)
      {
      webdatas+=c;
      }
  }
  if (client.connected()&& millis() - lastcont > timesout2)
  {
  //  Serial.println(" timeout22222");
    client.stop();
  }
  // put your setup code here, to run once:
  if (readyForUpdate && ui.getUiState()->frameState == FIXED) {
    webdatas="";
    tmp3=0.0;
  //  dist3=0.0;
    updateData(&display);
 //   Serial.println(webdata);
  }
if(dist3<DIST_CM&&isdisp)
{ 
  int remainingTimeBudget = ui.update();
}
else
{
   display.clear();
   display.display();
 //  Serial.println("is clear?");
   flashled();
  }

}
void drawProgress(OLEDDisplay *display, int percentage, String label) {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_10);
  display->drawString(64, 10, label);
  display->drawProgressBar(10, 28, 108, 12, percentage);
  display->display();
}
void setReadyForWeatherUpdate() {
 // Serial.println("Setting readyForUpdate to true");
  readyForUpdate = true;
}
void setDispClose() {
  //    Serial.println("  setDispClose ");  
     if (dist3>DIST_CM&&!isdisp){
        display.clear();
        isdisp=false;
        display.display();
  //  Serial.println("  setDispClose ");  
     }
}
void readDist()
{ 
  float a = sr04.Distance();
  dist3=a;   
  if (dist3<DIST_CM){
    isdisp=true;
    }
  }
void updateData(OLEDDisplay *display) {
     readvdrg();
  //  Serial.println("updateData");
  drawProgress(display, 30, "POST DATAS Weather...");
   delay(200);
  drawProgress(display, 60, "GET TEMP DATA....");
  tmp3=gettemp3();
  delay(200);
  drawProgress(display, 90, "GET DIST DATA....");
  dist3=getdist();
  delay(200);
  drawProgress(display, 100, "UPDATE DATA OK.");
  delay(400);
  readyForUpdate = false;
     postData("CN101270302");
}
float getdist() {
  float a = sr04.Distance();
//  Serial.println(a);
  return a;
}
String gettemp3(){
    sensors.requestTemperatures(); // Send the command to get temperatures
    float tmp2=sensors.getTempCByIndex(0);  
    String  tmp3=(String)tmp2;
    tmp3=tmp3.substring(0,tmp3.length()-1);
    return tmp3;
  }
/////////get weather data
void postData(String cityids) {
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    client.print("POST /heweather/weather/free?cityid=CN101270302");
    client.println(" HTTP/1.1");
    client.print("Host:");
    client.println(server);
    client.print("Accept: *");
    client.print("/");
    client.println("*");
    client.print("apikey:");
    client.println(W_APIKEY);
    client.println("Content-Length:23");
    client.println("Content-Type: application/x-www-form-urlencoded;charset=UTF-8");
    client.println("Connection:close");
    client.println();
    client.print("city=fushun");
    client.print("&cityid=");
    client.println(cityids);//CN101270302
  }
  else {
    client.stop();
  }
  lastcont = millis();
}
/*
{
    "HeWeather data service 3.0": [
        {
            "aqi": {
                "city": {
                    "aqi": "111",
                    "pm10": "113",
                    "pm25": "83",
                    "qlty": "轻度污染"
                }
            },
            "basic": {
                "city": "富顺",
                "cnty": "中国",
                "id": "CN101270302",
                ............................
*/
String getwdata(String wd,String key1, String key2)  
{                      //             city   pm25
  int s1=wd.indexOf(key1);  
  String temstr=wd.substring(s1,wd.length());
  int s2=temstr.length();
  temstr=temstr.substring(0,s2);
  s1=temstr.indexOf(key2);
  temstr=temstr.substring(s1,temstr.length());
   s2 = temstr.indexOf(",");
   temstr = temstr.substring(0, s2);
   s2 = temstr.indexOf(":");
   temstr = temstr.substring(s2 + 2, temstr.length() - 1);
  return temstr;
}
 
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
 //   Serial.println("drawFrame1");
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  String codesc = getwdata(webdatas, "now","code");
  String tmpc = getwdata(webdatas, "now", "tmp");
  String humc = getwdata(webdatas, "now", "hum");
  selectcode(codesc.toInt());
  display->drawString(90 + x, 0 + y,"Now");
  display->drawString(60 + x, 17 + y, "T:"+(String)tmpc+"°C");
  display->drawString(60 + x, 40 + y, "H:"+(String)humc+"%");
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 50 + y, "C:"+(String)codesc);
  display->drawXbm(x + 0, y-10, 50, 50, p);
  display->drawXbm(x + 120, y, 7, 4, v);// v
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
 //   Serial.println("drawFrame2");
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  String date=getwdata(webdatas, "daily_forecast","date");
  String codesd = getwdata(webdatas, "daily_forecast","code_d");
   String codesn = getwdata(webdatas, "daily_forecast","code_n");
  String tmpmax = getwdata(webdatas, "daily_forecast", "max");
  String tmpmin = getwdata(webdatas, "daily_forecast", "min");
  tmpmin=tmpmin.substring(0,tmpmin.length()-1);
  String pm25 = getwdata(webdatas, "city", "pm25");
  //selectcode(codesd.toInt());
  selectcode(codesn.toInt());
  display->drawString(60 + x, 0 + y, "High:  "+(String)tmpmax+"°C");
  display->drawString(60 + x, 15 + y, "Low :  "+(String)tmpmin+"°C");
  display->drawString(60 + x, 30 + y, "PM2.5:  "+(String)pm25);
  display->drawString(30 + x, 50 + y, "Date:"+(String)date);
  display->drawString(0 + x, 50 + y, "C:"+(String)codesn);
  display->drawXbm(x + 0, y-10, 50, 50, p);
  display->drawXbm(x + 120, y, 7, 4, v);// v
}
void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
//  Serial.println("drawFrame3");
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(x+50,y+0,"Indoor");
  display->setFont(ArialMT_Plain_24);
  display->drawString(x+50,y+20,tmp3+"°C");
  display->drawXbm(x, y, 50, 50, indoor);
  display->drawXbm(x + 120, y, 7, 4, v);// v
}

void selectcode(int code)
{
  
  switch (code) {
    case 100:
      p = c100;
      break;
    case 101:
      p = c101;
      break;
    case 102:
      p = c102;
      break;
    case 103:
      p = c103;
      break;
    case 104:
      p = c104;
      break;
    case 200:
      p = c20x;
      break;
    case 301:
      p = c301;
      break;
    case 302:
      p = c302;
      break;
    case 303:
      p = c303;
      break;
    case 305:
      p = c305;
      break;
    case 306:
      p = c306;
      break;
    case 307:
      p = c307;
      break;
    case 310:
      p = c310;
      break;
    case 401:
      p = c401;
      break;
    case 500:
      p = c500;
      break;
    case 501:
      p = c501;
      break;
    default:
      p = c999;
  }

}
void readvdrg(){
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
 float voltage =  analogRead(A0) * (5.0 / 1023.0);
// Serial.print("v:");
// Serial.println(voltage);
 v=power100;
 if(voltage<=3.79)
 {
  v=power20;
  } 
  else   if(voltage>3.79 &&voltage<3.87)
  {
    v=power40;
   }
     else  if(voltage>=3.87 &&voltage<4.0)
  {
    v=power60;
   }
     else  if(voltage>=4.0 &&voltage<=4.1)
  {
    v=power80;
   }
     else  if(voltage>4.1)
  {
    v=power100;
   }
}
void im2()// im2 video
{
  display.clear();
  demos[demoMode]();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0,40,"SYSTEM OK");
  display.display();
  if (millis() - timeSinceLastModeSwitch > DEMO_DURATION) {
    demoMode = (demoMode + 1)  % demoLength;
    timeSinceLastModeSwitch = millis();
  }
  counter++;
  delay(10);
}
void flashled(){
  int in=digitalRead(LED_BUILTIN);
  in=!in;
 digitalWrite(LED_BUILTIN, in);   
  }

