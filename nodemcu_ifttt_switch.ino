
/*
 * IFTTT INTEGRATED IOT SWITCH WORKS WITH NODEMCU ESP8266 USING IFTTT WEBHOOKS
 * 
 * Developer: Nik Valiris
 * Date:      17/10/2017
 * Version:   0.1
 * 
 * Thanks to :  http://arduino-er.blogspot.co.uk/2016/04/nodemcu-esp8266-to-display-on-128x64.html
 *              Virang for his IFTTT examples http://virang-a.blogspot.co.uk/2017/06/ifttt-button-with-nodemcu.html
 * 
 * Todo:        
 *              1. Webhook is in the clear, better to encrypt
 *              2. Add support for a light sensor
 *              3. Tidy up the code and some more functions
 * 
 * Updates:
 *              0.1 First version that works
*/

#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include "DHT.h"
#include "FS.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define OLED_RESET LED_BUILTIN //4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2


#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16zm


#define DHTPIN 2     // what digital pin we're connected to



// Uncomment whatever type you're using!

#define DHTTYPE DHT11   // DHT 11

//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

//#define DHTTYPE DHT21   // DHT 21 (AM2301)

String st;
String content;

String qhubip1,qhubip2,qhubip3,qhubip4,qhubport;


//AP definitions
const char* ssid = "";
const char* password = "";
const char* host = "maker.ifttt.com";
const char* apiKey = "";
String qapiKey, qswitch1, qswitch2, qswitch3, qswitch4, qdevname;

//Pin setup

int pin = 15; //D8
int pin2 = 14; //D5
int pin3 = 13; //D7
int pin4 = 12; //D6

int pin5 = 0; //D3 - not enabled yet
int pin6 = 2; //D4 - not enabled yet
int pin7 = 16; //D0 - not enabled yet

volatile int flag = false;
volatile int flag2 = false;
volatile int flag3 = false;
volatile int flag4 = false;
const char* PIN_state = "not pressed";
const char* PIN_state2 = "not pressed";
const char* PIN_state3 = "not pressed";
const char* PIN_state4 = "not pressed";

void PINStatus() {
            PIN_state = "switch1 pressed";  
             flag = true;
}
void PINStatus2() {
            PIN_state2 = "switch2 pressed";  
             flag2 = true;
}
void PINStatus3() {
            PIN_state3 = "switch3 pressed";  
             flag3 = true;
}
void PINStatus4() {
            PIN_state4 = "switch4 pressed";  
             flag4 = true;
}

//AP definitions for Access Point onboard
//const char *ssidapp = "";
//const char *passwordapp = "";
String ssidap = "";
String passwordap = "";

//Configuration for onboard webserver
ESP8266WebServer serverap(80);

void handleRoot() {


 IPAddress ip = WiFi.softAPIP();
 IPAddress ipwifi = WiFi.localIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  String ipwifiStr = String(ipwifi[0]) + '.' + String(ipwifi[1]) + '.' + String(ipwifi[2]) + '.' + String(ipwifi[3]);  
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String macStr = macToStr(mac);
  content = "<!DOCTYPE HTML>\n<html>";
  content += "<style>h1 {color: red; padding: 10px; text-decoration: underline;} ul {list-style-type: upper-roman; margin-left: 50px;} p {color: darkblue}</style>";
  content += "<h1>IFTTT Switch</h1>Version 0.1 <br><br><h2>Configuration:</h2><br><h3>Local IP and MacAddress: </h3>";
  content += ipStr;
  content += " (";
  content += macStr;
  content += ")";
  content += "<p>";
  content += st;
  content += "<p><h3>Device IP address on Wifi Router:</h3>";
  content += ipwifiStr;
  content += "<p><br><h3>WiFi Router:</h3>Current Wifi SSID: ";
  content += WiFi.SSID();
  content += "  Status: ";
  if (testWifi()) {
    content += "<b><font color=\"green\"> CONNECTED</font></b><br>";
  } else {
     content += "<b><font color=\"red\"> DISCONNECTED</font></b><br>";     
    }
  
  //content += " - password: ";
  //content += password;
  content += "<p><form method='get' action='setwifi'><label>Wifi SSID: </label><input name='ssidnew' length=32><input name='passwordnew' length=64><input type='submit' value='Connect'></form>";
  content += "<br>";
  content += "<p><h3>Device Access Point SSID and Password can be changed:</h3>";
  content += "Current Access Point SSID and Password: ";
  content += ssidap;
  content += ", ";
  content += passwordap;
  content += "<form method='get' action='setap'><label>SSID: </label><input name='ssidapin' length=32><input name='passwordapin' length=64><input type='submit' value='Change'></form>";
  content += "<p>If this, turn off and on switch and then you will see a new network device (SSID), choose this Wifi device and reconnect.";
  content += "<br>";
  
  content += "<p><h3>IFTTT API Key:</h3></b>";
  content += "Current API Key: ";
  content += qapiKey; 
  content += "<form method='get' action='setapi'><label>API Key: </label><input name='apikey' length=25><input type='submit' value='Change API Key'></form>";

  content += "<p><h3>IFTTT Webhook switch naming:</h3></b>";
  content += "Current Event Names 1-4: ";
  content += qswitch1;
  content += ", ";
  content += qswitch2;
  content += ", ";  
  content += qswitch3;
  content += ", ";
  content += qswitch4;
  content += "<p>Device name: " + qdevname +"<p>";    
  content += "<form method='get' action='setswitches'><label>Event Names 1-4: </label><input name='switch1' length=3><input name='switch2' length=3><input name='switch3' length=3><input name='switch4' length=3><label>Device name: </label><input name='devname' length=3><input type='submit' value='Change Names'></form>";
  content += "<p> <a href=\"http://" + WiFi.softAPIP().toString()+"\">Refresh Configuration</a> ";
  
  content += "</html>";
//   serverap.send(200, "text/html", "<br>Version 0.1<p>");
  serverap.send(200, "text/html", content);
    
}

