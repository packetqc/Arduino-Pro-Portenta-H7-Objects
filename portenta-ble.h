#pragma once

#include <ArduinoBLE.h>

//===================================================================================================
// 
//===================================================================================================
class MPBLE {
  private:
    String bleadvString = "156 MATTE HONEY";
    String bleadvUUID = "19b10000-e8f2-537e-4f6c-d104768a1214";
    String caracteristic_1_UUDI = "19b10001-e8f2-537e-4f6c-d104768a1214";

    // variables for button
    const int buttonPin = 2;
    int oldButtonState = LOW;
    int buttonState = HIGH;

    BLEByteCharacteristic *switchCharacteristic;
    BLEService *bleService;
    BLEDevice listener;

    BLECharacteristic ledCharacteristic;

    String bleUuid;
    String bleName;

    bool statusBLE;
    bool statusConnection;
    bool statusDataEncryption;

    MPBLE() = default; // Make constructor private  

    bool Init(String name, String uuid);
    bool SetCharacteristic(bool read, bool write, String setUuid);
    bool Start();

    bool ListenConnection();
    bool IncomingConnection();
    String GetIncomingAddress();

  public:
    //SINGLETON
    static MPBLE &getInstance(); // Accessor for singleton instance
    MPBLE(const MPBLE &) = delete; // no copying
    MPBLE &operator=(const MPBLE &) = delete;

  public:
    bool begin();
    void run();
    bool Connect();
    bool Disconnect();
    bool Interact();

    bool StatusBLE();
    bool StatusConnection();
    bool StatusDataEncryption();
};

extern MPBLE &mpBLE;
