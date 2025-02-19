#include "securite.h"
#include <Arduino.h>  // Nécessaire pour Serial
#include "config.h"
#include "capteurs.h"
// Variables supplémentaires pour le capteur bloqué
unsigned long tempsNiveauHautActif = 0; // Temps cumulé où niveauHaut est actif
           // Indicateur si le capteur est bloqué

bool verifierSecurite() {
    if (securiteDeclenche >= MAX_SECURITE) {
        continuerProgramme = false;
        Serial.println("Arrêt en raison de déclenchements consécutifs de la sécurité.");
        return false;
    }
    return true;
}
