#include "communication.h"

#include <cstdio>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

namespace {
    std::string normaliserNumeroSms(const std::string& numeroBrut) {
        std::string numero;
        numero.reserve(numeroBrut.size());
        for (char c : numeroBrut) {
            if (c != ' ') {
                numero.push_back(c);
            }
        }

        if (numero.size() == 10 && numero[0] == '0') {
            return "+33" + numero.substr(1);
        }
        if (numero.rfind("0033", 0) == 0) {
            return "+" + numero.substr(2);
        }
        return numero;
    }

    float convertirCoordDmVersDecimal(const std::string& coordonnee) {
        if (coordonnee.size() < 4) {
            return 0.0f;
        }

        const char hemisphere = coordonnee.back();
        const std::string nombre = coordonnee.substr(0, coordonnee.size() - 1);
        const int nbChiffresDegres = (hemisphere == 'N' || hemisphere == 'S') ? 2 : 3;

        const int degres = std::stoi(nombre.substr(0, static_cast<std::size_t>(nbChiffresDegres)));
        const float minutes = std::stof(nombre.substr(static_cast<std::size_t>(nbChiffresDegres)));
        float valeurDecimale = static_cast<float>(degres) + minutes / 60.0f;

        if (hemisphere == 'S' || hemisphere == 'W') {
            valeurDecimale = -valeurDecimale;
        }

        return valeurDecimale;
    }

    bool analyserReponseGpsAcp(const std::string& reponse, float& latitude, float& longitude) {
        const std::size_t debut = reponse.find("$GPSACP:");
        if (debut == std::string::npos) {
            return false;
        }

        std::string ligne = reponse.substr(debut);
        const std::size_t finLigne = ligne.find_first_of("\r\n");
        if (finLigne != std::string::npos) {
            ligne = ligne.substr(0, finLigne);
        }

        const std::size_t debutDonnees = ligne.find(':');
        if (debutDonnees == std::string::npos) {
            return false;
        }

        std::string donnees = ligne.substr(debutDonnees + 1);
        while (!donnees.empty() && donnees.front() == ' ') {
            donnees.erase(donnees.begin());
        }

        const std::size_t virguleLat = donnees.find(',');
        if (virguleLat == std::string::npos) {
            return false;
        }

        const std::string latitudeBrute = donnees.substr(0, virguleLat);
        donnees = donnees.substr(virguleLat + 1);

        const std::size_t virguleLon = donnees.find(',');
        if (virguleLon == std::string::npos) {
            return false;
        }

        const std::string longitudeBrute = donnees.substr(0, virguleLon);
        donnees = donnees.substr(virguleLon + 1);

        for (int i = 0; i < 3; ++i) {
            const std::size_t virgule = donnees.find(',');
            if (virgule == std::string::npos) {
                return false;
            }
            donnees = donnees.substr(virgule + 1);
        }

        const std::size_t virguleFix = donnees.find(',');
        if (virguleFix == std::string::npos) {
            return false;
        }

        const int indiceFix = std::stoi(donnees.substr(0, virguleFix));
        if (indiceFix == 0) {
            return false;
        }

        latitude = convertirCoordDmVersDecimal(latitudeBrute);
        longitude = convertirCoordDmVersDecimal(longitudeBrute);
        return true;
    }
}

Communication::Communication(const std::string& port) : descFichier_(-1) {
    descFichier_ = open(port.c_str(), O_RDWR | O_NOCTTY);
    if (descFichier_ < 0) {
        std::fprintf(stderr, "MODEM erreur ouverture %s\n", port.c_str());
        return;
    }

    termios configSerie{};
    if (tcgetattr(descFichier_, &configSerie) != 0) {
        std::fprintf(stderr, "MODEM erreur lecture configuration serie\n");
        close(descFichier_);
        descFichier_ = -1;
        return;
    }
    cfsetispeed(&configSerie, B115200);
    cfsetospeed(&configSerie, B115200);
    configSerie.c_cflag = (configSerie.c_cflag & ~CSIZE) | CS8 | CLOCAL | CREAD;
    configSerie.c_cflag &= ~(PARENB | CSTOPB);
    configSerie.c_lflag = 0;
    configSerie.c_iflag = 0;
    configSerie.c_oflag = 0;
    tcsetattr(descFichier_, TCSANOW, &configSerie);
}

