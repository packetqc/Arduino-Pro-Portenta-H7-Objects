// #ifdef CORE_CM7

#include "mbed.h"
#include "Arduino.h"

//===================================================================================================
// TYPE OF DEPLOYMENT: OBJECT OR CENTRAL/PARENT
//===================================================================================================
// OBJECT TYPE
// - BLE SERVER ADVERTISING
// - WIFI CLIENT WEB REQUEST

// CENTRAL/PARENT TYPE
// - BLE CLIENT, button pressed communication to an object/device
// - WIFI AP WEB SERVER

// bool runAsServerObject = true; // Server / Object type
bool runAsServerObject = false; // Client / Central type

//===================================================================================================
// ASSETS
//===================================================================================================
#include "portenta-ble.h"
// mpBLE singleton of class ready

#include "portenta-wifi.h"
// mpWL singleton of class ready

#include "portenta-monitor.h"
// mpMON singleton of class ready with Morse Code and LED capabilities

//===================================================================================================
// MULTI-THREAD DEFINITIONS
//===================================================================================================
using namespace mbed;
using namespace rtos;

bool runit = true;

Thread mpBLE_server_thread;
bool bleServiceStarted = false;

Thread mpBLE_client_thread;
bool bleStarted = false;

Thread mpWL_server_thread;
bool wlServiceStarted = false;

Thread mpWL_client_thread;
bool wlClientStarted = false;


//===================================================================================================
// SETUP AND THREAD STARTUPs  (watchdogs)
//===================================================================================================
void(* resetFunc) (void) = 0;  // declare reset fuction at address 0

void setup() {   
  mpMON.Init();  
  mpMON.Debug("Message 1");
  mpPERF.Run();  

  if( runAsServerObject ) { //Server Object
    mpWL_client_thread.start(callback(mpWL_client_thread_callback));
    mpBLE_server_thread.start(callback(mpBLE_server_thread_callback));
  }
  else { //Client or Central
    mpWL_server_thread.start(callback(mpWL_server_thread_callback));
    mpBLE_client_thread.start(callback(mpBLE_client_thread_callback));
  }
}

//===================================================================================================
// WIFI CLIENT THREAD (watchdog)
//===================================================================================================
void mpWL_client_thread_callback() {
  while(runit) {
    if(mpWL.Connect()) {
    }
    else {}

    if(mpWL.StatusWL()) {
      mpWL.Interact();
      mpWL.Disconnect();
    }  

    delay(5000);
  }
}

//===================================================================================================
// WIFI SERVER THREAD (watchdog)
//===================================================================================================
void mpWL_server_thread_callback() {  
  if(!wlServiceStarted) {
    wlServiceStarted = mpWL.Init();
  }

  mpWL.Run();
}

//===================================================================================================
// BLE CLIENT THREAD (watchdog)
//===================================================================================================
void mpBLE_client_thread_callback() {
  while(runit) {
      if(!bleStarted) {
          BLE.begin();
          if(!mpBLE.Connect()) {
            mpMON.Debug("BLE Client not started. ---");
            bleStarted = false;
          }
          else {
            mpMON.Debug("BLE Client started.");
            bleStarted = true;
          }
      }
      
      if(bleStarted) {
        mpBLE.Interact();
      }
      else {
        mpMON.Debug("BLE Client cannot interact with server: not started and connected. ---");        
      }

      bleStarted = mpBLE.Disconnect();

      delay(1000);
      // yield();
  }
}

//===================================================================================================
// BLE SERVER THREAD (watchdog)
//===================================================================================================
void mpBLE_server_thread_callback() {
  while(runit) {
    if(!bleServiceStarted) {            
      if(BLE.begin()) {
        bleServiceStarted = mpBLE.begin();    
        if(!bleServiceStarted) {
          mpMON.Debug("BLE Server not started. ---");
        }
        else {
          mpMON.Debug("BLE Server started.");
        }
      }
      else {
        mpMON.Debug("BLE Server cannot begin BLE.begin ---");
      }
    }
    else { 
      mpBLE.run();
    }
    
    delay(1000);
    // yield();
  }
}


//===================================================================================================
// MAIN LOOP, THREAD WATCH AND MGMT
//===================================================================================================
void loop() {
//  delay(10);
    mpMON.MorseCode("sos sos");          
    
    #if MBED_MAJOR_VERSION == 2
        wait_ms(30000);
    #elif MBED_MAJOR_VERSION == 5
        thread_sleep_for(30000);
    #else
        ThisThread::sleep_for(30000);
    #endif
}

// #endif


// #ifdef CORE_CM4
// #include "Arduino.h"


// void setup() {
  
// }

// void loop() {
  
// }

// #endif
