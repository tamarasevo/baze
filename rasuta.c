#include "rasuta.h"
#include "auto.h"
#include "parkiranje.h"
#include <string.h>

// Hash funkcija - metoda preklapanja
int hash_preklapanje(int kljuc) {
    // Primer: za kljuc 12345
    // podeli na 12 i 345 pa saberi
    int deo1 = kljuc / 1000;     // prve cifre
    int deo2 = kljuc % 1000;      // poslednje 3 cifre
    int rezultat = (deo1 + deo2) % B;
    return rezultat;
}

// Inicijalizuj baket (sve slogove postavi na prazno)
void inicijalizuj_baket(BAKET* baket) {
    for (int i = 0; i < FAKTOR_B; i++) {
        baket->slogovi[i].reg_oznaka = OZNAKA_KRAJA;
        baket->slogovi[i].status = ' ';
    }
}

// Proveri da li je baket pun
int baket_je_pun(BAKET* baket) {
    for (int i = 0; i < FAKTOR_B; i++) {
        if (baket->slogovi[i].reg_oznaka == OZNAKA_KRAJA) {
            return 0; // nije pun
        }
    }
    return 1; // pun je
}

// Nadji slobodno mesto u baketu
int nadji_slobodno_mesto(BAKET* baket) {
    for (int i = 0; i < FAKTOR_B; i++) {
        if (baket->slogovi[i].reg_oznaka == OZNAKA_KRAJA) {
            return i;
        }
    }
    return -1; // nema slobodnog mesta
}

