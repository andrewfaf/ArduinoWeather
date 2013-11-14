/*
  Repeating Web client
 
 This sketch connects to a a web server and makes a request
 using a Wiznet Ethernet shield. You can use the Arduino Ethernet shield, or
 the Adafruit Ethernet shield, either one will work, as long as it's got
 a Wiznet Ethernet module on board.
 
 This example uses DNS, by assigning the Ethernet client with a MAC address,
 IP address, and DNS address.
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 19 Apr 2012
 by Tom Igoe
 
 http://arduino.cc/en/Tutorial/WebClientRepeating
 This code is in the public domain.
 
 */
 
/*
ASF - 12Nov13 7:03pm
Working with Temperature passed as float to get_http()
text    data   bss  dec
15418   204    309  15931
7:14pm
text    data   bss  dec
15506   204    309  16019

13Nov13 4:36pm
Changing sampling to every 30 seconds
text    data   bss  dec
15506   204    309  16019
5:09pm
Added a state variable to alternate temp and humidity
text    data   bss  dec
15778   260    311  16349

9:14pm
Commented out the Temperature part and successfully uploading humidity
text    data   bss  dec
16112   216    309  16637

9:38pm
Fixed bug where I did double requests, simplified
text    data   bss  dec
15802   284    311  16397

14Nov14 4:05pm
Got rid of Strings and made static char arrays instead stored in progmem.
This should solve bad memory behaviour in Strings objects when concatenating.
text    data   bss  dec
13892   292    313  14497

6:00pm
Changed strings to reduce duplication and added Rain
text    data   bss  dec
13954   302    315  14497

10:06pm
Seems to be working now
text    data   bss  dec
14184   272    335  14791

*/
 
#include "DHT.h"
#include <SPI.h>
#include <Ethernet.h>
#include <avr/pgmspace.h>
#include <PString.h>

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);

#define  RAINPIN  5

// assign a MAC address for the ethernet controller.
// fill in your address here:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0x2E, 0xED};
// fill in an available IP address on your network here,
// for manual configuration:
IPAddress ip(192,168,2,57);

// fill in your Domain Name Server address here:
IPAddress myDns(192,168,2,1);

// initialize the library instance:
EthernetClient client;

//char server[] = "nimbits-02.appspot.com";
char server[] = "google.com";

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 30000;  // delay between updates, in milliseconds
int  stateVar = 0;
float val = 0;
int  nimbitsLength = 0;
int  rainCnt = 0;

PROGMEM char*  nimbitspreStr[] = {"email=afrawley.af@gmail.com&key=rsw2476&id="};   //43
PROGMEM char*  nimbitstempStr[] = {"Temperature&json="};                            //17
PROGMEM char*  nimbitshumidStr[] = {"Humidity&json="};                              //14
PROGMEM char*  nimbitsrainStr[] = {"Rain&json="};                                  //10
PROGMEM char*  nimbitsjsonpreStr[] = {"{\"d\":\""};                                 //6
PROGMEM char*  nimbitsjsonpostStr[] = {"\",\"n\":\"\"}"};                           //9

char buffer[10];
PString  valStr(buffer,sizeof(buffer));

void setup() {
  pinMode(RAINPIN,INPUT_PULLUP);
  dht.begin();
  // start serial port:
  Serial.begin(115200);
  // give the ethernet module time to boot up:
  delay(1000);
  // start the Ethernet connection using a fixed IP address and DNS server:
  Ethernet.begin(mac, ip, myDns);
  // print the Ethernet board/shield's IP address:
  Serial.print(F("My IP address: "));
  Serial.println(Ethernet.localIP());
}

void loop() {
  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
int  dotCnt = 0;

  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  if (Serial.available()) {
    char c = Serial.read();
    Serial.println(c);
    if (c =='r'){
      rainCnt = 0;
    }
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println(F("disconnecting."));
    client.stop();
  }
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    Serial.println();
    Serial.print(F("Making Request: "));
    
    switch (stateVar){
      case 0:
        stateVar = 1;
        val = dht.readTemperature();
        nimbitsLength = 75;
        Serial.println("Temperature");
        httpRequest(*nimbitstempStr);
      break;
      
      case 1:
        stateVar = 2;
        val = dht.readHumidity();
        nimbitsLength = 72;
        Serial.println("Humidity");
        httpRequest(*nimbitshumidStr);
      break;
      
      default:
        val = rainCnt;
        nimbitsLength = 68;
        Serial.println("Rainfall");
        httpRequest(*nimbitsrainStr);
        stateVar = 0;
    }

  }
  else{
    if(dotCnt++ == 50){
      dotCnt = 0;
      Serial.print(".");
    }
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();

  if (digitalRead(RAINPIN) == 0){
    rainCnt++;
    delay (250);
  }  
}

// this method makes a HTTP connection to the server:
void httpRequest(char* nimbitsStr) {
  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println(F("connecting..."));
    // send the HTTP PUT request:
    client.println(F("POST /service/v2/value HTTP/1.1"));
    client.println(F("Host:nimbits-02.appspot.com"));
    client.println(F("Connection:close"));
    client.println(F("User-Agent: Arduino/1.0"));
    client.println(F("Cache-Control:max-age=0"));
    client.println(F("Content-Type: application/x-www-form-urlencoded"));
    client.print(F("Content-Length: "));

    valStr.begin();
    valStr.print(val,4);
//    valStr += "\0";

    client.println(nimbitsLength + valStr.length());
    client.println();
//    Serial.println(nimbitsLength + valStr.length());
//    Serial.println();

    client.print(*nimbitspreStr);
    client.print(nimbitsStr);
    client.print(*nimbitsjsonpreStr);
    client.print(valStr);
    client.println(*nimbitsjsonpostStr);

//    Serial.print(*nimbitspreStr);
//    Serial.print(nimbitsStr);
//    Serial.print(*nimbitsjsonpreStr);
//    Serial.print(valStr);
//    Serial.println(*nimbitsjsonpostStr);
//    Serial.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println(F("connection failed"));
    Serial.println(F("disconnecting."));
    client.stop();
  }
}

//String floatToString(double number, uint8_t digits) 
//{ 
//  String resultString = "";
//  // Handle negative numbers
//  if (number < 0.0)
//  {
//    resultString += "-";
//    number = -number;
//  }
//
//  // Round correctly so that print(1.999, 2) prints as "2.00"
//  double rounding = 0.5;
//  for (uint8_t i=0; i<digits; ++i)
//    rounding /= 10.0;
//
//  number += rounding;
//
//  // Extract the integer part of the number and print it
//  unsigned long int_part = (unsigned long)number;
//  double remainder = number - (double)int_part;
//  resultString += int_part;
//
//  // Print the decimal point, but only if there are digits beyond
//  if (digits > 0)
//    resultString += "."; 
//
//  // Extract digits from the remainder one at a time
//  while (digits-- > 0)
//  {
//    remainder *= 10.0;
//    int toPrint = int(remainder);
//    resultString += toPrint;
//    remainder -= toPrint; 
//  } 
//  return resultString;
//}
//

