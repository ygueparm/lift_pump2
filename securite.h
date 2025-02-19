#ifndef SECURITE_H
#define SECURITE_H

#include "config.h"

extern bool continuerProgramme;
extern int securiteDeclenche;

// Variables supplémentaires
extern unsigned long tempsNiveauHautActif; // Temps cumulé où niveauHaut est actif


bool verifierSecurite();



#endif
