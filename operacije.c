#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "operacije.h"

void ocisti_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

FILE* postavi_aktivnu_datoteku(char* naz_dat){
    FILE* file = fopen(naz_dat, "rb+");
    if (file != NULL) {
        printf("Datoteka %s AKTIVIRANA\n", naz_dat);
    } else {
        printf("Greska pri otvaranju datoteke %s\n", naz_dat);
    }
    return file;
}

int napravi_datoteku_auto(char* naz_dat) {
    BLOK_AUTO b;
    FILE* f = fopen(naz_dat, "wb");
    if (f == NULL) return -1;
    
    // Inicijalizacija celog bloka sa oznakom kraja
    for (int i = 0; i < FBLOKIRANJA_1; i++) {
        b.automobili[i].reg_oznaka = OZNAKA_KRAJA_DATOTEKE;
        // Inicijalizacija stringova
        strcpy(b.automobili[i].marka, "");
        strcpy(b.automobili[i].model, "");
        strcpy(b.automobili[i].god_proizvodnje, "");
        strcpy(b.automobili[i].boja, "");
    }
    
    fwrite(&b, sizeof(BLOK_AUTO), 1, f);
    fclose(f);
    printf("Datoteka %s uspesno napravljena\n", naz_dat);
    return 0;
}

int napravi_datoteku_parking(char* naz_dat) {
    BLOK_PARKING b;
    FILE* f = fopen(naz_dat, "wb");
    if (f == NULL) return -1;
    
    // Inicijalizacija celog bloka sa oznakom kraja
    for (int i = 0; i < FBLOKIRANJA_2; i++) {
        b.parkinzi[i].id = OZNAKA_KRAJA_DATOTEKE;
        b.parkinzi[i].reg_oznaka = 0;
        strcpy(b.parkinzi[i].datum, "");
        b.parkinzi[i].sati = 0;
        b.parkinzi[i].zona = 0;
    }
    
    fwrite(&b, sizeof(BLOK_PARKING), 1, f);
    fclose(f);
    printf("Datoteka %s uspesno napravljena\n", naz_dat);
    return 0;
}

int nadji_auto_sek(FILE* fauto, int reg, SLOG_AUTO* slog){
    if(fauto == NULL){
        printf("Datoteka nije otvorena!\n");
        return 2;
    }

    BLOK_AUTO blok;
    int i;
    
    rewind(fauto); // Vrati na pocetak fajla
    
    while (fread(&blok, sizeof(BLOK_AUTO), 1, fauto) == 1) {
        for(i = 0; i < FBLOKIRANJA_1; i++){
            int k = blok.automobili[i].reg_oznaka;

            // Ako je oznaka kraja, stigli smo do kraja datoteke
            if(k == OZNAKA_KRAJA_DATOTEKE) {
                return 0; // Nije pronadjen, kraj datoteke
            }
            
            // Ako je trenutni ključ veći od traženog, dalje nema (jer je sortirano)
            if(k > reg) {
                return 0; // Nije pronadjen, presli smo mesto gde bi trebalo da bude
            }
            
            // Ako je jednak traženom, pronasli smo ga
            if(k == reg) { 
                *slog = blok.automobili[i]; 
                return 1; // Pronadjen
            }
            
            // Ako je k < reg, nastavljamo dalje
        }
    }

    if(ferror(fauto)) return 2; // Greska pri citanju
    return 0; // Nije pronadjen do kraja fajla
}

int dodaj_slog_sek(FILE* file, SLOG_AUTO* slog){
    if (file == NULL || slog == NULL) return 2;

    BLOK_AUTO blok;
    long trenutna_pozicija;
    int i;
    int pronadjen = 0;
    
    // Vrati na pocetak fajla
    rewind(file);
    
    // Ucitavaj blok po blok
    while (fread(&blok, sizeof(BLOK_AUTO), 1, file) == 1) {
        trenutna_pozicija = ftell(file) - sizeof(BLOK_AUTO); // Pozicija ovog bloka
        
        for (i = 0; i < FBLOKIRANJA_1; i++) {
            int k = blok.automobili[i].reg_oznaka;
            
            // Ako je oznaka kraja - slobodno mesto
            if (k == OZNAKA_KRAJA_DATOTEKE) {
                // Postavi novi slog na ovo mesto
                blok.automobili[i] = *slog;
                
                // Ako nije poslednje mesto u bloku, postavi sledece kao oznaku kraja
                if (i < FBLOKIRANJA_1 - 1) {
                    blok.automobili[i + 1].reg_oznaka = OZNAKA_KRAJA_DATOTEKE;
                }
                
                // Vrati se na pocetak bloka i upisi
                fseek(file, trenutna_pozicija, SEEK_SET);
                fwrite(&blok, sizeof(BLOK_AUTO), 1, file);
                fflush(file);
                
                printf("Dodat slog %d na postojece slobodno mesto.\n", slog->reg_oznaka);
                return 1;
            }
            
            // Ako je kljuc isti - proveri da li je logicki obrisan (ako imate polje za brisanje)
            // U ovom slucaju nemamo polje za logicko brisanje, pa je to greska
            if (k == slog->reg_oznaka) {
                printf("Slog sa kljucem %d vec postoji!\n", slog->reg_oznaka);
                return 0;
            }
            
            // Ako je trenutni kljuc veci od novog - umetni na ovo mesto
            if (k > slog->reg_oznaka) {
                // Privremeno cuvamo trenutni slog
                SLOG_AUTO privremeni = blok.automobili[i];
                
                // Stavljamo novi slog na ovo mesto
                blok.automobili[i] = *slog;
                
                // Upisujemo izmenjen blok
                fseek(file, trenutna_pozicija, SEEK_SET);
                fwrite(&blok, sizeof(BLOK_AUTO), 1, file);
                fflush(file);
                
                // Sada rekurzivno dodajemo privremeni slog (kao novi slog)
                // Ovo ce se nastaviti kroz ostale slogove
                return dodaj_slog_sek(file, &privremeni);
            }
        }
    }
    
    // Ako smo dosli do kraja fajla, dodajemo na kraj
    // Prvo proveri da li je poslednji blok pun
    // Idi na poslednji blok
    fseek(file, -sizeof(BLOK_AUTO), SEEK_END);
    fread(&blok, sizeof(BLOK_AUTO), 1, file);
    trenutna_pozicija = ftell(file) - sizeof(BLOK_AUTO);
    
    // Proveri da li ima slobodnog mesta u poslednjem bloku
    for (i = 0; i < FBLOKIRANJA_1; i++) {
        if (blok.automobili[i].reg_oznaka == OZNAKA_KRAJA_DATOTEKE) {
            // Ima mesta
            blok.automobili[i] = *slog;
            
            if (i < FBLOKIRANJA_1 - 1) {
                blok.automobili[i + 1].reg_oznaka = OZNAKA_KRAJA_DATOTEKE;
            }
            
            fseek(file, trenutna_pozicija, SEEK_SET);
            fwrite(&blok, sizeof(BLOK_AUTO), 1, file);
            fflush(file);
            
            printf("Dodat slog %d na kraj fajla (postojeci blok).\n", slog->reg_oznaka);
            return 1;
        }
    }
    
    // Ako nema mesta, kreiraj novi blok na kraju
    BLOK_AUTO noviBlok;
    
    // Inicijalizacija novog bloka
    for (i = 0; i < FBLOKIRANJA_1; i++) {
        noviBlok.automobili[i].reg_oznaka = OZNAKA_KRAJA_DATOTEKE;
    }
    
    // Stavi novi slog na prvo mesto
    noviBlok.automobili[0] = *slog;
    
    // Idi na kraj i upisi
    fseek(file, 0, SEEK_END);
    fwrite(&noviBlok, sizeof(BLOK_AUTO), 1, file);
    fflush(file);
    
    printf("Kreiran novi blok i dodat slog %d na kraj.\n", slog->reg_oznaka);
    return 1;
}

