
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
#include "LittleFS.h"
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
  WiFi.softAP(ssid, password, 13);  // Utilisation du canal 10
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());
  //resistance de tirage pour les entrée :
  pinMode(CAPTEUR_NIVEAU_HAUT, INPUT_PULLDOWN);
  pinMode(RELAIS_SECURITE, INPUT_PULLDOWN);
  pinMode(CAPTEUR_CONTACTEUR, INPUT_PULLDOWN);
  pinMode(MOTEUR_POMPE, OUTPUT);
  digitalWrite(MOTEUR_POMPE, HIGH);
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

//initilaisation de LittleFS pour les log
 if (!LittleFS.begin(true)) {
        Serial.println("Erreur lors de l'initialisation de LittleFS");
        return;
    }
    Serial.println("LittleFS initialisé avec succès");


  chargerTableau();  //charger les données sauvegardé au demarrage
  Serial.print("⬅️ Chargement de indice_tableau après reboot : ");
  Serial.println(indiceDerniereEntreeTempsDepuis);

  // Initialisation des variables
  initialiserVariables();
  initialiserLed();

  // lancement du serveur web
  initWebServer();
  server.begin();
    // Vérifier si le fichier existe, sinon le créer
    if (!LittleFS.exists("/compteurs.txt")) {
        Serial.println("Fichier compteurs.txt absent, création en cours...");
        File file = LittleFS.open("/compteurs.txt", FILE_WRITE);
        if (file) {
            file.close();
            Serial.println("Fichier compteurs.txt créé avec succès.");
        } else {
            Serial.println("Erreur lors de la création du fichier compteurs.txt !");
        }
    }
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
  unsigned long intervalActuel = (capteurBloque || relaisDeclenche) ? intervalled / 6 : intervalled;

  if (moteurEnMarche && !capteurBloque || moteurEnMarche && !relaisDeclenche) {
    digitalWrite(Led, HIGH);  // Allumer la LED en continu quand le moteur est en marche et le capteur n'est pas bloqué
  } else {
    if (currentMillisled - previousMillisled >= intervalActuel) {
      previousMillisled = currentMillisled;  // Mise à jour du dernier changement
      ledState = !ledState;                  // Inversion de l'état de la LED
      digitalWrite(Led, ledState);           // Mise à jour de la LED
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
    lireCompteurs();
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
        securiteDeclenche++;  //incremente le nombre de fois ou cela augmente
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


    // Mettre à jour les compteurs uniquement si l'état change
    compteurNiveauHaut += (capteurs.niveauHaut && !precedentNiveauHaut) ? 1 : 0;
    compteurContacteur += (capteurs.etatContacteur && !precedentContacteur) ? 1 : 0;
    compteurRelaisSecurite += (capteurs.etatRelaisSecurite && !precedentRelaisSecurite) ? 1 : 0;
    compteurMoteurEnMarche += (moteurEnMarche && !precedentMoteurEnMarche) ? 1 : 0;
    compteurCapteurBloque += (capteurBloque && !precedentCapteurBloque) ? 1 : 0;
   // Mettre à jour les états précédents
    precedentNiveauHaut = capteurs.niveauHaut;
    precedentContacteur = capteurs.etatContacteur;
    precedentRelaisSecurite = capteurs.etatRelaisSecurite;
    precedentMoteurEnMarche = moteurEnMarche;
    precedentCapteurBloque = capteurBloque;
   // Enregistrer les compteurs dans LittleFS (optionnel, à faire moins fréquemment)
    static unsigned long dernierEnregistrement = 0;
    if (millis() - dernierEnregistrement > 60000) {  // Enregistrer toutes les 60 secondes
        enregistrerCompteurs(compteurNiveauHaut, compteurContacteur, compteurRelaisSecurite, compteurMoteurEnMarche, compteurCapteurBloque);
        dernierEnregistrement = millis();
    }
  
  
}

void sauvegarderDonnees(const char* key, const uint32_t value) {
  prefsPompe.putULong(key, value);
}

uint32_t chargerDonnees(const char* key, uint32_t defaultValue = 0) {
  return prefsPompe.getULong(key, defaultValue);
}

void sauvegarderTableau() {
  for (int i = 0; i < MAX_ENTRIES; i++) {
    sauvegarderDonnees(("temps_" + String(i)).c_str(), tempsDepuisDernierDemarrage[i]);
  }
  sauvegarderDonnees("indice_tableau", indiceDerniereEntreeTempsDepuis);
  sauvegarderDonnees("dernierDemarrage", dernierDemarrage);

  Serial.println("Données du tableau sauvegardées avec succès.");
}

void chargerTableau() {
  for (int i = 0; i < MAX_ENTRIES; i++) {
    tempsDepuisDernierDemarrage[i] = chargerDonnees(("temps_" + String(i)).c_str());
  }
  indiceDerniereEntreeTempsDepuis = chargerDonnees("indice_tableau");
  dernierDemarrage = chargerDonnees("dernierDemarrage");
  Serial.println("Données du tableau chargées avec succès.");
}

//ecrire l'ensemble du fonctionnement de l'esp32 
void enregistrerCompteurs(int niveauHaut, int etatContacteur, int etatRelaisSecurite, int moteurEnMarche, int capteurBloque) {
    // Ouvrir le fichier en mode écriture (écrase le fichier existant)
    File file = LittleFS.open("/compteurs.txt", FILE_WRITE);
    if (!file) {
        Serial.println("Erreur lors de l'ouverture du fichier compteurs pour ecrire");
        return;
    }

    // Écrire les compteurs dans le fichier
    file.printf("Niveau haut: %d\nContacteur: %d\nRelais sécurité: %d\nMoteur en marche: %d\nCapteur bloqué: %d\n",
                niveauHaut, etatContacteur, etatRelaisSecurite, moteurEnMarche, capteurBloque);

    // Fermer le fichier
    file.close();
    Serial.println("Compteurs enregistrés");
}

//lire le fichier de log dans la console serie
void lireCompteurs() {
    // Ouvrir le fichier en mode lecture
    File file = LittleFS.open("/compteurs.txt", FILE_READ);
    if (!file) {
        Serial.println("Erreur lors de l'ouverture du fichier compteurs pour lire");
        return;
    }

    // Lire et afficher le contenu du fichier
    Serial.println("Compteurs :");
    while (file.available()) {
        Serial.write(file.read());
    }

    // Fermer le fichier
    file.close();
}