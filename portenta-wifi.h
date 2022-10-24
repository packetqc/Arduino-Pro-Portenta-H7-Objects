#ifndef MPWL_H
#define MPWL_H

#include <WiFi.h>

#include "arduino_secrets.h"

//===================================================================================================
// WIFI OBJECT: CLIENT OR SERVER
//===================================================================================================
class MPWL {
  private:
    String ssid  = SECRET_SSID;
    String pass  = SECRET_PASS;
    int keyIndex = 0;
    int status   = WL_IDLE_STATUS;
    
    WiFiServer *server;
    WiFiClient *webclient;

    String WebServerConnect = "192.168.3.1";
      
    bool statusWL;
    bool statusConnection;
    bool statusDataEncryption;

    MPWL() = default; // Make constructor private  

  public:
    //SINGLETON
    static MPWL &getInstance(); // Accessor for singleton instance
    MPWL(const MPWL &) = delete; // no copying
    MPWL &operator=(const MPWL &) = delete;

  public:    
    //SERVER
    bool Init();
    void Run();

    //CLIENT
    bool Connect();
    bool Disconnect();
    bool Interact();

    //STATUS
    void PrintWiFiStatus();
    bool StatusWL();
    bool StatusConnection();
    bool StatusDataEncryption();
};

extern MPWL &mpWL;

#endif