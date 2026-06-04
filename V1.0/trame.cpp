#include "trame.h"

#include <cstdio>

namespace {
    std::string extraireTexteSms(const std::string& reponseModem) {
        const std::size_t debut = reponseModem.find("+CMGL:");
        if (debut == std::string::npos) {
            return "";
        }

        const std::size_t finEntete = reponseModem.find('\n', debut);
        if (finEntete == std::string::npos) {
            return "";
        }

        std::string texte = reponseModem.substr(finEntete + 1);
        const std::size_t finOk = texte.find("\r\nOK");
        if (finOk != std::string::npos) {
            texte = texte.substr(0, finOk);
        }

        while (!texte.empty() && (texte.back() == '\r' || texte.back() == '\n')) {
            texte.pop_back();
        }

        return texte;
    }

    std::string formaterTrame(const std::string& telephone, const DonneesPeriodiques& donnees,
        const char* etat) {
        char tampon[256];
        std::snprintf(tampon, sizeof(tampon), "%s;%.3f;%.6f;%.6f;%.1f;%s",
            telephone.c_str(),
            donnees.masseKg,
            donnees.longitude,
            donnees.latitude,
            donnees.temperature,
            etat);
        return tampon;
    }
}

std::string EncodeurTrame::creer(const std::string& telephone, const DonneesPeriodiques& donnees,
    const std::string& etat) {
    return formaterTrame(telephone, donnees, etat.c_str());
}

bool EncodeurTrame::estDemandeApiculteur(const std::string& reponseModem) {
    const std::string texte = extraireTexteSms(reponseModem);
    if (texte.empty()) {
        return false;
    }

    return texte.find("ETAT") != std::string::npos
        || texte.find("INFO") != std::string::npos
        || texte.find("MESURE") != std::string::npos
        || texte.find("etat") != std::string::npos
        || texte.find("info") != std::string::npos
        || texte.find("mesure") != std::string::npos;
}
