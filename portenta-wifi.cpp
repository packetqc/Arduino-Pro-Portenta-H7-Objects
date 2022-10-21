#include "portenta-wifi.h"
#include "portenta-monitor.h"

bool MPWL::Connect() {
  bool retour = true;   

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
  
      // wait 3 seconds for connection:
      if(!status) {
        delay(3000);
      }
    }
    mpMON.Debug("Connected to wifi");

    statusWL = true;

    // PrintWiFiStatus();
  }
  
  return retour;
}

bool MPWL::Interact() {
  bool retour = false;
  
  if(!StatusWL()) {
    Connect();
  }

  clientWiFi = new WiFiClient;

  // PrintWiFiStatus();

  mpMON.Debug("Starting connection to server...");
  
  // if you get a connection, report back via serial:
  if (clientWiFi->connect(WebServerConnect.c_str(), 80)) {
    mpMON.Debug("connected to server");
    retour = true;
    
    // Make a HTTP request:
    clientWiFi->println("GET /index.html HTTP/1.1");
    clientWiFi->print("Host: ");
    clientWiFi->println(WebServerConnect.c_str());
    clientWiFi->println("Connection: close");
    clientWiFi->println();
    clientWiFi->println();

    // delay(3000);

    // if there are incoming bytes available
    // from the server, read them and print them:
    int retry = 0;
    do { 
      if(!clientWiFi->available()) {
        mpMON.Debug("no response available from request");
        retry++;
        // retour = false;
      }
      else {
        // mpMON.Debug("response available from request");
        // break;
        retry = 3;
      }

      if(retry<3) {
        delay(1000);
      }
      // else {
      //   retour = false;
      //   // break;
      // }      
    } while (retry<3);

    if(clientWiFi->available()) {
      retour = true;
      mpMON.Debug("response available from request");
      
    }

    if(retour) {
      int responseCount = 0;
      while (clientWiFi->available()) {
        char c = clientWiFi->read();
        // Serial.write(c);
        responseCount++;
      }
      mpMON.Debug("received number of characters: ");
      mpMON.Debug(String(responseCount));
    }
  }
  else {
    mpMON.Debug("cannot connect to server---");
    retour = false;
  }

  // if the server's disconnected, stop the clientWiFi:
  // if (!clientWiFi->connected()) {   
  //   mpMON.Debug("disconnecting from Web server.");
  //   mpMON.Debug("stopping Web client");
  //   clientWiFi->flush();
  //   clientWiFi->stop();
  //   clientWiFi = NULL;
  // }
  
  return retour;
}

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

bool MPWL::Run() {
  bool retour = true;

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

  WiFiClient webClient = server->available();   // listen for incoming clients

  if (webClient) {                             // if you get a client,
    mpMON.Debug("new client request");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client

    while (webClient.connected()) {            // loop while the client's connected

      if (webClient.available()) {             // if there's bytes to read from the client,
        char c = webClient.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            webClient.println("HTTP/1.1 200 OK");
            webClient.println("Content-type:text/html");
            webClient.println();
            webClient.println();
            // the content of the HTTP response follows the header:
            webClient.print("<html><head>");
            webClient.print("<style>");
            webClient.print("* { font-family: sans-serif;}");
            webClient.print("body { padding: 2em; font-size: 2em; text-align: center;}");
            webClient.print("a { -webkit-appearance: button;-moz-appearance: button;appearance: button;text-decoration: none;color: initial; padding: 25px;} #red{color:red;} #green{color:green;} #blue{color:blue;}");
            webClient.print("</style></head>");
            webClient.print("<body><h1> LED CONTROLS </h1>");
            webClient.print("<h2><span id=\"red\">RED </span> LED </h2>");
            webClient.print("<a href=\"/Hr\">ON</a> <a href=\"/Lr\">OFF</a>");
            webClient.print("<h2> <span id=\"green\">GREEN</span> LED </h2>");
            webClient.print("<a href=\"/Hg\">ON</a> <a href=\"/Lg\">OFF</a>");
            webClient.print("<h2> <span id=\"blue\">BLUE</span> LED </h2>");
            webClient.print("<a href=\"/Hb\">ON</a> <a href=\"/Lb\">OFF</a>");
            webClient.print("</body></html>");

            // The HTTP response ends with another blank line:
            webClient.println();
            // break out of the while loop:
            break;
          } else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /Hr")) {
          digitalWrite(LEDR, LOW);               // GET /Hr turns the Red LED on
        }
        if (currentLine.endsWith("GET /Lr")) {
          digitalWrite(LEDR, HIGH);                // GET /Lr turns the Red LED off
        }
        if (currentLine.endsWith("GET /Hg")) {
          digitalWrite(LEDG, LOW);                // GET /Hg turns the Green LED on
        }
        if (currentLine.endsWith("GET /Lg")) {
          digitalWrite(LEDG, HIGH);                // GET /Hg turns the Green LED on
        }
        if (currentLine.endsWith("GET /Hb")) {
          digitalWrite(LEDB, LOW);                // GET /Hg turns the Green LED on
        }
        if (currentLine.endsWith("GET /Lb")) {
          digitalWrite(LEDB, HIGH);                // GET /Hg turns the Green LED on
        }

      }
    }
    
    // close the connection:
    webClient.stop();
    mpMON.Debug("WiFi Web client disconnected");
  }

  return retour;
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

//===================================================================================================
// SINGLETON
//===================================================================================================
MPWL &MPWL::getInstance() {
  static MPWL instance;
  return instance;
}

MPWL &mpWL = mpWL.getInstance();
