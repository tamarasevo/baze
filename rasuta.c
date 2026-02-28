#include "rasuta.h"
#include "auto.h"
#include "parkiranje.h"
#include <string.h>
#include <math.h>

// Hash funkcija - metoda preklapanja
int hash_preklapanje(int kljuc) {
    // Podeli kljuc na delove od po 2 cifre
    int delovi[10];
    int broj_delova = 0;
    int temp = kljuc;
    
    while (temp > 0) {
        delovi[broj_delova++] = temp % 100;
        temp /= 100;
    }
    
    // Ako je neparan broj delova, dodaj 0
    if (broj_delova % 2 == 1) {
        delovi[broj_delova++] = 0;
    }
    
    // Preklopi i saberi
    int T = 0;
    for (int i = 0; i < broj_delova / 2; i++) {
        T += delovi[i] + delovi[broj_delova - 1 - i];
    }
    
    T = T % 100;
    
    // Adresa baketa
    return (T * B) / 100;
}

// Inicijalizuj baket
void inicijalizuj_baket(BAKET* baket) {
    for (int i = 0; i < FAKTOR_B; i++) {
        baket->slogovi[i].podaci.reg_oznaka = OZNAKA_KRAJA;
        baket->slogovi[i].podaci.status = ' ';
        strcpy(baket->slogovi[i].podaci.marka, "");
        strcpy(baket->slogovi[i].podaci.model, "");
        strcpy(baket->slogovi[i].podaci.godina_proizvodnje, "");
        strcpy(baket->slogovi[i].podaci.boja, "");
        baket->slogovi[i].podaci.duz_bela = 0;
        baket->slogovi[i].podaci.duz_crvena = 0;
        baket->slogovi[i].podaci.duz_plava = 0;
        baket->slogovi[i].sledeci_u_lancu_sinonima = -1;
    }
    
    baket->prethodni_u_lancu_slobodnih = -1;
    baket->sledeci_u_lancu_slobodnih = -1;
    baket->broj_slobodnih = FAKTOR_B;
    baket->prvi_u_lancu_sinonima = -1;
}

// Proveri da li je baket pun
int baket_je_pun(BAKET* baket) {
    for (int i = 0; i < FAKTOR_B; i++) {
        if (baket->slogovi[i].podaci.reg_oznaka == OZNAKA_KRAJA) {
            return 0;
        }
    }
    return 1;
}

// Nadji slobodno mesto u baketu
int nadji_slobodno_mesto(BAKET* baket) {
    for (int i = 0; i < FAKTOR_B; i++) {
        if (baket->slogovi[i].podaci.reg_oznaka == OZNAKA_KRAJA) {
            return i;
        }
    }
    return -1;
}

// Aktiviraj agregiranu datoteku
FILE* aktiviraj_agregiranu_datoteku(char* naziv) {
    FILE* f = fopen(naziv, "rb+");
    if (f != NULL) {
        printf("Agregirana datoteka '%s' aktivirana.\n", naziv);
    } else {
        printf("Greska pri otvaranju agregirane datoteke!\n");
    }
    return f;
}

// Formiraj agregiranu datoteku
int formiraj_agregiranu_datoteku(char* naziv, FILE* fauto, FILE* fpark) {
    if (fauto == NULL || fpark == NULL) {
        printf("Datoteke automobila i parkinga moraju biti aktivirane!\n");
        return 2;
    }
    
    FILE* frasuta = fopen(naziv, "wb");
    if (frasuta == NULL) {
        printf("Greska pri kreiranju agregirane datoteke!\n");
        return 2;
    }
    
    // Inicijalizuj sve bakete
    for (int b = 0; b < B; b++) {
        BAKET baket;
        inicijalizuj_baket(&baket);
        
        baket.prethodni_u_lancu_slobodnih = (b > 0) ? b-1 : -1;
        baket.sledeci_u_lancu_slobodnih = (b < B-1) ? b+1 : -1;
        
        fwrite(&baket, sizeof(BAKET), 1, frasuta);
    }
    fflush(frasuta);
    
    printf("Kreirano %d praznih baketa.\n", B);
    
    // Upis automobila (ako ih ima)
    rewind(fauto);
    
    int upisano = 0;
    BLOK_AUTO blok_auto;
    
    while (fread(&blok_auto, sizeof(BLOK_AUTO), 1, fauto) == 1) {
        for (int i = 0; i < FBLOKIRANJA_1; i++) {
            if (blok_auto.automobili[i].reg_oznaka == OZNAKA_KRAJA_DATOTEKE ||
                blok_auto.automobili[i].reg_oznaka <= 0) {
                continue;
            }
            
            int reg = blok_auto.automobili[i].reg_oznaka;
            
            SLOG_AGREGAT novi;
            novi.reg_oznaka = reg;
            strcpy(novi.marka, blok_auto.automobili[i].marka);
            strcpy(novi.model, blok_auto.automobili[i].model);
            strcpy(novi.godina_proizvodnje, blok_auto.automobili[i].god_proizvodnje);
            strcpy(novi.boja, blok_auto.automobili[i].boja);
            novi.duz_bela = 0;
            novi.duz_crvena = 0;
            novi.duz_plava = 0;
            novi.status = ' ';
            
            // Saberi parking
            rewind(fpark);
            BLOK_PARKING blok_park;
            while (fread(&blok_park, sizeof(BLOK_PARKING), 1, fpark) == 1) {
                for (int j = 0; j < FBLOKIRANJA_2; j++) {
                    if (blok_park.parkinzi[j].id == OZNAKA_KRAJA_DATOTEKE) continue;
                    if (blok_park.parkinzi[j].reg_oznaka == reg) {
                        switch(blok_park.parkinzi[j].zona) {
                            case 0: novi.duz_plava += blok_park.parkinzi[j].sati; break;
                            case 1: novi.duz_bela += blok_park.parkinzi[j].sati; break;
                            case 2: novi.duz_crvena += blok_park.parkinzi[j].sati; break;
                        }
                    }
                }
            }
            
            int baket_idx = hash_preklapanje(reg);
            
            BAKET baket;
            fseek(frasuta, baket_idx * sizeof(BAKET), SEEK_SET);
            fread(&baket, sizeof(BAKET), 1, frasuta);
            
            int nadjeno = -1;
            for (int k = 0; k < FAKTOR_B; k++) {
                if (baket.slogovi[k].podaci.reg_oznaka == OZNAKA_KRAJA) {
                    nadjeno = k;
                    break;
                }
            }
            
            if (nadjeno >= 0) {
                baket.slogovi[nadjeno].podaci = novi;
                baket.broj_slobodnih--;
                
                fseek(frasuta, baket_idx * sizeof(BAKET), SEEK_SET);
                fwrite(&baket, sizeof(BAKET), 1, frasuta);
                fflush(frasuta);
                
                upisano++;
            }
        }
    }
    
    fclose(frasuta);
    
    printf("Formiranje zavrseno: %d automobila upisano.\n", upisano);
    
    return 0;
}