int formiraj_agregiranu_datoteku(char* naziv, FILE* fauto, FILE* fpark) {
    if (fauto == NULL || fpark == NULL) {
        printf("Datoteke automobila i parkinga moraju biti aktivirane!\n");
        return 2;
    }
    
    // 1. Izračunaj broj slogova u auto.dat (za statistiku)
    int N = 0;
    rewind(fauto);
    BLOK_AUTO blok_auto;
    while (fread(&blok_auto, sizeof(BLOK_AUTO), 1, fauto) == 1) {
        for (int i = 0; i < FBLOKIRANJA_1; i++) {
            if (blok_auto.automobili[i].reg_oznaka != OZNAKA_KRAJA_DATOTEKE) {
                N++;
            }
        }
    }
    
    printf("Ukupno automobila: %d\n", N);
    
    // 2. Otvori novu rasutu datoteku za pisanje
    FILE* frasuta = fopen(naziv, "wb");
    if (frasuta == NULL) {
        printf("Greska pri kreiranju agregirane datoteke!\n");
        return 2;
    }
    
    // 3. Formiraj praznu datoteku sa B=9 baketa
    //    Svaki baket ima FAKTOR_B=5 slogova + pokazivač na sledeći baket za prekoračioce
    typedef struct {
        SLOG_AGREGAT slogovi[FAKTOR_B];
        int sledeci_baket;  // -1 ako nema prekoračenja, inače indeks baketa za prekoračioce
    } BAKET_SA_SPREGOM;
    
    BAKET_SA_SPREGOM prazan_baket;
    for (int i = 0; i < FAKTOR_B; i++) {
        prazan_baket.slogovi[i].reg_oznaka = OZNAKA_KRAJA;
        prazan_baket.slogovi[i].status = ' ';
        strcpy(prazan_baket.slogovi[i].marka, "");
        strcpy(prazan_baket.slogovi[i].model, "");
        strcpy(prazan_baket.slogovi[i].godina_proizvodnje, "");
        strcpy(prazan_baket.slogovi[i].boja, "");
        prazan_baket.slogovi[i].duz_bela = 0;
        prazan_baket.slogovi[i].duz_crvena = 0;
        prazan_baket.slogovi[i].duz_plava = 0;
    }
    prazan_baket.sledeci_baket = -1;  // nema prekoračenja
    
    // 4. Upisi svih 9 baketa
    for (int i = 0; i < B; i++) {
        fwrite(&prazan_baket, sizeof(BAKET_SA_SPREGOM), 1, frasuta);
    }
    fflush(frasuta);
    
    printf("Kreirano %d praznih baketa sa sprezanjem.\n", B);
    
    // 5. Vrati se na pocetak auto.dat
    rewind(fauto);
    
    // 6. Za svaki auto, kreiraj agregirani slog i upisi ga
    int uspesno_upisanih = 0;
    int prekoracioci = 0;
    
    while (fread(&blok_auto, sizeof(BLOK_AUTO), 1, fauto) == 1) {
        for (int i = 0; i < FBLOKIRANJA_1; i++) {
            if (blok_auto.automobili[i].reg_oznaka == OZNAKA_KRAJA_DATOTEKE) {
                continue;
            }
            
            int reg = blok_auto.automobili[i].reg_oznaka;
            
            // Kreiraj agregirani slog
            SLOG_AGREGAT novi_slog;
            novi_slog.reg_oznaka = reg;
            strcpy(novi_slog.marka, blok_auto.automobili[i].marka);
            strcpy(novi_slog.model, blok_auto.automobili[i].model);
            strcpy(novi_slog.godina_proizvodnje, blok_auto.automobili[i].god_proizvodnje);
            strcpy(novi_slog.boja, blok_auto.automobili[i].boja);
            novi_slog.duz_bela = 0;
            novi_slog.duz_crvena = 0;
            novi_slog.duz_plava = 0;
            novi_slog.status = ' ';
            
            // Saberi sate iz parkinga
            rewind(fpark);
            BLOK_PARKING blok_park;
            while (fread(&blok_park, sizeof(BLOK_PARKING), 1, fpark) == 1) {
                for (int j = 0; j < FBLOKIRANJA_2; j++) {
                    if (blok_park.parkinzi[j].id == OZNAKA_KRAJA_DATOTEKE) break;
                    if (blok_park.parkinzi[j].reg_oznaka == reg) {
                        switch(blok_park.parkinzi[j].zona) {
                            case 0: novi_slog.duz_plava += blok_park.parkinzi[j].sati; break;
                            case 1: novi_slog.duz_bela += blok_park.parkinzi[j].sati; break;
                            case 2: novi_slog.duz_crvena += blok_park.parkinzi[j].sati; break;
                        }
                    }
                }
            }
            
            // Izračunaj hash (glavni baket)
            int glavni_baket = hash_preklapanje(reg);
            
            // Pokušaj da upišeš u glavni baket ili njegov lanac prekoračenja
            int upisan = 0;
            int trenutni_baket = glavni_baket;
            
            while (!upisan) {
                // Učitaj trenutni baket
                BAKET_SA_SPREGOM baket;
                fseek(frasuta, trenutni_baket * sizeof(BAKET_SA_SPREGOM), SEEK_SET);
                fread(&baket, sizeof(BAKET_SA_SPREGOM), 1, frasuta);
                
                // Traži slobodno mesto u ovom baketu
                for (int k = 0; k < FAKTOR_B; k++) {
                    if (baket.slogovi[k].reg_oznaka == OZNAKA_KRAJA) {
                        // Nasli smo slobodno mesto
                        baket.slogovi[k] = novi_slog;
                        
                        // Vrati izmenjen baket
                        fseek(frasuta, trenutni_baket * sizeof(BAKET_SA_SPREGOM), SEEK_SET);
                        fwrite(&baket, sizeof(BAKET_SA_SPREGOM), 1, frasuta);
                        fflush(frasuta);
                        
                        upisan = 1;
                        if (trenutni_baket != glavni_baket) {
                            prekoracioci++;
                        }
                        uspesno_upisanih++;
                        break;
                    }
                }
                
                // Ako nije upisan, a ima sledeći baket, idi na njega
                if (!upisan) {
                    if (baket.sledeci_baket != -1) {
                        trenutni_baket = baket.sledeci_baket;
                    } else {
                        // Nema sledećeg - treba kreirati novi baket za prekoračenje
                        // Ali pošto je statička organizacija, svi baketi su već kreirani
                        // Moramo naći neki slobodan baket
                        
                        int novi_baket = -1;
                        for (int b = 0; b < B; b++) {
                            if (b == glavni_baket) continue;
                            
                            BAKET_SA_SPREGOM provera;
                            fseek(frasuta, b * sizeof(BAKET_SA_SPREGOM), SEEK_SET);
                            fread(&provera, sizeof(BAKET_SA_SPREGOM), 1, frasuta);
                            
                            // Proveri da li je baket potpuno prazan
                            int prazan = 1;
                            for (int k = 0; k < FAKTOR_B; k++) {
                                if (provera.slogovi[k].reg_oznaka != OZNAKA_KRAJA) {
                                    prazan = 0;
                                    break;
                                }
                            }
                            
                            if (prazan && provera.sledeci_baket == -1) {
                                novi_baket = b;
                                break;
                            }
                        }
                        
                        if (novi_baket == -1) {
                            printf("GRESKA: Nema slobodnih baketa za prekoracenje!\n");
                            break;
                        }
                        
                        // Poveži trenutni baket sa novim
                        baket.sledeci_baket = novi_baket;
                        fseek(frasuta, trenutni_baket * sizeof(BAKET_SA_SPREGOM), SEEK_SET);
                        fwrite(&baket, sizeof(BAKET_SA_SPREGOM), 1, frasuta);
                        
                        trenutni_baket = novi_baket;
                    }
                }
            }
        }
    }
    
    fclose(frasuta);
    
    printf("Formiranje zavrseno:\n");
    printf("  Ukupno automobila: %d\n", uspesno_upisanih);
    printf("  Prekoracioca: %d\n", prekoracioci);
    
    return 0;
}