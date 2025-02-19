#ifndef POMPE_H
#define POMPE_H

#include <Arduino.h>  // Ajout de l'API Arduino
#include "config.h"
#include "capteurs.h"

void activerPompe();
void desactiverPompe();
void initialiserLed();
bool peutDemarrerPompe(DonneesCapteurs capteursActuels);  // Déclaration seulement



#endif