// Nadji slog u rasutoj datoteci
int nadji_u_rasutoj(FILE* frasuta, int reg, SLOG_AGREGAT* pronadjen, int* baket, int* pozicija) {
    if (frasuta == NULL) return -1;
    
    int maticni = hash_preklapanje(reg);
    
    // Proveri maticni baket
    BAKET b;
    fseek(frasuta, maticni * sizeof(BAKET), SEEK_SET);
    fread(&b, sizeof(BAKET), 1, frasuta);
    
    for (int i = 0; i < FAKTOR_B; i++) {
        if (b.slogovi[i].podaci.reg_oznaka == reg) {
            *pronadjen = b.slogovi[i].podaci;
            *baket = maticni;
            *pozicija = i;
            return 1;
        }
    }
    
    // Proveri lanac sinonima
    int tek_sin = b.prvi_u_lancu_sinonima;
    while (tek_sin != -1) {
        BAKET sin_baket;
        fseek(frasuta, tek_sin * sizeof(BAKET), SEEK_SET);
        fread(&sin_baket, sizeof(BAKET), 1, frasuta);
        
        for (int i = 0; i < FAKTOR_B; i++) {
            if (sin_baket.slogovi[i].podaci.reg_oznaka == reg) {
                *pronadjen = sin_baket.slogovi[i].podaci;
                *baket = tek_sin;
                *pozicija = i;
                return 2;
            }
        }
        
        tek_sin = sin_baket.slogovi[0].sledeci_u_lancu_sinonima;
    }
    
    return 0;
}

// Dodaj slog u rasutu datoteku
int dodaj_slog_u_rasutu(FILE* frasuta, SLOG_AGREGAT* novi_podaci, int* E) {
    if (frasuta == NULL || novi_podaci == NULL) return -1;
    
    // Proveri da li vec postoji
    SLOG_AGREGAT pronadjen;
    int baket, pozicija;
    int status = nadji_u_rasutoj(frasuta, novi_podaci->reg_oznaka, &pronadjen, &baket, &pozicija);
    
    if (status != 0) {
        printf("Auto %d vec postoji u baketu %d!\n", novi_podaci->reg_oznaka, baket);
        return -1;
    }
    
    int maticni = hash_preklapanje(novi_podaci->reg_oznaka);
    
    // Ucitaj maticni baket
    BAKET maticni_baket;
    fseek(frasuta, maticni * sizeof(BAKET), SEEK_SET);
    fread(&maticni_baket, sizeof(BAKET), 1, frasuta);
    
    // Pokusaj da upises u maticni baket
    for (int i = 0; i < FAKTOR_B; i++) {
        if (maticni_baket.slogovi[i].podaci.reg_oznaka == OZNAKA_KRAJA) {
            maticni_baket.slogovi[i].podaci = *novi_podaci;
            maticni_baket.slogovi[i].sledeci_u_lancu_sinonima = -1;
            maticni_baket.broj_slobodnih--;
            
            if (*E == maticni && maticni_baket.broj_slobodnih == 0) {
                *E = maticni_baket.sledeci_u_lancu_slobodnih;
            }
            
            fseek(frasuta, maticni * sizeof(BAKET), SEEK_SET);
            fwrite(&maticni_baket, sizeof(BAKET), 1, frasuta);
            fflush(frasuta);
            
            return 0;
        }
    }
    
    // Maticni baket pun - koristi prvi slobodan
    if (*E == -1) {
        printf("GRESKA: Nema slobodnih baketa!\n");
        return -1;
    }
    
    int slobodan = *E;
    BAKET slobodni_baket;
    fseek(frasuta, slobodan * sizeof(BAKET), SEEK_SET);
    fread(&slobodni_baket, sizeof(BAKET), 1, frasuta);
    
    for (int i = 0; i < FAKTOR_B; i++) {
        if (slobodni_baket.slogovi[i].podaci.reg_oznaka == OZNAKA_KRAJA) {
            slobodni_baket.slogovi[i].podaci = *novi_podaci;
            slobodni_baket.slogovi[i].sledeci_u_lancu_sinonima = -1;
            slobodni_baket.broj_slobodnih--;
            
            // Povezi sa maticnim - ISPRAVLJENO!
            if (maticni_baket.prvi_u_lancu_sinonima == -1) {
                maticni_baket.prvi_u_lancu_sinonima = slobodan;
            } else {
                int poslednji = maticni_baket.prvi_u_lancu_sinonima;
                BAKET temp;
                
                while (poslednji != -1) {  // <-- ISPRAVNO!
                    fseek(frasuta, poslednji * sizeof(BAKET), SEEK_SET);
                    fread(&temp, sizeof(BAKET), 1, frasuta);
                    
                    if (temp.slogovi[0].sledeci_u_lancu_sinonima == -1) {
                        temp.slogovi[0].sledeci_u_lancu_sinonima = slobodan;
                        fseek(frasuta, poslednji * sizeof(BAKET), SEEK_SET);
                        fwrite(&temp, sizeof(BAKET), 1, frasuta);
                        break;
                    }
                    poslednji = temp.slogovi[0].sledeci_u_lancu_sinonima;
                }
            }
            
            if (slobodni_baket.broj_slobodnih == 0) {
                *E = slobodni_baket.sledeci_u_lancu_slobodnih;
            }
            
            fseek(frasuta, maticni * sizeof(BAKET), SEEK_SET);
            fwrite(&maticni_baket, sizeof(BAKET), 1, frasuta);
            
            fseek(frasuta, slobodan * sizeof(BAKET), SEEK_SET);
            fwrite(&slobodni_baket, sizeof(BAKET), 1, frasuta);
            fflush(frasuta);
            
            return 0;
        }
    }
    
    return -1;
}