void zaglavlje_datoteke_auto(){
    printf("\n%9s %9s %12s %20s %20s %10s %20s\n",
        "Rbr.blok",
        "Rbr.slog",
        "Reg.oznaka",
        "Marka",
        "Model",
        "Godina",
        "Boja"
    );
    printf("---------------------------------------------------------------------------------------------------------\n");
}

void ispisi_slog_auto(SLOG_AUTO* slog){
    printf("%12d %20s %20s %10s %20s\n",
        slog->reg_oznaka,
        slog->marka,
        slog->model,
        slog->god_proizvodnje,
        slog->boja
    );
}

void ispisi_datoteku_auto(FILE* file){
    if(file == NULL){
        printf("Datoteka nije otvorena!\n");
        return;
    }

    BLOK_AUTO blok;
    int rbrb = 0, rbrs;
    int brojac_slogova = 0;
    int prazno = 1;

    rewind(file);
    zaglavlje_datoteke_auto();

    while(fread(&blok, sizeof(BLOK_AUTO), 1, file) == 1){
        for(rbrs = 0; rbrs < FBLOKIRANJA_1; rbrs++){
            // Ako naidjemo na OZNAKA_KRAJA_DATOTEKE, prekidamo citanje ovog bloka
            if(blok.automobili[rbrs].reg_oznaka == OZNAKA_KRAJA_DATOTEKE){
                break;  // Prekidamo petlju za ovaj blok
            }
            
            printf("%9d %9d ", rbrb + 1, rbrs + 1);
            ispisi_slog_auto(&blok.automobili[rbrs]);
            brojac_slogova++;
            prazno = 0;
        }
        rbrb++;
    }

    if(prazno){
        printf("\nDatoteka je prazna.\n");
    } else {
        printf("\nUkupno blokova: %d, Ukupno slogova: %d\n", rbrb, brojac_slogova);
    }
}

// ===== Funkcije za rad sa parkiranjima =====
int nadji_parking_serijski(FILE* fpark, int id, SLOG_PARKING* slog) {
    if (fpark == NULL) return 2;
    
    BLOK_PARKING blok;
    rewind(fpark);
    
    while (fread(&blok, sizeof(BLOK_PARKING), 1, fpark) == 1) {
        for (int i = 0; i < FBLOKIRANJA_2; i++) {
            if (blok.parkinzi[i].id == OZNAKA_KRAJA_DATOTEKE) {
                return 0; // Kraj slogova
            }
            if (blok.parkinzi[i].id == id) {
                *slog = blok.parkinzi[i];
                return 1;
            }
        }
    }
    
    if (ferror(fpark)) return 2;
    return 0;
}

int dodaj_slog_park_serijski(FILE* file, SLOG_PARKING* slog) {
    if (file == NULL || slog == NULL) return 2;
    
    // Provera da li ID vec postoji
    SLOG_PARKING postojeci;
    if (nadji_parking_serijski(file, slog->id, &postojeci) == 1) {
        printf("Parking sa ID %d vec postoji!\n", slog->id);
        return 0;
    }
    
    // Idi na kraj fajla
    fseek(file, 0, SEEK_END);
    long velicina = ftell(file);
    
    // Ako je fajl prazan
    if (velicina == 0) {
        BLOK_PARKING noviBlok;
        
        // Inicijalizuj ceo blok
        for (int i = 0; i < FBLOKIRANJA_2; i++) {
            noviBlok.parkinzi[i].id = OZNAKA_KRAJA_DATOTEKE;
        }
        
        // Dodaj novi slog na prvo mesto
        noviBlok.parkinzi[0] = *slog;
        
        fwrite(&noviBlok, sizeof(BLOK_PARKING), 1, file);
        fflush(file);
        return 1;
    }
    
    // Izracunaj broj blokova
    long brojBlokova = velicina / sizeof(BLOK_PARKING);
    
    // Idi na pocetak poslednjeg bloka
    fseek(file, (brojBlokova - 1) * sizeof(BLOK_PARKING), SEEK_SET);
    
    BLOK_PARKING poslednjiBlok;
    if (fread(&poslednjiBlok, sizeof(BLOK_PARKING), 1, file) != 1) {
        return 2;
    }
    
    // Nadji prvu slobodnu poziciju u poslednjem bloku
    int pozicija = -1;
    for (int i = 0; i < FBLOKIRANJA_2; i++) {
        if (poslednjiBlok.parkinzi[i].id == OZNAKA_KRAJA_DATOTEKE) {
            pozicija = i;
            break;
        }
    }
    
    if (pozicija != -1) {
        // Ima mesta u poslednjem bloku
        poslednjiBlok.parkinzi[pozicija] = *slog;
        
        fseek(file, -sizeof(BLOK_PARKING), SEEK_CUR);
        fwrite(&poslednjiBlok, sizeof(BLOK_PARKING), 1, file);
        fflush(file);
        return 1;
    } else {
        // Nema mesta, kreiraj novi blok
        BLOK_PARKING noviBlok;
        
        for (int i = 0; i < FBLOKIRANJA_2; i++) {
            noviBlok.parkinzi[i].id = OZNAKA_KRAJA_DATOTEKE;
        }
        
        noviBlok.parkinzi[0] = *slog;
        
        fseek(file, 0, SEEK_END);
        fwrite(&noviBlok, sizeof(BLOK_PARKING), 1, file);
        fflush(file);
        return 1;
    }
}

int parking_id_postoji(FILE* fpark, int id) {
    if (fpark == NULL) return 0;
    
    BLOK_PARKING blok;
    rewind(fpark);
    
    while (fread(&blok, sizeof(BLOK_PARKING), 1, fpark) == 1) {
        for (int i = 0; i < FBLOKIRANJA_2; i++) {
            if (blok.parkinzi[i].id == OZNAKA_KRAJA_DATOTEKE) {
                return 0;
            }
            if (blok.parkinzi[i].id == id) {
                return 1;
            }
        }
    }
    
    return 0;
}

