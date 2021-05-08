#include <ESP8266WiFi.h>
#include <string.h>
#include <LittleFS.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>

#define BAUD_RATE 9600

/* ============= CHANGE WIFI CREDENTIALS ============= */
const char *ssid = "Chat Server";
/* ============= ======================= ============= */

DNSServer dnsServer;
AsyncWebServer server(80);
FSInfo fs_info;
File f;

void setup()
{
    Serial.begin(BAUD_RATE);

    // configure IP and netmask
    IPAddress apIP(192, 168, 0, 1);
    IPAddress netMask(255, 255, 255, 0);

    // initialize wifi-ap
    WiFi.mode(WIFI_STA);
    WiFi.softAPConfig(apIP, apIP, netMask);
    WiFi.softAP(ssid);

    // mount filesystem etc.
    EEPROM.begin(4096);
    LittleFS.begin();

    // add mdns (http://chat.local/)
    MDNS.addService("http", "tcp", 80);
    MDNS.begin("chat");

    // start dns server to redirect all sites
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());

    // create empty file for text messages
    f = LittleFS.open("/messages.txt", "w");
    f.close();

    // route traffic to index.html
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    // route to /styles.css file
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/styles.css", "text/css");
    });

    // append text message to file when endpoint is called
    server.on("/sendText", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params_count = request->params();

        char *name = NULL;
        char *text = NULL;

        for (int i = 0; i < params_count; i++)
        {
            AsyncWebParameter *p = request->getParam(i);

            char *paramName = (char *)p->name().c_str();
            char *paramValue = (char *)p->value().c_str();

            // replace some characters
            for (int i = 0; i < strlen(paramValue); i++)
            {
                // replace pipe with /
                if (paramValue[i] == '|')
                {
                    paramValue[i] = '/';
                }
            }

            // filter out input with only spaces
            bool onlySpaces = true;
            for (int i = 0; i < strlen(paramValue); i++)
            {
                if (paramValue[i] != ' ')
                {
                    onlySpaces = false;
                    break;
                }
            }

            if (onlySpaces)
            {
                Serial.print("Stopping because of only spaces\n");
                return;
            }

            if (strcmp(paramName, "nickname") == 0 && strlen(paramValue) > 0)
            {
                name = paramValue;
            }
            else if (strcmp(paramName, "text") == 0 && strlen(paramValue) > 0)
            {
                text = paramValue;
            }
        }

        if (name == NULL)
        {
            // default nickname is 'anon'
            name = (char *)"anon";
        }

        if (text == NULL)
        {
            // illegal input -> return
            Serial.print("Stopping because of no text input.\n");
            return;
        }

        // allocate memory for new string
        // breaks down into:
        // - size of both strings
        // - 3 extra characters
        // - 1 for null-byte '\0' at the end
        char *resultString = (char *)malloc(strlen(name) + strlen(text) + 4);

        // combine strings
        sprintf(resultString, "%s: %s|", name, text);

        // append string to file
        f = LittleFS.open("/messages.txt", "a");
        f.write(resultString, strlen(resultString));
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

    // route rest of traffic to index.html (triggers captive portal popup)
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    // start the webserver
    server.begin();
}

void loop()
{
    dnsServer.processNextRequest();
}