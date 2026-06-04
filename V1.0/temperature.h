#pragma once

#include <string>

/******************************************************************************
 * HEADER TEMPERATURE
 * Lecture simple temperature via /sys/class/hwmon/hwmon0/temp1_input
 ******************************************************************************/

class Temperature {
public:
    Temperature();

    bool demarrer();
    bool lire(float& temperatureCelsius);

private:
    std::string cheminTemperature_;
};
