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

    // create file for text messages if it doesn't exist yet
    f = LittleFS.open("/messages.txt", "a");
    f.close();

    // route traffic to index.html file
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    // route to /styles.css file
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/styles.css", "text/css");
    });

    // route to /scripts.js file
    server.on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/scripts.js", "text/js");
    });

    // route to send text
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
            for (unsigned int i = 0; i < strlen(paramValue); i++)
            {
                // replace pipe with /
                // because we use pipe as separator between messages.
                // if anyone used the pipe symbol in their messages, it would
                // mess up everything.
                if (paramValue[i] == '|')
                {
                    paramValue[i] = '/';
                }
            }

            // test if input only has spaces
            bool onlySpaces = true;

            if (strlen(paramValue) == 0)
            {
                onlySpaces = false;
            }

            for (unsigned int i = 0; i < strlen(paramValue); i++)
            {
                if (paramValue[i] != ' ')
                {
                    onlySpaces = false;
                    break;
                }
            }

            if (strcmp(paramName, "nickname") == 0 && strlen(paramValue) > 0 && !onlySpaces)
            {
                name = paramValue;
            }
            else if (strcmp(paramName, "text") == 0 && strlen(paramValue) > 0 && !onlySpaces)
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
            // illegal input -> abort
            request->redirect("/");
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
        request->redirect("/");
    });

    // route to show message-file contents
    server.on("/showText", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/messages.txt", "text/html");
    });

    // route to clear message-file
    server.on("/clear", HTTP_GET, [](AsyncWebServerRequest *request) {
        f = LittleFS.open("/messages.txt", "w");
        f.close();
        request->redirect("/");
    });

    // route rest of traffic to index (triggers captive portal popup)
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });

    // start the webserver
    server.begin();
}

void loop()
{
    dnsServer.processNextRequest();
}