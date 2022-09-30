#ifdef CORE_CM7

#include "portenta-thread.h"
#include "portenta-wifi.h"
#include "portenta-monitor.h"

#include "mbed.h"
#include "Arduino.h"

// bool runAsServerObject = true; // Server / Object
bool runAsServerObject = false; // Client / Central

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

void setup() {   
  mpMON.Init();  
  mpMON.Debug("Message 1");
  
  mpPERF.Run();
  
  // Serial.begin(115200);
//  bootM4();
//  delay(10);

  if( runAsServerObject ) { //Server Object
    // mpWL_client_thread.start(callback(mpWL_client_thread_callback));
    mpBLE_server_thread.start(callback(mpBLE_server_thread_callback));
  }
  else { //Client or Central
    // mpWL_server_thread.start(callback(mpWL_server_thread_callback));
    mpBLE_client_thread.start(callback(mpBLE_client_thread_callback));
  }
}

void mpWL_client_thread_callback() {
  while(runit) {
    if(!mpWL.StatusWL()) {
      mpWL.Connect();
//      wlClientStarted = true;
    }

    if(mpWL.StatusWL()) {
      if(!mpWL.Interact()) {
        delay(5000);
        mpWL.Connect();
      }
      else {
        yield();
        delay(5000);
      }
    }
  }
}

void mpWL_server_thread_callback() {
  while(runit) {
    if(!wlServiceStarted) {
      mpWL.Init();
      wlServiceStarted = true;
    }
  
    mpWL.Run();
    yield();
  }
}

void mpBLE_client_thread_callback() {
  while(runit) {
      if(!bleStarted) {
          BLE.begin();
          bleStarted = true;
          mpMON.Debug("BLE started.");
      }

      mpBLE.Connect();
      delay(1000);
      yield();
  }
}

void mpBLE_server_thread_callback() {
  while(runit) {
    if(!bleServiceStarted) {
      mpBLE.begin();
      bleStarted = true;
      bleServiceStarted = true;
    }
  
    mpBLE.run();
    delay(1000);
    yield();
  }
}

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

#endif


#ifdef CORE_CM4
#include "Arduino.h"


void setup() {
  
}

void loop() {
  
}

#endif
