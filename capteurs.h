#ifndef CAPTEURS_H
#define CAPTEURS_H
#include "config.h"

struct DonneesCapteurs {
    int etatContacteur;
    int niveauHaut;
    int etatRelaisSecurite;
    unsigned long tempsActuel;
};

extern DonneesCapteurs capteurs;

DonneesCapteurs lireCapteurs();

#endif
