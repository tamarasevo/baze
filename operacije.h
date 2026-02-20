#ifndef OPERACIJE_H_INCLUDED
#define OPERACIJE_H_INCLUDED
#include "auto.h"
#include "parkiranje.h"

void napravi_datoteku_auto(char*);
void napravi_datoteku_parking(char*);

FILE* postavi_aktivnu_datoteku(char*);

void ispisi_datoteku(FILE*);
void ispisi_slog(SLOG_AUTO*);
void zaglavlje_datoteke();
void unos_sloga(FILE*,SLOG_AUTO*);
int trazenje_sloga(FILE*,char*,SLOG_AUTO*);
int dodaj_slog(FILE*,SLOG_AUTO*);
int trazenje_sloga_sek(FILE*,char*,SLOG_AUTO*);
int dodaj_slog_sek(FILE*,SLOG_AUTO*);

#endif //SEKVENCIJALNA_H_INCLUDED