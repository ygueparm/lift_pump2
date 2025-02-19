#include "web_server.h"
#include <ESPAsyncWebServer.h>
#include "config.h"
#include <Preferences.h>

extern Preferences prefsPompe;
extern Preferences prefsnewTime1;

AsyncWebServer server(80);


void initWebServer() {
  server.on("/", [](AsyncWebServerRequest * request) {
    char html[5000];  // Ajustez la taille selon vos besoins
    snprintf(html, sizeof(html),
             "<!DOCTYPE html><html><head>"
             "<meta charset='UTF-8'>"
             "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
             "<title>Contrôle de la pompe</title>"
             "<style>"
             "body { font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center; padding: 20px; }"
             "button { width: 100%%; padding: 15px; font-size: 18px; color: white; border: none; border-radius: 5px; cursor: pointer; margin: 10px 0; }"
             ".btn-allumer { background-color: #4CAF50; }"
             ".btn-eteindre { background-color: #f44336; }"
             ".btn-modifier  { background-color: #0070ff; }"
             ".btn-temps { background-color: #0070ff; }"
             ".btn-tableau { background-color: #006400; }"  // Vert foncé pour le nouveau bouton
             ".btn-fermer { background-color: #7a7a7a; }"     // Gris foncé pour le bouton "Fermer"
             ".btn-reset { background-color: #FF8C00; }" //jaune foncé pour la remise a 0
             "#modalTableau { display: none; position: fixed; top: 50%%; left: 50%%; transform: translate(-50%%, -50%%); background: white; padding: 20px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); z-index: 1000; }"
             "</style>"
             "</head><body>"
             "<h1>Contrôle de la pompe</h1>"
             "<button class='btn-allumer' onclick='toggleMoteur(\"allumer\")'>Allumer le moteur</button>"
             "<button class='btn-eteindre' onclick='toggleMoteur(\"eteindre\")'>Éteindre le moteur</button>"
             "<h2>Temps de fonctionnement de la pompe</h2>"
             "<button class='btn-temps' onclick='getTempsFonctionnement()'>Voir temps de fonctionnement</button>"
             "<p id='tempsFonctionnement'></p>"
             "<button class='btn-modifier' onclick='modifierTemps()'>Modification du temps de fonctionnement</button>"
             "<p id='messageModifier'></p>"
             "<button class='btn-tableau' onclick='afficherTableauFonctionnement()'>Affichage tableau fonctionnement</button>"  // Nouveau bouton
             "<button class='btn-reset' onclick='resetTableau()'>Mise à 0</button>"  // Bouton "Mise à 0" sur la page principale
             "<div id='modalTableau'>"  // Modal pour afficher le tableau
             "<h2>Tableau de fonctionnement</h2>"
             "<pre id='tableauData'></pre>"
             "<button class='btn-fermer' onclick='document.getElementById(\"modalTableau\").style.display = \"none\";'>Fermer</button>"
             "</div>"
             "<script>"
             "function toggleMoteur(action) { fetch('/' + action); location.reload(); }"
             "function getTempsFonctionnement() {"
             "    fetch('/temps')"
             "        .then(response => response.text())"
             "        .then(data => {"
             "            document.getElementById('tempsFonctionnement').innerText = 'Temps de fonctionnement : ' + data;"
             "        });"
             "}"
             "function resetTemps() {"
             "    fetch('/reset');"
             "    location.reload();"
             "}"
             "function modifierTemps() {"
             "    let newTime = prompt('Entrez le nouveau temps de fonctionnement (en secondes) :');"
             "    if (newTime && !isNaN(newTime)) {"
             "        fetch('/set-time?new-time=' + encodeURIComponent(newTime))"
             "            .then(response => response.text())"
             "            .then(data => {"
             "                document.getElementById('messageModifier').innerText = data;"
             "                location.reload();"
             "            });"
             "    } else {"
             "        alert('Veuillez entrer un nombre valide.');"
             "    }"
             "}"
             "function afficherTableauFonctionnement() {"  // Fonction pour afficher le tableau
             "    fetch('/tableau-fonctionnement')"
             "        .then(response => response.text())"
             "        .then(data => {"
             "            document.getElementById('tableauData').innerText = data;"
             "            document.getElementById('modalTableau').style.display = 'block';"
             "        });"
             "}"
             "function resetTableau() {"
             "    if (confirm('Êtes-vous sûr de vouloir réinitialiser le tableau ?')) {"
             "        fetch('/reset-tableau')"
             "            .then(response => response.text())"
             "            .then(data => {"
             "                alert(data);"
             "                location.reload();"
             "            })"
             "            .catch(error => {"
             "                console.error('Erreur lors de la réinitialisation du tableau :', error);"
             "                alert('Une erreur est survenue lors de la réinitialisation du tableau.');"
             "            });"
             "    }"
             "}"
             "</script>"
             "</body></html>"
            );

    request->send(200, "text/html; charset=utf-8", html);

    // Spécifiez l'encodage UTF-8 dans la réponse HTTP
    request->send(200, "text/html; charset=utf-8", html);
  });

  // marche forcé moteur
  server.on("/allumer", [](AsyncWebServerRequest * request) {
    digitalWrite(MOTEUR_POMPE, LOW); // Allume la pompe
    request->send(200, "text/plain; charset=utf-8", "Moteur allumé");
  });

  //arret forcé moteur
  server.on("/eteindre", [](AsyncWebServerRequest * request) {
    digitalWrite(MOTEUR_POMPE, HIGH); // Éteint la pompe
    request->send(200, "text/plain; charset=utf-8", "Moteur éteint");
  });

  //affichage du temps de focntionnement
  server.on("/temps", [](AsyncWebServerRequest * request) {
    float tempsSecondes = TEMP_FONCTIONNEMENT_MOTEUR / 1000.0;
    String tempsFonctionnement = String(tempsSecondes, 1) + " secondes";
    request->send(200, "text/plain; charset=utf-8", tempsFonctionnement.c_str());
  });

  //mise a jour du temps de fonctionnement
  server.on("/set-time", HTTP_GET, [](AsyncWebServerRequest * request) {
    String newTimeStr = request->arg("new-time");  // Récupère le paramètre "new-time"
    if (newTimeStr.length() > 0) {
      unsigned long newTime = newTimeStr.toInt();  // Convertit en entier
      if (newTime >= 0) {  // Vérifie que la valeur est positive
        prefsnewTime1.putULong("timeValue", newTime);  // Sauvegarde dans les préférences
        request->send(200, "text/plain; charset=utf-8", "Temps mis à jour avec succès !");
      } else {
        request->send(400, "text/plain; charset=utf-8", "Valeur incorrecte pour le temps.");
      }
    } else {
      request->send(400, "text/plain; charset=utf-8", "Paramètre 'new-time' manquant.");
    }
  });

  //affichage tableau de focntionnement en j minute avant l'envoie vers le client
  server.on("/tableau-fonctionnement", HTTP_GET, [](AsyncWebServerRequest * request) {
    String data = "Indice | Temps écoulé\n";
    for (int i = 0; i < MAX_ENTRIES; i++) {
      char key[15];
      snprintf(key, sizeof(key), "temps_%d", i);
      unsigned long temps_tab = prefsPompe.getULong(key, 0);  // Récupère le temps en secondes

      // Conversion en jours, heures et minutes
      unsigned long jours_tab = temps_tab / JOUR;           // 1 jour = 86400 secondes
      unsigned long heures_tab = (temps_tab % JOUR) / HEURE; // 1 heure = 3600 secondes
      unsigned long minutes_tab = ((temps_tab % JOUR) % HEURE) / MINUTE;   // 1 minute = 60 secondes

      // Formatage du temps
      String tempsFormatte = String(jours_tab) + " j " + String(heures_tab) + " h " + String(minutes_tab) + " min";
      data += String(i) + " | " + tempsFormatte + "\n";
    }
    request->send(200, "text/plain", data);
  });

  //initaliser le tableau a 0
  server.on("/reset-tableau", HTTP_GET, [](AsyncWebServerRequest * request) {
    for (int i = 0; i < MAX_ENTRIES; i++) {
      char key[15];
      snprintf(key, sizeof(key), "temps_%d", i);
      prefsPompe.remove(key);  // Supprime chaque entrée du tableau
    }
    prefsPompe.putInt("indice_tableau", 0);  // Réinitialise l'index circulaire
    request->send(200, "text/plain", "Tableau réinitialisé avec succès !");
  });
  server.begin();
}
