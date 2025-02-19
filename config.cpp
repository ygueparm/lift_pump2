#include "config.h"

// Définition des variables
const char* ssid = "Pompe_Relevage6";     // Définie une seule fois
const char* password = "123456789";      // Définie une seule fois
const char* apHostname = "pompe1.com"; // Définie une seule fois

int Led = 2;

bool capteurBloque = false; // Valeur initiale
bool relaisDeclenche = false; // Valeur initiale
int securiteDeclenche = 0; // Valeur initiale
unsigned long tempsDebut = 0;
unsigned long tempsRelaisDeclenche = 0;
bool moteurEnMarche = false;
unsigned long tempsrelai = 0;
unsigned long tempsTotal = 0;
unsigned long jours = 0, heures = 0, minutes = 0, secondes = 0;
unsigned long dernierDemarrage = 0;
extern unsigned long TEMP_FONCTIONNEMENT_MOTEUR = 0;

unsigned long tempsDepuisDernierDemarrage[MAX_ENTRIES] = {0};
unsigned long tempsReelDeFonctionnement[MAX_ENTRIES] = {0};
int indiceDerniereEntreeTempsDepuis = 0;
int indiceDerniereEntreeTempsReel = 0;
int nombreEntries = 0;
int indiceDerniereEntree = 0;

volatile unsigned long tempsDebutReel = 0;
volatile unsigned long tempsFinReel = 0;
volatile bool contacteurDeclenche = false;
int simulateurContacteur = 0;
unsigned long tempsReel = 0;
bool miseAJourEffectuee = false;
bool initialisationVoyant = false;
bool continuerProgramme = true;  
unsigned long debutNiveauHaut = 0;
const unsigned long SEUIL_BLOCAGE = 5000;  // 10 secondes

// Définition des variables
const long intervalled = 1000;
unsigned long previousMillisled = 0;
bool ledState = false;
unsigned long previousMillis_print = 0; // Variable pour stocker le dernier temps
const unsigned long interval_print = 4000;



void initialiserVariables() {
    securiteDeclenche = 0;
    continuerProgramme = true;
}
