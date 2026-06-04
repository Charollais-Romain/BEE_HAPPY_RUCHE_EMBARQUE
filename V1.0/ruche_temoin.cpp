#include "ruche_temoin.h"

#include <cstdio>
#include <thread>

namespace {
    const char* PORT_MODEM = "/dev/ttyUSB0";
    const int INTERVALLE_MIN_ALERTE_SEC = 60;
}

RucheTemoin::RucheTemoin()
    : idRuchier_("RH001"),
      idRuche_("R003"),
      numeroTelephone_("0622400469"),
      numeroApiculteur_("0659599042"),
      intervalleEnvoiMin_(15),
      modem_(PORT_MODEM),
      capteurTemperature_(),
      capteurMouvement_("/dev/i2c-1", 0x6A),
      heureDernierEnvoi_(std::chrono::steady_clock::now()),
      heureDerniereAlerte_(std::chrono::steady_clock::now() - std::chrono::seconds(INTERVALLE_MIN_ALERTE_SEC)) {}

bool RucheTemoin::demarrer() {
    std::printf("RUCHE demarrage %s / %s\n", idRuchier_.c_str(), idRuche_.c_str());
    std::printf("RUCHE alertes SMS vers %s\n", numeroApiculteur_.c_str());
    std::printf("RUCHE modem %s\n", PORT_MODEM);

    const std::string reponseAt = modem_.executerCommande("AT", 1500);
    std::printf("MODEM test AT: %s\n", reponseAt.find("OK") != std::string::npos ? "OK" : "ECHEC");
    const std::string reponseCpin = modem_.executerCommande("AT+CPIN?", 1500);
    const std::string reponseCreg = modem_.executerCommande("AT+CREG?", 1500);
    const std::string reponseCsq = modem_.executerCommande("AT+CSQ", 1500);
    std::printf("MODEM CPIN: %s\n", reponseCpin.c_str());
    std::printf("MODEM CREG: %s\n", reponseCreg.c_str());
    std::printf("MODEM CSQ : %s\n", reponseCsq.c_str());

    mouvementActif_ = capteurMouvement_.demarrer();
    if (!mouvementActif_) {
        std::fprintf(stderr, "RUCHE avertissement: accelero/gyroscope indisponible\n");
    }

    temperatureActive_ = capteurTemperature_.demarrer();
    if (!temperatureActive_) {
        std::fprintf(stderr, "RUCHE avertissement: capteur temperature indisponible\n");
    }
    detecteur_.initialiser();

    if (!mouvementActif_ && !temperatureActive_) {
        std::fprintf(stderr, "RUCHE erreur: aucun capteur disponible\n");
        return false;
    }
    return true;
}

MesuresLocales RucheTemoin::lireMesures() {
    MesuresLocales mesures;

    if (temperatureActive_ && !capteurTemperature_.lire(mesures.temperature)) {
        std::fprintf(stderr, "RUCHE avertissement: lecture temperature echouee\n");
    }
    if (mouvementActif_) {
        if (!capteurMouvement_.lireAcceleration(mesures.accelerationX, mesures.accelerationY, mesures.accelerationZ)) {
            std::fprintf(stderr, "RUCHE avertissement: lecture acceleration echouee\n");
        }
        if (!capteurMouvement_.lireGyroscope(mesures.gyroscopeX, mesures.gyroscopeY, mesures.gyroscopeZ)) {
            std::fprintf(stderr, "RUCHE avertissement: lecture gyroscope echouee\n");
        }
    }

    return mesures;
}

DonneesPeriodiques RucheTemoin::construireDonnees(const MesuresLocales& mesures) const {
    DonneesPeriodiques donnees;
    donnees.masseKg = 0.0f;
    donnees.longitude = longitude_;
    donnees.latitude = latitude_;
    donnees.temperature = mesures.temperature;
    return donnees;
}

void RucheTemoin::envoyerTrame(const MesuresLocales& mesures, const std::string& etat) {
    const DonneesPeriodiques donnees = construireDonnees(mesures);
    const std::string trame = encodeur_.creer(numeroTelephone_, donnees, etat);
    std::printf("RUCHE trame (%s): %s\n", etat.c_str(), trame.c_str());
    modem_.envoyerSMS(numeroApiculteur_, trame);
}

void RucheTemoin::envoyerMesuresPeriodiques(const MesuresLocales& mesures) {
    envoyerTrame(mesures, "OK");
}

void RucheTemoin::traiterDemandeSms(const MesuresLocales& mesures) {
    const std::string smsRecu = modem_.lireSMS();
    if (!encodeur_.estDemandeApiculteur(smsRecu)) {
        return;
    }

    std::printf("RUCHE demande apiculteur recue\n");
    envoyerMesuresPeriodiques(mesures);
    heureDernierEnvoi_ = std::chrono::steady_clock::now();
}

void RucheTemoin::verifierEtEnvoyerAlertes(const MesuresLocales& mesures) {
    const std::string alerte = detecteur_.detecter(mesures);
    if (alerte.empty()) {
        return;
    }
    const auto maintenant = std::chrono::steady_clock::now();
    const auto secondesDepuisAlerte = std::chrono::duration_cast<std::chrono::seconds>(
        maintenant - heureDerniereAlerte_);
    if (secondesDepuisAlerte.count() < INTERVALLE_MIN_ALERTE_SEC) {
        return;
    }

    std::printf("RUCHE ALERTE detectee: %s\n", alerte.c_str());
    envoyerTrame(mesures, alerte);
    heureDerniereAlerte_ = maintenant;
}

void RucheTemoin::executerBoucle() {
    while (true) {
        const MesuresLocales mesures = lireMesures();

        std::printf("RUCHE T=%.1fC ACC[g]=%.3f,%.3f,%.3f GYRO[dps]=%.2f,%.2f,%.2f\n",
            mesures.temperature,
            mesures.accelerationX, mesures.accelerationY, mesures.accelerationZ,
            mesures.gyroscopeX, mesures.gyroscopeY, mesures.gyroscopeZ);

        traiterDemandeSms(mesures);
        verifierEtEnvoyerAlertes(mesures);

        const auto maintenant = std::chrono::steady_clock::now();
        const auto minutesEcoulees = std::chrono::duration_cast<std::chrono::minutes>(
            maintenant - heureDernierEnvoi_);
        if (minutesEcoulees.count() >= intervalleEnvoiMin_) {
            envoyerMesuresPeriodiques(mesures);
            heureDernierEnvoi_ = maintenant;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