//brisanje parking
int obrisi_parking_fizicki(FILE* fpark, int id) {
    if (fpark == NULL) {
        printf("Datoteka nije otvorena!\n");
        return 2;
    }

    // Prvo nadji slog
    SLOG_PARKING pronadjeni_slog;
    rewind(fpark);
    int status_trazenja = nadji_parking_serijski(fpark, id, &pronadjeni_slog);
    
    if (status_trazenja != 1) {
        printf("Parking sa ID %d nije pronadjen!\n", id);
        return 2;
    }
    
    // Vrati se na pocetak
    rewind(fpark);
    
    BLOK_PARKING blok;
    BLOK_PARKING naredni_blok;
    int trenutni_id = id;
    int redni_broj_bloka = 0;
    int status_citanja;
    int pronadjen = 0;
    
    // Ucitaj prvi blok
    status_citanja = fread(&blok, sizeof(BLOK_PARKING), 1, fpark);
    
    while (status_citanja == 1 && !pronadjen) {
        long pozicija_bloka = ftell(fpark) - sizeof(BLOK_PARKING);
        
        for (int i = 0; i < FBLOKIRANJA_2; i++) {
            int k = blok.parkinzi[i].id;
            
            // Ako smo dosli do kraja
            if (k == OZNAKA_KRAJA_DATOTEKE) {
                if (i == 0) {
                    // Oznaka kraja je prvi slog u poslednjem bloku
                    // Treba skratiti datoteku
                    fseek(fpark, 0, SEEK_END);
                    long velicina = ftell(fpark);
                    long nova_velicina = redni_broj_bloka * sizeof(BLOK_PARKING);
                    
                    if (nova_velicina < velicina) {
                        // Na Windowsu ne mozemo lako skratiti, ali mozemo ignorisati
                        // jer je bitno da je oznaka kraja na pravom mestu
                    }
                }
                pronadjen = 1;
                break;
            }
            
            // Pronasli smo slog za brisanje
            if (k == trenutni_id) {
                // Pomeri sve slogove iza i za jedno mesto unapred
                for (int j = i; j < FBLOKIRANJA_2 - 1; j++) {
                    blok.parkinzi[j] = blok.parkinzi[j + 1];
                }
                
                // Postavi poslednji slog na -1 (privremeno)
                blok.parkinzi[FBLOKIRANJA_2 - 1].id = OZNAKA_KRAJA_DATOTEKE;
                
                // Proveri da li ima narednih blokova
                long sledeca_pozicija = (redni_broj_bloka + 1) * sizeof(BLOK_PARKING);
                fseek(fpark, sledeca_pozicija, SEEK_SET);
                int ima_naredni = fread(&naredni_blok, sizeof(BLOK_PARKING), 1, fpark);
                
                if (ima_naredni == 1) {
                    // Postoji naredni blok - uzmi prvi slog iz njega
                    blok.parkinzi[FBLOKIRANJA_2 - 1] = naredni_blok.parkinzi[0];
                    trenutni_id = naredni_blok.parkinzi[0].id;
                    
                    // Pomeri slogove u narednom bloku
                    for (int j = 0; j < FBLOKIRANJA_2 - 1; j++) {
                        naredni_blok.parkinzi[j] = naredni_blok.parkinzi[j + 1];
                    }
                    naredni_blok.parkinzi[FBLOKIRANJA_2 - 1].id = OZNAKA_KRAJA_DATOTEKE;
                    
                    // Upisi izmenjen naredni blok
                    fseek(fpark, sledeca_pozicija, SEEK_SET);
                    fwrite(&naredni_blok, sizeof(BLOK_PARKING), 1, fpark);
                    fflush(fpark);
                }
                
                // Upisi izmenjen trenutni blok
                fseek(fpark, pozicija_bloka, SEEK_SET);
                fwrite(&blok, sizeof(BLOK_PARKING), 1, fpark);
                fflush(fpark);
                
                // Vrati se na pocetak trenutnog bloka
                fseek(fpark, pozicija_bloka, SEEK_SET);
                
                // Ako nema vise slogova u narednom bloku, zavrsi
                if (ima_naredni != 1) {
                    pronadjen = 1;
                }
            }
        }
        
        redni_broj_bloka++;
        status_citanja = fread(&blok, sizeof(BLOK_PARKING), 1, fpark);
    }
    
    printf("Parking sa ID %d je uspesno fizicki obrisan.\n", id);
    return 0;
}



void zaglavlje_datoteke_parking() {
    printf("\n%9s %9s %12s %12s %12s %10s %10s\n",
        "Rbr.blok", "Rbr.slog", "ID", "Reg.oznaka", "Datum", "Sati", "Zona");
    printf("----------------------------------------------------------------------------------------\n");
}

void ispisi_slog_parking(SLOG_PARKING* slog) {
    char* zona_str;
    switch(slog->zona) {
        case 0: zona_str = "PLAVA"; break;
        case 1: zona_str = "BELA"; break;
        case 2: zona_str = "CRVENA"; break;
        default: zona_str = "NEPOZNATA";
    }
    
    printf("%12d %12d %12s %10d %10s\n",
        slog->id,
        slog->reg_oznaka,
        slog->datum,
        slog->sati,
        zona_str
    );
}

void ispisi_datoteku_parking(FILE* file) {
    if (file == NULL) {
        printf("Datoteka nije otvorena!\n");
        return;
    }
    
    BLOK_PARKING blok;
    int rbrb = 0, rbrs;
    int brojac_slogova = 0;
    int prazno = 1;
    
    rewind(file);
    zaglavlje_datoteke_parking();
    
    while (fread(&blok, sizeof(BLOK_PARKING), 1, file) == 1) {
        for (rbrs = 0; rbrs < FBLOKIRANJA_2; rbrs++) {
            if (blok.parkinzi[rbrs].id != OZNAKA_KRAJA_DATOTEKE) {
                char* zona_str;
                switch(blok.parkinzi[rbrs].zona) {
                    case 0: zona_str = "PLAVA"; break;
                    case 1: zona_str = "BELA"; break;
                    case 2: zona_str = "CRVENA"; break;
                    default: zona_str = "NEPOZNATA";
                }
                
                printf("%9d %9d %12d %12d %12s %10d %10s\n",
                    rbrb + 1,
                    rbrs + 1,
                    blok.parkinzi[rbrs].id,
                    blok.parkinzi[rbrs].reg_oznaka,
                    blok.parkinzi[rbrs].datum,
                    blok.parkinzi[rbrs].sati,
                    zona_str
                );
                brojac_slogova++;
                prazno = 0;
            }
        }
        rbrb++;
    }
    
    if (prazno) {
        printf("\nDatoteka je prazna.\n");
    } else {
        printf("\nUkupno blokova: %d, Ukupno slogova: %d\n", rbrb, brojac_slogova);
    }
}

