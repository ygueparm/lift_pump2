#include "web_alarm.h"
#include <ESPAsyncWebServer.h>

void initWebAlarm(AsyncWebServer& server) {
    // Définir la page HTML pour l'alerte
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        char html[2000];  // Ajustez la taille selon vos besoins
        snprintf(html, sizeof(html),
            "<!DOCTYPE html><html><head>"
            "<meta charset='UTF-8'>"
            "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
            "<title>Alerte - Problème détecté</title>"
            "<style>"
            "body { font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center; padding: 20px; }"
            "h1 { color: #d9534f; }"  // Rouge pour indiquer une alerte
            "button { width: 100%%; padding: 15px; font-size: 18px; color: white; border: none; border-radius: 5px; cursor: pointer; margin: 10px 0; }"
            ".btn-retour { background-color: #0070ff; }"  // Bouton pour retourner à la page principale
            "</style>"
            "</head><body>"
            "<h1>⚠️ Alerte - Problème détecté</h1>"
            "<p>Un problème a été détecté dans le système. Veuillez vérifier les paramètres ou contacter l'administrateur.</p>"
            "<button class='btn-retour' onclick='window.location.href=\"/\"'>Retour à la page principale</button>"
            "</body></html>"
        );
        request->send(200, "text/html", html);
    });
}
