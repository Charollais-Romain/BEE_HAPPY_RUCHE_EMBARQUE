#pragma once

#include "mesures.h"
#include <string>

/******************************************************************************
 * HEADER DETECTION
 * Detection d evenements anormaux (accel/gyro non envoyes dans trame normale)
 ******************************************************************************/

class DetecteurEvenements {
public:
    void initialiser();

    std::string detecter(const MesuresLocales& mesures);

private:
    float accelerationXPrecedente_ = 0.0f;
    float accelerationYPrecedente_ = 0.0f;
    float accelerationZPrecedente_ = 0.0f;
    bool premiereMesure_ = true;

    std::string detecterMouvement(const MesuresLocales& mesures);
    std::string detecterTemperature(float temperatureCelsius);
};