int obrisi_parking(FILE* fpark, int id) {
    if (fpark == NULL) return 2;
    
    BLOK_PARKING blok;
    long pozicija = -1;
    int indeks = -1;
    
    rewind(fpark);
    
    while (fread(&blok, sizeof(BLOK_PARKING), 1, fpark) == 1) {
        for (int i = 0; i < FBLOKIRANJA_2; i++) {
            if (blok.parkinzi[i].id == id) {
                pozicija = ftell(fpark) - sizeof(BLOK_PARKING);
                indeks = i;
                break;
            }
        }
        if (indeks != -1) break;
    }
    
    if (indeks == -1) {
        printf("Parking sa ID %d nije pronadjen!\n", id);
        return 0;
    }
    
    // Ucitaj blok koji sadrzi slog za brisanje
    fseek(fpark, pozicija, SEEK_SET);
    fread(&blok, sizeof(BLOK_PARKING), 1, fpark);
    
    // Pomeri sve sledece slogove u bloku za jedno mesto unazad
    for (int j = indeks; j < FBLOKIRANJA_2 - 1; j++) {
        blok.parkinzi[j] = blok.parkinzi[j + 1];
    }
    
    // Postavi poslednji slog na -1
    blok.parkinzi[FBLOKIRANJA_2 - 1].id = OZNAKA_KRAJA_DATOTEKE;
    blok.parkinzi[FBLOKIRANJA_2 - 1].reg_oznaka = 0;
    strcpy(blok.parkinzi[FBLOKIRANJA_2 - 1].datum, "");
    blok.parkinzi[FBLOKIRANJA_2 - 1].sati = 0;
    blok.parkinzi[FBLOKIRANJA_2 - 1].zona = 0;
    
    // Upisi izmenjen blok
    fseek(fpark, pozicija, SEEK_SET);
    fwrite(&blok, sizeof(BLOK_PARKING), 1, fpark);
    fflush(fpark);
    
    printf("Parking sa ID %d je obrisan.\n", id);
    return 1;
}

int izmeni_parking(FILE* fpark, FILE* fauto, int id) {
    if (fpark == NULL || fauto == NULL) return 2;
    
    BLOK_PARKING blok;
    long pozicija = -1;
    int indeks = -1;
    int postoji = 0;
    
    rewind(fpark);
    
    // Prvo pokusaj da nadjes parking sa datim ID
    while (fread(&blok, sizeof(BLOK_PARKING), 1, fpark) == 1) {
        for (int i = 0; i < FBLOKIRANJA_2; i++) {
            if (blok.parkinzi[i].id == id) {
                pozicija = ftell(fpark) - sizeof(BLOK_PARKING);
                indeks = i;
                postoji = 1;
                break;
            }
        }
        if (indeks != -1) break;
    }
    
    // Ako parking ne postoji, pitaj korisnika da li zeli da ga kreira
    if (!postoji) {
        printf("\nParking sa ID %d ne postoji u datoteci.\n", id);
        printf("Da li zelite da kreirate novi parking sa ovim ID? (1-DA, 0-NE): ");
        
        int izbor;
        scanf("%d", &izbor);
        ocisti_buffer();
        
        if (izbor == 1) {
            // Kreiraj novi parking slog
            SLOG_PARKING novi_slog;
            novi_slog.id = id;
            
            printf("\n=== KREIRANJE NOVOG PARKINGA ===\n");
            
            // Unos registarske oznake (auto mora postojati)
            do {
                printf("Registarska oznaka automobila: ");
                scanf("%d", &novi_slog.reg_oznaka);
                ocisti_buffer();
                
                SLOG_AUTO pronadjen_auto;
                if (nadji_auto_sek(fauto, novi_slog.reg_oznaka, &pronadjen_auto) != 1) {
                    printf("GRESKA: Automobil sa oznakom %d ne postoji! Pokusajte ponovo.\n", 
                           novi_slog.reg_oznaka);
                } else {
                    break;
                }
            } while (1);
            
            printf("Datum (npr. 2024-03-15): ");
            scanf("%s", novi_slog.datum);
            ocisti_buffer();
            
            printf("Broj sati: ");
            scanf("%d", &novi_slog.sati);
            ocisti_buffer();
            
            printf("Zona (0-PLAVA, 1-BELA, 2-CRVENA): ");
            int zona;
            scanf("%d", &zona);
            ocisti_buffer();
            novi_slog.zona = (Zona)zona;
            
            // Dodaj novi slog
            int rez = dodaj_slog_park_serijski(fpark, &novi_slog);
            if (rez == 1) {
                printf("Novi parking sa ID %d je uspesno kreiran.\n", id);
                return 1;
            } else {
                printf("Greska pri kreiranju parkinga.\n");
                return 2;
            }
        } else {
            printf("Parking nije kreiran.\n");
            return 0;
        }
    }
    
    // Ako parking postoji, ponudi izbor polja za izmenu
    int izbor = -1;
    int stari_id = id;
    int stara_reg = blok.parkinzi[indeks].reg_oznaka;
    int promenjen_id = 0;
    
    do {
        printf("\n=== IZMENA PARKINGA SA ID %d ===\n", stari_id);
        printf("Trenutni podaci:\n");
        printf("1. ID: %d\n", blok.parkinzi[indeks].id);
        printf("2. Registarska oznaka: %d\n", blok.parkinzi[indeks].reg_oznaka);
        printf("3. Datum: %s\n", blok.parkinzi[indeks].datum);
        printf("4. Broj sati: %d\n", blok.parkinzi[indeks].sati);
        printf("5. Zona: %d ", blok.parkinzi[indeks].zona);
        switch(blok.parkinzi[indeks].zona) {
            case 0: printf("(PLAVA)\n"); break;
            case 1: printf("(BELA)\n"); break;
            case 2: printf("(CRVENA)\n"); break;
        }
        printf("0. Kraj izmene i cuvanje\n\n");
        
        printf("Izaberite polje za izmenu (1-5, 0 za kraj): ");
        scanf("%d", &izbor);
        ocisti_buffer();
        
        switch(izbor) {
            case 1: // ID
                {
                    int novi_id;
                    printf("Trenutni ID: %d\n", blok.parkinzi[indeks].id);
                    printf("Unesite novi ID: ");
                    scanf("%d", &novi_id);
                    ocisti_buffer();
                    
                    if (novi_id != stari_id) {
                        // Proveri da li novi ID vec postoji - KORISTIMO POSTOJECU FUNKCIJU
                        SLOG_PARKING provera;
                        rewind(fpark);
                        int postoji_id = nadji_parking_serijski(fpark, novi_id, &provera);
                        
                        if (postoji_id == 1) {
                            printf("GRESKA: Parking sa ID %d vec postoji! ID nije promenjen.\n", novi_id);
                        } else {
                            blok.parkinzi[indeks].id = novi_id;
                            promenjen_id = 1;
                            printf("ID je promenjen sa %d na %d.\n", stari_id, novi_id);
                        }
                    } else {
                        printf("ID nije promenjen (ista vrednost).\n");
                    }
                }
                break;
                
            case 2: // Registarska oznaka
                {
                    int nova_reg;
                    printf("Trenutna registarska oznaka: %d\n", blok.parkinzi[indeks].reg_oznaka);
                    printf("Unesite novu registarsku oznaku: ");
                    scanf("%d", &nova_reg);
                    ocisti_buffer();
                    
                    if (nova_reg != stara_reg) {
                        SLOG_AUTO pronadjen_auto;
                        if (nadji_auto_sek(fauto, nova_reg, &pronadjen_auto) == 1) {
                            blok.parkinzi[indeks].reg_oznaka = nova_reg;
                            printf("Registarska oznaka je promenjena na %d.\n", nova_reg);
                        } else {
                            printf("GRESKA: Automobil sa oznakom %d ne postoji! Registarska oznaka nije promenjena.\n", nova_reg);
                        }
                    } else {
                        printf("Registarska oznaka nije promenjena (ista vrednost).\n");
                    }
                }
                break;
                
            case 3: // Datum
                printf("Trenutni datum: %s\n", blok.parkinzi[indeks].datum);
                printf("Unesite novi datum (npr. 2024-03-15): ");
                scanf("%s", blok.parkinzi[indeks].datum);
                ocisti_buffer();
                printf("Datum je uspesno izmenjen!\n");
                break;
                
            case 4: // Broj sati
                printf("Trenutni broj sati: %d\n", blok.parkinzi[indeks].sati);
                printf("Unesite novi broj sati: ");
                scanf("%d", &blok.parkinzi[indeks].sati);
                ocisti_buffer();
                printf("Broj sati je uspesno izmenjen!\n");
                break;
                
            case 5: // Zona
                printf("Trenutna zona: %d ", blok.parkinzi[indeks].zona);
                switch(blok.parkinzi[indeks].zona) {
                    case 0: printf("(PLAVA)\n"); break;
                    case 1: printf("(BELA)\n"); break;
                    case 2: printf("(CRVENA)\n"); break;
                }
                printf("Unesite novu zonu (0-PLAVA, 1-BELA, 2-CRVENA): ");
                int nova_zona;
                scanf("%d", &nova_zona);
                ocisti_buffer();
                if (nova_zona >= 0 && nova_zona <= 2) {
                    blok.parkinzi[indeks].zona = (Zona)nova_zona;
                    printf("Zona je uspesno izmenjena!\n");
                } else {
                    printf("GRESKA: Nepostojeca zona! Zona nije promenjena.\n");
                }
                break;
                
            case 0:
                printf("Cuvanje izmena...\n");
                break;
                
            default:
                printf("Nepostojeci izbor! Pokusajte ponovo.\n");
        }
        
    } while (izbor != 0);
    
    // Ako je promenjen ID, moramo da obrisemo stari i dodamo novi slog
    if (promenjen_id) {
        // Sacuvaj izmenjen slog
        SLOG_PARKING izmenjen_slog = blok.parkinzi[indeks];
        
        // Obrisi stari slog (fizicki)
        int status_brisanja = obrisi_parking_fizicki(fpark, stari_id);
        
        if (status_brisanja == 0) {
            // Dodaj novi slog
            dodaj_slog_park_serijski(fpark, &izmenjen_slog);
            printf("Parking je uspesno izmenjen (ID promenjen sa %d na %d).\n", 
                   stari_id, izmenjen_slog.id);
        } else {
            printf("Greska pri brisanju starog parkinga!\n");
        }
    } else {
        // Samo upisi izmenjen blok (isti ID)
        fseek(fpark, pozicija, SEEK_SET);
        fwrite(&blok, sizeof(BLOK_PARKING), 1, fpark);
        fflush(fpark);
        
        printf("Parking sa ID %d je uspesno izmenjen.\n", stari_id);
    }
    
    return 1;
}