void ispisi_agregiranu_datoteku(FILE* frasuta) {
    if (frasuta == NULL) {
        printf("Datoteka nije otvorena!\n");
        return;
    }
    
    printf("\n");
    printf("========================================================================================================\n");
    printf("                           SADRZAJ AGREGIRANE DATOTEKE\n");
    printf("========================================================================================================\n");
    printf("%5s %5s %12s %20s %20s %8s %20s %8s %8s %8s\n", 
           "Baket", "Poz", "Reg.ozn", "Marka", "Model", "Godina", 
           "Boja", "Bela", "Crvena", "Plava");
    printf("--------------------------------------------------------------------------------------------------------\n");
    
    int ukupno_slogova = 0;
    int prekoracioci = 0;
    
    rewind(frasuta);
    
    for (int b = 0; b < B; b++) {
        BAKET baket;
        fseek(frasuta, b * sizeof(BAKET), SEEK_SET);
        fread(&baket, sizeof(BAKET), 1, frasuta);
        
        for (int i = 0; i < FAKTOR_B; i++) {
            // Proveri da li slog postoji (nije OZNAKA_KRAJA) i da je aktivan (status ' ')
            if (baket.slogovi[i].podaci.reg_oznaka != OZNAKA_KRAJA && 
                baket.slogovi[i].podaci.status == ' ') {
                
                printf("%5d %5d %12d %20s %20s %8s %20s %8d %8d %8d",
                       b, i,
                       baket.slogovi[i].podaci.reg_oznaka,
                       baket.slogovi[i].podaci.marka,
                       baket.slogovi[i].podaci.model,
                       baket.slogovi[i].podaci.godina_proizvodnje,
                       baket.slogovi[i].podaci.boja,
                       baket.slogovi[i].podaci.duz_bela,
                       baket.slogovi[i].podaci.duz_crvena,
                       baket.slogovi[i].podaci.duz_plava);
                
                int maticni = hash_preklapanje(baket.slogovi[i].podaci.reg_oznaka);
                if (maticni != b) {
                    printf("  (PREK)");
                    prekoracioci++;
                }
                printf("\n");
                
                ukupno_slogova++;
            }
        }
    }
    
    printf("========================================================================================================\n");
    printf("UKUPNO SLOGOVA: %d\n", ukupno_slogova);
    printf("PREKORACILACA: %d\n", prekoracioci);
    printf("========================================================================================================\n\n");
}

int fizicki_obrisi_iz_rasute(FILE* frasuta, int reg, int* E) {
    if (frasuta == NULL) {
        printf("Datoteka nije otvorena!\n");
        return -1;
    }
    
    // 1. PRVO NADJI SLOG (USPESNO TRAZENJE)
    SLOG_AGREGAT pronadjen;
    int baket, pozicija;
    int status = nadji_u_rasutoj(frasuta, reg, &pronadjen, &baket, &pozicija);
    
    if (status == 0) {
        printf("Auto %d ne postoji u agregiranoj datoteci!\n", reg);
        return -1;
    }
    
    printf("Pronadjen auto %d u baketu %d, pozicija %d\n", reg, baket, pozicija);
    
    // 2. UCITAJ BAKET U KOME SE NALAZI SLOG
    BAKET b;
    fseek(frasuta, baket * sizeof(BAKET), SEEK_SET);
    fread(&b, sizeof(BAKET), 1, frasuta);
    
    // 3. FIZICKI OBRISI SLOG (postavi na OZNAKA_KRAJA)
    b.slogovi[pozicija].podaci.reg_oznaka = OZNAKA_KRAJA;
    b.slogovi[pozicija].podaci.status = ' ';
    strcpy(b.slogovi[pozicija].podaci.marka, "");
    strcpy(b.slogovi[pozicija].podaci.model, "");
    strcpy(b.slogovi[pozicija].podaci.godina_proizvodnje, "");
    strcpy(b.slogovi[pozicija].podaci.boja, "");
    b.slogovi[pozicija].podaci.duz_bela = 0;
    b.slogovi[pozicija].podaci.duz_crvena = 0;
    b.slogovi[pozicija].podaci.duz_plava = 0;
    
    // NE DIRAMO sledeci_u_lancu_sinonima - ostaje -1
    
    // 4. POVECAJ BROJ SLOBODNIH MESTA
    b.broj_slobodnih++;
    
    // 5. AZURIRAJ POKAZIVACE U LANCU SLOBODNIH BAKETA
    // Ako je baket bio pun pre brisanja, sada ima 1 slobodno mesto
    if (b.broj_slobodnih == 1) {
        // Treba ga vratiti u lanac slobodnih baketa
        // Postavi ga na pocetak lanca (ili bilo gde, ali na pocetak je najlakse)
        
        if (*E != -1) {
            // Postoje drugi slobodni baketi
            BAKET prvi_slobodni;
            fseek(frasuta, *E * sizeof(BAKET), SEEK_SET);
            fread(&prvi_slobodni, sizeof(BAKET), 1, frasuta);
            
            // Povezi trenutni baket sa prvim u lancu
            b.sledeci_u_lancu_slobodnih = *E;
            b.prethodni_u_lancu_slobodnih = -1;
            
            // Azuriraj prethodni pokazivac prvog slobodnog
            prvi_slobodni.prethodni_u_lancu_slobodnih = baket;
            
            fseek(frasuta, *E * sizeof(BAKET), SEEK_SET);
            fwrite(&prvi_slobodni, sizeof(BAKET), 1, frasuta);
        } else {
            // Nema drugih slobodnih baketa
            b.sledeci_u_lancu_slobodnih = -1;
            b.prethodni_u_lancu_slobodnih = -1;
        }
        
        // Azuriraj E da pokazuje na ovaj baket
        *E = baket;
    }
    
    // 6. AZURIRAJ LANAC SINONIMA (ako je slog bio u lancu)
    if (status == 2) { // pronadjen u lancu sinonima
        // Treba pronaci prethodni slog u lancu i azurirati njegov pokazivac
        
        int maticni = hash_preklapanje(reg);
        BAKET maticni_baket;
        fseek(frasuta, maticni * sizeof(BAKET), SEEK_SET);
        fread(&maticni_baket, sizeof(BAKET), 1, frasuta);
        
        if (maticni_baket.prvi_u_lancu_sinonima == baket) {
            // Ovo je bio prvi u lancu
            maticni_baket.prvi_u_lancu_sinonima = b.slogovi[0].sledeci_u_lancu_sinonima;
            
            fseek(frasuta, maticni * sizeof(BAKET), SEEK_SET);
            fwrite(&maticni_baket, sizeof(BAKET), 1, frasuta);
        } else {
            // Nije prvi - treba naci prethodni
            int prethodni_baket = maticni_baket.prvi_u_lancu_sinonima;
            BAKET prethodni;
            
            while (prethodni_baket != -1) {
                fseek(frasuta, prethodni_baket * sizeof(BAKET), SEEK_SET);
                fread(&prethodni, sizeof(BAKET), 1, frasuta);
                
                if (prethodni.slogovi[0].sledeci_u_lancu_sinonima == baket) {
                    // Nasli smo prethodni
                    prethodni.slogovi[0].sledeci_u_lancu_sinonima = b.slogovi[0].sledeci_u_lancu_sinonima;
                    
                    fseek(frasuta, prethodni_baket * sizeof(BAKET), SEEK_SET);
                    fwrite(&prethodni, sizeof(BAKET), 1, frasuta);
                    break;
                }
                
                prethodni_baket = prethodni.slogovi[0].sledeci_u_lancu_sinonima;
            }
        }
    }
    
    // 7. UPISI IZMENJEN BAKET
    fseek(frasuta, baket * sizeof(BAKET), SEEK_SET);
    fwrite(&b, sizeof(BAKET), 1, frasuta);
    fflush(frasuta);
    
    printf("Auto %d uspesno fizicki obrisan iz baketa %d.\n", reg, baket);
    return 0;
}

