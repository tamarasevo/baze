#ifndef RASUTE_H
#define RASUTE_H

#include <stdio.h>
#include "agregirani.h"

// Hash funkcija - metoda preklapanja
int hash_preklapanje(int kljuc);

// Osnovne funkcije za rad sa rasutom datotekom
FILE* aktiviraj_agregiranu_datoteku(char* naziv);
int formiraj_agregiranu_datoteku(char* naziv, FILE* fauto, FILE* fpark);
int upisi_u_baket(FILE* fagregat, int baket_indeks, SLOG_AGREGAT* slog, int* pozicija);
int nadji_u_agregiranoj(FILE* fagregat, int kljuc, SLOG_AGREGAT* slog, int* baket, int* pozicija);
int azuriraj_agregiranu(FILE* fagregat, int kljuc, SLOG_AGREGAT* novi_slog);
int logicki_obrisi_agregiranu(FILE* fagregat, int kljuc);
void ispisi_agregiranu_datoteku(FILE* fagregat);

int dodaj_slog_u_rasutu(FILE* , SLOG_AGREGAT* , int* );
int fizicki_obrisi_iz_rasute(FILE* , int , int* );

// Funkcije za rad sa datotekom promena
int formiraj_datoteku_promena(char* naziv);
int dodaj_u_datoteku_promena(FILE* fpromena, int kljuc, SLOG_AGREGAT* slog, char operacija);
int direktna_obrada(FILE* fagregat, FILE* fpromena, FILE* flog);
void ispisi_datoteku_promena(FILE* fpromena);

// Funkcije za rad sa log datotekom
int formiraj_log_datoteku(char* naziv);
int dodaj_u_log(FILE* flog, int id_auta, char* vrsta);
void prikazi_log_izvestaj(FILE* flog);
void ispisi_log_datoteku(FILE* flog);

// Funkcije za propagaciju iz dela 1
int propagiraj_iz_automobila(FILE* fagregat, FILE* fauto, FILE* fpark, FILE* flog);
int propagiraj_jedan_auto(FILE* fagregat, FILE* fauto, FILE* fpark, FILE* flog, int reg);

// Pomocne funkcije
int baket_je_pun(BAKET* baket);
int nadji_slobodno_mesto(BAKET* baket);
void inicijalizuj_baket(BAKET* baket);

#endif