int obrisi_auto_sa_parkiranjima(FILE* fauto, FILE* fpark, int reg) {
    if (fauto == NULL) return 2;
    
    // Prvo proveri da li auto postoji
    SLOG_AUTO slog_auto;
    rewind(fauto);
    int status = nadji_auto_sek(fauto, reg, &slog_auto);
    if (status != 1) {
        printf("Automobil sa oznakom %d ne postoji!\n", reg);
        return 0;
    }
    
    // Pronadji sva parkiranja za ovaj auto
    int broj_parkiranja = 0;
    int id_parkiranja[100];
    
    if (fpark != NULL) {
        BLOK_PARKING blok_p;
        rewind(fpark);
        
        while (fread(&blok_p, sizeof(BLOK_PARKING), 1, fpark) == 1) {
            for (int i = 0; i < FBLOKIRANJA_2; i++) {
                if (blok_p.parkinzi[i].id == OZNAKA_KRAJA_DATOTEKE) break;
                if (blok_p.parkinzi[i].reg_oznaka == reg) {
                    id_parkiranja[broj_parkiranja++] = blok_p.parkinzi[i].id;
                }
            }
        }
    }
    
    if (broj_parkiranja > 0) {
        printf("\nAutomobil %d ima %d povezanih parkiranja.\n", reg, broj_parkiranja);
        printf("Da li zelite da obrisete i sva povezana parkiranja? (1-DA, 0-NE): ");
        int izbor;
        scanf("%d", &izbor);
        ocisti_buffer();
        
        if (izbor == 1 && fpark != NULL) {
            for (int i = 0; i < broj_parkiranja; i++) {
                obrisi_parking(fpark, id_parkiranja[i]);
            }
            printf("Obrisano %d povezanih parkiranja.\n", broj_parkiranja);
        }
    }
    
    // Brisanje auta iz sekvencijalne datoteke
    FILE* temp = fopen("temp_auto.tmp", "wb");
    if (temp == NULL) return 2;
    
    BLOK_AUTO blok;
    int obrisan = 0;
    
    rewind(fauto);
    
    while (fread(&blok, sizeof(BLOK_AUTO), 1, fauto) == 1) {
        BLOK_AUTO noviBlok;
        
        // Inicijalizacija novog bloka
        for (int i = 0; i < FBLOKIRANJA_1; i++) {
            noviBlok.automobili[i].reg_oznaka = OZNAKA_KRAJA_DATOTEKE;
            strcpy(noviBlok.automobili[i].marka, "");
            strcpy(noviBlok.automobili[i].model, "");
            strcpy(noviBlok.automobili[i].god_proizvodnje, "");
            strcpy(noviBlok.automobili[i].boja, "");
        }
        
        int j = 0;
        for (int i = 0; i < FBLOKIRANJA_1; i++) {
            if (blok.automobili[i].reg_oznaka == reg) {
                obrisan = 1;
                continue;
            }
            if (blok.automobili[i].reg_oznaka != OZNAKA_KRAJA_DATOTEKE) {
                noviBlok.automobili[j++] = blok.automobili[i];
            }
        }
        
        fwrite(&noviBlok, sizeof(BLOK_AUTO), 1, temp);
    }
    
    fclose(fauto);
    fclose(temp);
    
    // Zatvori i ponovo otvori fajl (ne mozemo direktno da dodelimo FILE*)
    // Ova funkcija treba da vrati novi FILE* ali zbog jednostavnosti,
    // preporuka je da se fajl ponovo otvori u glavnom programu
    
    printf("Automobil sa oznakom %d je uspesno obrisan.\n", reg);
    return obrisan ? 1 : 0;
}

