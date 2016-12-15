#include <A6lib.h>
#include <A6httplib.h>

// Instantiate the library with TxPin, RxPin.
A6lib A6l(7, 8);
A6httplib httprequest(&A6l);

//  Pin number by which the power of module is controlled.
#ifndef ESP8266
  #define module_powerpin 0
#else
  #define module_powerpin D0
#endif

String apn="airtelworld.com";
String host="ipinfo.io";
String path="/ip";


void setup() 
{
    Serial.begin(115200);
    delay(1000);
    // Power-cycle the module to reset it.
//    A6l.powerCycle(module_powerpin);
    A6l.blockUntilReady(9600);

    httprequest.ConnectGPRS(apn);
    String rcvd_data=httprequest.Get(host,path);
//    Serial.println(rcvd_data);
    httprequest.getResponseData(rcvd_data);

}

void loop() 
{
    // Relay things between Serial and the module's SoftSerial.
    while (A6l.A6conn->available() > 0) {
        Serial.write(A6l.A6conn->read());
    }
    while (Serial.available() > 0) {
        A6l.A6conn->write(Serial.read());
    } 
}
