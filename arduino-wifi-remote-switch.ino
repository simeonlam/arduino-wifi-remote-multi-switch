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
void updateValueA(int val) {
  valueA = val;
}
int valueB = LOW;
void updateValueB(int val) {
  valueB = val;
}

// control 1 in D6 - GPIO12 ?
// control 2 in D5 - GPIO14 ?

// int ledPin = 13; // GPIO13---D7 of NodeMCU
int control1Pin = 12;
int control2Pin = 14;

void setup() {
  DebugBegin(115200);
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

  // pinMode(ledPin, OUTPUT);
  pinMode(control1Pin, OUTPUT);
  pinMode(control2Pin, OUTPUT);

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

  // web handling

  server.on("/", []() {
    server.send(200, "text/json", "{\"status\":\"OK\"}");
  });

  server.on("/a/on", []() {
    // turn on device
    digitalWrite(control1Pin, HIGH);
    updateValueA(HIGH);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/json", "{\"updated\":\"success\",\"status\":1}");
  });

  server.on("/a/off", []() {
    // turn off device
    digitalWrite(control1Pin, LOW);
    updateValueA(LOW);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/json", "{\"updated\":\"success\",\"status\":0}");
  });

  server.on("/b/on", []() {
    // turn on device
    digitalWrite(control2Pin, HIGH);
    updateValueB(HIGH);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/json", "{\"updated\":\"success\",\"status\":1}");
  });

  server.on("/b/off", []() {
    // turn off device
    digitalWrite(control2Pin, LOW);
    updateValueB(LOW);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/json", "{\"updated\":\"success\",\"status\":0}");
  });

  server.on("/info", []() {
    // get device info
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/json", "{\"ip\":\"" + WiFi.localIP().toString() + "\"," +
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
        "{" +
          "\"label\": \"light-a\"," +
          "\"fullPath\": \"http://" + HOSTNAME + ".local/a/on\"," +
          "\"path\": \"/on\"," +
          "\"desc\": \"turn on kitchen light\"" +
        "}, " +
        "{" +
          "\"label\": \"light-a\"," +
          "\"fullPath\": \"http://" + HOSTNAME + ".local/a/off\"," +
          "\"path\": \"/off\"," +
          "\"desc\": \"turn off kitchen light\"" +
        "}, " +
        "{" +
          "\"label\": \"light-b\"," +
          "\"fullPath\": \"http://" + HOSTNAME + ".local/b/on\"," +
          "\"path\": \"/on\"," +
          "\"desc\": \"turn on kitchen light\"" +
        "}, " +
        "{" +
          "\"label\": \"light-b\"," +
          "\"fullPath\": \"http://" + HOSTNAME + ".local/b/off\"," +
          "\"path\": \"/off\"," +
          "\"desc\": \"turn off kitchen light\"" +
        "}, " +
        "{" +
          "\"label\": \"info\"," +
          "\"fullPath\": \"http://" + HOSTNAME + ".local/info\"," +
          "\"path\": \"/info\"," +
          "\"desc\": \"get info of this board\"" +
        "}" +
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

// collect header example
// https://github.com/esp8266/Arduino/blob/4897e0006b5b0123a2fa31f67b14a3fff65ce561/libraries/ESP8266WebServer/examples/SimpleAuthentification/SimpleAuthentification.ino#L124