char* htmlBody_help = "<h1>Help</h1><br/>\n"
  "Visit http://192.168.4.1/ to access web server.<br/>\n"
  "Visit http://192.168.4.1/help to access this page.<br/>\n";

void handleHelp(){
  serverap.send(200, "text/html", htmlBody_help);
}


void handleSetAp(){
  // Setup Access Point
  
  int httpstatus = 200;
  String qsid = serverap.arg("ssidapin");
  String qpass = serverap.arg("passwordapin");

  
  if (qsid.length() > 0 && qpass.length() > 0) {
    
    for (int i = 0; i < qsid.length(); i++)
    {
      // Deal with (potentially) plus-encoded ssid
      qsid[i] = (qsid[i] == '+' ? ' ' : qsid[i]);
    }
    for (int i = 0; i < qpass.length(); i++)
    {
      // Deal with (potentially) plus-encoded password
      qpass[i] = (qpass[i] == '+' ? ' ' : qpass[i]);
    }
    
    //WiFi.mode(WIFI_AP_STA);
    Serial.println();
    Serial.println("Configuring access point...");
    Serial.println(qsid.c_str());
    Serial.println(qpass.c_str());

    
    //WiFi.softAP(qsid.c_str(), qpass.c_str());
    //delay(500);
    
    //WiFi.mode(WIFI_AP_STA);
    //WiFi.begin();
    ssidap = qsid;
    passwordap = qpass;

    if (save_setting()) {
    serverap.send(httpstatus, "text/html", content);
    }
    
  }
}

void handleSetSwitches(){
  // Set the switch variables
  
  int httpstatus = 200;
   qswitch1 = serverap.arg("switch1");
   qswitch2 = serverap.arg("switch2");
   qswitch3 = serverap.arg("switch3");
   qswitch4 = serverap.arg("switch4");
   qdevname = serverap.arg("devname");
   
      
//Save percistent parameters
//save_setting();

if (save_setting()) {
    serverap.send(httpstatus, "text/html", content);
}

}

void handleSetApi(){
  // Set the Hub IP and port
  
  int httpstatus = 200;
   qapiKey = serverap.arg("apikey");

//Save percistent parameters
//save_setting();

if (save_setting()) {
    serverap.send(httpstatus, "text/html", content);
}

}

void handleSetHub(){
  // Set the Hub IP and port
  
  int httpstatus = 200;
   qhubip1 = serverap.arg("hubip1");
   qhubip2 = serverap.arg("hubip2");
   qhubip3 = serverap.arg("hubip3");
   qhubip4 = serverap.arg("hubip4");
   qhubport = serverap.arg("hubport");

//Save percistent parameters
//save_setting();

if (save_setting()) {
    content = "<!DOCTYPE HTML>\n<html>";
    content += "Changed Smartthings Hub ";
    content += "<p><a href=\"http://192.168.4.1/\">Click here to return</a> </html>";
    serverap.send(httpstatus, "text/html", content);
}

}



