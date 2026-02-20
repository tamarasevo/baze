#ifndef AUTO_H_INCLUDED
#define AUTO_H_INCLUDED
#define FBLOKIRANJA_1 3

#define OZNAKA_KRAJA_DATOTEKE -1

typedef struct slog_auto{
    int reg_oznaka;
    char marka[31];
    char model[31];
    char god_proizvodnje[5];
    char boja[20];

}SLOG_AUTO;

typedef struct blok_auto{
   SLOG_AUTO automobili[FBLOKIRANJA_1];
}BLOK_AUTO;

#endif //AUTO_H_INCLUDED