int azuriraj_agregiranu(FILE* fagregat, int kljuc) {
    if (fagregat == NULL) {
        printf("Datoteka nije otvorena!\n");
        return 2;
    }

    // 1. Prvo nadji slog koristeci PRAVU funkciju
    int baket, pozicija;
    SLOG_AGREGAT slog;
    
    int status = nadji_u_rasutoj(fagregat, kljuc, &slog, &baket, &pozicija);
    
    if (status == 0) {
        printf("Automobil sa oznakom %d nije pronadjen u rasutoj datoteci!\n", kljuc);
        return 0;
    }
    
    // 2. Prikazi trenutne podatke
    printf("\n=== IZMENA SLOGA U RASUTOJ DATOTECI ===\n");
    printf("Automobil sa oznakom: %d\n", kljuc);
    printf("Lokacija: baket %d, pozicija %d\n", baket, pozicija + 1);
    printf("\nTrenutni podaci:\n");
    printf("1. Marka: %s\n", slog.marka);
    printf("2. Model: %s\n", slog.model);
    printf("3. Godina proizvodnje: %s\n", slog.godina_proizvodnje);
    printf("4. Boja: %s\n", slog.boja);
    printf("5. Duzina boravka - Bela zona: %d sati\n", slog.duz_bela);
    printf("6. Duzina boravka - Crvena zona: %d sati\n", slog.duz_crvena);
    printf("7. Duzina boravka - Plava zona: %d sati\n", slog.duz_plava);
    printf("8. Status: %c\n", slog.status);
    printf("0. Kraj izmene i cuvanje\n");
    
    // 3. Menjanje podataka
    int izbor = -1;
    SLOG_AGREGAT izmenjen = slog;
    
    do {
        printf("\nIzaberite polje za izmenu (1-8, 0 za kraj): ");
        scanf("%d", &izbor);
        while(getchar() != '\n');
        
        switch(izbor) {
            case 1: // Marka
                printf("Trenutna marka: %s\n", izmenjen.marka);
                printf("Unesite novu marku: ");
                scanf(" %30[^\n]", izmenjen.marka);
                while(getchar() != '\n');
                printf("Marka je uspesno izmenjena!\n");
                break;
                
            case 2: // Model
                printf("Trenutni model: %s\n", izmenjen.model);
                printf("Unesite novi model: ");
                scanf(" %30[^\n]", izmenjen.model);
                while(getchar() != '\n');
                printf("Model je uspesno izmenjen!\n");
                break;
                
            case 3: // Godina proizvodnje
                printf("Trenutna godina proizvodnje: %s\n", izmenjen.godina_proizvodnje);
                printf("Unesite novu godinu proizvodnje: ");
                scanf(" %4[^\n]", izmenjen.godina_proizvodnje);
                while(getchar() != '\n');
                printf("Godina proizvodnje je uspesno izmenjena!\n");
                break;
                
            case 4: // Boja
                printf("Trenutna boja: %s\n", izmenjen.boja);
                printf("Unesite novu boju: ");
                scanf(" %20[^\n]", izmenjen.boja);
                while(getchar() != '\n');
                printf("Boja je uspesno izmenjena!\n");
                break;
                
            case 5: // Bela zona
                printf("Trenutna duzina u beloj zoni: %d sati\n", izmenjen.duz_bela);
                printf("Unesite novu vrednost: ");
                scanf("%d", &izmenjen.duz_bela);
                while(getchar() != '\n');
                printf("Duzina u beloj zoni je uspesno izmenjena!\n");
                break;
                
            case 6: // Crvena zona
                printf("Trenutna duzina u crvenoj zoni: %d sati\n", izmenjen.duz_crvena);
                printf("Unesite novu vrednost: ");
                scanf("%d", &izmenjen.duz_crvena);
                while(getchar() != '\n');
                printf("Duzina u crvenoj zoni je uspesno izmenjena!\n");
                break;
                
            case 7: // Plava zona
                printf("Trenutna duzina u plavoj zoni: %d sati\n", izmenjen.duz_plava);
                printf("Unesite novu vrednost: ");
                scanf("%d", &izmenjen.duz_plava);
                while(getchar() != '\n');
                printf("Duzina u plavoj zoni je uspesno izmenjena!\n");
                break;
                
            case 8: // Status
                printf("Trenutni status: %c\n", izmenjen.status);
                printf("Unesite novi status ( ' ' - aktivan, '*' - obrisan): ");
                scanf(" %c", &izmenjen.status);
                while(getchar() != '\n');
                printf("Status je uspesno izmenjen!\n");
                break;
                
            case 0:
                printf("Cuvanje izmena...\n");
                break;
                
            default:
                printf("Nepostojeci izbor! Pokusajte ponovo.\n");
        }
        
    } while (izbor != 0);
    
    // 4. Upisi izmenjen slog nazad u baket
    // Ponovo ucitaj ceo baket
    BAKET baket_struct;
    fseek(fagregat, baket * sizeof(BAKET), SEEK_SET);
    fread(&baket_struct, sizeof(BAKET), 1, fagregat);
    
    // Izmeni slog na odgovarajućoj poziciji
    baket_struct.slogovi[pozicija].podaci = izmenjen;
    
    // Vrati izmenjen baket
    fseek(fagregat, baket * sizeof(BAKET), SEEK_SET);
    fwrite(&baket_struct, sizeof(BAKET), 1, fagregat);
    fflush(fagregat);
    
    printf("\nAutomobil sa oznakom %d je uspesno izmenjen.\n", kljuc);
    return 1;
}

