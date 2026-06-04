#include "temperature.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <unistd.h>

namespace {
    // hwmon0 expose la temperature CPU, generalement plus elevee que l'ambiante.
    // Calibration locale: 43.3C lue pour ~20.0C reel -> offset ~= -23.3C.
    const float OFFSET_CPU_VERS_AMBIANT_C = -23.3f;
}

Temperature::Temperature() {}

bool Temperature::demarrer() {
    const std::string cheminCpu = "/sys/class/hwmon/hwmon0/temp1_input";
    if (access(cheminCpu.c_str(), R_OK) != 0) {
        std::fprintf(stderr,
            "TEMPERATURE: impossible d'acceder a %s\n",
            cheminCpu.c_str());
        return false;
    }

    cheminTemperature_ = cheminCpu;
    std::printf("TEMPERATURE lecture via hwmon0 (%s)\n", cheminTemperature_.c_str());
    return true;
}

bool Temperature::lire(float& temperatureCelsius) {
    std::ifstream fichier(cheminTemperature_);
    if (!fichier.is_open()) {
        return false;
    }

    int valeurBrute = 0;
    fichier >> valeurBrute;
    if (!fichier.good()) {
        return false;
    }

    const float temperatureCpuCelsius = static_cast<float>(valeurBrute) / 1000.0f;
    temperatureCelsius = temperatureCpuCelsius + OFFSET_CPU_VERS_AMBIANT_C;
    return true;
}
