#include "ruche_temoin.h"
#include <cstdio>


int main() {
    RucheTemoin ruche;

    if (!ruche.demarrer()) {
        std::fprintf(stderr, "MAIN erreur demarrage\n");
        return 1;
    }

    ruche.executerBoucle();
    return 0;
}
