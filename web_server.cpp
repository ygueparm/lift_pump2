#include "web_server.h"
#include <ESPAsyncWebServer.h>
#include "config.h"
#include <Preferences.h>

extern Preferences prefsPompe;
extern Preferences prefsnewTime1;

extern AsyncWebServer server;


void initWebServer() {
  Serial.print("capteurBloque: ");
  Serial.println(capteurBloque);
  Serial.print("relaisDeclenche: ");
  Serial.println(relaisDeclenche);
  Serial.print("securiteDeclenche: ");
  Serial.println(securiteDeclenche);
  Serial.print("MAX_SECURITE: ");
  Serial.println(MAX_SECURITE);

  server.on("/", [](AsyncWebServerRequest* request) {
    if (capteurBloque == true) {
      Serial.println("🚨 Condition capteurBloque OK");
      Serial.print("🔍 Valeur brute : ");
      Serial.println((int)capteurBloque);
      char html[3500];
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
               "<h1>⚠️ Alerte - capteur niveau haut</h1>"
               "<p>Un problème est survenu. la capteur niveau haut semble bloqué en position haute, il faut le nettoyer pour le debloquer</p>"
               "<button class='btn-retour' onclick='window.location.href=\"/\"'>Retour à la page principale</button>"
               "<button class='btn-reset' onclick='resetCapteur()'>Réinitialiser le capteur de niveau haut</button>"
               "<script>"
               "function resetCapteur() {"
               "    document.body.style.cursor = 'wait';"
               "    fetch('/reset-capteur', { method: 'GET' })"
               "        .then(response => {"
               "            if (!response.ok) {"
               "                throw new Error('Erreur lors de la réinitialisation du capteur.');"
               "            }"
               "            return response.text();"
               "        })"
               "        .then(data => {"
               "            alert(data);"
               "            setTimeout(() => {"
               "                window.location.href = '/';"
               "                document.body.style.cursor = 'default';"
               "            }, 2000);"
               "        })"
               "        .catch(error => {"
               "            console.error('Erreur réseau :', error);"
               "            alert('Une erreur est survenue lors de la réinitialisation du capteur.');"
               "            document.body.style.cursor = 'default';"
               "        });"
               "}"
               "</script>"
               "</body></html>");
      Serial.println("🚀 Envoi de la page web");
      request->send(200, "text/html", html);
    } else if (relaisDeclenche) {
      Serial.println("⚡ Condition relaisDeclenche OK");
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
               "<p>Un problème est survenu. Le relais de controle de courant à detecté une anomalie veuillez verifer la pompe. En particulier si il n'y a pas un corps etranger qui bloque le rotor, temps d'attente environ 2 minute avant le prochain demarrage</p>"
               "<button class='btn-retour' onclick='window.location.href=\"/\"'>Retour à la page principale</button>"
               "<button class='btn-reset' onclick='resetRelais()'>Réinitialiser le relais de securité</button>"  // Nouveau bouton
               "<script>"
               "function resetRelais() {"
               "    fetch('/reset-relais')"  // Envoyer une requête pour réinitialiser relaisDeclenche
               "        .then(response => response.text())"
               "        .then(data => {"
               "            alert(data);"                 // Afficher un message de confirmation
               "            window.location.href = '/';"  // Recharger la page
               "        });"
               "}"
               "</script>"
               "</body></html>");
      request->send(200, "text/html", html);
    }
    else if (securiteDeclenche >= MAX_SECURITE) {
      Serial.println("🔒 Condition securiteDeclenche OK");
      char html[3500];
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
               "<h1>⚠️ Alerte - Trop de sur/sous courant détecté</h1>"
               "<p>Un problème est survenu. le relais de surveillance de courant c'est declanché trop souvent😫, la pompe est peut etre bloqué😢 😭</p>"
               "<button class='btn-retour' onclick='window.location.href=\"/\"'>Retour à la page principale</button>"
               "<button class='btn-reset' onclick='reset-securite()'>Réinitialiser la securité</button>"
               "<script>"
               "function resetSecurite() {"
               "    document.body.style.cursor = 'wait';"
               "    fetch('/reset-securite', { method: 'GET' })"
               "        .then(response => {"
               "            if (!response.ok) {"
               "                throw new Error('Erreur lors de la réinitialisation du capteur.');"
               "            }"
               "            return response.text();"
               "        })"
               "        .then(data => {"
               "            alert(data);"
               "            setTimeout(() => {"
               "                window.location.href = '/';"
               "                document.body.style.cursor = 'default';"
               "            }, 2000);"
               "        })"
               "        .catch(error => {"
               "            console.error('Erreur réseau :', error);"
               "            alert('Une erreur est survenue lors de la réinitialisation du capteur.');"
               "            document.body.style.cursor = 'default';"
               "        });"
               "}"
               "</script>"
               "</body></html>");
      request->send(200, "text/html", html);
    } else {
      Serial.println("✅ Aucune condition remplie");
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
               ".btn-fermer { background-color: #7a7a7a; }"   // Gris foncé pour le bouton "Fermer"
               ".btn-reset { background-color: #006400; }"    //Vert foncé
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
               "<button class='btn-reset' onclick='resetTableau()'>Mise à 0 du tableau</button>"                                             // Bouton "Mise à 0" sur la page principale
               "<div id='modalTableau'>"                                                                                          // Modal pour afficher le tableau
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
               "</body></html>");


      // Spécifiez l'encodage UTF-8 dans la réponse HTTP
      request->send(200, "text/html; charset=utf-8", html);
    }
  });  //cloture de la focntion

  // marche forcé moteur
  server.on("/allumer", [](AsyncWebServerRequest* request) {
    digitalWrite(MOTEUR_POMPE, LOW);  // Allume la pompe
    request->send(200, "text/plain; charset=utf-8", "Moteur allumé");
  });

  //arret forcé moteur
  server.on("/eteindre", [](AsyncWebServerRequest* request) {
    digitalWrite(MOTEUR_POMPE, HIGH);  // Éteint la pompe
    request->send(200, "text/plain; charset=utf-8", "Moteur éteint");
  });

  //affichage du temps de focntionnement
  server.on("/temps", [](AsyncWebServerRequest* request) {
    float tempsSecondes = TEMP_FONCTIONNEMENT_MOTEUR / 1000.0;
    String tempsFonctionnement = String(tempsSecondes, 1) + " secondes";
    request->send(200, "text/plain; charset=utf-8", tempsFonctionnement.c_str());
  });

  //mise a jour du temps de fonctionnement
  server.on("/set-time", HTTP_GET, [](AsyncWebServerRequest* request) {
    String newTimeStr = request->arg("new-time");  // Récupère le paramètre "new-time"
    if (newTimeStr.length() > 0) {
      unsigned long newTime = newTimeStr.toInt();      // Convertit en entier
      if (newTime >= 0) {                              // Vérifie que la valeur est positive
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
  server.on("/tableau-fonctionnement", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Récupérer l'indice récent depuis les préférences
    int indiceRecent = indiceDerniereEntreeTempsDepuis;  //prefsPompe.getInt("indice_tableau", 0); // Valeur par défaut : 0

    // Calculer l'indice de la dernière entrée
    int dernierEntree = (indiceRecent > 0) ? (indiceRecent - 1) : (MAX_ENTRIES - 1);

    // Récupérer la dernière valeur enregistrée
    char dernierKey[15];
    snprintf(dernierKey, sizeof(dernierKey), "temps_%d", dernierEntree);
    unsigned long derniereValeur = tempsDepuisDernierDemarrage[dernierEntree];  //  prefsPompe.getULong(dernierKey, 0); // Valeur par défaut : 0

    Serial.println("Lecture de : " + String(dernierKey));
    Serial.println("Valeur récupérée : " + String(derniereValeur));
    // Conversion de la dernière valeur en jours, heures et minutes
    unsigned long jours_dernier = derniereValeur / JOUR;
    unsigned long heures_dernier = (derniereValeur % JOUR) / HEURE;
    unsigned long minutes_dernier = ((derniereValeur % JOUR) % HEURE) / MINUTE;

    // Formatage de la dernière valeur
    String derniereValeurFormattee = String(jours_dernier) + " j " + String(heures_dernier) + " h " + String(minutes_dernier) + " min";

    // Créer la chaîne de réponse
    String data = "Indice récent : " + String(indiceRecent) + "\n";                                          // Indice récent
    data += "Dernière entrée (indice " + String(dernierEntree) + ") : " + derniereValeurFormattee + "\n\n";  // Dernière entrée

    // Ajouter toutes les entrées du tableau
    data += "Indice | Temps écoulé\n";
    for (int i = 0; i < MAX_ENTRIES; i++) {
      char key[15];
      snprintf(key, sizeof(key), "temps_%d", i);              // Clé pour chaque entrée
      unsigned long temps_tab = prefsPompe.getULong(key, 0);  // Récupérer la valeur (valeur par défaut : 0)

      // Conversion en jours, heures et minutes
      unsigned long jours_tab = temps_tab / JOUR;
      unsigned long heures_tab = (temps_tab % JOUR) / HEURE;
      unsigned long minutes_tab = ((temps_tab % JOUR) % HEURE) / MINUTE;

      // Formatage du temps
      String tempsFormatte = String(jours_tab) + " j " + String(heures_tab) + " h " + String(minutes_tab) + " min";

      // Ajouter la ligne au résultat
      data += String(i) + " | " + tempsFormatte + "\n";
    }

    // Envoyer la réponse au client
    request->send(200, "text/plain", data);
  });

  //reinitialiser la memoire du tableau
  server.on("/reset-tableau", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Effacer toutes les préférences liées au tableau
    prefsPompe.begin("pompe", false);  // Ouvrir le namespace "pompe"
    prefsPompe.clear();                // Effacer toutes les clés dans ce namespace
    prefsPompe.end();                  // Fermer le namespace

    // Réinitialiser manuellement l'indice du tableau
    prefsPompe.putInt("indice_tableau", 0);

    request->send(200, "text/plain", "Tableau réinitialisé avec succès !");
  });

  
  // Route pour réinitialiser relais securité declenche
  server.on("/reset-relais", HTTP_GET, [](AsyncWebServerRequest* request) {
    relaisDeclenche = false;  // Réinitialiser relaisDeclenche
    request->send(200, "text/plain", "Relais réinitialisé avec succès !");
  });

  // Route pour réinitialiser capteur bloque
  server.on("/reset-capteur", HTTP_GET, [](AsyncWebServerRequest* request) {
     capteurBloque = false;  // Réinitialiser capteurBloque
    // Créer une réponse HTML avec une balise <h1>
    const char* responseHtml =
      "Capteur réinitialisé avec succès !";
    request->send(200, "text/html;charset=UTF-8", responseHtml);
  });

  server.on("/reset-securite", HTTP_GET, [](AsyncWebServerRequest* request) {
   securiteDeclenche = 0;  // Réinitialiser capteurBloque
    // Créer une réponse HTML avec une balise <h1>
    relaisDeclenche = false;
    const char* responseHtml =
      "relais réinitialisé avec succès !";
    request->send(200, "text/html;charset=UTF-8", responseHtml);
  });

  server.begin();
}
