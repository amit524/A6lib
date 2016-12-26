#include <Arduino.h>
#include <A6lib.h>
#include "A6httplib.h"

String dummy_string = "";
String body_rcvd_data = "";
byte var = A6_NOTOK;
String rcv_str = "+CIPRCV";

A6httplib::A6httplib(A6lib *A6l) {
    _A6l = A6l;

}

A6httplib::~A6httplib() {

}


bool A6httplib::HTTPPostInitiate(String host, String path) {
    _host = host;
    _path = path;

    _port = 80;
    byte var = A6_NOTOK;

    dummy_string = "AT+CIPSTART=\"TCP\",\"" + _host + "\"," + _port;
    while (var != A6_OK) {
        //close prior connections if any.
        _A6l->A6command("AT+CIPCLOSE", "OK", "yy", 2000, 2, NULL); //start up the connection

        var = _A6l->A6command(dummy_string.c_str(), "CONNECT OK", "yy", 30000, 1, NULL); //start up the connection
        logln(var);
        while (_A6l->getFree(3000) != 0);
        _A6l->A6command("AT+CSQ", "OK", "yy", 2000, 2, NULL); //start up the connection
        logln("AT+CSQ");
        delay(200);
        while (_A6l->getFree(3000) != 0);

    }

    logln(F("checking current staus : issued cipstart command"));
    _A6l->A6command((const char *)"AT+CIPSTATUS", "OK", "yy", 10000, 2, NULL);
    logln(F("AT+CIPSTATUS"));
    _A6l->A6command((const char *)"AT+CIPSEND", ">", "yy", 10000, 1, NULL); //begin send data to remote server
    logln(F("AT+CIPSEND"));
    delay(500);

    dummy_string = "POST " + _path;
    _A6l->A6conn->write(dummy_string.c_str());
    log("POST " + _path);
    _A6l->A6conn->write(" HTTP/1.1");
    log(F(" HTTP/1.1"));
    _A6l->A6conn->write("\r\n");
    log("\r\n");

    _A6l->A6conn->write("User-Agent: A6 Modem");
    log(F("User-Agent: A6 Modem"));
    _A6l->A6conn->write("\r\n");
    log("\r\n");

    _A6l->A6conn->write("HOST: ");
    log(F("HOST: "));
    _A6l->A6conn->write(_host.c_str());
    log(_host);
    _A6l->A6conn->write("\r\n");
    log("\r\n");

}

void A6httplib::AddHeader(String header) {
    _A6l->A6conn->write(header.c_str());
    _A6l->A6conn->write("\r\n");
    logln(header.c_str()) ;
}


void A6httplib::HTTPPostRequest(const char *postbody) {
    char end_c[2];
    end_c[0] = 0x1a;
    end_c[1] = '\0';

    int i = 0;
    int PostBodyLength = 0;
    while (postbody[i]) {
        PostBodyLength++;
        i++;
    }
    log("body content length is\t");
    logln(PostBodyLength);

    dummy_string = "Content-Type: application/json";
    _A6l->A6conn->write(dummy_string.c_str());
    _A6l->A6conn->write("\r\n");
    logln(dummy_string.c_str()) ;

    dummy_string = "Content-Length: ";
    dummy_string += String(PostBodyLength);
    _A6l->A6conn->write(dummy_string.c_str());
    _A6l->A6conn->write("\r\n");

    logln(dummy_string.c_str()) ;
    _A6l->A6conn->write("Connection: close");
    log(F("Connection: close"));

    _A6l->A6conn->write("\r\n");
    _A6l->A6conn->write("\r\n");
    logln();
    logln();

    _A6l->A6conn->write(postbody);
    log(postbody);
    _A6l->A6conn->write("\r\n");
    log("\r\n");
    _A6l->A6command(end_c, "OK", "yy", 30000, 1, NULL); //begin send data to remote server
    long ptime = millis();
    int response_bytes = 0;
    while (millis() - ptime < TIMEOUT_HTTP_RESPONSE && response_bytes < EXPECTED_RESPONSE_LENGTH) {
        if (_A6l->A6conn->available()) {
            log((char)_A6l->A6conn->read());
            response_bytes++;
        }

        //do further things here like storing data in some variable.
    }
    CloseTCPConn();

}


bool A6httplib::ConnectGPRS(String apn) {
    String dummy_string = "";
    String _APN = apn;

    byte retstatus = 1;

    while (retstatus != A6_OK) {
        retstatus = _A6l->A6command((const char *)"AT+CGATT?", "OK", "yy", 20000, 1, NULL);
        while (_A6l->getFree(3000) != 0);
    }
    retstatus = 1;

    while (retstatus != A6_OK) {
        retstatus = _A6l->A6command((const char *)"AT+CGATT=1", "OK", "yy", 20000, 2, NULL);
        while (_A6l->getFree(3000) != 0);
    }
    retstatus = 1;

    _A6l->A6conn->write("AT+CSQ"); //Signal Quality
    logln("AT+CSQ");
    delay(200);
    while (_A6l->getFree(3000) != 0);

    while (retstatus != A6_OK) {
        dummy_string = "AT+CGDCONT=1,\"IP\",\"" + _APN + "\"";
        retstatus = _A6l->A6command(dummy_string.c_str(), "OK", "yy", 20000, 2, NULL); //bring up wireless connection
        while (_A6l->getFree(3000) != 0);
    }
    retstatus = 1;

    while (retstatus != A6_OK) {
        retstatus = _A6l->A6command((const char *)"AT+CGACT=1,1", "OK", "yy", 10000, 2, NULL);
        while (_A6l->getFree(3000) != 0);
        // delay(10000);
    }
}