double prosecna_duzina_za_auto(FILE* fpark, int reg) {
    if (fpark == NULL) return 0.0;
    
    BLOK_PARKING blok;
    int ukupno_sati = 0;
    int broj_parkiranja = 0;
    
    rewind(fpark);
    
    while (fread(&blok, sizeof(BLOK_PARKING), 1, fpark) == 1) {
        for (int i = 0; i < FBLOKIRANJA_2; i++) {
            if (blok.parkinzi[i].id == OZNAKA_KRAJA_DATOTEKE) break;
            if (blok.parkinzi[i].reg_oznaka == reg) {
                ukupno_sati += blok.parkinzi[i].sati;
                broj_parkiranja++;
            }
        }
    }
    
    if (broj_parkiranja == 0) return 0.0;
    return (double)ukupno_sati / broj_parkiranja;
}

void prikazi_auto_sa_parkiranjima_iznad_proseka(FILE* fauto, FILE* fpark) {
    if (fauto == NULL || fpark == NULL) {
        printf("Datoteke nisu otvorene!\n");
        return;
    }
    
    printf("\n=== AUTOMOBILI SA PARKIRANJIMA IZNAD PROSEKA ===\n");
    
    BLOK_AUTO blok_auto;
    int ukupno_prikazanih = 0;
    
    rewind(fauto);
    
    while (fread(&blok_auto, sizeof(BLOK_AUTO), 1, fauto) == 1) {
        for (int i = 0; i < FBLOKIRANJA_1; i++) {
            if (blok_auto.automobili[i].reg_oznaka == OZNAKA_KRAJA_DATOTEKE) continue;
            
            int reg = blok_auto.automobili[i].reg_oznaka;
            double prosek = prosecna_duzina_za_auto(fpark, reg);
            
            if (prosek > 0) {
                printf("\nAutomobil: %d %s %s\n", 
                       reg, 
                       blok_auto.automobili[i].marka,
                       blok_auto.automobili[i].model);
                printf("Prosecna duzina boravka: %.2f sati\n", prosek);
                printf("Parkiranja iznad proseka:\n");
                printf("  ID    Datum      Sati   Zona\n");
                printf("  -----------------------------\n");
                
                BLOK_PARKING blok_p;
                rewind(fpark);
                int ima_parkiranja = 0;
                
                while (fread(&blok_p, sizeof(BLOK_PARKING), 1, fpark) == 1) {
                    for (int j = 0; j < FBLOKIRANJA_2; j++) {
                        if (blok_p.parkinzi[j].id == OZNAKA_KRAJA_DATOTEKE) break;
                        if (blok_p.parkinzi[j].reg_oznaka == reg && 
                            blok_p.parkinzi[j].sati > prosek) {
                            
                            char* zona_str;
                            switch(blok_p.parkinzi[j].zona) {
                                case 0: zona_str = "PLAVA"; break;
                                case 1: zona_str = "BELA"; break;
                                case 2: zona_str = "CRVENA"; break;
                                default: zona_str = "NEPOZNATA";
                            }
                            
                            printf("  %3d  %10s  %4d  %6s\n",
                                   blok_p.parkinzi[j].id,
                                   blok_p.parkinzi[j].datum,
                                   blok_p.parkinzi[j].sati,
                                   zona_str);
                            ima_parkiranja = 1;
                        }
                    }
                }
                
                if (ima_parkiranja) {
                    ukupno_prikazanih++;
                }
            }
        }
    }
    
    if (ukupno_prikazanih == 0) {
        printf("\nNema automobila sa parkiranjima iznad proseka.\n");
    }
}

void prikazi_naplatu_po_automobilu(FILE* fauto, FILE* fpark) {
    if (fauto == NULL || fpark == NULL) {
        printf("Datoteke nisu otvorene!\n");
        return;
    }
    
    printf("\n=== NAPLATA PARKIRANJA PO AUTOMOBILU ===\n");
    printf("Cene po satu: BELA - 100 din, CRVENA - 70 din, PLAVA - 50 din\n");
    
    BLOK_AUTO blok_auto;
    int ukupna_naplata_svih = 0;
    int ukupno_auta_sa_parkiranjem = 0;
    
    rewind(fauto);
    
    while (fread(&blok_auto, sizeof(BLOK_AUTO), 1, fauto) == 1) {
        for (int i = 0; i < FBLOKIRANJA_1; i++) {
            if (blok_auto.automobili[i].reg_oznaka == OZNAKA_KRAJA_DATOTEKE) continue;
            
            int reg = blok_auto.automobili[i].reg_oznaka;
            int naplata_bela = 0, naplata_crvena = 0, naplata_plava = 0;
            int sati_bela = 0, sati_crvena = 0, sati_plava = 0;
            int ukupno_sati = 0;
            
            BLOK_PARKING blok_p;
            rewind(fpark);
            
            while (fread(&blok_p, sizeof(BLOK_PARKING), 1, fpark) == 1) {
                for (int j = 0; j < FBLOKIRANJA_2; j++) {
                    if (blok_p.parkinzi[j].id == OZNAKA_KRAJA_DATOTEKE) break;
                    
                    if (blok_p.parkinzi[j].reg_oznaka == reg) {
                        int sati = blok_p.parkinzi[j].sati;
                        ukupno_sati += sati;
                        
                        switch(blok_p.parkinzi[j].zona) {
                            case 0: // PLAVA
                                naplata_plava += sati * 50;
                                sati_plava += sati;
                                break;
                            case 1: // BELA
                                naplata_bela += sati * 100;
                                sati_bela += sati;
                                break;
                            case 2: // CRVENA
                                naplata_crvena += sati * 70;
                                sati_crvena += sati;
                                break;
                        }
                    }
                }
            }
            
            int ukupno_auto = naplata_bela + naplata_crvena + naplata_plava;
            
            if (ukupno_auto > 0) {
                printf("\nAutomobil: %d %s %s\n", 
                       reg, 
                       blok_auto.automobili[i].marka,
                       blok_auto.automobili[i].model);
                if (sati_bela > 0)
                    printf("  BELA zona:   %5d din (%d sati)\n", naplata_bela, sati_bela);
                if (sati_crvena > 0)
                    printf("  CRVENA zona: %5d din (%d sati)\n", naplata_crvena, sati_crvena);
                if (sati_plava > 0)
                    printf("  PLAVA zona:  %5d din (%d sati)\n", naplata_plava, sati_plava);
                printf("  UKUPNO:      %5d din (%d sati)\n", ukupno_auto, ukupno_sati);
                printf("  ----------------------------------------\n");
                
                ukupna_naplata_svih += ukupno_auto;
                ukupno_auta_sa_parkiranjem++;
            }
        }
    }
    
    if (ukupno_auta_sa_parkiranjem > 0) {
        printf("\nUKUPNA NAPLATA ZA SVE AUTOMOBILE: %d dinara\n", ukupna_naplata_svih);
    } else {
        printf("\nNema automobila sa parkiranjima.\n");
    }
}


