#pragma once

#include <ArduinoBLE.h>

class MPBLE {
  private:
    // variables for button
    const int buttonPin = 2;
    int oldButtonState = LOW;

    BLEByteCharacteristic *switchCharacteristic;
    BLEService *bleService;
    BLEDevice listener;
    String bleUuid;
    String bleName;

    bool statusBLE;
    bool statusConnection;
    bool statusDataEncryption;

    MPBLE() = default; // Make constructor private  

    bool Init(String name, const char* uuid);
    bool SetCharacteristic(bool read, bool write, String setUuid);
    bool Start();

    bool ListenConnection();
    bool IncomingConnection();
    String GetIncomingAddress();

    void Interact(BLEDevice peripheral);

  public:
    //SINGLETON
    static MPBLE &getInstance(); // Accessor for singleton instance
    MPBLE(const MPBLE &) = delete; // no copying
    MPBLE &operator=(const MPBLE &) = delete;

  public:
    void begin();
    void run();
    bool Connect();
    
    bool StatusBLE();
    bool StatusConnection();
    bool StatusDataEncryption();
};

extern MPBLE &mpBLE;
