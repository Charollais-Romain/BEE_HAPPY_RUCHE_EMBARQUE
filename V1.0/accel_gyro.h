#pragma once

#include <cstdint>
#include <string>

/******************************************************************************
 * HEADER CAPTEUR
 * Accelerometre et gyroscope LSM6DS3 (detection d evenements uniquement)
 ******************************************************************************/

class AcceleroGyroscope {
public:
    AcceleroGyroscope(const std::string& cheminI2c, uint8_t adresse);
    ~AcceleroGyroscope();

    bool demarrer();
    void arreter();
    bool lireAcceleration(float& axeX, float& axeY, float& axeZ);
    bool lireGyroscope(float& axeX, float& axeY, float& axeZ);

private:
    std::string cheminI2c_;
    uint8_t adresse_;
    int descFichier_;

    bool ecrireRegistre(uint8_t registre, uint8_t valeur);
    bool lireRegistre(uint8_t registre, uint8_t& valeur);
    bool lireAxe(uint8_t registreBas, int16_t& brut);
};
