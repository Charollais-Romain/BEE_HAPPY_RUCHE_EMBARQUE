#pragma once

#include <string>

/******************************************************************************
 * HEADER GSM
 * Communication SMS via modem GSM (commandes AT)
 ******************************************************************************/

class Communication {
public:
    Communication(const std::string& port);
    ~Communication();

    void envoyerSMS(const std::string& numero, const std::string& texte);
    std::string lireSMS();

    std::string executerCommande(const std::string& commandeAt, int attenteMs = 2000);

    bool lirePositionGps(float& latitude, float& longitude);

private:
    int descFichier_;
};