int formiraj_datoteku_promena(char* naziv) {
    FILE* f = fopen(naziv, "wb");
    if (f == NULL) {
        printf("Greska pri kreiranju datoteke promena!\n");
        return -1;
    }
    
    // Inicijalizuj prvi blok sa oznakom kraja
    BLOK_PROMENA blok;
    for (int i = 0; i < FDP; i++) {
        blok.slogovi[i].kljuc = OZNAKA_KRAJA;
        blok.slogovi[i].operacija = ' ';
    }
    
    fwrite(&blok, sizeof(BLOK_PROMENA), 1, f);
    fflush(f);
    fclose(f);
    
    printf("Datoteka promena '%s' uspesno formirana.\n", naziv);
    return 0;
}

int dodaj_u_datoteku_promena(FILE* fpromena, SLOG_PROMENA* novi_slog) {
    if (fpromena == NULL || novi_slog == NULL) return -1;
    
    // Provera da li već postoji slog sa istim ključem
    SLOG_PROMENA pronadjen;
    if (nadji_u_promenama(fpromena, novi_slog->kljuc, &pronadjen) == 1) {
        printf("Slog sa kljucem %d vec postoji u datoteci promena!\n", novi_slog->kljuc);
        return -1;
    }
    
    rewind(fpromena);
    
    BLOK_PROMENA blok;
    BLOK_PROMENA naredni_blok;
    int redni_broj_bloka = 0;
    int ubacen = 0;
    long pozicija_za_upis = -1;
    int pozicija_u_bloku = -1;
    SLOG_PROMENA slog_za_ubacivanje = *novi_slog;
    
    // Pronađi mesto za ubacivanje
    while (fread(&blok, sizeof(BLOK_PROMENA), 1, fpromena) == 1 && !ubacen) {
        long trenutna_pozicija = ftell(fpromena) - sizeof(BLOK_PROMENA);
        
        for (int i = 0; i < FDP; i++) {
            // Ako smo došli do kraja
            if (blok.slogovi[i].kljuc == OZNAKA_KRAJA) {
                // Ubaci na ovo mesto
                blok.slogovi[i] = slog_za_ubacivanje;
                
                fseek(fpromena, trenutna_pozicija, SEEK_SET);
                fwrite(&blok, sizeof(BLOK_PROMENA), 1, fpromena);
                fflush(fpromena);
                
                ubacen = 1;
                printf("Dodat slog sa kljucem %d na kraj datoteke.\n", slog_za_ubacivanje.kljuc);
                break;
            }
            
            // Ako je trenutni ključ veći od novog, ubaci ispred njega
            if (blok.slogovi[i].kljuc > slog_za_ubacivanje.kljuc) {
                // Sačuvaj slog koji se pomera
                SLOG_PROMENA temp = blok.slogovi[i];
                blok.slogovi[i] = slog_za_ubacivanje;
                
                // Upisujemo trenutni blok sa novim slogom
                fseek(fpromena, trenutna_pozicija, SEEK_SET);
                fwrite(&blok, sizeof(BLOK_PROMENA), 1, fpromena);
                fflush(fpromena);
                
                // Sada rekurzivno dodajemo temp slog (on ide dalje)
                slog_za_ubacivanje = temp;
                break;
            }
        }
        
        redni_broj_bloka++;
    }
    
    // Ako nije ubaceno ni posle svih blokova, dodaj na kraj
    if (!ubacen) {
        // Idi na poslednji blok
        fseek(fpromena, -sizeof(BLOK_PROMENA), SEEK_END);
        fread(&blok, sizeof(BLOK_PROMENA), 1, fpromena);
        long poslednja_pozicija = ftell(fpromena) - sizeof(BLOK_PROMENA);
        
        // Nađi prvo prazno mesto u poslednjem bloku
        for (int i = 0; i < FDP; i++) {
            if (blok.slogovi[i].kljuc == OZNAKA_KRAJA) {
                blok.slogovi[i] = slog_za_ubacivanje;
                
                fseek(fpromena, poslednja_pozicija, SEEK_SET);
                fwrite(&blok, sizeof(BLOK_PROMENA), 1, fpromena);
                fflush(fpromena);
                
                ubacen = 1;
                printf("Dodat slog sa kljucem %d na kraj datoteke.\n", slog_za_ubacivanje.kljuc);
                break;
            }
        }
        
        // Ako je poslednji blok pun, kreiraj novi
        if (!ubacen) {
            BLOK_PROMENA novi_blok;
            for (int i = 0; i < FDP; i++) {
                novi_blok.slogovi[i].kljuc = OZNAKA_KRAJA;
                novi_blok.slogovi[i].operacija = ' ';
            }
            novi_blok.slogovi[0] = slog_za_ubacivanje;
            
            fseek(fpromena, 0, SEEK_END);
            fwrite(&novi_blok, sizeof(BLOK_PROMENA), 1, fpromena);
            fflush(fpromena);
            
            printf("Kreiran novi blok i dodat slog sa kljucem %d.\n", slog_za_ubacivanje.kljuc);
        }
    }
    
    return 0;
}

