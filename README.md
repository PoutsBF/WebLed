# WebLed

Objectif : faire un éclairage variable et programmable pour un cadre 3D peint par les enfants.
Configuration par BP ou par page web

# Support
L'environnement sélectionné est un ESP8266, avec des leds NEOPIXEL

# Etapes
- programmation de l'environnement (serveur web, websocket, programmation OTA...)
- intégration de la commande des leds
- intégration des paramètres et scénarios
- intégration du paramétrage par page web

# sources
A compléter
FreeRTOS ; Adafruit pour le BME680 ; ESPHome pour la gestion web asynchrone ; ...

## Gestion du wifi / serveur / websocket
<https://github.com/OttoWinter/AsyncTCP>
<https://github.com/OttoWinter/ESPAsyncTCP>
<https://github.com/OttoWinter/ESPAsyncWebServer>

## Gestion leds NeoPixel, librairie Adafruit :
<https://github.com/adafruit/Adafruit_NeoPixel>

## ArdinoJSON par Benoit Blanchon
<https://arduinojson.org/v6/example/parser/>

## Bootstrap
<https://getbootstrap.com/>
## range-slider 
<https://seiyria.com/bootstrap-slider/>
## pour facilement faire le squelette html
<https://www.layoutit.com/>

# Résultats
A compléter...

version 0.1 :

![page_web](documentation/pageWeb.PNG?raw=true "page_web1")

version 0.2 :

![page_web](documentation/pageWeb%202020-11-17.PNG?raw=true "page_web2")
