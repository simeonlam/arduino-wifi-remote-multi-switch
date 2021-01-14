#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include "config.h"
#include "config-name.h"

#define DebugBegin(baud_rate)    Serial.begin(baud_rate)
#define DebugPrintln(message)    Serial.println(message)
#define DebugPrint(message)    Serial.print(message)
#define DebugPrintF(...) Serial.printf( __VA_ARGS__ )

ESP8266WebServer server(PORT);

int valueA = LOW;
int valueB = LOW;

// control 1 in D6 - GPIO12 ?
// control 2 in D5 - GPIO14 ?

// int ledPin = 13; // GPIO13---D7 of NodeMCU
int control1Pin = 12;
int control2Pin = 14;

String labelA, labelB, onState, offState;
String labels[2];
String status[2];
int labelLength = 2;
int statusLength = 2;

void setup() {
  labelA = "light-a";
  labelB = "light-b";
  labels[0] = labelA;
  labels[1] = labelB;

  onState = "on";
  offState = "off";
  status[0] = onState;
  status[1] = offState;

  DebugBegin(115200);

  connectWIFI();

  setupOTA();

  // pinMode(ledPin, OUTPUT);
  pinMode(control1Pin, OUTPUT);
  pinMode(control2Pin, OUTPUT);

  // web handling

  server.on("/", []() {
    server.send(200, "text/json", "{\"status\":\"OK\"}");
  });

  server.on("/" + labelA + "/" + onState , []() {
    actionLambda(control1Pin, valueA, HIGH);
  });
  server.on("/" + labelA + "/" + offState , []() {
    actionLambda(control1Pin, valueA, LOW);
  });

  server.on("/" + labelB + "/" + onState , []() {
    actionLambda(control2Pin, valueB, HIGH);
  });
  server.on("/" + labelB + "/" + offState , []() {
    actionLambda(control2Pin, valueB, LOW);
  });

  server.on("/" + labelA + "/status", []() {
    server.send(200, "text", valueToString(valueA));
  });

  server.on("/" + labelB + "/status", []() {
    server.send(200, "text", valueToString(valueB));
  });

  server.on("/info", []() {
    String apiStr = apisString();
    // get device info
    setHeader();
    server.send(200, "text/json", "{\"ip\":\"" + WiFi.localIP().toString() + "\"," +
      "\"version\":\"" + VERSION + "\"," +
      "\"location\":\"" + LOCATION + "\"," +
      "\"hostname\":\"" + HOSTNAME + "\"," +
      "\"protocol\":\"http\"," +
      "\"devices\":{" +
        "\"lightA\":{" +
          "\"led_port\":" + control1Pin + "," +
          "\"status\":" + valueA +
        "}," +
        "\"lightB\":{" +
          "\"led_port\":" + control2Pin + "," +
          "\"status\":" + valueB +
        "}" +
      "}," +
      "\"api\":[" +
        apiStr +
      "]" +
    "}");
  });

  server.begin();
  ArduinoOTA.begin();

  DebugPrint("Open http://");
  DebugPrint(WiFi.localIP());
  DebugPrintln("/ in your browser to see it working");
  DebugPrint(WiFi.hostname());
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}

// ---------------------------------------------------------

void connectWIFI() {
  WiFi.hostname(HOSTNAME);      // DHCP Hostname (useful for finding device for static lease)
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFi_SSID, WiFi_Password);
  while(WiFi.status() != WL_CONNECTED){
		/*Note: if connection is established, and then lost for some reason, ESP will automatically reconnect. This will be done automatically by Wi-Fi library, without any user intervention.*/
		delay(1000);
		DebugPrint(".");
	}
	DebugPrint("\nConnected to " + String(WiFi_SSID) + " with IP Address: "); DebugPrint(WiFi.localIP()); DebugPrint("\n");

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    DebugPrintln("WiFi Connect Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }
  // ArduinoOTA.begin();

  // MDNS.begin("testing-simeon");
  if (!MDNS.begin(HOSTNAME)) {
    DebugPrintln("Error setting up MDNS responder!");
  }
  MDNS.addService(SERVICE_NAME, "tcp", PORT);
}

void setupOTA() {
  // ota handling

  ArduinoOTA.onStart([]() {
    String type;
  //判斷一下OTA內容
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    DebugPrintln("OTA Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    DebugPrintln("\nOTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DebugPrintF("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DebugPrintF("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      DebugPrintln("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      DebugPrintln("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      DebugPrintln("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      DebugPrintln("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      DebugPrintln("End Failed");
    }
  });
}

// utils

void updateValue(int &value, int val) {
  value = val;
}

String valueToString (int value) {
  return value == HIGH ? "1" : "0";
}

void setHeader() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
}

void actionLambda(int controlPin, int &value, int newValue) {
  String strVal = newValue == HIGH ? "1" : "0";
  digitalWrite(controlPin, newValue);
  updateValue(value, newValue);
  setHeader();
  server.send(200, "text/json", "{\"updated\":\"success\",\"status\":" + strVal + ",\"pin\":" + controlPin + "}");
}

String apisString () {
  String result;
  String url = "http://" + (String)HOSTNAME + ".local";
  result = "";
  for (int i=0; i<labelLength; i++) {
    String label = labels[i];
    for (int j=0; j<statusLength; j++) {
      String state = status[j];
      String path = "/" + label + "/" + state;
      result += "{" +
        (String)"\"label\": \"" + label + "\"," +
        "\"fullPath\": \"" + url + path + "\"," +
        "\"path\": \"" + path + "\"," +
        "\"desc\": \"turn " + state + " " + LOCATION + " " + label + "\"" +
      "}, ";
    }
    String statusPath = "/" + label + "/status";
    result += "{" +
        (String)"\"label\": \"" + label + "\"," +
        "\"fullPath\": \"" + url + statusPath + "\"," +
        "\"path\": \"" + statusPath + "\"," +
        "\"desc\": \"Show status on " + LOCATION + " " + label + "\"" +
      "}, ";
  }

  result += "{" +
    (String)"\"label\": \"info\"," +
    "\"fullPath\": \"" + url + "/info\"," +
    "\"path\": \"/info\"," +
    "\"desc\": \"get info of " + LOCATION + " board\"" +
  "}";
  return result;
}

// collect header example
// https://github.com/esp8266/Arduino/blob/4897e0006b5b0123a2fa31f67b14a3fff65ce561/libraries/ESP8266WebServer/examples/SimpleAuthentification/SimpleAuthentification.ino#L124
