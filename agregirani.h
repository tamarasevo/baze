#ifndef AGREGIRANI_H
#define AGREGIRANI_H

// ===== KONSTANTE =====
#define B 9                 // broj baketa
#define FAKTOR_B 5          // slogova po baketu (b)
#define FDP 4               // faktor blokiranja datoteke promena
#define F_LOG 6             // faktor blokiranja log datoteke
#define OZNAKA_KRAJA -1

// ===== STRUKTURE =====

// Slog za agregiranu datoteku (rasuta organizacija)
typedef struct {
    int reg_oznaka;                 // identifikator (ključ)
    char marka[31];                  // do 30 karaktera + \0
    char model[31];                  // do 30 karaktera + \0
    char godina_proizvodnje[5];      // 4 cifre + \0
    char boja[21];                   // do 20 karaktera + \0
    int duz_bela;                    // ukupna dužina u beloj zoni
    int duz_crvena;                   // ukupna dužina u crvenoj zoni
    int duz_plava;                    // ukupna dužina u plavoj zoni
    char status;                      // statusno polje: ' ' - aktivan, '*' - logički obrisan
} SLOG_AGREGAT;

// Baket (blok) za rasutu datoteku
typedef struct {
    SLOG_AGREGAT podaci;                    // stvarni podaci
    int sledeci_u_lancu_sinonima;           // -1 ako nema sledeceg
} SLOG_SA_POKAZIVACEM;

// ===== NOVO: Baket sa dva pokazivaca =====
typedef struct {
    // 1. ZAGLAVLJE BAKETA - za lanac slobodnih baketa
    int prethodni_u_lancu_slobodnih;        // -1 ako je prvi u lancu
    int sledeci_u_lancu_slobodnih;          // -1 ako je poslednji u lancu
    int broj_slobodnih;                      // koliko slobodnih mesta ima u ovom baketu
    
    // 2. ZA MATICNI BAKET - pokazivac na prvi slog u lancu sinonima
    int prvi_u_lancu_sinonima;               // -1 ako nema prekoracilaca
    
    // 3. SLOGOVI U BAKETU (svaki sa svojim pokazivacem)
    SLOG_SA_POKAZIVACEM slogovi[FAKTOR_B];
} BAKET;


// Datoteka promena (za direktnu obradu)
typedef struct {
    int kljuc;                          // identifikator automobila
    SLOG_AGREGAT slog;                   // ceo slog rasute datoteke
    char operacija;                      // 'n' - novi, 'm' - modifikacija, 'b' - brisanje
} SLOG_PROMENA;

typedef struct {
    SLOG_PROMENA slogovi[FDP];
} BLOK_PROMENA;

// Log datoteka (serijska organizacija)
typedef struct {
    int identifikator;                   // auto-increment (1,2,3...)
    int identifikator_automobila;        // reg oznaka automobila
    char vrsta_operacije[13];             // "UNOS" ili "MODIFIKACIJA"
} SLOG_LOG;

typedef struct {
    SLOG_LOG slogovi[F_LOG];
} BLOK_LOG;

#endif