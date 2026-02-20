#ifndef PARKIRANJE_H_INCLUDED
#define PARKIRANJE_H_INCLUDED
#include <stdio.h>

#define FBLOKIRANJA_2 4 


#define OZNAKA_KRAJA_DATOTEKE -1

typedef enum {PLAVA,BELA,CRVENA}Zona;


typedef struct slog_parking{
    int id;
    int reg_oznaka;
    char datum[11];
    int sati;
    Zona zona;

}SLOG_PARKING;

typedef struct blok_parking{
   SLOG_PARKING automobili[FBLOKIRANJA_2];
}BLOK_PARKING;
#endif //PARKIRANJE_H_INCLUDED