void ispisi_datoteku_promena(FILE* fpromena) {
    if (fpromena == NULL) {
        printf("Datoteka promena nije otvorena!\n");
        return;
    }
    
    printf("\n");
    printf("====================================================================\n");
    printf("           DATOTEKA PROMENA (sekvencijalna - sortirana)\n");
    printf("====================================================================\n");
    printf("%5s %10s %12s %s\n", "Blok", "Poz", "Kljuc", "Operacija");
    printf("--------------------------------------------------------------------\n");
    
    rewind(fpromena);
    BLOK_PROMENA blok;
    int rbrb = 0;
    int ukupno = 0;
    
    while (fread(&blok, sizeof(BLOK_PROMENA), 1, fpromena) == 1) {
        for (int i = 0; i < FDP; i++) {
            if (blok.slogovi[i].kljuc != OZNAKA_KRAJA) {
                char op_str[20];
                switch(blok.slogovi[i].operacija) {
                    case 'n': strcpy(op_str, "NOVI"); break;
                    case 'm': strcpy(op_str, "MODIFIKACIJA"); break;
                    case 'b': strcpy(op_str, "BRISANJE"); break;
                    default: strcpy(op_str, "NEPOZNATO");
                }
                
                printf("%5d %10d %12d %s\n", 
                       rbrb, i, blok.slogovi[i].kljuc, op_str);
                ukupno++;
            }
        }
        rbrb++;
    }
    
    printf("--------------------------------------------------------------------\n");
    printf("Ukupno slogova: %d\n", ukupno);
    printf("====================================================================\n\n");
}

int nadji_u_promenama(FILE* fpromena, int kljuc, SLOG_PROMENA* pronadjen) {
    if (fpromena == NULL) return -1;
    
    rewind(fpromena);
    
    BLOK_PROMENA blok;
    int rbrb = 0;
    
    while (fread(&blok, sizeof(BLOK_PROMENA), 1, fpromena) == 1) {
        for (int i = 0; i < FDP; i++) {
            int k = blok.slogovi[i].kljuc;
            
            if (k == OZNAKA_KRAJA) {
                return 0;  // nema ga dalje
            }
            
            if (k == kljuc) {
                *pronadjen = blok.slogovi[i];
                return 1;  // pronađen
            }
            
            if (k > kljuc) {
                return 0;  // preskočili smo, nema ga
            }
        }
        rbrb++;
    }
    
    return 0;
}

// =================================================================
// FIZIČKO BRISANJE IZ DATOTEKE PROMENA (sortirano)
// =================================================================
int fizicki_obrisi_iz_promena(FILE* fpromena, int kljuc) {
    if (fpromena == NULL) {
        printf("Datoteka promena nije otvorena!\n");
        return 2;
    }

    rewind(fpromena);
    
    BLOK_PROMENA blok;
    BLOK_PROMENA naredni_blok;
    int redni_broj_bloka = 0;
    int pronadjen = 0;
    
    while (fread(&blok, sizeof(BLOK_PROMENA), 1, fpromena) == 1 && !pronadjen) {
        long pozicija_bloka = ftell(fpromena) - sizeof(BLOK_PROMENA);
        
        for (int i = 0; i < FDP; i++) {
            int k = blok.slogovi[i].kljuc;
            
            if (k == OZNAKA_KRAJA) {
                break;
            }
            
            if (k == kljuc) {
                pronadjen = 1;
                
                // Pomeri sve slogove iza i za jedno mesto unapred
                for (int j = i; j < FDP - 1; j++) {
                    blok.slogovi[j] = blok.slogovi[j + 1];
                }
                
                // Proveri da li ima narednih blokova
                long sledeca_pozicija = (redni_broj_bloka + 1) * sizeof(BLOK_PROMENA);
                fseek(fpromena, sledeca_pozicija, SEEK_SET);
                int ima_naredni = fread(&naredni_blok, sizeof(BLOK_PROMENA), 1, fpromena);
                
                if (ima_naredni == 1) {
                    // Uzmi prvi slog iz narednog bloka
                    blok.slogovi[FDP - 1] = naredni_blok.slogovi[0];
                    
                    // Pomeri slogove u narednom bloku
                    for (int j = 0; j < FDP - 1; j++) {
                        naredni_blok.slogovi[j] = naredni_blok.slogovi[j + 1];
                    }
                    naredni_blok.slogovi[FDP - 1].kljuc = OZNAKA_KRAJA;
                    naredni_blok.slogovi[FDP - 1].operacija = ' ';
                    
                    // Upisi izmenjen naredni blok
                    fseek(fpromena, sledeca_pozicija, SEEK_SET);
                    fwrite(&naredni_blok, sizeof(BLOK_PROMENA), 1, fpromena);
                    fflush(fpromena);
                } else {
                    // Nema narednog bloka, samo postavi oznaku kraja
                    blok.slogovi[FDP - 1].kljuc = OZNAKA_KRAJA;
                    blok.slogovi[FDP - 1].operacija = ' ';
                }
                
                // Upisi izmenjen trenutni blok
                fseek(fpromena, pozicija_bloka, SEEK_SET);
                fwrite(&blok, sizeof(BLOK_PROMENA), 1, fpromena);
                fflush(fpromena);
                
                printf("Slog sa kljucem %d je uspesno obrisan iz datoteke promena.\n", kljuc);
                return 0;
            }
            
            if (k > kljuc) {
                // Preskočili smo mesto gde bi trebalo da bude
                printf("Slog sa kljucem %d nije pronadjen (preskocen).\n", kljuc);
                return 2;
            }
        }
        
        redni_broj_bloka++;
    }
    
    if (!pronadjen) {
        printf("Slog sa kljucem %d nije pronadjen u datoteci promena!\n", kljuc);
        return 2;
    }
    
    return 0;
}

int logicki_obrisi_iz_rasute(FILE* frasuta, int kljuc) {
    if (frasuta == NULL) {
        printf("Datoteka nije otvorena!\n");
        return -1;
    }
    
    // 1. NADJI SLOG
    SLOG_AGREGAT pronadjen;
    int baket, pozicija;
    int status = nadji_u_rasutoj(frasuta, kljuc, &pronadjen, &baket, &pozicija);
    
    if (status == 0) {
        printf("Auto %d ne postoji u agregiranoj datoteci!\n", kljuc);
        return -1;
    }
    
    // 2. UCITAJ BAKET
    BAKET b;
    fseek(frasuta, baket * sizeof(BAKET), SEEK_SET);
    fread(&b, sizeof(BAKET), 1, frasuta);
    
    // 3. PROVERI DA LI JE VEC OBRISAN
    if (b.slogovi[pozicija].podaci.status == '*') {
        printf("Auto %d je vec logicki obrisan!\n", kljuc);
        return -1;
    }
    
    // 4. LOGIČKI OBRIŠI
    b.slogovi[pozicija].podaci.status = '*';
    printf("Auto %d logicki obrisan (bio aktivan).\n", kljuc);
    
    // 5. UPISI IZMENJEN BAKET
    fseek(frasuta, baket * sizeof(BAKET), SEEK_SET);
    fwrite(&b, sizeof(BAKET), 1, frasuta);
    fflush(frasuta);
    
    return 0;
}


