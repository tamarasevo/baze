#ifndef OPERACIJE_H_INCLUDED
#define OPERACIJE_H_INCLUDED
#include "auto.h"
#include "parkiranje.h"

void ocisti_buffer();

// ===== Postojece funkcije =====
FILE* postavi_aktivnu_datoteku(char* naz_dat);
int napravi_datoteku_auto(char* naz_dat);
int napravi_datoteku_parking(char* naz_dat);
int nadji_auto_sek(FILE* fauto, int reg, SLOG_AUTO* slog);
int dodaj_slog_sek(FILE* file, SLOG_AUTO* slog);
void ispisi_datoteku_auto(FILE* file);
void ispisi_slog_auto(SLOG_AUTO* slog);
void zaglavlje_datoteke_auto();

// ===== Funkcije za parkiranja =====
int nadji_parking_serijski(FILE* fpark, int id, SLOG_PARKING* slog);
int dodaj_slog_park_serijski(FILE* file, SLOG_PARKING* slog);
int parking_id_postoji(FILE* fpark, int id);
void ispisi_datoteku_parking(FILE* file);
void ispisi_slog_parking(SLOG_PARKING* slog);
void zaglavlje_datoteke_parking();

// ===== NOVE FUNKCIJE (dodati ovo) =====
int obrisi_parking(FILE* fpark, int id);
int izmeni_parking(FILE* fpark, FILE* fauto, int id);
int obrisi_auto_sa_parkiranjima(FILE* fauto, FILE* fpark, int reg);
void prikazi_auto_sa_parkiranjima_iznad_proseka(FILE* fauto, FILE* fpark);
void prikazi_naplatu_po_automobilu(FILE* fauto, FILE* fpark);

int obrisi_auto_fizicki(FILE* , int );
// Deklaracija funkcije za reorganizaciju
int reorganizuj_blokove(FILE* , BLOK_AUTO* , BLOK_AUTO* ,int* , int , int , int , long );
int izmeni_auto(FILE* , int );
int obrisi_parking_fizicki(FILE* , int );

#endif