#include "portenta-wifi.h"
#include "portenta-monitor.h"

// #define WEB_CLIENT_WAIT 30
unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long waitTime = 10L * 1000L; // delay between updates, in milliseconds

//===================================================================================================
// CLIENT
//===================================================================================================
bool MPWL::Disconnect() {
  bool retour = false;   

  // webclient->stop();
  webclient = NULL;

  mpMON.Debug("WiFi client is disconnecting now...");  
  WiFi.end();
  retour = true;
  mpMON.Debug("WiFi client disconnected, end, as expected");  
  StatusWL();

  return retour;
}

bool MPWL::Connect() {
  bool retour = true;   
  int retry = 0;

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_SHIELD) {
    mpMON.Debug("Communication with WiFi module failed!");
    // don't continue
    retour = false;
    statusWL = false;
  }

  if(retour) {
    // attempt to connect to Wifi network:
    while (WiFi.status() != WL_CONNECTED) {
      mpMON.Debug("Attempting to connect to SSID WiFi: "+ssid);

      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(ssid.c_str(), pass.c_str());
  
      if(!StatusWL()) {
        retry++;        
      }
      else {
        retry = 0;
      }

      if(retry > 5) {
        retour = false;
        mpMON.Debug("Failed to connected multiple times, resetting the device...bye now...");
        // resetFunc(); //call reset
        break;
      }

      // wait 3 seconds for connection:
      if(!status) {
        delay(3000);
      }
      else {
        mpMON.Debug("WiFi client connected to access point");        
        StatusWL();
        webclient = new WiFiClient;
      }
    }    
  }
  
  return retour;
}

bool MPWL::Interact() {
  bool retour = false;
  int retry = 1;
  int responseCount = 0;

  mpMON.Debug("Starting connection to Web server...");
  
  // if you get a connection, report back via serial:
  if (webclient->connect(WebServerConnect.c_str(), 80)) {
    lastConnectionTime = millis();

    mpMON.Debug("connected to Web server");
    mpMON.Debug("sending request to Web server...");
    
    // Make a HTTP request:
    webclient->println("GET /index.html HTTP/1.1");
    webclient->print("Host: ");
    webclient->println(WebServerConnect.c_str());
    webclient->println("Connection: close");
    webclient->println();

    while(webclient->connected() || webclient->available()) {
      if( webclient->available()) {
        retour = true;

        char c = webclient->read();
        Serial.write(c);
        responseCount++;
      }
      
      if (millis() - lastConnectionTime > waitTime) {
        retour = false;
        mpMON.Debug("connected to Web server but no response available in wait time threshold from Web server for request ---");
        break;
      }
    }

    if(retour) {
      mpMON.Debug("response available from Web request");      
      mpMON.Debug("received number of characters from Web response: "+String(responseCount));      
      mpMON.Debug("Web request completed");
    }
    else {
      mpMON.Debug("connected to Web server but no response available from Web server for request ---");
    }
  }
  else {
    mpMON.Debug("cannot connect to server for Web request ---");
    retour = false;
  }

  mpMON.Debug("Web client is stopping now...");    
  webclient->stop();
  mpMON.Debug("Web client stopped as expected");    

  return retour;
}

//===================================================================================================
// SERVER
//===================================================================================================
bool MPWL::Init() {
  bool retour = true;
  
  server = new WiFiServer(80);

  // The AP needs the password be at least 8 characters long
  if(strlen(pass.c_str()) < 8){    
    mpMON.Debug("Creating access point failed");
    mpMON.Debug("The WiFi password must be at least 8 characters long");
    // don't continue
    retour = false;
  }

  //Create the Access point
  if(retour) {
    status = WiFi.beginAP(ssid.c_str(), pass.c_str());
    if (status != WL_AP_LISTENING) {
      mpMON.Debug("Creating access point failed");
      // don't continue
      retour = false;
    }
    else {
      mpMON.Debug("Access point created");
      // PrintWiFiStatus();
    }
  }

  if(retour) {
    // delay(10000);

    mpMON.Debug("Starting Web server...");
    server->begin();
    mpMON.Debug("Web server started.");
  }
  
  return retour;
}

void MPWL::Run() {

  while(true) {  
      // compare the previous status to the current status
    if (status != WiFi.status()) {
      // it has changed update the variable
      status = WiFi.status();

      if (status == WL_AP_CONNECTED) {
        // a device has connected to the AP
        mpMON.Debug("Device connected to AP");
      } else {
        // a device has disconnected from the AP, and we are back in listening mode
        mpMON.Debug("Device disconnected from AP");
      }
    }

    WiFiClient webrequest = server->available();   // listen for incoming clients

    if (webrequest) {                             // if you get a client,
      mpMON.Debug("new Web client request");           // print a message out the serial port
      String currentLine = "";                // make a String to hold incoming data from the client
      int requestreceived = 0;

      delay(500);
      while (webrequest.available()) {
          char c = webrequest.read();             // read a byte, then
          Serial.write(c);                  // print it out the serial monitor  
          requestreceived++;
                           
          if (c == '\n') {                    // if the byte is a newline character
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0) {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              webrequest.println("HTTP/1.1 200 OK");
              webrequest.println("CONTENT-TYPE:text/html");
              webrequest.println();

              // the content of the HTTP response follows the header:
              webrequest.println("<HTML>");
              webrequest.println("<HEAD>");
              webrequest.println("</HEAD>");
              webrequest.println("<BODY>");
              webrequest.println("</BODY>");
              webrequest.println("</HTML>");

              // break out of the while loop:
              break;
            } else {      // if you got a newline, then clear currentLine:
              currentLine = "";
            }
          } else if (c != '\r') {    // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
          }
      }

      mpMON.Debug("Number of characters received from Web request: "+String(requestreceived));
      
      if(!webrequest.connected()) {
        mpMON.Debug("WiFi Web client is connected, stopping...");      
        webrequest.stop();   
        mpMON.Debug("WiFi Web client stopped as expected");
      }
    }     
  }
}

//===================================================================================================
// STATUS
//===================================================================================================
bool MPWL::StatusWL() {  
  if(WiFi.status() == WL_CONNECTED) {
    statusWL = true;
  }
  else {
    statusWL = false;
  }

  return statusWL;
}

void MPWL::PrintWiFiStatus() {
  // print the SSID of the network you're attached to:
  
  mpMON.Debug("SSID: "+String(WiFi.SSID()));
  // mpMON.Debug(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  mpMON.Debug("IP Address: "+String(ip));
  // mpMON.Debug(String(ip.c_str()));

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  mpMON.Debug("Signal strength (RSSI): "+String(rssi)+" dBm");
  
  // print where to go in a browser:
  // mpMON.Debug("To see this page in action, open a browser to http://");
  // mpMON.Debug(WebServerConnect.c_str());  
  
}
//===================================================================================================
// SINGLETON
//===================================================================================================
MPWL &MPWL::getInstance() {
  static MPWL instance;
  return instance;
}

MPWL &mpWL = mpWL.getInstance();
