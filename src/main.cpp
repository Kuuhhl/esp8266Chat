#include <ESP8266WiFi.h>
#include <string.h>
#include <LittleFS.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>

#define BAUD_RATE 9600

/* ============= CHANGE WIFI CREDENTIALS ============= */
const char *ssid = "Chat Server";
/* ============= ======================= ============= */

AsyncWebServer server(80);
FSInfo fs_info;
File f;

void setup()
{
    Serial.begin(BAUD_RATE);

    // initialize wifi-ap
    WiFi.mode(WIFI_STA);
    WiFi.softAP(ssid);

    // mount filesystem etc.
    EEPROM.begin(4096);
    LittleFS.begin();

    // add mdns
    MDNS.addService("http", "tcp", 80);

    // create empty file for text messages
    f = LittleFS.open("/messages.txt", "w");
    f.close();

    // route traffic to index.html
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    // route /styles to /styles.css file
    server.on("/styles", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/styles.css", "text/css");
    });

    // append text message to file when endpoint is called
    server.on("/sendText", HTTP_POST, [](AsyncWebServerRequest *request) {
        // get Name
        const char *PARAM_KEY = "name";
        char *name;
        if (request->hasParam(PARAM_KEY))
        {
            name = (char *)request->getParam(PARAM_KEY)->value().c_str();
        }
        else
        {
            name = (char *)"anon";
        }

        // get text
        const char *PARAM_KEY2 = "name";
        char *text;
        if (request->hasParam(PARAM_KEY2))
        {
            text = (char *)request->getParam(PARAM_KEY2)->value().c_str();
        }
        else
        {
            text = (char *)"";
        }

        // append some separators
        name = strcat(name, ": ");
        text = strcat(text, ";");
        text = strcat(name, text);

        // append text to file
        f = LittleFS.open("/messages.txt", "a");
        f.write(text);
        f.close();

        //redirect to index.html
        request->send(LittleFS, "/index.html", "text/html");
    });

    // show messages file content when /showText is called
    server.on("/showText", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/messages.txt", "text/html");
    });

    // when /clear is called, delete file contents and redirect to /index.html
    server.on("/clear", HTTP_GET, [](AsyncWebServerRequest *request) {
        f = LittleFS.open("/messages.txt", "w");
        f.close();
        request->send(LittleFS, "/index.html", "text/html");
    });

    // start the webserver
    server.begin();
}

void loop()
{
}