void handleSetWifi(){
  // Connect to WiFi network
  
  int httpstatus = 200;
  String qsid = serverap.arg("ssidnew");
  String qpass = serverap.arg("passwordnew");

 // content = "<!DOCTYPE HTML>Trying to connect....\n<html>";
//        content += "</html>";
//          serverap.send(httpstatus, "text/html", content);
  
  if (qsid.length() > 0 && qpass.length() > 0) {
    
    for (int i = 0; i < qsid.length(); i++)
    {
      // Deal with (potentially) plus-encoded ssid
      qsid[i] = (qsid[i] == '+' ? ' ' : qsid[i]);
    }
    for (int i = 0; i < qpass.length(); i++)
    {
      // Deal with (potentially) plus-encoded password
      qpass[i] = (qpass[i] == '+' ? ' ' : qpass[i]);
    }
    
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(qsid.c_str(), qpass.c_str());
      serverap.begin();
    
    if (testWifi()) {
      Serial.println("\nGreat Success!");
      //  delay(3000);
     // abort();
    }
    
    if (testWifi()) {
      content = "<!DOCTYPE HTML>\n<html>";
      content += "Connected successfully to: <p> ";
      content += qsid.c_str();
      content += "<br>with password: <br> ";
      content += qpass.c_str();
      content += "<p><a href=\"http://192.168.4.1/\">Click here to return</a> </html>";
    } else {
      content = "<!DOCTYPE HTML>\n<html>";
      content += "Failed to connect to: <p>";
      content += qsid.c_str();
      content += "<br>with password: <br> ";
      content += qpass.c_str();
      content += ", try again.  <p> <a href=\"http://192.168.4.1/\">Click here to return</a> /html>";
    }
  } else {
      content = "<!DOCTYPE HTML><html>";
      content += "Error, no ssid or password set?</html>";
      content += qsid.c_str();
      content += "<br>with password: <br> ";
      content += qpass.c_str();
      Serial.println("Sending 404");
      httpstatus = 404;
  }
  serverap.send(httpstatus, "text/html", content);
}


const unsigned int serverPort = 9060; // port to run the http server on


bool read_setting();


int reportInterval = 600; // in secs


long debounceDelay = 10;    // the debounce time; increase if false positives


WiFiServer server(serverPort); //server

WiFiClient client; //client

String readString;



int rptCnt;
DHT dht(DHTPIN, DHTTYPE);


void setup() {

  Serial.begin(115200);

    Serial.println("ITFFF switch");
    delay(100);
    pinMode(pin, OUTPUT);
    pinMode(pin2, OUTPUT); 
    pinMode(pin3, OUTPUT); 
    pinMode(pin4, OUTPUT); 
    attachInterrupt(digitalPinToInterrupt(pin), PINStatus, FALLING);
    attachInterrupt(digitalPinToInterrupt(pin2), PINStatus2, FALLING);
    attachInterrupt(digitalPinToInterrupt(pin3), PINStatus3, FALLING);
    attachInterrupt(digitalPinToInterrupt(pin4), PINStatus4, FALLING);

  Serial.println("\nVStarting SPIFFS");
  SPIFFS.begin();
  // Next lines have to be done ONLY ONCE!!!!!When SPIFFS is formatted ONCE you can comment these lines out!!
 //  Serial.println("Please wait 30 secs for SPIFFS to be formatted");
 //  SPIFFS.format();
 //  Serial.println("Spiffs formatted");


  // Read percistent parameters
  read_setting();

  //Setup the Access Point 
    Serial.println();

    ssidap.trim();
    passwordap.trim();
    if (ssidap != NULL) {
      Serial.println("Configuring access point...");
      Serial.println(ssidap +", "+ passwordap);
      WiFi.softAP(ssidap.c_str(), passwordap.c_str());
      Serial.println(WiFi.softAP(ssidap.c_str(), passwordap.c_str()) ? "Ready" : "Failed!");
       //WiFi.softAP(ssidapp, passwordapp);
    }
    delay(500);

    IPAddress apip = WiFi.softAPIP();
    Serial.print("visit: \n");
    Serial.println(apip);
 
    
    serverap.on("/", handleRoot);
    serverap.on("/help", handleHelp);
    serverap.on("/setwifi", handleSetWifi);
    serverap.on("/setap", handleSetAp);
    serverap.on("/sethub", handleSetHub);
    serverap.on("/setapi", handleSetApi);     
    serverap.on("/setswitches", handleSetSwitches);     
    serverap.begin();
    Serial.println("HTTP server beginned");


  // prepare GPIOs

  pinMode(12, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin();

  dht.begin();
  server.begin();

  rptCnt=reportInterval;
  Serial.println(WiFi.localIP());
  String s = WiFi.macAddress();
  Serial.println(s);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();

  delay(2000); 
  display.clearDisplay();

    // text display tests
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("IFTTT");
  display.setTextSize(1);
  display.display();
  delay(2000);
  if (testWifi()) {
         display.println("Connected to Wifi");
         display.println(WiFi.SSID());
         display.display();
  }
  if (ssidap != NULL) {
        display.println("SSID: "+ ssidap);
    } else {
          display.println("SSID: (default) ");
    }
  display.println("APIP: "+ WiFi.softAPIP().toString());
  display.println("WIFI: "+ WiFi.localIP().toString());

  delay(2000);
  display.display();
}


bool testWifi(void) {
  int c = 0;
  Serial.println("\nWaiting for Wifi to connect...");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
       Serial.println("\nConnected to Wifi...");
    }
    delay(500);
    Serial.print(WiFi.status());
    c++;
  }
  //Serial.println("\nConnect timed out, opening AP");
  //return false;
}


