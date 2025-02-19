#include "pompe.h"
#include <Arduino.h>  // Nécessaire pour `digitalWrite`
#include "config.h"
#include "capteurs.h"


void updateLedState() {
  if (moteurEnMarche) {
    digitalWrite(Led, HIGH);  // Allume la LED lorsque le moteur est en marche
  } else {
    digitalWrite(Led, LOW);  // Éteint la LED lorsque le moteur est à l'arrêt
  }
}

void initialiserLed() {
  pinMode(Led, OUTPUT);  // Déplacer ici la configuration de la broche Led
}

void activerPompe() {
  if (!moteurEnMarche) {  // Empêche de redémarrer si déjà en marche
    digitalWrite(MOTEUR_POMPE, LOW);
    // digitalWrite(Led, HIGH);
    Serial.println("commande moteur marche");
    updateLedState();       // Mettre à jour l'état de la LED
    tempsDebut = millis();  // Enregistre le moment du démarrage
  }
}

void desactiverPompe() {
  if (moteurEnMarche) {  // Empêche d'arrêter si déjà à l'arrêt
    digitalWrite(MOTEUR_POMPE, HIGH);
    // digitalWrite(Led, LOW);
    Serial.println("commande moteur arret");
    updateLedState();  // Mettre à jour l'état de la LED
  }
}

bool peutDemarrerPompe(DonneesCapteurs capteursActuels) {
  static unsigned long previousMillisAffichage = 0;  // Temps du dernier affichage
  const unsigned long intervalAffichage = 5000;      // Intervalle d'affichage (4 secondes)
  unsigned long currentMillis = millis();            // Temps actuel

  // Vérifie si l'intervalle d'affichage est écoulé
  if (currentMillis - previousMillisAffichage >= intervalAffichage) {
    previousMillisAffichage = currentMillis;  // Mise à jour du dernier affichage

    // Affichage des conditions de démarrage
    Serial.println("Vérification des conditions de démarrage...");
    Serial.print("État du niveau Haut : ");
    Serial.println(capteursActuels.niveauHaut);

    if (capteursActuels.niveauHaut == HIGH) {
      Serial.println("Condition remplie : niveauHaut est haut.");
    } else {
      Serial.println("Condition non remplie : niveauHaut est bas.");
    }

    if (capteursActuels.etatContacteur != LOW) {
      Serial.println("Condition non remplie : contacteur pas en position.");
    }

    if (capteursActuels.etatRelaisSecurite != LOW) {
      Serial.println("Condition non remplie : relais sécurité actif.");
    }
  }
  //ajoute un delai avant redeclecnhement de la pompe
  if (relaisDeclenche && (currentMillis - tempsRelaisDeclenche < DELAI_ATTENTE_REDEMARRAGE)) {
    // Calcul du temps restant avant le redémarrage autorisé
    unsigned long tempsRestant = (DELAI_ATTENTE_REDEMARRAGE - (currentMillis - tempsRelaisDeclenche)) / 1000;
    Serial.print("Redémarrage impossible : attente après déclenchement du relais. ");
    Serial.print(tempsRestant);
    Serial.println(" secondes restantes.");
    return false;  // Empêcher le démarrage pendant le délai
    // Vérification logique des conditions
  }
  if (capteursActuels.niveauHaut == HIGH && capteursActuels.etatContacteur == LOW && capteursActuels.etatRelaisSecurite == LOW) {
    if (currentMillis - previousMillisAffichage >= intervalAffichage) {
      Serial.println("Toutes les conditions sont remplies, démarrage possible !");
    }
    return true;
  } else {
    return false;
  }
}
