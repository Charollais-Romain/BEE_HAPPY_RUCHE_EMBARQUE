#include "accel_gyro.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
    const uint8_t REG_CTRL1_XL = 0x10;
    const uint8_t REG_CTRL2_G = 0x11;
    const uint8_t REG_OUTX_L_A = 0x28;
    const uint8_t REG_OUTY_L_A = 0x2A;
    const uint8_t REG_OUTZ_L_A = 0x2C;
    const uint8_t REG_OUTX_L_G = 0x22;
    const uint8_t REG_OUTY_L_G = 0x24;
    const uint8_t REG_OUTZ_L_G = 0x26;

    const float FACTEUR_G = 0.000061f;
    const float FACTEUR_GYROSCOPE = 0.00875f;
}

AcceleroGyroscope::AcceleroGyroscope(const std::string& cheminI2c, uint8_t adresse)
    : cheminI2c_(cheminI2c), adresse_(adresse), descFichier_(-1) {}

AcceleroGyroscope::~AcceleroGyroscope() {
    arreter();
}

bool AcceleroGyroscope::demarrer() {
    arreter();

    descFichier_ = open(cheminI2c_.c_str(), O_RDWR);
    if (descFichier_ < 0) {
        std::fprintf(stderr, "CAPTEUR erreur ouverture I2C: %s\n", std::strerror(errno));
        return false;
    }

    if (ioctl(descFichier_, I2C_SLAVE, adresse_) < 0) {
        std::fprintf(stderr, "CAPTEUR erreur adresse I2C\n");
        arreter();
        return false;
    }

    ecrireRegistre(REG_CTRL1_XL, 0x40);
    ecrireRegistre(REG_CTRL2_G, 0x40);
    return true;
}

void AcceleroGyroscope::arreter() {
    if (descFichier_ >= 0) {
        close(descFichier_);
        descFichier_ = -1;
    }
}

bool AcceleroGyroscope::ecrireRegistre(uint8_t registre, uint8_t valeur) {
    uint8_t donnees[2] = {registre, valeur};
    return write(descFichier_, donnees, 2) == 2;
}

bool AcceleroGyroscope::lireRegistre(uint8_t registre, uint8_t& valeur) {
    if (write(descFichier_, &registre, 1) != 1) {
        return false;
    }
    return read(descFichier_, &valeur, 1) == 1;
}

bool AcceleroGyroscope::lireAxe(uint8_t registreBas, int16_t& brut) {
    uint8_t octetBas = 0;
    uint8_t octetHaut = 0;
    if (!lireRegistre(registreBas, octetBas) || !lireRegistre(registreBas + 1, octetHaut)) {
        return false;
    }
    brut = static_cast<int16_t>((static_cast<uint16_t>(octetHaut) << 8) | octetBas);
    return true;
}

bool AcceleroGyroscope::lireAcceleration(float& axeX, float& axeY, float& axeZ) {
    int16_t brutX = 0;
    int16_t brutY = 0;
    int16_t brutZ = 0;

    if (!lireAxe(REG_OUTX_L_A, brutX) || !lireAxe(REG_OUTY_L_A, brutY) || !lireAxe(REG_OUTZ_L_A, brutZ)) {
        return false;
    }

    axeX = static_cast<float>(brutX) * FACTEUR_G;
    axeY = static_cast<float>(brutY) * FACTEUR_G;
    axeZ = static_cast<float>(brutZ) * FACTEUR_G;
    return true;
}

bool AcceleroGyroscope::lireGyroscope(float& axeX, float& axeY, float& axeZ) {
    int16_t brutX = 0;
    int16_t brutY = 0;
    int16_t brutZ = 0;

    if (!lireAxe(REG_OUTX_L_G, brutX) || !lireAxe(REG_OUTY_L_G, brutY) || !lireAxe(REG_OUTZ_L_G, brutZ)) {
        return false;
    }

    axeX = static_cast<float>(brutX) * FACTEUR_GYROSCOPE;
    axeY = static_cast<float>(brutY) * FACTEUR_GYROSCOPE;
    axeZ = static_cast<float>(brutZ) * FACTEUR_GYROSCOPE;
    return true;
}
