#include "capteurs.h"
#include <Arduino.h>  // Nécessaire pour `digitalRead` et `millis`
#include "config.h"

#define CAPTEUR_CONTACTEUR 21
#define CAPTEUR_NIVEAU_HAUT 23
#define RELAIS_SECURITE 21
DonneesCapteurs capteurs; 

DonneesCapteurs lireCapteurs() {
    DonneesCapteurs data;
    data.etatContacteur = digitalRead(CAPTEUR_CONTACTEUR);
    data.niveauHaut = digitalRead(CAPTEUR_NIVEAU_HAUT);
    data.etatRelaisSecurite = digitalRead(RELAIS_SECURITE);
    data.tempsActuel = millis();
    return data;
}
