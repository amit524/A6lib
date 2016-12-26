#ifndef A6httplib_h
#define A6httplib_h

#include <Arduino.h>

class A6httplib {
private:
    String _host;
    int _port;
    String _path;
    String _apn;
    A6lib *_A6l;
    int _ResponseLength;

public:
    A6httplib();
    A6httplib(A6lib *);

    ~A6httplib();

    bool ConnectGPRS(String apn);

    bool HTTPPostInitiate(String host, String path);
    void AddHeader(String);
    void HTTPPostRequest(const char *);

    String Get(String host, String path);
    String getResponseData(String);
    int getResponseLength();

    void CloseTCPConn();

};
#endif


