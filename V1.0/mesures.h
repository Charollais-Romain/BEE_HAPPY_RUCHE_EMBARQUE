#pragma once

/******************************************************************************
 * HEADER MESURES
 * Structures de donnees echangees entre capteurs, detection et trames
 ******************************************************************************/

struct MesuresLocales {
    float temperature = 0.0f;
    float accelerationX = 0.0f;
    float accelerationY = 0.0f;
    float accelerationZ = 0.0f;
    float gyroscopeX = 0.0f;
    float gyroscopeY = 0.0f;
    float gyroscopeZ = 0.0f;
};

struct DonneesPeriodiques {
    float masseKg = 0.0f;
    float longitude = 0.0f;
    float latitude = 0.0f;
    float temperature = 0.0f;
};
