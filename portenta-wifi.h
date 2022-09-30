#pragma once

#include <WiFi.h>

#include "arduino_secrets.h"

class MPWL {
  private:
    String ssid  = SECRET_SSID;
    String pass  = SECRET_PASS;
    int keyIndex = 0;
    int status   = WL_IDLE_STATUS;
    
    WiFiServer *server;
    WiFiClient *client;

    String WebServerConnect = "192.168.3.1";
      
    bool statusWL;
    bool statusConnection;
    bool statusDataEncryption;

    MPWL() = default; // Make constructor private  

    void PrintWiFiStatus();

  public:
    //SINGLETON
    static MPWL &getInstance(); // Accessor for singleton instance
    MPWL(const MPWL &) = delete; // no copying
    MPWL &operator=(const MPWL &) = delete;

  public:    
    bool Init();
    bool Run();

    bool Connect();
    bool Interact();

    bool StatusWL();
    bool StatusConnection();
    bool StatusDataEncryption();
};

extern MPWL &mpWL;
