#include "detection.h"

#include <cmath>

namespace {
    // Seuils ajustes pour un mouvement manuel de la ruche.
    const float SEUIL_DELTA_AXE_G = 0.12f;
    const float SEUIL_GYRO_DPS = 35.0f;
    const float SEUIL_BASCULEMENT_G = 0.50f;
    const float SEUIL_TEMPERATURE_MIN_C = 5.0f;
    const float SEUIL_TEMPERATURE_MAX_C = 40.0f;
}

void DetecteurEvenements::initialiser() {
    accelerationXPrecedente_ = 0.0f;
    accelerationYPrecedente_ = 0.0f;
    accelerationZPrecedente_ = 0.0f;
    premiereMesure_ = true;
}

std::string DetecteurEvenements::detecter(const MesuresLocales& mesures) {
    std::string alerte = detecterMouvement(mesures);
    if (!alerte.empty()) {
        return alerte;
    }

    return detecterTemperature(mesures.temperature);
}

std::string DetecteurEvenements::detecterMouvement(const MesuresLocales& mesures) {
    if (premiereMesure_) {
        accelerationXPrecedente_ = mesures.accelerationX;
        accelerationYPrecedente_ = mesures.accelerationY;
        accelerationZPrecedente_ = mesures.accelerationZ;
        premiereMesure_ = false;
        return "";
    }

    const float deltaX = std::abs(mesures.accelerationX - accelerationXPrecedente_);
    const float deltaY = std::abs(mesures.accelerationY - accelerationYPrecedente_);
    const float deltaZ = std::abs(mesures.accelerationZ - accelerationZPrecedente_);

    accelerationXPrecedente_ = mesures.accelerationX;
    accelerationYPrecedente_ = mesures.accelerationY;
    accelerationZPrecedente_ = mesures.accelerationZ;

    if (deltaX >= SEUIL_DELTA_AXE_G || deltaY >= SEUIL_DELTA_AXE_G || deltaZ >= SEUIL_DELTA_AXE_G) {
        return "A3";
    }

    if (std::abs(mesures.gyroscopeX) >= SEUIL_GYRO_DPS
        || std::abs(mesures.gyroscopeY) >= SEUIL_GYRO_DPS
        || std::abs(mesures.gyroscopeZ) >= SEUIL_GYRO_DPS) {
        return "A3";
    }

    if (std::abs(mesures.accelerationZ) < SEUIL_BASCULEMENT_G) {
        return "A2";
    }

    return "";
}

std::string DetecteurEvenements::detecterTemperature(float temperatureCelsius) {
    if (temperatureCelsius <= 0.0f) {
        return "";
    }

    if (temperatureCelsius < SEUIL_TEMPERATURE_MIN_C || temperatureCelsius > SEUIL_TEMPERATURE_MAX_C) {
        return "A4";
    }

    return "";
}
