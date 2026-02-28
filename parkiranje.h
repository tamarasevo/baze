#ifndef PARKIRANJE_H_INCLUDED
#define PARKIRANJE_H_INCLUDED
#include <stdio.h>

#define FBLOKIRANJA_2 4 
#define OZNAKA_KRAJA_DATOTEKE -1

typedef enum {PLAVA, BELA, CRVENA} Zona;

typedef struct slog_parking {
    int id;
    int reg_oznaka;
    char datum[15];  // +1 za null karakter (DD-MM-YYYY)
    int sati;
    Zona zona;
} SLOG_PARKING;

typedef struct blok_parking {
    SLOG_PARKING parkinzi[FBLOKIRANJA_2];  // promenjeno ime za konzistentnost
} BLOK_PARKING;

#endif //PARKIRANJE_H_INCLUDED