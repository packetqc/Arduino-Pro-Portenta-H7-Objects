#include "portenta-ble.h"
#include "portenta-monitor.h"

#include <Arduino.h>

//===================================================================================================
// BLE CLIENT ON A CENTRAL/PARENT
//===================================================================================================
bool MPBLE::Disconnect() {
  bool retour = true;
  
  if (BLE.connected()) {    
    if(!BLE.disconnect()) {
      retour = false;
    }
  }

  return retour;
}

bool MPBLE::Connect() {
  bool retour = true;
  
  if(BLE.scanForName(bleadvString)) {
    mpMON.Debug("BLE Client scan started for name: "+bleadvString+" ...");
  }
  else if(BLE.scanForUuid(bleadvUUID)) {
    mpMON.Debug("BLE Client scan started for uuid: "+bleadvUUID+" ...");
  }
  else {
    retour = false;
    mpMON.Debug("BLE Client scan could not be started ---");
  }

  delay(1000);

  if(retour) {
    listener = BLE.available();
    if (listener) {
      mpMON.Debug("BLE Client detected server object");
      mpMON.Debug(listener.address());
      mpMON.Debug(listener.localName());
      mpMON.Debug(listener.advertisedServiceUuid());

      // stop scanning
      BLE.stopScan();
    }
    else {
      retour = false;
      mpMON.Debug("BLE Client not detected any server object ---");
      // mpMON.Debug("BLE Client listener not available ---");
    }
  }

  if(retour) {
    // connect to the peripheral
    mpMON.Debug("BLE Client connecting ...");

    if (listener.connect()) {
      mpMON.Debug("BLE Client connected with server object");
    } else {
      mpMON.Debug("BLE Client failed to connect with server object ---");
      retour = false;
    }
  }

  if(retour) {
    // discover peripheral attributes
    mpMON.Debug("BLE Client discovering attributes ...");
    if (listener.discoverAttributes()) {
      mpMON.Debug("BLE Client attributes discovered");
    } else {
      mpMON.Debug("BLE Client attribute discovery failed ---");
      listener.disconnect();
      retour = false;
    }
  }

  if(retour) {
    // retrieve the LED characteristic
    ledCharacteristic = listener.characteristic(caracteristic_1_UUDI.c_str());

    if (!ledCharacteristic) {
      mpMON.Debug("BLE Client peripheral does not have LED characteristic ---");
      listener.disconnect();
      retour = false;
    } else if (!ledCharacteristic.canWrite()) {
      mpMON.Debug("BLE Client peripheral does not have a writable LED characteristic ---");
      listener.disconnect();
      retour = false;
    }
    else {
      mpMON.Debug("BLE Client peripheral ready with server");
    }
  }
   
  return retour;
}

bool MPBLE::Interact() {
  bool retour = true;
   
  if(!listener.connected()) {
    mpMON.Debug("BLE Client listener not connected with server object ---");
    retour = false;
  }
  else {  // while(listener.connected()) {    
    if (oldButtonState != buttonState) {
      // button changed
      oldButtonState = buttonState;

      if (buttonState) {
        mpMON.Debug("BLE Client button pressed");

        // button is pressed, write 0x01 to turn the LED on
        ledCharacteristic.writeValue((byte)0x01);
      } else {
        mpMON.Debug("BLE Client button released");

        // button is released, write 0x00 to turn the LED off
        ledCharacteristic.writeValue((byte)0x00);
      }
      // delay(1000);
      if(buttonState == HIGH)
        buttonState = LOW;
      else
        buttonState = HIGH;
    }
  }  

  return retour;
}

//===================================================================================================
// BLE SERVER ON AN OBJECT
//===================================================================================================
bool MPBLE::begin() {
  bool retour = true;

  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);

  if(!Init(bleadvString, bleadvUUID)) {
    mpMON.Debug("BLE Server init failed! End. ---");
    retour = false;
  }
  else {
    mpMON.Debug("BLE Server initiated");
  }
  
  if(retour) {
    if(!SetCharacteristic(true,true,caracteristic_1_UUDI)) {
      mpMON.Debug("BLE Server characteristic configuration failed! End. ---");
      retour = false;
    }
    else {
      mpMON.Debug("BLE Server characteristic configuration completed");
    }
  }

  if(retour) {
    if(!Start()) {
      mpMON.Debug("BLE Server start failed! End. ---");
      retour = false;
    }
    else {
      mpMON.Debug("BLE Server started, Advertising now");      
    }
  }

  return retour;
}