String A6httplib::Get(String host, String path) {
//  body_rcvd_data.reserve(1000);
    _path = path;
    _host = host;
    _port = 80;
    char end_c[2];
    end_c[0] = 0x1a;
    end_c[1] = '\0';

    String rcv = "";
    rcv.reserve(50);

    char ch = '\0';
    int count_incoming_bytes = 0;
    String bytes_in_body = "";
    int bytes_read = 0;

    var = A6_NOTOK;

    dummy_string = "AT+CIPSTART=\"TCP\",\"" + _host + "\"," + _port;
    while (var != A6_OK) {
        //close prior connections if any.
        _A6l->A6command("AT+CIPCLOSE", "OK", "yy", 2000, 2, NULL); //start up the connection

        var = _A6l->A6command(dummy_string.c_str(), "CONNECT OK", "yy", 30000, 1, NULL); //start up the connection
        //logln(dummy_string.c_str());
        logln(var);
        while (_A6l->getFree(3000) != 0);
        _A6l->A6command("AT+CSQ", "OK", "yy", 2000, 2, NULL); //start up the connection
        logln(F("AT+CSQ"));
        delay(200);
        while (_A6l->getFree(3000) != 0);

    }

    logln(F("checking current staus : issued cipstart command"));
    _A6l->A6command((const char *)"AT+CIPSTATUS", "OK", "yy", 10000, 2, NULL);
    logln(F("AT+CIPSTATUS"));
    _A6l->A6command((const char *)"AT+CIPSEND", ">", "yy", 10000, 1, NULL); //begin send data to remote server
    logln(F("AT+CIPSEND"));
    delay(500);


    dummy_string = "GET " + _path;
    _A6l->A6conn->write(dummy_string.c_str());
    log("GET " + _path);
    _A6l->A6conn->write(" HTTP/1.1");
    log(F(" HTTP/1.1"));
    _A6l->A6conn->write("\r\n");
    log(F("\r\n"));
    _A6l->A6conn->write("HOST: ");
    log(F("HOST: "));
    _A6l->A6conn->write(_host.c_str());
    log(_host);
    _A6l->A6conn->write("\r\n\r\n");
    log(F("\r\n"));


    _A6l->A6command(end_c, "OK", "yy", 30000, 1, NULL); //begin send data to remote server
    _A6l->A6conn->write(end_c);


    while (1) {
        if (_A6l->A6conn->available()) {
            rcv = "";
            rcv = _A6l->A6conn->readStringUntil('\r');
            int index = rcv.indexOf('+');
            if (index != -1) {
                int colon_index = rcv.indexOf(':');
                String cutfromrecvd = rcv.substring(index, colon_index);
                if (cutfromrecvd == rcv_str) {
                    logln(F("\nrecieving data"));
                }
                int comma_index = rcv.indexOf(',');
                if (comma_index != -1) {
                    bytes_in_body = rcv.substring(colon_index + 1, comma_index);
                    _ResponseLength = bytes_in_body.toInt();
                    int no_of_bytes = _ResponseLength;
                    long now = millis();
                    long ntime = 0;
                    while (1) {
                        long ntime = millis();
                        if (ntime - now > 10000 || (bytes_read == no_of_bytes)) {
                            break;
                        }
                        if (_A6l->A6conn->available()) {
                            body_rcvd_data += (char) _A6l->A6conn->read();
                            bytes_read += 1;
                        }
                    }
                    break;
                } else {
                    logln(F("Incorrect Data found"));
                }
            }
        }


    }
    logln(body_rcvd_data);
    CloseTCPConn();
    return body_rcvd_data;
}

int A6httplib::getResponseLength() {
    return _ResponseLength;
}

void A6httplib::CloseTCPConn() {
    logln(F("now going to close tcp connection"));
    _A6l->A6command((const char *)"AT+CIPCLOSE", "OK", "yy", 10000, 2, NULL);
    delay(100);
    logln(F("closed"));
    _A6l->A6command((const char *)"AT+CIPSTATUS", "OK", "yy", 10000, 1, NULL);
}


String A6httplib::getResponseData(String body_rcvd_data) {
    int body_content_start = 0;
    int body_content_start_content = 0;
    int body_content_end = 0;

    int bytes_read = strlen(body_rcvd_data.c_str());
    logln(bytes_read);
    body_content_start = body_rcvd_data.indexOf("\r\n\r\n");

    log(F("found double newline at  "));
    logln(body_content_start);
    // int body_content_end=0;
    if (body_content_start != -1) {
        for (int i = body_content_start; i < bytes_read; i++)
            if (isAlphaNumeric(body_rcvd_data[i])) {
                body_content_start_content = i;
            }
        body_content_end = bytes_read;
        log(F("body content start at "));
        logln(body_content_start_content + 1);
        log(F("body content end at  "));
        logln(body_content_end);
        String body_content = body_rcvd_data.substring(body_content_start + 4, body_content_end);
        logln(body_content);
    } else {
        logln("bad body content");
    }
    return "\0";
}
