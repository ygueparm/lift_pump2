#include "securite.h"
#include <Arduino.h>  // Nécessaire pour Serial
#include "config.h"
#include "capteurs.h"

bool verifierSecurite() {
    if (securiteDeclenche >= MAX_SECURITE) {
        continuerProgramme = false;
        Serial.println("Arrêt en raison de déclenchements consécutifs de la sécurité.");
        return false;
    }
    return true;
}