int obrisi_auto_fizicki(FILE* fauto, int reg) {
    if (fauto == NULL) {
        printf("Datoteka nije otvorena!\n");
        return 2;
    }

    // Prvo proveri da li auto postoji
    SLOG_AUTO pronadjeni_slog;
    rewind(fauto);
    int status_trazenja = nadji_auto_sek(fauto, reg, &pronadjeni_slog);
    
    if (status_trazenja != 1) {
        printf("Automobil sa oznakom %d nije pronadjen!\n", reg);
        return 2;
    }
    
    // Vrati se na pocetak
    rewind(fauto);
    
    BLOK_AUTO blok;
    BLOK_AUTO naredni_blok;
    int redni_broj_bloka = 0;
    int pronadjen = 0;
    
    while (fread(&blok, sizeof(BLOK_AUTO), 1, fauto) == 1 && !pronadjen) {
        long pozicija_bloka = ftell(fauto) - sizeof(BLOK_AUTO);
        
        for (int i = 0; i < FBLOKIRANJA_1; i++) {
            int k = blok.automobili[i].reg_oznaka;
            
            // Ako smo dosli do kraja, prekidamo
            if (k == OZNAKA_KRAJA_DATOTEKE) {
                break;
            }
            
            // Pronasli smo slog za brisanje (samo onaj sa trazenim kljucem)
            if (k == reg) {
                pronadjen = 1;
                
                // Pomeri sve slogove iza i za jedno mesto unapred
                for (int j = i; j < FBLOKIRANJA_1 - 1; j++) {
                    blok.automobili[j] = blok.automobili[j + 1];
                }
                
                // Proveri da li ima narednih blokova
                long sledeca_pozicija = (redni_broj_bloka + 1) * sizeof(BLOK_AUTO);
                fseek(fauto, sledeca_pozicija, SEEK_SET);
                int ima_naredni = fread(&naredni_blok, sizeof(BLOK_AUTO), 1, fauto);
                
                if (ima_naredni == 1) {
                    // Uzmi prvi slog iz narednog bloka
                    blok.automobili[FBLOKIRANJA_1 - 1] = naredni_blok.automobili[0];
                    
                    // Pomeri slogove u narednom bloku
                    for (int j = 0; j < FBLOKIRANJA_1 - 1; j++) {
                        naredni_blok.automobili[j] = naredni_blok.automobili[j + 1];
                    }
                    naredni_blok.automobili[FBLOKIRANJA_1 - 1].reg_oznaka = OZNAKA_KRAJA_DATOTEKE;
                    
                    // Upisi izmenjen naredni blok
                    fseek(fauto, sledeca_pozicija, SEEK_SET);
                    fwrite(&naredni_blok, sizeof(BLOK_AUTO), 1, fauto);
                    fflush(fauto);
                } else {
                    // Nema narednog bloka, samo postavi oznaku kraja
                    blok.automobili[FBLOKIRANJA_1 - 1].reg_oznaka = OZNAKA_KRAJA_DATOTEKE;
                }
                
                // Upisi izmenjen trenutni blok
                fseek(fauto, pozicija_bloka, SEEK_SET);
                fwrite(&blok, sizeof(BLOK_AUTO), 1, fauto);
                fflush(fauto);
                
                // ZAVRSAVAMO - obrisali smo samo jedan slog
                printf("Automobil sa oznakom %d je uspesno obrisan.\n", reg);
                return 0;
            }
        }
        
        redni_broj_bloka++;
    }
    
    if (!pronadjen) {
        printf("Automobil sa oznakom %d nije pronadjen!\n", reg);
        return 2;
    }
    
    return 0;
}

// PROCES reorganizuj_blokove
int reorganizuj_blokove(FILE* fauto, BLOK_AUTO* blok, BLOK_AUTO* naredni_blok, 
                        int* trenutni_kljuc, int kljuc_za_brisanje, 
                        int rbr_bloka, int i, long pozicija_bloka) {
    
    // AKO JE blok.slogovi[i].kljuc == kljuc I blok.slogovi[i].obrisan == 1
    // (nemamo polje obrisan, pa preskacemo ovu proveru)
    
    // POSTAVI j <- i + 1
    int j = i + 1;
    
    // RADI pomeranje_slogova DOK JE j < VELICINA_BLOKA
    while (j < FBLOKIRANJA_1) {
        // POSTAVI blok.slogovi[j-1] <- blok.slogovi[j]
        (*blok).automobili[j - 1] = (*blok).automobili[j];
        j++;
    }
    
    // CITAJ_BLOK narednog bloka
    long sledeca_pozicija = (rbr_bloka + 1) * sizeof(BLOK_AUTO);
    fseek(fauto, sledeca_pozicija, SEEK_SET);
    int status_citanja_narednog = fread(naredni_blok, sizeof(BLOK_AUTO), 1, fauto);
    
    // AKO JE status_citanja_narednog_bloka == 0 (u C-u je 1 ako je uspesno)
    if (status_citanja_narednog == 1) {
        // (* postoje blokovi posle trenutnog *)
        // POSTAVI blok.slogovi[VELICINA_BLOKA -1] <- naredniBlok.slogovi[0]
        (*blok).automobili[FBLOKIRANJA_1 - 1] = (*naredni_blok).automobili[0];
        // POSTAVI trenutni_kljuc <- naredniBlok.slogovi[0].kljuc
        *trenutni_kljuc = (*naredni_blok).automobili[0].reg_oznaka;
        
        // Pomeri slogove u narednom bloku
        for (int k = 1; k < FBLOKIRANJA_1; k++) {
            (*naredni_blok).automobili[k - 1] = (*naredni_blok).automobili[k];
        }
        // Postavi poslednji slog na OZNAKA_KRAJA_DATOTEKE
        (*naredni_blok).automobili[FBLOKIRANJA_1 - 1].reg_oznaka = OZNAKA_KRAJA_DATOTEKE;
        
        // Upisi izmenjen naredni blok
        fseek(fauto, sledeca_pozicija, SEEK_SET);
        fwrite(naredni_blok, sizeof(BLOK_AUTO), 1, fauto);
        fflush(fauto);
    } else {
        // Nema narednog bloka
        (*blok).automobili[FBLOKIRANJA_1 - 1].reg_oznaka = OZNAKA_KRAJA_DATOTEKE;
    }
    
    // UPIŠI_BLOK trenutnog bloka
    fseek(fauto, pozicija_bloka, SEEK_SET);
    fwrite(blok, sizeof(BLOK_AUTO), 1, fauto);
    fflush(fauto);
    
    // Vrati se na poziciju trenutnog bloka za nastavak
    fseek(fauto, pozicija_bloka, SEEK_SET);
    
    return 0;
}