// Save and read precistent data such as Hub parameters.  Will survive power cycle.
bool save_setting() {
  // open file for writing
  File f = SPIFFS.open("/f.txt", "w");
  if (!f) {
      Serial.println("file open failed");
      return false;
  } else {
  Serial.println("====== Writing to SPIFFS file =========");
  // write 
    f.println(qhubip1); 
    f.println(qhubip2); 
    f.println(qhubip3); 
    f.println(qhubip4);
    f.println(qhubport);
    f.println(qapiKey);
    f.println(qswitch1);
    f.println(qswitch2);
    f.println(qswitch3);
    f.println(qswitch4);
    f.println(ssidap);  
    f.println(passwordap);
    f.println(qdevname);
   f.close();
    Serial.println("Setting hub spiffs");
    Serial.println(qhubip1.toInt());
    Serial.println(qhubip2.toInt());
    Serial.println(qhubip3.toInt());
    Serial.println(qhubip4.toInt());
    Serial.println(qapiKey);
    Serial.println(qswitch1);
    Serial.println(qswitch2);
    Serial.println(qswitch3);
    Serial.println(qswitch4);
    Serial.println(qdevname);
   return true;
  }
}

bool read_setting() {
  // open file for reading
 File f = SPIFFS.open("/f.txt", "r");
  if (!f) {
      Serial.println("file open failed");
      return false;
  }  
 else { 
  Serial.println("====== Reading from SPIFFS file =======");
  // write strings to file
    qhubip1=f.readStringUntil('\n');
    qhubip2=f.readStringUntil('\n');
    qhubip3=f.readStringUntil('\n');
    qhubip4=f.readStringUntil('\n');
    qhubport=f.readStringUntil('\n');
    qapiKey=f.readStringUntil('\n');
    qswitch1=f.readStringUntil('\n');
    qswitch2=f.readStringUntil('\n');
    qswitch3=f.readStringUntil('\n');
    qswitch4=f.readStringUntil('\n');
    ssidap=f.readStringUntil('\n');
    passwordap=f.readStringUntil('\n');
    qdevname=f.readStringUntil('\n');
    f.close();
    Serial.println("Setting hub spiffs write");
    Serial.println(qhubip1.toInt());
    Serial.println(qhubip2.toInt());
    Serial.println(qhubip3.toInt());
    Serial.println(qhubip4.toInt());
    Serial.println(qapiKey);
    Serial.println(qswitch1);
    Serial.println(qswitch2);
    Serial.println(qswitch3);
    Serial.println(qswitch4);
    Serial.println(qdevname);

    return true;
 }
}


float getTemp() {

  float temp;

  do {

    //read temp
    temp = dht.readTemperature(true);
    
    Serial.print("Temperature: ");

    Serial.println(temp);

  } while (temp == 85.0 || temp == (-127.0));

  return temp;

}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
} 




float getLight() {

  float light, lightsens;

  
    lightsens = analogRead(A0);
    light = 0;

    if (lightsens >= 800) {
      light = 3;
    } else if (lightsens >= 400) {
       light = 2;
    } else if (lightsens >= 0) {
      light = 1;
    }
  
    Serial.print("Light: ");

    Serial.println(light);


  return lightsens;

}



