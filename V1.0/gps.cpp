#include "gps.h"

#include <cstdio>
#include <fstream>
#include <string>

namespace {
    const char* FICHIER_LATITUDE = "/tmp/ruche_latitude";
    const char* FICHIER_LONGITUDE = "/tmp/ruche_longitude";
    const int INTERVALLE_LECTURE_SEC = 30;
    const int TIMEOUT_FIX_GPS_SEC = 180;

    bool lireFlottantDepuisFichier(const char* chemin, float& valeur) {
        std::ifstream fichier(chemin);
        if (!fichier.is_open()) {
            return false;
        }

        std::string ligne;
        std::getline(fichier, ligne);
        if (ligne.empty()) {
            return false;
        }

        valeur = std::stof(ligne);
        return true;
    }
}

PositionGps::PositionGps(Communication& modem)
    : modem_(modem), heureDemarrage_(std::chrono::steady_clock::now()) {}

bool PositionGps::demarrer() {
    std::printf("GPS init GM862 (AT$GPSR warm start) test\n");
    heureDemarrage_ = std::chrono::steady_clock::now();
    acquisitionAbandonnee_ = false;
    modem_.executerCommande("AT$GPSR=1", 5000);
    return true;
}

bool PositionGps::lire(float& latitude, float& longitude) {
    const auto maintenant = std::chrono::steady_clock::now();
    if (positionValide_) {
        const auto secondes = std::chrono::duration_cast<std::chrono::seconds>(maintenant - heureDerniereLecture_);
        if (secondes.count() < INTERVALLE_LECTURE_SEC) {
            latitude = latitude_;
            longitude = longitude_;
            return true;
        }
    }

    float latitudeFichier = 0.0f;
    float longitudeFichier = 0.0f;
    if (lireFlottantDepuisFichier(FICHIER_LATITUDE, latitudeFichier)
        && lireFlottantDepuisFichier(FICHIER_LONGITUDE, longitudeFichier)) {
        latitude_ = latitudeFichier;
        longitude_ = longitudeFichier;
        positionValide_ = true;
        heureDerniereLecture_ = maintenant;
        latitude = latitude_;
        longitude = longitude_;
        return true;
    }

    if (acquisitionAbandonnee_) {
        if (positionValide_) {
            latitude = latitude_;
            longitude = longitude_;
            return true;
        }
        return false;
    }

    float latitudeModem = 0.0f;
    float longitudeModem = 0.0f;
    if (!modem_.lirePositionGps(latitudeModem, longitudeModem)) {
        const auto delaiDemarrage = std::chrono::duration_cast<std::chrono::seconds>(maintenant - heureDemarrage_);
        if (!positionValide_ && delaiDemarrage.count() >= TIMEOUT_FIX_GPS_SEC) {
            acquisitionAbandonnee_ = true;
            std::fprintf(stderr, "GPS timeout: aucun fix apres 3 minutes, skip GPS\n");
        }
        if (positionValide_) {
            latitude = latitude_;
            longitude = longitude_;
            return true;
        }
        return false;
    }

    latitude_ = latitudeModem;
    longitude_ = longitudeModem;
    positionValide_ = true;
    heureDerniereLecture_ = maintenant;
    latitude = latitude_;
    longitude = longitude_;
    return true;
}
