#pragma once

#include "mesures.h"
#include <string>

/******************************************************************************
 * HEADER TRAME
 * Format SMS : [Tel];[Masse];[Long];[Lat];[Temp];[Etat]
 * Trames periodiques et alertes identiques ; seul [Etat] change (OK, A1, A2, ...)
 ******************************************************************************/

class EncodeurTrame {
public:
    std::string creer(const std::string& telephone, const DonneesPeriodiques& donnees,
        const std::string& etat);

    bool estDemandeApiculteur(const std::string& reponseModem);
};
