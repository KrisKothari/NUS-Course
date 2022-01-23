                    
/*
  WebSocket LED
  ESP32-C3 Running  WebSocket Server Interfacing to the onBoard
  LED on the TomatoCube Core-Module
  
  LEDs
  --  IO2
  --  PA7 (3)(ATTINY1614)
  IR_RECV
  --  IO3
  I2C
  --  IO18 (SCL) <-> PB0 (ATTINY1614)
  --  IO19 (SDA) <-> PB1 (ATTINY1614)
  
*/

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <WebSocketsServer.h>
#include <WebServer.h>

WiFiMulti WiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(81);
WebServer server(80);


#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <Adafruit_NeoPixel.h>


#define PEXP_I2CADDR 0x23
#define OLED_I2CAADR 0x3C

SSD1306AsciiWire oled;

// i2c PCF8574(A) Port expander
// PCF8574  -> 0 0 1 0  0 A2 A1 A0    0 0 1 1
// PCF8574A -> 0 0 1 1  1 A2 A1 A0    1 0 1 1

#define PEXP_I2CADDR 0x23
//#define PEXP_A_I2CADDR 0x3B

// i2c Slave Co-processor
#define I2CADDR 0x13
// I2C address  - MainBoard
#define I2CADDR_B 0x12

#define I2C_SDA 19
#define I2C_SCL 18
#define LED 2
#define IR_RECV 4
#define NEO_PIXEL 5
#define LED_COUNT 3
#define STOP 0
#define CW 1
#define CCW 2

byte attinySlaveRcvArray[3];
byte attinySlaveArrayBoard[3];

// Replace with your network credentials
const char* ssid = "arasu2";
const char* password = "hemalatha";

bool ledState = 0;


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  </style>
<title>ESP Web Server</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <h1>C3 CoreModule - CockroachBot WebSocket Server</h1>
  </div>
  <div class="content">
    <div class="card">
      <h2>LED - GPIO 2</h2>
      <p class="state">state: <span id="state">-</span></p>
      <p><button id="button" class="button">Toggle</button></p>
    </div>
  </div>
<script>
  var gateway = `ws://${window.location.hostname}:81`;
  var websocket;
  window.addEventListener('load', onLoad);
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; // <-- add this line
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    if (event.data == "LED=1"){
      state = "ON";
    }
    else if (event.data == "LED=0"){
      state = "OFF";
    }
    document.getElementById('state').innerHTML = state;
  }
  function onLoad(event) {
    initWebSocket();
    initButton();
  }
  function initButton() {
    document.getElementById('button').addEventListener('click', toggle);
  }
  function toggle(){
    websocket.send('toggle');
  }
</script>
</body>
</html>
)rawliteral";


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
int setMotorRunning(uint8_t motorState) {
    attinySlaveArrayBoard[0] = 0x01;  // Command 0x01
    attinySlaveArrayBoard[1] = motorState? 0x01:0x00;  // Param1 - Stop/Run
    attinySlaveArrayBoard[2] = 0x00;  // Param2 - Dummy in this case
    delay(10);
    Wire.beginTransmission(I2CADDR_B);
    Wire.write(attinySlaveArrayBoard, 3); // Sends 3 bytes i2c to Co-processor.
    if (Wire.endTransmission () == 0) { // Receive 0 = success (ACK response) 
        Serial.println("i2c Write to 0x12 Sucessfull");
        return 0;
    }
    else {
        Serial.println("i2c Write Failed");
        return 1;
    }
}

int setDirection(int motor, byte direction) {
    attinySlaveArrayBoard[0] = motor == 0 ? 0x13 : 0x23;  // Command 0x13 or 0x23
    attinySlaveArrayBoard[1] = (direction >= 0) && (direction <= 2) ? direction: 0;  
                                                          // Param1 - Stop/CW/CCW
    attinySlaveArrayBoard[2] = 0x00;  // Param2 - Dummy in this case
    delay(10);
    Wire.beginTransmission(I2CADDR_B);
    Wire.write(attinySlaveArrayBoard, 3); // Sends 3 bytes i2c to Co-processor.
    if (Wire.endTransmission () == 0) { // Receive 0 = success (ACK response) 
        Serial.println("i2c Write to 0x12 Sucessfull");
        return 0;
    }
    else {
        Serial.println("i2c Write Failed");
        return 1;
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

                // send message to client
                webSocket.sendTXT(num, "Connected");
                webSocket.sendTXT(num, ledState? "LED=1": "LED=0");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);
            if (strcmp((char*)payload, "toggle") == 0) {
              ledState = !ledState;
              webSocket.broadcastTXT(ledState? "LED=1": "LED=0");
            }

            else if (strcmp((char*)payload, "Forward") == 0) {
              oled.println ("Move Forward");
              setDirection(0, CCW);
              setDirection(1, CW);
              setMotorRunning(HIGH);
              ledState = !ledState;
              webSocket.broadcastTXT(ledState? "LED=1": "LED=0");
            }

            else if (strcmp((char*)payload, "Backward") == 0) {
              oled.println ("Move Backward");
              setDirection(0, CW);
              setDirection(1, CCW);
              setMotorRunning(HIGH);
              ledState = !ledState;
              webSocket.broadcastTXT(ledState? "LED=1": "LED=0");
            }

            else if (strcmp((char*)payload, "Left") == 0) {
              oled.println ("Turn Left");
              setDirection(0, CCW);
              setDirection(1, CCW);
              setMotorRunning(HIGH);
              ledState = !ledState;
              webSocket.broadcastTXT(ledState? "LED=1": "LED=0");
            }

            else if (strcmp((char*)payload, "Right") == 0) {
              oled.println ("Turn Right");
              setDirection(0, CW);
              setDirection(1, CW);
              setMotorRunning(HIGH);
              ledState = !ledState;
              webSocket.broadcastTXT(ledState? "LED=1": "LED=0");
            }

            else if (strcmp((char*)payload, "Stop") == 0) {
              setMotorRunning (LOW);
              ledState = !ledState;
              webSocket.broadcastTXT(ledState? "LED=1": "LED=0");
            }

            break;
        case WStype_BIN:
        case WStype_ERROR:      
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
          break;
    }

}

void setup(){
  Wire.begin(I2C_SDA, I2C_SCL);    
  oled.begin(&Adafruit128x64, OLED_I2CAADR); 
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  delay(2000);

  Serial.begin(115200);

  oled.setFont(Adafruit5x7);
  oled.clear();
  oled.println("WebSocket Server");
  
  WiFiMulti.addAP(ssid, password);
  
  while(WiFiMulti.run() != WL_CONNECTED) {
      delay(100);
  }
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
  oled.println(WiFi.localIP());
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // handle index
  server.on("/", []() {
      // send index.html
      server.send(200, "text/html", index_html);
  });
  
  server.onNotFound(handleNotFound);

  server.begin();                                      
}

void loop() {
  webSocket.loop();
  server.handleClient();
  digitalWrite(LED, ledState);
  delay(2);//allow the cpu to switch to other tasks
}
                    
                
      