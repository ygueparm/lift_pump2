
#include "config.h"
#include "pompe.h"
#include "capteurs.h"
#include "securite.h"
//#include <Arduino.h>
#include <Preferences.h>
#include <esp_task_wdt.h>
#include "web_server.h"
//#define TEST   // laisser pour les test sur planche a pain
#undef TEST  // a activer pour la  production

#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
// initialisation serveur AP
String contenuTableau;
DNSServer dnsServer;
AsyncWebServer server(80);  // Déclaration du serveur sur le port 80

//instance pour sauvegarder les données
Preferences prefsPompe;
Preferences prefsnewTime1;  // pour stocker le temps de focntionnement de la pompe
unsigned long dernierDemarrageRecupere = 0;
// Prototype de la fonction chargerTableau()
void chargerTableau();
void setup() {
  Serial.begin(115200);
  Serial.println("\n[*] Creating AP");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password, 10);  // Utilisation du canal 10
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());
  //resistance de tirage pour les entrée :
  pinMode(CAPTEUR_NIVEAU_HAUT, INPUT_PULLDOWN);

  // Configure le DNS local avec le serveur AP
  //**** affiche le tableau des dernier demarrages
  // Configure le DNS local avec le serveur AP
  dnsServer.start(53, apHostname, WiFi.softAPIP());

  //pour enregistrer les donnés
  prefsPompe.begin("pompe", false);  // Nom de namespace "pompe"
  // initialisation des preference pour le temps de fonctionnement
  prefsnewTime1.begin("newTime1", false);

  //configuration watchdog
  esp_task_wdt_init(WDT_TIMEOUT, true);  //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);                //add current thread to WDT watch

  chargerTableau();  //charger les données sauvegardé au demarrage

  // Initialisation des variables
  initialiserVariables();
  initialiserLed();
  // lancement du serveur web
  initWebServer();
  server.begin();
}
void loop() {
  dnsServer.processNextRequest();  //pour permettre au serveur DNS de traiter les requêtes entrantes de manière continue.
  //reveil du watchdog
  esp_task_wdt_reset();
  unsigned long currentMillisled = millis();
  //recuperation de la valeur sauvegardé du temps de fonctionnement moteur
  TEMP_FONCTIONNEMENT_MOTEUR = prefsnewTime1.getULong("timeValue", 0) * 1000;
  capteurs = lireCapteurs();  // Rafraîchir les valeurs avant le test

  //gestion du clignotement de la led sans utiliser delay🌞🌞
  if (moteurEnMarche) {
    digitalWrite(Led, HIGH);  // Forcer l'allumage de la LED lorsque le moteur est en marche
  } else {
    if (!moteurEnMarche && (currentMillisled - previousMillisled >= intervalled)) {
      previousMillisled = currentMillisled;  // Mettre à jour le dernier moment de changement d'état
      ledState = !ledState;                  // Inverser l'état de la LED
      digitalWrite(Led, ledState);           // Mettre à jour l'état de la LED
      }
  }

  // 👿👿si il y a trop de securité on arrete l'esp32, ouais carrement peut etre a modifier vers une jolie page
  if (!verifierSecurite()) {
    return;
  }
  // debug sans utiliser de delay
  if (currentMillisled - previousMillis_print >= interval_print) {
    previousMillis_print = currentMillisled;  // Mise à jour du dernier temps

    // Affichage des informations
    Serial.print("void loop État du capteur de niveau haut : ");
    Serial.println(capteurs.niveauHaut);
    Serial.print("void loop État contacteur : ");
    Serial.println(capteurs.etatContacteur);
    Serial.print("void loop État du relais sécurité : ");
    Serial.println(capteurs.etatRelaisSecurite);
    Serial.print("void loop moteur en marche : ");
    Serial.println(moteurEnMarche);
    Serial.print("etat si le capteur a été bloqué ");
    Serial.println(capteurBloque);
  }
  // Démarrage du moteur de la pompe😍
  if (peutDemarrerPompe(capteurs) && !moteurEnMarche && !capteurBloque) {
    activerPompe();
    moteurEnMarche = true;
    tempsDebut = capteurs.tempsActuel;
    tempsRelaisDeclenche = 0;  // Réinitialise le temps de déclenchement du relais de sécurité
    tempsReel = 0;             // Mise à zéro du temps de fonctionnement réel
    tempsNiveauHautActif = 0;  // Réinitialise le temps où niveauHaut est actif
    relaisDeclenche = false;   // Réinitialise l'état du relais déclenché
    Serial.println("Démarrage du moteur.");
  } else {
    if (moteurEnMarche) {  // verification si le capteur niveau haut ne reste pas bloqué
      if (capteurs.niveauHaut == HIGH) {
        if (debutNiveauHaut == 0) {
          debutNiveauHaut = millis();  // Premier passage en HIGH
        } else if (millis() - debutNiveauHaut >= SEUIL_BLOCAGE) {
          capteurBloque = true;
          Serial.println("⚠️ Capteur niveauHaut bloqué détecté !");
          debutNiveauHaut = 0;  // Reset pour éviter répétition du message
        }
      } else {
        debutNiveauHaut = 0;  // Réinitialiser si le capteur repasse à LOW
      }

      // Gestion sécurité lorsque le moteur fonctionne 😖😖
      if (capteurs.etatRelaisSecurite == HIGH) {
        desactiverPompe();
        moteurEnMarche = false;
        relaisDeclenche = true;
        tempsRelaisDeclenche = capteurs.tempsActuel;  // Enregistre le moment du déclenchement
        // Incrémenter le compteur de sécurité
        securiteDeclenche++; //incremente le nombre de fois ou cela augmente
        Serial.println("Relais de sécurité déclenché.");
      }
      // Arrêt automatique après un certain temps💓💓
      else if (capteurs.tempsActuel - tempsDebut >= TEMP_FONCTIONNEMENT_MOTEUR) {
        desactiverPompe();
        moteurEnMarche = false;
        Serial.println("Arrêt automatique du moteur après un certain temps");

        // Calcul du temps écoulé depuis le dernier démarrage
        unsigned long tempsEcoule_1 = capteurs.tempsActuel - dernierDemarrage;
        uint32_t jours = tempsEcoule_1 / JOUR;
        uint32_t heures = (tempsEcoule_1 % JOUR) / HEURE;
        uint32_t minutes = ((tempsEcoule_1 % JOUR) % HEURE) / MINUTE;

        // Affichage du temps écoulé
        Serial.print("Dernier démarrage il y a ");
        Serial.print(jours);
        Serial.print("j ");
        Serial.print(heures);
        Serial.print(":");
        if (minutes < 10) {
          Serial.print("0");
        }
        Serial.println(minutes);

        // Sauvegarde des données📥📥
        if (tempsDebut != 0) {
          tempsDepuisDernierDemarrage[indiceDerniereEntreeTempsDepuis] = tempsEcoule_1;
          indiceDerniereEntreeTempsDepuis = (indiceDerniereEntreeTempsDepuis + 1) % MAX_ENTRIES;
          Serial.print("Indice tableau temps depuis dernier démarrage : ");
          Serial.println(indiceDerniereEntreeTempsDepuis);
        }

        // Réinitialisation des variables
        dernierDemarrage = capteurs.tempsActuel;                    // Met à jour le temps du dernier démarrage
        prefsPompe.putULong("dernierDemarrage", dernierDemarrage);  // Sauvegarde le dernier démarrage dans prefs
        sauvegarderTableau();

        // Réinitialiser les compteurs liés au capteur
        tempsNiveauHautActif = 0;
      }
    }
  }

  //calcul du temps reel de fcontionnement pas encore utiliser 

  if (tempsDebut != 0) {
    if (capteurs.etatContacteur == HIGH && !contacteurDeclenche) {
      tempsDebutReel = millis();
      contacteurDeclenche = true;
    } else if (capteurs.etatContacteur == LOW && contacteurDeclenche) {
      tempsFinReel = millis();
      //  tempsReel = tempsFinReel - tempsDebutReel;

      tempsReel = (tempsFinReel - tempsDebutReel) / 1000;  // Conversion en secondes

      Serial.print("Temps de fonctionnement réel : ");
      Serial.print(tempsReel);
      Serial.println(" secondes  ");
      miseAJourEffectuee = true;  //valide la mise a jour du tableau
      contacteurDeclenche = false;
    }
  }
}

void sauvegarderTableau() {
  // Sauvegarder chaque élément du tableau
  for (int i = 0; i < MAX_ENTRIES; i++) {
    String key = "temps_" + String(i);  // Clé unique pour chaque entrée
    prefsPompe.putULong(key.c_str(), tempsDepuisDernierDemarrage[i]);
  }
  // Sauvegarder l'index circulaire
  prefsPompe.putInt("indice_tableau", indiceDerniereEntreeTempsDepuis);
  Serial.println("Données du tableau sauvegardées avec succès.");
}

void chargerTableau() {
  // Charger chaque élément du tableau
  for (int i = 0; i < MAX_ENTRIES; i++) {
    String key = "temps_" + String(i);
    tempsDepuisDernierDemarrage[i] = prefsPompe.getULong(key.c_str(), 0);  // Valeur par défaut à 0 si absent
  }
  // Charger l'index circulaire
  indiceDerniereEntreeTempsDepuis = prefsPompe.getInt("indice_tableau", 0);  // Valeur par défaut à 0 si absent
  // Charger le dernier démarrage
  dernierDemarrage = prefsPompe.getULong("dernierDemarrage", 0);  // Valeur par défaut à 0 si absent
  Serial.println("Données du tableau chargées avec succès.");
}