void MPBLE::run() {
  if( ListenConnection()) { 
    if( IncomingConnection()) {
      //  mpMON.Debug("BLE Server processing incoming connection on connected device completed");
    }
    else {
      mpMON.Debug("BLE Server failure processing incoming connection ---");
    }
  }
  else {
    // mpMON.Debug("BLE Server have no device connected");
  }
}

//========================================================================================================================
// BLE SERVER's LIBRARY
//========================================================================================================================
bool MPBLE::Init(String name, String uuid) {
  bool retour = true;

  statusBLE = false;
  statusConnection = false;
  statusDataEncryption = false;
  
  bleUuid = uuid;
  bleName = name;
  
  bleService = new BLEService(bleUuid.c_str());
  
  BLE.setLocalName(bleName.c_str());
  
  // Begin initialization
  if (!BLE.begin()) {
    // Stop if BLE couldn't be initialized.
    retour = false;
  }
  else {
    mpMON.Debug("BLE Server advertising name: "+bleName);
    mpMON.Debug("BLE Server advertising uuid: "+bleUuid);
    BLE.setAdvertisedService(*bleService);
  }
  
  return retour;
}

bool MPBLE::SetCharacteristic(bool read, bool write, String setUuid) {
  bool retour = true;
  
  switchCharacteristic = new BLEByteCharacteristic (setUuid.c_str(), BLERead | BLEWrite);
  
  // Add the characteristic to the service
  bleService->addCharacteristic(*switchCharacteristic); 
  mpMON.Debug("BLE Server characteristic added: "+setUuid);
  
  // Set the initial value for the characeristic:
  switchCharacteristic->writeValue(0);
  mpMON.Debug("BLE Server characteristic initial value set to: 0");

  return retour;
}

bool MPBLE::Start() {
  bool retour = true;

  // Add service  
  BLE.addService(*bleService);
  
  // start advertising
  if( !BLE.advertise()) {
    retour = false;
    statusBLE = false;
  }
  else {
    statusBLE = true;
  }

  return retour;
}

bool MPBLE::ListenConnection() {
  bool retour = true;
  
  listener = BLE.central();
  
  if( !listener ) {
    // mpMON.Debug("BLE Server have no central/parent device connected");
    retour = false;
  }
  else {
    // mpMON.Debug("BLE Server have a central/parent device connected: "+GetIncomingAddress());
    statusConnection = true;
  }

  return retour;  
}

bool MPBLE::IncomingConnection() {
  bool retour = true;

  if(listener.connected()) {
    statusConnection = true;
    // mpMON.Debug("BLE Server have a request to process");

    // If the remote device wrote to the characteristic,
    // Use the value to control the LED:
    if (switchCharacteristic->written()) {
      mpMON.Debug("BLE Server have a request to process from "+GetIncomingAddress());
      mpMON.Debug("BLE Server processing received value: "+String(switchCharacteristic->value()));

      if (switchCharacteristic->value()) {   // Any value other than 0
        //          mpMON.Debug("LED on");
        //          digitalWrite(ledPin, LOW);          // Will turn the Portenta LED on
      } else {
        //          mpMON.Debug("LED off");
        //          digitalWrite(ledPin, HIGH);         // Will turn the Portenta LED off
      }
    }
    else {
      // mpMON.Debug("BLE Server have a no request to process");
      statusConnection = false;
    }
  }
  
  return retour;
}

String MPBLE::GetIncomingAddress() {
  return listener.address();
}

bool MPBLE::StatusBLE() {
  return statusBLE;
}

bool MPBLE::StatusConnection() {
  return statusConnection;
}

bool MPBLE::StatusDataEncryption() {
  return statusDataEncryption;
}


//===================================================================================================
// SINGLETON
//===================================================================================================
MPBLE &MPBLE::getInstance() {
  static MPBLE instance;
  return instance;
}

MPBLE &mpBLE = mpBLE.getInstance();
