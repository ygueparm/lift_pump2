#include "web_alarm.h"
#include "config.h"  // Pour accéder à relaisDeclenche
#include <ESPAsyncWebServer.h>

void initWebAlarm() {
    // Route pour afficher la page d'alerte
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        char html[2000];
        snprintf(html, sizeof(html),
            "<!DOCTYPE html><html><head>"
            "<meta charset='UTF-8'>"
            "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
            "<title>Alerte - Problème détecté</title>"
            "<style>"
            "body { font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center; padding: 20px; }"
            "h1 { color: #d9534f; }"
            "button { width: 100%%; padding: 15px; font-size: 18px; color: white; border: none; border-radius: 5px; cursor: pointer; margin: 10px 0; }"
            ".btn-retour { background-color: #0070ff; }"
            ".btn-reset { background-color: #FF8C00; }"  // Bouton pour réinitialiser relaisDeclenche
            "</style>"
            "</head><body>"
            "<h1>⚠️ Alerte - le rotor est bloqué</h1>"
            "<p>Un problème est survenu. Le relais de controle de courant à detecté une anomalie veuillez verifer la pompe. En particulier si il n'y a pas un corps etranger qui bloque le rotor</p>"
            "<button class='btn-retour' onclick='window.location.href=\"/\"'>Retour à la page principale</button>"
            "<button class='btn-reset' onclick='resetRelais()'>Réinitialiser le relais de securité</button>"  // Nouveau bouton
            "<script>"
            "function resetRelais() {"
            "    fetch('/reset-relais')"  // Envoyer une requête pour réinitialiser relaisDeclenche
            "        .then(response => response.text())"
            "        .then(data => {"
            "            alert(data);"  // Afficher un message de confirmation
            "            window.location.href = '/';"  // Recharger la page
            "        });"
            "}"
            "</script>"
            "</body></html>"
        );
        request->send(200, "text/html", html);
    });

    // Route pour réinitialiser relaisDeclenche
    server.on("/reset-relais", HTTP_GET, [](AsyncWebServerRequest* request) {
        relaisDeclenche = false;  // Réinitialiser relaisDeclenche
        request->send(200, "text/plain", "Relais réinitialisé avec succès !");
    });
}