int formiraj_log_datoteku(char* naziv) {
    FILE* f = fopen(naziv, "wb");
    if(f == NULL) return -1;
    
    // PRAZAN FAJL – ne pišemo ništa!
    fclose(f);
    
    printf("Log datoteka '%s' kreirana (prazna).\n", naziv);
    return 0;
}

int dodaj_u_log(FILE* flog, int id_auta, char* vrsta) {
    if(flog == NULL) return -1;
    
    // Idi na kraj
    fseek(flog, 0, SEEK_END);
    
    // Izračunaj sledeći ID
    long velicina = ftell(flog);
    int novi_id = (velicina / sizeof(SLOG_LOG)) + 1;
    
    SLOG_LOG novi;
    novi.identifikator = novi_id;
    novi.identifikator_automobila = id_auta;
    strcpy(novi.vrsta_operacije, vrsta);
    
    fwrite(&novi, sizeof(SLOG_LOG), 1, flog);
    fflush(flog);
    
    return novi_id;
}

void prikazi_log_izvestaj(FILE* flog) {
    if (flog == NULL) return;
    
    rewind(flog);
    
    // Za brojanje operacija po automobilu
    struct {
        int id;
        int unos;
        int modifikacija;
    } statistika[1000];
    int br_automobila = 0;
    
    BLOK_LOG blok;
    while (fread(&blok, sizeof(BLOK_LOG), 1, flog) == 1) {
        for (int i = 0; i < F_LOG; i++) {
            if (blok.slogovi[i].identifikator == OZNAKA_KRAJA) continue;
            
            int id_auta = blok.slogovi[i].identifikator_automobila;
            char* vrsta = blok.slogovi[i].vrsta_operacije;
            
            // Pronađi ili dodaj u statistiku
            int nadjen = -1;
            for (int j = 0; j < br_automobila; j++) {
                if (statistika[j].id == id_auta) {
                    nadjen = j;
                    break;
                }
            }
            
            if (nadjen == -1) {
                nadjen = br_automobila++;
                statistika[nadjen].id = id_auta;
                statistika[nadjen].unos = 0;
                statistika[nadjen].modifikacija = 0;
            }
            
            if (strcmp(vrsta, "UNOS") == 0)
                statistika[nadjen].unos++;
            else if (strcmp(vrsta, "MODIFIKACIJA") == 0)
                statistika[nadjen].modifikacija++;
        }
    }
    
    printf("\n========================================\n");
    printf("       IZVESTAJ IZ LOG DATOTEKE\n");
    printf("========================================\n");
    printf("%12s %10s %15s\n", "ID auta", "UNOS", "MODIFIKACIJA");
    printf("----------------------------------------\n");
    
    for (int i = 0; i < br_automobila; i++) {
        printf("%12d %10d %15d\n", 
               statistika[i].id, 
               statistika[i].unos, 
               statistika[i].modifikacija);
    }
    printf("========================================\n");
}

int direktna_obrada(FILE* fagregat, FILE* fpromena, FILE* flog, int* E) {
    if (fagregat == NULL || fpromena == NULL) {
        printf("Datoteke nisu otvorene!\n");
        return -1;
    }
    
    printf("\n=== DIREKTNA OBRADA ===\n");
    printf("Vodeca: promena.bin -> Obradivana: rasuta.bin\n\n");
    
    rewind(fpromena);  // Početak vodeće datoteke
    
    BLOK_PROMENA blok;
    int obradjeno = 0;
    int greske = 0;
    
    while (fread(&blok, sizeof(BLOK_PROMENA), 1, fpromena) == 1) {
        for (int i = 0; i < FDP; i++) {
            if (blok.slogovi[i].kljuc == OZNAKA_KRAJA) continue;
            
            SLOG_PROMENA* p = &blok.slogovi[i];
            
            printf("Obrada: kljuc=%d, operacija=%c\n", p->kljuc, p->operacija);
            
            switch(p->operacija) {
                case 'n': // NOVI SLOG
                {
                    int rez = dodaj_slog_u_rasutu(fagregat, &p->slog, E);
                    if (rez == 0) {
                        if (flog) dodaj_u_log(flog, p->kljuc, "UNOS");
                        obradjeno++;
                    } else {
                        greske++;
                    }
                    break;
                }
                
                case 'm': // MODIFIKACIJA
                {
                    SLOG_AGREGAT stari;
                    int baket, poz;
                    int postoji = nadji_u_rasutoj(fagregat, p->kljuc, &stari, &baket, &poz);
                    
                    if (postoji != 0) {
                        // Postoji – modifikuj
                        BAKET b;
                        fseek(fagregat, baket * sizeof(BAKET), SEEK_SET);
                        fread(&b, sizeof(BAKET), 1, fagregat);
                        b.slogovi[poz].podaci = p->slog;
                        fseek(fagregat, baket * sizeof(BAKET), SEEK_SET);
                        fwrite(&b, sizeof(BAKET), 1, fagregat);
                        fflush(fagregat);
                        
                        if (flog) dodaj_u_log(flog, p->kljuc, "MODIFIKACIJA");
                        obradjeno++;
                    } else {
                        // Ne postoji – dodaj kao novi
                        printf("   Auto %d ne postoji – dodajem kao NOVI\n", p->kljuc);
                        int rez = dodaj_slog_u_rasutu(fagregat, &p->slog, E);
                        if (rez == 0) {
                            if (flog) dodaj_u_log(flog, p->kljuc, "UNOS");
                            obradjeno++;
                        } else {
                            greske++;
                        }
                    }
                    break;
                }
                
                case 'b': // BRISANJE
                {
                    int rez = fizicki_obrisi_iz_rasute(fagregat, p->kljuc, E);
                    if (rez == 0) {
                        obradjeno++;
                    } else {
                        greske++;
                    }
                    break;
                }
                
                default:
                    printf("   Nepoznata operacija: %c\n", p->operacija);
                    greske++;
            }
        }
    }
    
    printf("\n=== DIREKTNA OBRADA ZAVRSENA ===\n");
    printf("Uspesno obradjeno: %d\n", obradjeno);
    printf("Gresaka: %d\n", greske);
    
    // NE DIRAMO promena.dat – ostaje netaknuta!
    
    return 0;
}

