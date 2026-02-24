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
            
            // Povezi sa maticnim
            if (maticni_baket.prvi_u_lancu_sinonima == -1) {
                maticni_baket.prvi_u_lancu_sinonima = slobodan;
            } else {
                int poslednji = maticni_baket.prvi_u_lancu_sinonima;
                BAKET temp;
                while (1) {
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

// Ispisi agregiranu datoteku
void ispisi_agregiranu_datoteku(FILE* frasuta) {
    if (frasuta == NULL) {
        printf("Datoteka nije otvorena!\n");
        return;
    }
    
    printf("\n");
    printf("============================================================================================================\n");
    printf("                                    SADRZAJ AGREGIRANE DATOTEKE\n");
    printf("============================================================================================================\n");
    printf("%5s %5s %12s %20s %20s %8s %20s %8s %8s %8s %6s\n", 
           "Baket", "Poz", "Reg.ozn", "Marka", "Model", "Godina", 
           "Boja", "Bela", "Crvena", "Plava", "Status");
    printf("------------------------------------------------------------------------------------------------------------\n");
    
    int ukupno_slogova = 0;
    int prekoracioci = 0;
    
    for (int b = 0; b < B; b++) {
        BAKET baket;
        fseek(frasuta, b * sizeof(BAKET), SEEK_SET);
        fread(&baket, sizeof(BAKET), 1, frasuta);
        
        for (int i = 0; i < FAKTOR_B; i++) {
            if (baket.slogovi[i].podaci.reg_oznaka != OZNAKA_KRAJA) {
                printf("%5d %5d %12d %20s %20s %8s %20s %8d %8d %8d ",
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
        
        /*if (baket.sledeci_u_lancu_slobodnih != -1) {
            printf("      Baket %d -> sledeci slobodan: %d\n", b, baket.sledeci_u_lancu_slobodnih);
        }
        if (baket.prvi_u_lancu_sinonima != -1) {
            printf("      Baket %d -> prvi u lancu sinonima: %d\n", b, baket.prvi_u_lancu_sinonima);
        }*/
    }
    
    printf("============================================================================================================\n");
    printf("UKUPNO SLOGOVA: %d\n", ukupno_slogova);
    printf("PREKORACILACA: %d\n", prekoracioci);
    printf("============================================================================================================\n\n");
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