Communication::~Communication() {
    if (descFichier_ >= 0) {
        close(descFichier_);
    }
}

std::string Communication::executerCommande(const std::string& commandeAt, int attenteMs) {
    if (descFichier_ < 0) {
        return "";
    }

    tcflush(descFichier_, TCIOFLUSH);

    const std::string ligne = commandeAt + "\r";
    write(descFichier_, ligne.data(), ligne.size());
    usleep(static_cast<useconds_t>(attenteMs) * 1000);

    std::string reponse;
    char tampon[256];
    for (int i = 0; i < 75; ++i) {
        const ssize_t nbOctets = read(descFichier_, tampon, sizeof(tampon));
        if (nbOctets > 0) {
            reponse.append(tampon, tampon + nbOctets);
        }
        usleep(20000);
    }

    return reponse;
}

bool Communication::lirePositionGps(float& latitude, float& longitude) {
    const std::string reponse = executerCommande("AT$GPSACP", 3000);
    return analyserReponseGpsAcp(reponse, latitude, longitude);
}

void Communication::envoyerSMS(const std::string& numero, const std::string& texte) {
    if (descFichier_ < 0) {
        std::fprintf(stderr, "MODEM SMS echec: port serie non ouvert\n");
        return;
    }

    const std::string numeroNormalise = normaliserNumeroSms(numero);
    std::printf("MODEM SMS destination=%s (normalise=%s)\n", numero.c_str(), numeroNormalise.c_str());

    const std::string reponseAt = executerCommande("AT", 1500);
    if (reponseAt.find("OK") == std::string::npos) {
        std::fprintf(stderr, "MODEM SMS echec: AT sans OK\n");
    }

    const std::string reponseModeTexte = executerCommande("AT+CMGF=1", 2000);
    if (reponseModeTexte.find("OK") == std::string::npos) {
        std::fprintf(stderr, "MODEM SMS echec: AT+CMGF=1 sans OK\n");
    }

    tcflush(descFichier_, TCIFLUSH);
    const std::string reponseCmgs = executerCommande("AT+CMGS=\"" + numeroNormalise + "\"", 4000);
    if (reponseCmgs.find('>') == std::string::npos) {
        std::fprintf(stderr, "MODEM SMS echec: prompt '>' absent avant message\n");
        return;
    }

    write(descFichier_, texte.data(), texte.size());
    const char ctrlZ = 0x1A;
    write(descFichier_, &ctrlZ, 1);
    usleep(5000000);

    std::string reponseEnvoi;
    char tampon[256];
    for (int i = 0; i < 150; ++i) {
        const ssize_t nbOctets = read(descFichier_, tampon, sizeof(tampon));
        if (nbOctets > 0) {
            reponseEnvoi.append(tampon, tampon + nbOctets);
            if (reponseEnvoi.find("OK") != std::string::npos || reponseEnvoi.find("ERROR") != std::string::npos) {
                break;
            }
        }
        usleep(20000);
    }

    if (reponseEnvoi.find("+CMGS:") != std::string::npos && reponseEnvoi.find("OK") != std::string::npos) {
        std::printf("MODEM SMS confirme (+CMGS)\n");
    } else {
        std::fprintf(stderr, "MODEM SMS non confirme. Reponse brute: %s\n", reponseEnvoi.c_str());
    }
}

std::string Communication::lireSMS() {
    return executerCommande("AT+CMGL=\"REC UNREAD\"", 1500);
}
