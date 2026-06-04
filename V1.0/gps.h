#pragma once

#include "communication.h"

#include <chrono>

/******************************************************************************
 * HEADER GPS
 * Position via GM862-GPS (AT$GPSACP) ou fichiers /tmp pour tests
 ******************************************************************************/

class PositionGps {
public:
    explicit PositionGps(Communication& modem);

    bool demarrer();
    bool lire(float& latitude, float& longitude);

private:
    Communication& modem_;
    float latitude_ = 0.0f;
    float longitude_ = 0.0f;
    bool positionValide_ = false;
    bool acquisitionAbandonnee_ = false;
    std::chrono::steady_clock::time_point heureDemarrage_{};
    std::chrono::steady_clock::time_point heureDerniereLecture_{};
};
