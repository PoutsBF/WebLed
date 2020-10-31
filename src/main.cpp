#include <ArduinoOTA.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "WifiConfig.h"

#include <SPIFFSEditor.h>

#include <WS2812FX.h>

#include <gestionBP.h>

// ----- config de la barette neoPixel ---
// patte connectée sur SDin
#define LED_PIN D1
// nombre de leds
#define LED_COUNT 8
// définir les intervals des leds
unsigned long led_interval = 5000;
// création de l'objet led
WS2812FX leds = WS2812FX(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

// ----- création des objets serveurs et paramètres ---
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char *ssid = YOUR_WIFI_SSID;
const char *password = YOUR_WIFI_PASSWD;
const char *hostName = "esp-async";
const char *http_username = "admin";
const char *http_password = "admin";

void onWsEvent( AsyncWebSocket *server, 
                AsyncWebSocketClient *client, 
                AwsEventType type, 
                void *arg, 
                uint8_t *data, 
                size_t len);

GestionBP gestionBP;

/******************************************************************************
*   SETUP
******************************************************************************/
void setup() 
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    //-----------------------------------------------------------------------
    // Configuration du WIFI
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(hostName);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.printf("STA: Failed!\n");
        WiFi.disconnect(false);
        delay(1000);
        WiFi.begin(ssid, password);
    }

  //-----------------------------------------------------------------------
  // Programmation Over The Air
    ArduinoOTA.setHostname(hostName);
    ArduinoOTA.onStart([]() { Serial.println("Start"); });
    ArduinoOTA.onEnd([]()   { Serial.println("\nEnd"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
    {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) 
    {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
            Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
            Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
            Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
            Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
            Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    //-----------------------------------------------------------------------
    // Configuration du système de fichier, du serveur web et du websocket

    MDNS.addService("http", "tcp", 80);

    SPIFFS.begin();
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.addHandler(new SPIFFSEditor(http_username, http_password));
    server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

    server.onNotFound([](AsyncWebServerRequest *request) {
        Serial.printf("NOT_FOUND: ");
        if (request->method() == HTTP_GET)
            Serial.printf("GET");
        else if (request->method() == HTTP_POST)
            Serial.printf("POST");
        else if (request->method() == HTTP_DELETE)
            Serial.printf("DELETE");
        else if (request->method() == HTTP_PUT)
            Serial.printf("PUT");
        else if (request->method() == HTTP_PATCH)
            Serial.printf("PATCH");
        else if (request->method() == HTTP_HEAD)
            Serial.printf("HEAD");
        else if (request->method() == HTTP_OPTIONS)
            Serial.printf("OPTIONS");
        else
            Serial.printf("UNKNOWN");
        Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

        if (request->contentLength())
        {
            Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
            Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
        }

        int headers = request->headers();
        int i;
        for (i = 0; i < headers; i++)
        {
            AsyncWebHeader *h = request->getHeader(i);
            Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
        }

        int params = request->params();
        for (i = 0; i < params; i++)
        {
            AsyncWebParameter *p = request->getParam(i);
            if (p->isFile())
            {
                Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
            }
            else if (p->isPost())
            {
                Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
            else
            {
                Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
        }
        request->send(404);
    });
    server.begin();

    //-------------------------------------------------------------------------
    // Affichage des informations de connexion
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    gestionBP.init();

    leds.init();
    leds.setBrightness(255);
    leds.setSpeed(1000);
    leds.setColor(0x007BFF);
    leds.setMode(FX_MODE_STATIC);
    leds.start();
}

/******************************************************************************
*   LOOP
******************************************************************************/
void loop()
{
//    static unsigned long led_tempo = 0;
    static uint8_t modeLed = 0;

//    unsigned long now = millis();
    BP_struct_msg msg;

    ArduinoOTA.handle();
    ws.cleanupClients();

    leds.service();

    // if((millis() - led_tempo) > led_interval)
    // {
    //     leds.setMode((leds.getMode() + 1) % leds.getModeCount());
    //     Serial.printf("mode : %3d / %3d\n", leds.getMode(), leds.getModeCount());
    //     led_tempo = now;
    // }

    if(gestionBP.handle(&msg))
    {
        switch (msg.idBP)
        {
            case 0 :
            {
                if (msg.idMsg == BP_MESS_APPUIE_COURT)
                {
                    modeLed++;
                    if (modeLed == leds.getModeCount())
                        modeLed = 0;
                }
                else if (msg.idMsg == BP_MESS_APPUIE_LONG)
                {
                    modeLed += 5;
                    if (modeLed > leds.getModeCount())
                        modeLed = 0;
                }
                        } break;
            case 1 :
            {
                if (msg.idMsg == BP_MESS_APPUIE_COURT)
                {
                    if (modeLed == 0)
                    {
                        modeLed = leds.getModeCount() - 1;
                    }
                    else
                    {
                        modeLed--;
                    }
                }
                else if (msg.idMsg == BP_MESS_APPUIE_LONG)
                {
                    if (modeLed < 5)
                    {
                        modeLed = leds.getModeCount() - 1;
                    }
                    else
                    {
                        modeLed -= 5;
                    }
                }
            }
            break;
            case 2 :
            {
                if(msg.idMsg == BP_MESS_APPUIE_DOUBLE)
                {
                    modeLed = 0;
                }
            } break;
        }
        Serial.printf("mode led : %d\n", modeLed);
        leds.setMode(modeLed);
    }
}

/******************************************************************************
*   Fonctions  
******************************************************************************/
//-------------------------------------------------------------------------
// onWsEvent : gestion du web socket 
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("Hello Client %u :)", client->id());
        client->ping();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    }
    else if (type == WS_EVT_ERROR)
    {
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
    }
    else if (type == WS_EVT_PONG)
    {
        Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        String msg = "";
        if (info->final && info->index == 0 && info->len == len)
        {
            //the whole message is in a single frame and we got all of it's data
            Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

            if (info->opcode == WS_TEXT)
            {
                for (size_t i = 0; i < info->len; i++)
                {
                    msg += (char)data[i];
                }
            }
            else
            {
                char buff[3];
                for (size_t i = 0; i < info->len; i++)
                {
                    sprintf(buff, "%02x ", (uint8_t)data[i]);
                    msg += buff;
                }
            }
            Serial.printf("%s\n", msg.c_str());

            if (info->opcode == WS_TEXT)
                client->text("I got your text message");
            else
                client->binary("I got your binary message");
        }
    }
}