int izmeni_auto(FILE* fauto, int stari_reg) {
    if (fauto == NULL) {
        printf("Datoteka nije otvorena!\n");
        return 2;
    }

    // Prvo proveri da li auto sa starom oznakom postoji
    SLOG_AUTO slog;
    rewind(fauto);
    int status = nadji_auto_sek(fauto, stari_reg, &slog);
    
    if (status != 1) {
        printf("Automobil sa oznakom %d nije pronadjen!\n", stari_reg);
        return 2;
    }
    
    // Kopija sloga za rad
    SLOG_AUTO izmenjen_slog = slog;
    int novi_reg = stari_reg;
    int izbor = -1;
    int promenjen_kljuc = 0;
    
    do {
        printf("\n=== IZMENA AUTOMOBILA ===\n");
        printf("Trenutni podaci:\n");
        printf("1. Registarska oznaka: %d\n", izmenjen_slog.reg_oznaka);
        printf("2. Marka: %s\n", izmenjen_slog.marka);
        printf("3. Model: %s\n", izmenjen_slog.model);
        printf("4. Godina proizvodnje: %s\n", izmenjen_slog.god_proizvodnje);
        printf("5. Boja: %s\n", izmenjen_slog.boja);
        printf("0. Kraj izmene i cuvanje\n\n");
        
        printf("Izaberite polje za izmenu (1-5, 0 za kraj): ");
        scanf("%d", &izbor);
        ocisti_buffer();
        
        switch(izbor) {
            case 1: // Registarska oznaka
                printf("Trenutna registarska oznaka: %d\n", izmenjen_slog.reg_oznaka);
                printf("Unesite novu registarsku oznaku: ");
                scanf("%d", &novi_reg);
                ocisti_buffer();
                
                // Provera da li novi kljuc vec postoji
                if (novi_reg != stari_reg) {
                    SLOG_AUTO provera;
                    rewind(fauto);
                    int postoji = nadji_auto_sek(fauto, novi_reg, &provera);
                    
                    if (postoji == 1) {
                        printf("GRESKA: Automobil sa oznakom %d vec postoji!\n", novi_reg);
                        printf("Registarska oznaka nije promenjena.\n");
                        novi_reg = stari_reg;
                    } else {
                        izmenjen_slog.reg_oznaka = novi_reg;
                        promenjen_kljuc = 1;
                        printf("Registarska oznaka je uspesno promenjena na %d!\n", novi_reg);
                    }
                } else {
                    printf("Registarska oznaka nije promenjena (ista vrednost).\n");
                }
                break;
                
            case 2: // Marka
                printf("Trenutna marka: %s\n", izmenjen_slog.marka);
                printf("Unesite novu marku: ");
                scanf("%s", izmenjen_slog.marka);
                ocisti_buffer();
                printf("Marka je uspesno izmenjena!\n");
                break;
                
            case 3: // Model
                printf("Trenutni model: %s\n", izmenjen_slog.model);
                printf("Unesite novi model: ");
                scanf("%s", izmenjen_slog.model);
                ocisti_buffer();
                printf("Model je uspesno izmenjen!\n");
                break;
                
            case 4: // Godina proizvodnje
                printf("Trenutna godina proizvodnje: %s\n", izmenjen_slog.god_proizvodnje);
                printf("Unesite novu godinu proizvodnje: ");
                scanf("%s", izmenjen_slog.god_proizvodnje);
                ocisti_buffer();
                printf("Godina proizvodnje je uspesno izmenjena!\n");
                break;
                
            case 5: // Boja
                printf("Trenutna boja: %s\n", izmenjen_slog.boja);
                printf("Unesite novu boju: ");
                scanf("%s", izmenjen_slog.boja);
                ocisti_buffer();
                printf("Boja je uspesno izmenjena!\n");
                break;
                
            case 0:
                printf("Cuvanje izmena...\n");
                break;
                
            default:
                printf("Nepostojeci izbor! Pokusajte ponovo.\n");
        }
        
        printf("\n");
        
    } while (izbor != 0);
    
    // Ako je promenjen kljuc, moramo da obrisemo stari i dodamo novi slog
    if (promenjen_kljuc) {
        // SACUVAJ TRENUTNU POZICIJU
        long trenutna_pozicija = ftell(fauto);
        
        // Obrisi stari slog (fizicki)
        rewind(fauto);
        obrisi_auto_fizicki(fauto, stari_reg);
        
        // Dodaj novi slog (automatski ce biti na pravo mesto)
        // Vrati se na pocetak jer dodaj_slog_sek ocekuje da moze da trazi
        rewind(fauto);
        dodaj_slog_sek(fauto, &izmenjen_slog);
        
        // Vrati se na originalnu poziciju (opciono)
        fseek(fauto, trenutna_pozicija, SEEK_SET);
        
        printf("Automobil je uspesno izmenjen (kljuc promenjen sa %d na %d).\n", 
               stari_reg, novi_reg);
    } else {
        // Ako nije promenjen kljuc, samo nadji i izmeni slog na mestu
        rewind(fauto);
        
        BLOK_AUTO blok;
        int pronadjen = 0;
        
        while (fread(&blok, sizeof(BLOK_AUTO), 1, fauto) == 1 && !pronadjen) {
            long pozicija_bloka = ftell(fauto) - sizeof(BLOK_AUTO);
            
            for (int i = 0; i < FBLOKIRANJA_1; i++) {
                if (blok.automobili[i].reg_oznaka == stari_reg) {
                    // Zameni slog
                    blok.automobili[i] = izmenjen_slog;
                    
                    // Upisi izmenjen blok
                    fseek(fauto, pozicija_bloka, SEEK_SET);
                    fwrite(&blok, sizeof(BLOK_AUTO), 1, fauto);
                    fflush(fauto);
                    
                    pronadjen = 1;
                    break;
                }
            }
        }
        
        printf("Automobil sa oznakom %d je uspesno izmenjen.\n", stari_reg);
    }
    
    return 0;
}