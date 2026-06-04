#pragma once

#include "accel_gyro.h"
#include "communication.h"
#include "detection.h"
#include "mesures.h"
#include "temperature.h"
#include "trame.h"

#include <chrono>
#include <string>

/******************************************************************************
 * HEADER RUCHE
 * Orchestration: capteurs, detection, trames SMS
 ******************************************************************************/

class RucheTemoin {
public:
    RucheTemoin();

    bool demarrer();
    void executerBoucle();

private:
    std::string idRuchier_;
    std::string idRuche_;
    std::string numeroTelephone_;
    std::string numeroApiculteur_;
    float longitude_ = 0.0f;
    float latitude_ = 0.0f;
    int intervalleEnvoiMin_;
    bool temperatureActive_ = false;
    bool mouvementActif_ = false;

    Communication modem_;
    Temperature capteurTemperature_;
    AcceleroGyroscope capteurMouvement_;
    DetecteurEvenements detecteur_;
    EncodeurTrame encodeur_;

    std::chrono::steady_clock::time_point heureDernierEnvoi_;
    std::chrono::steady_clock::time_point heureDerniereAlerte_;

    MesuresLocales lireMesures();
    DonneesPeriodiques construireDonnees(const MesuresLocales& mesures) const;
    void envoyerTrame(const MesuresLocales& mesures, const std::string& etat);
    void traiterDemandeSms(const MesuresLocales& mesures);
    void verifierEtEnvoyerAlertes(const MesuresLocales& mesures);
    void envoyerMesuresPeriodiques(const MesuresLocales& mesures);
};