void loop() {

  serverap.handleClient();

//ifffcode here

          if(flag){
          Serial.print("connecting to ");
          Serial.println(host);
          
          WiFiClient client;
          const int httpPort = 80;
          if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
            display.println("connect IFTTT failed");
            display.display();
            return;
          }
    
          qapiKey.trim();
          qswitch1.trim();
          String url = "/trigger/"+qswitch1+"/with/key/"+qapiKey;

          Serial.print("Requesting URL: ");
          Serial.println(url);
          client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                       "Host: " + host + "\r\n" + 
                       "Content-Type: application/x-www-form-urlencoded\r\n" + 
                       "Content-Length: 23\r\n\r\n" +
                       "value1=" + PIN_state + "\r\n");

          if (qswitch1 != NULL){
            display.println(qswitch1+" press");
            display.display();
            delay(5000); //delay for screen before refreshing
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(WHITE);
            display.setCursor(0,0);
            if (qdevname == NULL) {
              display.println("IFTTT");
            } else {
              display.println(qdevname);              
            }
            display.setTextSize(1);
            display.display();
          } else {
            display.println("switch name not set");
            display.display();  
          }
          
          flag = false;
      }
      if(flag2){
          Serial.print("connecting to ");
          Serial.println(host);
          
          WiFiClient client;
          const int httpPort = 80;
          if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
            return;
          }
    
          qapiKey.trim();
          qswitch2.trim();
          String url = "/trigger/"+qswitch2+"/with/key/"+qapiKey;
          
          Serial.print("Requesting URL: ");
          Serial.println(url);
          client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                       "Host: " + host + "\r\n" + 
                       "Content-Type: application/x-www-form-urlencoded\r\n" + 
                       "Content-Length: 13\r\n\r\n" +
                       "value1=" + PIN_state2 + "\r\n");
          if (qswitch2 != NULL){ 
            display.println(qswitch2+" press");
            display.display();
            delay(5000); //delay for screen before refreshing
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(WHITE);
            display.setCursor(0,0);
            if (qdevname == NULL) {
              display.println("IFTTT");
            } else {
              display.println(qdevname);              
            }
            display.setTextSize(1);
            display.display();  
          } else {
            display.println("switch name not set");
            display.display();  
          }
                       
          flag2 = false;
      }
      if(flag3){
          Serial.print("connecting to ");
          Serial.println(host);
          
          WiFiClient client;
          const int httpPort = 80;
          if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
            return;
          }
    
          qapiKey.trim();
          qswitch3.trim();
          String url = "/trigger/"+qswitch3+"/with/key/"+qapiKey;
          
          Serial.print("Requesting URL: ");
          Serial.println(url);
          client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                       "Host: " + host + "\r\n" + 
                       "Content-Type: application/x-www-form-urlencoded\r\n" + 
                       "Content-Length: 13\r\n\r\n" +
                       "value1=" + PIN_state3 + "\r\n");

           if (qswitch3 != NULL){
            display.println(qswitch3+" press");
            display.display();
            delay(5000); //delay for screen before refreshing
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(WHITE);
            display.setCursor(0,0);
            if (qdevname == NULL) {
              display.println("IFTTT");
            } else {
              display.println(qdevname);              
            }
            display.setTextSize(1);
            display.display();  
          } else {
            display.println("switch name not set");
            display.display();  
          }
          
          flag3 = false;
      }
      if(flag4){
          Serial.print("connecting to ");
          Serial.println(host);
          
          WiFiClient client;
          const int httpPort = 80;
          if (!client.connect(host, httpPort)) {
            Serial.println("connection failed");
            return;
          }
    
          qapiKey.trim();
          qswitch4.trim();
          String url = "/trigger/"+qswitch4+"/with/key/"+qapiKey;
          
          Serial.print("Requesting URL: ");
          Serial.println(url);
          client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                       "Host: " + host + "\r\n" + 
                       "Content-Type: application/x-www-form-urlencoded\r\n" + 
                       "Content-Length: 13\r\n\r\n" +
                       "value1=" + PIN_state4 + "\r\n");

          if (qswitch4 != NULL){
            display.println(qswitch4+" press");
            display.display();
            delay(5000); //delay for screen before refreshing
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(WHITE);
            display.setCursor(0,0);
            if (qdevname == NULL) {
              display.println("IFTTT");
            } else {
              display.println(qdevname);              
            }
            display.setTextSize(1);
            display.display();  
          } else {
            display.println("switch name not set");
            display.display();  
          }
                       
          flag4 = false;
      }  
      delay(100);

}