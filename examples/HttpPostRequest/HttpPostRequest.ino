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


// Your cloud provider specific keys or IDs
String X_IOTFY_ID 		= "XXXXX";
String X_IOTFY_CLIENT 	= "XXXXX";

// End point Path to your Host
String PostRequestHost="cloud.iotfy.co";
String PostRequestPath="/api/telemetry/v1/post_text_data";


void setup() 
{
  
    Serial.begin(115200);
    delay(1000);

// Power-cycle the module to reset it.
    A6l.powerCycle(module_powerpin);
    A6l.blockUntilReady(9600);

    httprequest.ConnectGPRS(apn);
    httprequest.HTTPPostInitiate(PostRequestHost, PostRequestPath);
    String header="X-IOTFY-ID: "+ X_IOTFY_ID;
    httprequest.AddHeader(header);
    
    header="X-IOTFY-CLIENT: " + X_IOTFY_CLIENT;
    httprequest.AddHeader(header);


    String group_iotfy="DemoGroup";
    String id_iotfy   ="D01";
    String tag_iotfy  ="TempSensor Data";
    
    String data="{\"group\":\"";
    data+=group_iotfy;
    data+="\", \"device\":\"";
    data+=id_iotfy;
    data+="\", \"data\":";
    data+="[{\"tag\":\"";
    data+=tag_iotfy;
    data+="\",\"text\":\"";
    data+=String(25);       // any sample value
    data+="\"}]}";
    const char *PostRequestBody=data.c_str();
    Serial.println(PostRequestBody);
    Serial.println("printed");
    httprequest.HTTPPostRequest(PostRequestBody);
    
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
