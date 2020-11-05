#include <ArduinoOTA.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "WifiConfig.h"

#include <SPIFFSEditor.h>

#include <WS2812FX.h>

#include <gestionBP.h>

#include <ArduinoJson.h>

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

char json_liste_modes[2048];

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
    leds.setBrightness(50);
    leds.setSpeed(1000);
    leds.setColor(0x0000FF);
    leds.setMode(FX_MODE_FIREWORKS_RANDOM);
    leds.start();

// objectif : json_liste_mode = {modes:["mode a", "mode b"...]}
    strncpy(json_liste_modes, "{\"modes\":[", 2048);
    for (uint8_t i = 0; i < leds.getModeCount(); i++)
    {
        char buffer[40];
        if(i == 0)
            snprintf(buffer, 40, "\"%s\"", (const char *)leds.getModeName(i));
        else
            snprintf(buffer, 40, ", \"%s\"", (const char *)leds.getModeName(i));

        strncat(json_liste_modes, buffer, 2048);
    }
    strncat(json_liste_modes, "]}", 2048);
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

    if(gestionBP.handle(&msg))
    {
        Serial.printf("BP%d - msg %d\n", msg.idBP, msg.idMsg);
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
                if (msg.idMsg == BP_MESS_APPUIE_COURT)
                {
                    leds.setSpeed((leds.getSpeed() > 10) ? (leds.getSpeed() - 1) : 0);
                }
                else if (msg.idMsg == BP_MESS_APPUIE_LONG)
                {
                    leds.setSpeed(leds.getSpeed() - 1);
                }
                else if (msg.idMsg == BP_MESS_APPUIE_DOUBLE)
                {
                    modeLed = 0;
                }
            } break;
        }
        Serial.printf("mode led : %d\n", modeLed);
        leds.setMode(modeLed);
        char buffer[48];
        snprintf(buffer, 48, "{\"mode\":%d,\"speed\":%d,\"couleur\":%d,\"lum\":%d}",
                 leds.getMode(), leds.getSpeed(),
                 leds.getColor(), leds.getBrightness());
        ws.textAll(buffer);
    }
}

/******************************************************************************
*   Fonctions  
******************************************************************************/
//-------------------------------------------------------------------------
// onWsEvent : gestion du web socket 
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    static StaticJsonDocument<200> doc;

    if (type == WS_EVT_CONNECT)
    {
        char buffer[48];

        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->text(json_liste_modes);
        snprintf(buffer, 48, "{\"mode\":%d,\"speed\":%d,\"couleur\":%d,\"lum\":%d}",
                 leds.getMode(), leds.getSpeed(),
                 leds.getColor(), leds.getBrightness());
        client->text(buffer);
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
            //the whole message is in a single frame and we got all of it's data
            Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

            if (info->opcode == WS_TEXT)
            {
                // ---------------------- Debug
                for (size_t i = 0; i < info->len; i++)
                {
                    msg += (char)data[i];
                }
                Serial.printf("%s\n", msg.c_str());
                // ---------------------- Debug

                // Deserialize the JSON document et
                // Test if parsing succeeds.
                if (deserializeJson(doc, data))
                {
                    Serial.println(F("deserializeJson() failed: "));
                    return;
                }
                else
                {
                    JsonVariant mode = doc["mode"];
                    if (!mode.isNull())
                    {
                        Serial.printf("mode : %d ", mode.as<uint8_t>());
                        leds.setMode((uint8_t)mode.as<uint8_t>());
                    }
                    mode = doc["couleur"];
                    if (!mode.isNull())
                    {
                        Serial.printf("couleur : %d ", mode.as<uint32_t>());
                        leds.setColor(mode.as<uint32_t>());
                    }
                    mode = doc["lum"];
                    if (!mode.isNull())
                    {
                        Serial.printf("luminosité : %d ", mode.as<uint8_t>());
                        leds.setBrightness(mode.as<uint8_t>());
                    }
                    mode = doc["speed"];
                    if (!mode.isNull())
                    {
                        Serial.printf("vitesse : %d ", mode.as<uint16_t>());
                        leds.setSpeed(mode.as<uint16_t>());
                    }
                }
            }
    }
}
