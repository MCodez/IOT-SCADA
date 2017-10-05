// initialisation of all variables

#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
const char* ssid = "SSID HERE";
const char* password = "PASSWORD HERE";
const char* servers = "api.thingspeak.com";
String apiKey ="THINGSPEAK API KEY";
int ledPin = 2; // GPIO5
WiFiServer server(80);
const unsigned long period = 1000;
const char *GScriptId = "GOOGLE SCRIPT ID ";
const int dataPostDelay = 2000;  // 15 minutes = 15 * 60 * 1000
const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";
const int httpsPort =  443;
HTTPSRedirect client(httpsPort);
String url = String("/macros/s/") + GScriptId + "/exec?";
const char* fingerprint = "F0 5C 74 77 3F 6B 25 D7 3B 66 4D 43 2F 7E BC 5B E9 28 86 AD";
volatile int flow_frequency1;
volatile int flow_frequency2;
unsigned int l_hour1; 
unsigned int l_hour2;
unsigned char flowsensor1 = 4; 
unsigned char flowsensor2 = 5;
unsigned long currentTime;
unsigned long cloopTime;


// functions definitions


void flow1 () // Interrupt function
{
   flow_frequency1++;
}

void flow2 () // Interrupt function
{
   flow_frequency2++;
}

void postData(String tag1, float value1,String tag2,float value2){
  if (!client.connected()){
    Serial.println("Connecting to client again..."); 
    client.connect(host, httpsPort);
  }
  String urlFinal = url + "tag=" + tag1 + "&value=" + String(value1)+ "tag=" + tag2 + "&value=" + String(value2);
  client.printRedir(urlFinal, host, googleRedirHost);
}


void setup()
{
   Serial.begin(11500);
   Serial.println("Connecting to wifi: ");
    Serial.println(ssid);
    Serial.flush();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" IP address: ");
  Serial.println(WiFi.localIP());
   Serial.print(String("Connecting to "));
  Serial.println(host);
   bool flag = false;
  for (int i=0; i<5; i++){
    int retval = client.connect(host, httpsPort);
    if (retval == 1) {
       flag = true;
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  Serial.println("Connection Status: " + String(client.connected()));
  Serial.flush();
  
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    Serial.flush();
    return;
  }

  // Data will still be pushed even certification don't match.
  if (client.verify(fingerprint, host)) {
    Serial.println("Certificate match.");
  } else {
    Serial.println("Certificate mis-match");
  }
   
   
   pinMode(ledPin, OUTPUT);
   digitalWrite(ledPin, LOW);
   pinMode(flowsensor1, INPUT);
   digitalWrite(flowsensor1, HIGH); // Optional Internal Pull-Up
   pinMode(flowsensor1, INPUT);
   digitalWrite(flowsensor1, HIGH); // Optional Internal Pull-Up
   attachInterrupt(4, flow1, RISING); // Setup Interrupt
   attachInterrupt(5, flow2, RISING); // Setup Interrupt
   sei(); // Enable interrupts
   currentTime = millis();
   cloopTime = currentTime;
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
  
  int value =0;
  currentTime = millis();
   if(currentTime >= (cloopTime + 1000))
   {
      cloopTime = currentTime; 
      l_hour1 = (flow_frequency1 * 60 / 7.5); 
      flow_frequency1 = 0; 
      Serial.print(l_hour1, DEC); 
      Serial.println(" L/hour");
      l_hour2 = (flow_frequency2 * 60 / 7.5);
      flow_frequency2 = 0; 
      Serial.print(l_hour2, DEC); 
      Serial.println(" L/hour");
   }

  if (request.indexOf("/LED=ON") != -1)  {
    digitalWrite(ledPin, HIGH);
    
  }
  if (request.indexOf("/LED=OFF") != -1)  {
    digitalWrite(ledPin, LOW);
  
  }
  //if (request.indexOf("/LED=O") != -1)  {
    
   // Meter.tick(period);
    //waterflow=Meter.getCurrentFlowrate();
    //if(pot>=1000 || waterflow<=0)
    //{
     // digitalWrite(ledPin,LOW);
   // }
    //else
    //{
     // digitalWrite(ledPin,HIGH);
    //}
    
  //}
 
   //int f=(pot/10);
   postData("Water Flow 1",l_hour1,"Water Flow 2",l_hour2);
delay (dataPostDelay);
   client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<br><br>");
  client.print("Motor is now: ");
  value=digitalRead(ledPin);
  if (value==0)
  {
   client.print("STOPPED"); 
  }
  else
  {
    client.print("RUNNING");
  }
  client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>Turn On </button></a>");
  client.print("    ");
  client.println("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />");
  client.println("<br><br>");
 // client.println("<a href=\"/LED=O\"\"><button>Refresh </button></a><br />");
  //client.println("<br><br>");
  client.print("Tank Fill  ");
  //client.print(f);  
  //client.println(" %");
  client.println("<br><br>");
 // client.print("Water Flow Rate  ");
  //client.print(String(waterflow));
  client.println("<br><br>");
  client.println("<a href=\"https://thingspeak.com/channels/171957\"> Analyse Real Time Tank Level</a>");
  //sendTeperatureTS(f);
  //client.print("Pot value ");
  //client.print(pot);
  client.println("</html>");
 delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
 }
/*void sendTeperatureTS(int temp)
{  
   WiFiClient client;
  
   if (client.connect(servers, 80)) { // use ip 184.106.153.149 or api.thingspeak.com
   Serial.println("WiFi Client connected ");
   
   String postStr = apiKey;
   postStr += "&field1=";
   postStr += String(temp);
   postStr += "\r\n\r\n";
   client.print("POST /update HTTP/1.1\n");
   client.print("Host: api.thingspeak.com\n");
   client.print("Connection: close\n");
   client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
   client.print("Content-Type: application/x-www-form-urlencoded\n");
   client.print("Content-Length: ");
   client.print(postStr.length());
   client.print("\n\n");
   client.print(postStr);
   delay(1000);
   
   }
   sent++;
 client.stop();
}*/
 // https://thingspeak.com/channels/171957