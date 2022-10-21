#include "portenta-ble.h"
#include "portenta-monitor.h"

#include <Arduino.h>

bool MPBLE::Connect() {
  bool retour = true;

  mpMON.Debug("BLE Client scan uuid...");
  BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    mpMON.Debug(peripheral.address());
    mpMON.Debug(peripheral.localName());
    mpMON.Debug(peripheral.advertisedServiceUuid());

    // stop scanning
    BLE.stopScan();

    Interact(peripheral);
    
    mpMON.Debug("Peripheral disconnected");
  }
    
  return retour;
}

void MPBLE::Interact(BLEDevice peripheral) {
  // connect to the peripheral
  mpMON.Debug("Connecting ...");

  if (peripheral.connect()) {
    mpMON.Debug("Connected");
  } else {
    mpMON.Debug("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  mpMON.Debug("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    mpMON.Debug("Attributes discovered");
  } else {
    mpMON.Debug("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the LED characteristic
  BLECharacteristic ledCharacteristic = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");

  if (!ledCharacteristic) {
    mpMON.Debug("Peripheral does not have LED characteristic!");
    peripheral.disconnect();
    return;
  } else if (!ledCharacteristic.canWrite()) {
    mpMON.Debug("Peripheral does not have a writable LED characteristic!");
    peripheral.disconnect();
    return;
  }

  int buttonState = HIGH;
  while (peripheral.connected()) {
    // while the peripheral is connected

    // read the button pin
//    int buttonState = digitalRead(buttonPin);

    if (oldButtonState != buttonState) {
      // button changed
      oldButtonState = buttonState;

      if (buttonState) {
        mpMON.Debug("button pressed");

        // button is pressed, write 0x01 to turn the LED on
        ledCharacteristic.writeValue((byte)0x01);
      } else {
        mpMON.Debug("button released");

        // button is released, write 0x00 to turn the LED off
        ledCharacteristic.writeValue((byte)0x00);
      }
      delay(1000);
      if(buttonState == HIGH)
        buttonState = LOW;
      else
        buttonState = HIGH;
    }
  }  
}

void MPBLE::begin() {
  pinMode(LED_BUILTIN, OUTPUT);

  // // blink the led a few times
  // for (int i = 0; i < 5; i++) {
  //   digitalWrite(LED_BUILTIN, LOW);
  //   delay(100);
  //   digitalWrite(LED_BUILTIN, HIGH);
  //   delay(100);
  // }

  digitalWrite(LED_BUILTIN, LOW);

  if(!Init("156 MATTE HONEY 1", "19b10000-e8f2-537e-4f6c-d104768a1214")) {
    mpMON.Debug("BLE Init failed! End.");
    while(1);
  }
  else {
    mpMON.Debug("BLE Initiated");
  }
  
  if(!SetCharacteristic(true,true,"19b10001-e8f2-537e-4f6c-d104768a1214")) {
    mpMON.Debug("BLE Characteristic configuration failed! End.");
  }
  else {
    mpMON.Debug("BLE Characteristic configuration completed");
  }
  
  if(!Start()) {
    mpMON.Debug("BLE Start failed! End.");
    while(1);
  }
  else {
    mpMON.Debug("BLE Started, Advertising now");      
  }
}

void MPBLE::run() {
  // // blink the LED
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(200);
  // digitalWrite(LED_BUILTIN, LOW);
  // delay(800);

  if( ListenConnection()) {
//      mpMON.Debug("Connected to client: ");
  // Print the central's MAC address:
//      mpMON.Debug(ble.GetIncomingAddress());
  
  if( IncomingConnection()) {
//        mpMON.Debug("Disconnected from central: ");
//        mpMON.Debug(ble.GetIncomingAddress());
  }
}
}

MPBLE &MPBLE::getInstance() {
  static MPBLE instance;
  return instance;
}

MPBLE &mpBLE = mpBLE.getInstance();

//========================================================================================================================
// MPBLE LIBRARY
//========================================================================================================================
bool MPBLE::Init(String name, const char* uuid) {
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
    BLE.setAdvertisedService(*bleService);
  }
  
  return retour;
}

bool MPBLE::SetCharacteristic(bool read, bool write, String setUuid) {
  bool retour = true;
  
  switchCharacteristic = new BLEByteCharacteristic (setUuid.c_str(), BLERead | BLEWrite);
  
  // Add the characteristic to the service
  bleService->addCharacteristic(*switchCharacteristic);
  
  // Set the initial value for the characeristic:
  switchCharacteristic->writeValue(0);

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
    retour = false;
  }
  else {
    statusConnection = true;
  }

  return retour;  
}

bool MPBLE::IncomingConnection() {
  bool retour = true;

    // While the central is still connected to peripheral:
//    while (listener.connected()) {
    if(listener.connected()) {
      statusConnection = true;
      // If the remote device wrote to the characteristic,
      // Use the value to control the LED:
      if (switchCharacteristic->written()) {
        if (switchCharacteristic->value()) {   // Any value other than 0
//          mpMON.Debug("LED on");
//          digitalWrite(ledPin, LOW);          // Will turn the Portenta LED on
        } else {
//          mpMON.Debug("LED off");
//          digitalWrite(ledPin, HIGH);         // Will turn the Portenta LED off
        }
      }
      else {
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
