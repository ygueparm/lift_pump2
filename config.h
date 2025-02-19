#ifndef CONFIG_H
#define CONFIG_H

// Broches
#define MOTEUR_POMPE         22 
#define CAPTEUR_NIVEAU_HAUT  23
#define RELAIS_SECURITE      21 
#define CAPTEUR_CONTACTEUR   4
// watchdog en seconde : 
#define WDT_TIMEOUT 5
#define DELAI_ATTENTE_REDEMARRAGE 30000
extern int Led;  // Déclaration externe de la variable Led

// Constantes de temps
const unsigned long MILLISECONDE = 1;
const unsigned long SECONDE = 1000 * MILLISECONDE;
const unsigned long MINUTE = 60 * SECONDE;
const unsigned long HEURE = 60 * MINUTE;
const unsigned long JOUR = 24 * HEURE;

//gestion led clignotante
extern const long intervalled;
extern unsigned long previousMillisled;
extern bool ledState;

// Configuration
const unsigned long TEMP_FONCTIONNEMENT_MOTEUR_CONFIG = 2000;
const unsigned long TEMP_ATTENTE_SECURITE = 10000UL;
const int MAX_SECURITE = 5;
const int MAX_ENTRIES = 10;
const int MAX_STRING_LENGTH = 100;

//constante pour aficher les message de debug gentiment
extern unsigned long previousMillis_print ; // Variable pour stocker le dernier temps
extern const unsigned long interval_print ; // Intervalle de temps en millisecondes (3 secondes)


// Options
#define SIMULATEUR_CONTACTEUR 1
#define SCRIPT 1

// Réseau
extern const char* ssid;
extern const char* password;
extern const char* apHostname; // Le nom d'hôte que vous souhaitez utiliser

// Variables d'état
extern bool capteurBloque;
extern bool relaisDeclenche;
extern unsigned long tempsDebut;
extern unsigned long tempsRelaisDeclenche;
extern bool moteurEnMarche;
extern unsigned long tempsrelai;
extern unsigned long tempsTotal;
extern unsigned long jours, heures, minutes, secondes;
extern unsigned long dernierDemarrage;
extern unsigned long TEMP_FONCTIONNEMENT_MOTEUR;

// Tableaux de stockage
extern unsigned long tempsDepuisDernierDemarrage[MAX_ENTRIES];
extern unsigned long tempsReelDeFonctionnement[MAX_ENTRIES];
extern int indiceDerniereEntreeTempsDepuis;
extern int indiceDerniereEntreeTempsReel;
extern int nombreEntries;
extern int indiceDerniereEntree;
extern unsigned long debutNiveauHaut;
extern const unsigned long SEUIL_BLOCAGE;  // 10 secondes


// Calcul du temps réel de fonctionnement avec retour du contacteur
extern volatile unsigned long tempsDebutReel;
extern volatile unsigned long tempsFinReel;
extern volatile bool contacteurDeclenche;
extern int simulateurContacteur; // Simulation retour contacteur
extern unsigned long tempsReel; // Variable du temps réel de fonctionnement
extern bool miseAJourEffectuee; // Indicateur de mise à jour tableau
extern bool initialisationVoyant;

extern bool moteurEnMarche;
extern int securiteDeclenche;
extern bool continuerProgramme;

void initialiserVariables();


#endif