int izmeni_u_promenama(FILE* fpromena, int kljuc, SLOG_PROMENA* novi_slog) {
    if (fpromena == NULL || novi_slog == NULL) return -1;
    
    rewind(fpromena);
    
    BLOK_PROMENA blok;
    int rbrb = 0;
    int pronadjen = 0;
    
    while (fread(&blok, sizeof(BLOK_PROMENA), 1, fpromena) == 1 && !pronadjen) {
        long pozicija_bloka = ftell(fpromena) - sizeof(BLOK_PROMENA);
        
        for (int i = 0; i < FDP; i++) {
            if (blok.slogovi[i].kljuc == OZNAKA_KRAJA) {
                break;
            }
            
            if (blok.slogovi[i].kljuc == kljuc) {
                pronadjen = 1;
                
                // Prikaži trenutne podatke
                printf("\n=== IZMENA U DATOTECI PROMENA ===\n");
                printf("Trenutni slog:\n");
                printf("  Kljuc: %d\n", blok.slogovi[i].kljuc);
                printf("  Operacija: %c\n", blok.slogovi[i].operacija);
                printf("  Marka: %s\n", blok.slogovi[i].slog.marka);
                printf("  Model: %s\n", blok.slogovi[i].slog.model);
                printf("  Godina: %s\n", blok.slogovi[i].slog.godina_proizvodnje);
                printf("  Boja: %s\n", blok.slogovi[i].slog.boja);
                printf("  Bela zona: %d\n", blok.slogovi[i].slog.duz_bela);
                printf("  Crvena zona: %d\n", blok.slogovi[i].slog.duz_crvena);
                printf("  Plava zona: %d\n", blok.slogovi[i].slog.duz_plava);
                
                // Zameni sa novim slogom
                blok.slogovi[i] = *novi_slog;
                
                // Upisi izmenjen blok
                fseek(fpromena, pozicija_bloka, SEEK_SET);
                fwrite(&blok, sizeof(BLOK_PROMENA), 1, fpromena);
                fflush(fpromena);
                
                printf("\nSlog uspesno izmenjen!\n");
                return 0;
            }
            
            if (blok.slogovi[i].kljuc > kljuc) {
                // Preskočili smo mesto gde bi trebalo da bude
                printf("Slog sa kljucem %d nije pronadjen.\n", kljuc);
                return -1;
            }
        }
        rbrb++;
    }
    
    printf("Slog sa kljucem %d nije pronadjen.\n", kljuc);
    return -1;
}

int propagiraj_iz_automobila(FILE* fagregat, FILE* fauto, FILE* fpark, FILE* flog) {
    if (fagregat == NULL || fauto == NULL || fpark == NULL) {
        printf("Datoteke nisu aktivirane!\n");
        return -1;
    }
    
    printf("\n=== PROPAGACIJA IZ DATOTEKA DELA 1 ===\n");
    
    rewind(fauto);
    BLOK_AUTO blok_auto;
    int propagirano = 0;
    int greske = 0;
    
    while (fread(&blok_auto, sizeof(BLOK_AUTO), 1, fauto) == 1) {
        for (int i = 0; i < FBLOKIRANJA_1; i++) {
            if (blok_auto.automobili[i].reg_oznaka == OZNAKA_KRAJA_DATOTEKE) {
                continue;
            }
            
            int reg = blok_auto.automobili[i].reg_oznaka;
            
            // Kreiraj agregirani slog
            SLOG_AGREGAT novi;
            novi.reg_oznaka = reg;
            strcpy(novi.marka, blok_auto.automobili[i].marka);
            strcpy(novi.model, blok_auto.automobili[i].model);
            strcpy(novi.godina_proizvodnje, blok_auto.automobili[i].god_proizvodnje);
            strcpy(novi.boja, blok_auto.automobili[i].boja);
            novi.duz_bela = 0;
            novi.duz_crvena = 0;
            novi.duz_plava = 0;
            novi.status = ' ';
            
            // Saberi sate iz parkinga
            rewind(fpark);
            BLOK_PARKING blok_park;
            
            while (fread(&blok_park, sizeof(BLOK_PARKING), 1, fpark) == 1) {
                for (int j = 0; j < FBLOKIRANJA_2; j++) {
                    if (blok_park.parkinzi[j].id == OZNAKA_KRAJA_DATOTEKE) break;
                    if (blok_park.parkinzi[j].reg_oznaka == reg) {
                        switch(blok_park.parkinzi[j].zona) {
                            case 0: novi.duz_plava += blok_park.parkinzi[j].sati; break;
                            case 1: novi.duz_bela += blok_park.parkinzi[j].sati; break;
                            case 2: novi.duz_crvena += blok_park.parkinzi[j].sati; break;
                        }
                    }
                }
            }
            
            // Dodaj u agregiranu datoteku
            int E_local = 0; // privremeno, ne koristimo globalno E
            int rez = dodaj_slog_u_rasutu(fagregat, &novi, &E_local);
            
            if (rez == 0) {
                if (flog != NULL) {
                    dodaj_u_log(flog, reg, "UNOS");
                }
                propagirano++;
                printf("Propagiran auto %d\n", reg);
            } else {
                greske++;
                printf("Greska pri propagaciji auta %d\n", reg);
            }
        }
    }
    
    printf("\n=== PROPAGACIJA ZAVRSENA ===\n");
    printf("Uspesno propagirano: %d\n", propagirano);
    printf("Gresaka: %d\n", greske);
    
    return 0;
}

void ispisi_log_datoteku(FILE* flog) {
    if(flog == NULL) {
        printf("Log nije otvoren!\n");
        return;
    }
    
    rewind(flog);
    
    printf("\n========================================\n");
    printf("         SADRZAJ LOG DATOTEKE\n");
    printf("========================================\n");
    printf("%5s %15s %20s\n", "ID", "ID auta", "Operacija");
    printf("----------------------------------------\n");
    
    SLOG_LOG slog;
    int broj = 0;
    
    while(fread(&slog, sizeof(SLOG_LOG), 1, flog) == 1) {
        broj++;
        printf("%5d %15d %20s\n", 
               slog.identifikator, 
               slog.identifikator_automobila, 
               slog.vrsta_operacije);
    }
    
    printf("----------------------------------------\n");
    printf("Ukupno slogova: %d\n", broj);
    printf("========================================\n\n");
}