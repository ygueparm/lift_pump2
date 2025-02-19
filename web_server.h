#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>

// Déclaration externe de la variable server
extern AsyncWebServer server;

// Déclaration de la fonction initWebServer()
void initWebServer();

#endif // WEB_SERVER_H