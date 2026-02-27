#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "auto.h"
#include "parkiranje.h"
#include "operacije.h"
#include "rasuta.h"
#include "agregirani.h"



int main() {
    int glavnaOpcija;
    int podOpcija;
    FILE* afile = NULL;
    FILE* pfile = NULL;

    FILE* agregat_file = NULL;   
    FILE* fpromena = NULL;      
    FILE* flog = NULL; 
    
    int E;  // pokazivac na prvi slobodan baket
    int prebroj_prekoracioci; 

    char aktivna_dat[256];
    
    do {
        printf("\n=== GLAVNI MENI ===\n");
        printf("1. Rad sa datotekom automobila\n");
        printf("2. Rad sa datotekom parkiranja\n");
        printf("3. Izvestaji\n");
        printf("4. Rad sa agregiranim podacima\n");
        printf("0. Izlaz iz programa\n");
        printf("Izaberite opciju: ");
        scanf("%d", &glavnaOpcija);
        while(getchar() != '\n');
        
        switch(glavnaOpcija) {
            case 1:
                do {
                    printf("\n--- MENI ZA AUTOMOBILE ---\n");
                    printf("1. Aktiviraj datoteku\n");
                    printf("2. Napravi novu datoteku\n");
                    printf("3. Pretraga automobila\n");
                    printf("4. Ispis svih automobila\n");
                    printf("5. Dodavanje automobila\n");
                    printf("6. Brisanje automobila\n");
                    printf("7. Modifikacija automobila\n");
                    printf("0. Povratak na glavni meni\n");
                    printf("Izaberite opciju: ");
                    scanf("%d", &podOpcija);
                    while(getchar() != '\n');
                    
                    switch(podOpcija) {
                        case 1:
                            if(afile != NULL) {
                                fclose(afile);
                                afile = NULL;
                            }
                            printf("Unesite naziv datoteke za automobile: ");
                            scanf("%s", aktivna_dat);
                            //while(getchar() != '\n');
                            afile = postavi_aktivnu_datoteku(aktivna_dat);
                            if(afile != NULL) {
                                printf("Datoteka '%s' uspesno aktivirana.\n", aktivna_dat);
                            } else {
                                printf("Greska pri otvaranju datoteke!\n");
                            }
                            break;
                            
                        case 2: {
                            char naz_dat[256];
                            printf("Unesite naziv nove datoteke za automobile: ");
                            scanf("%s", naz_dat);
                            while(getchar() != '\n');
                            
                            if(napravi_datoteku_auto(naz_dat) == 0) {
                                printf("Datoteka '%s' uspesno kreirana.\n", naz_dat);
                                if(afile != NULL) fclose(afile);
                                afile = postavi_aktivnu_datoteku(naz_dat);
                            } else {
                                printf("Greska pri kreiranju datoteke!\n");
                            }
                            break;
                        }
                        
                        case 3: {
                            if(afile == NULL) {
                                printf("Prvo aktivirajte datoteku!\n");
                                break;
                            }
                            
                            int reg;
                            SLOG_AUTO slog;
                            printf("Unesite registarsku oznaku za pretragu: ");
                            scanf("%d", &reg);
                            while(getchar() != '\n');
                            
                            int rez = nadji_auto_sek(afile, reg, &slog);
                            if(rez == 1) {
                                printf("\nAutomobil pronadjen:\n");
                                zaglavlje_datoteke_auto();
                                ispisi_slog_auto(&slog);
                            } else if(rez == 0) {
                                printf("Automobil sa oznakom %d ne postoji.\n", reg);
                            } else {
                                printf("Greska pri citanju datoteke!\n");
                            }
                            break;
                        }
                        
                        case 4:
                            if(afile == NULL) {
                                printf("Prvo aktivirajte datoteku!\n");
                                break;
                            }
                            ispisi_datoteku_auto(afile);
                            break;
                            
                        case 5: {
                            if(afile == NULL) {
                                printf("Prvo aktivirajte datoteku!\n");
                                break;
                            }
                            
                            SLOG_AUTO a;
                            
                            printf("\n=== UNOS NOVOG AUTOMOBILA ===\n");
                            
                            printf("Registarska oznaka: ");
                            scanf("%d", &a.reg_oznaka);
                            
                            printf("Marka: ");
                            scanf("%s", a.marka);
                            
                            printf("Model: ");
                            scanf("%s", a.model);
                            
                            printf("Godina proizvodnje: ");
                            scanf("%s", a.god_proizvodnje);
                            
                            printf("Boja: ");
                            scanf("%s", a.boja);
                            
                            printf("\nProvera unosa:\n");
                            printf("Reg: %d\n", a.reg_oznaka);
                            printf("Marka: %s\n", a.marka);
                            printf("Model: %s\n", a.model);
                            printf("Godina: %s\n", a.god_proizvodnje);
                            printf("Boja: %s\n", a.boja);
                            
                            int status = dodaj_slog_sek(afile, &a);
                            if (status == 1) {
                                printf("Automobil uspesno dodat!\n");
                            } else {
                                printf("Greska pri dodavanju automobila!\n");
                            }
                            break;
                        }
                                                
                        case 6: {
                            if(afile == NULL) {
                                printf("Prvo aktivirajte datoteku!\n");
                                break;
                            }
                            
                            int reg;
                            printf("Unesite registarsku oznaku automobila za brisanje: ");
                            scanf("%d", &reg);
                            ocisti_buffer(); // Umesto while(getchar() != '\n');
                            
                            // Prvo proveri da li auto uopste postoji
                            SLOG_AUTO slog;
                            rewind(afile);
                            int status_postoji = nadji_auto_sek(afile, reg, &slog);
                            
                            if(status_postoji != 1) {
                                printf("Automobil sa oznakom %d ne postoji.\n", reg);
                                break;
                            }
                            
                            // Pronadji sva parkiranja za ovaj auto (ako parking datoteka postoji)
                            int broj_parkiranja = 0;
                            if(pfile != NULL) {
                                BLOK_PARKING blok_p;
                                rewind(pfile);
                                
                                while(fread(&blok_p, sizeof(BLOK_PARKING), 1, pfile) == 1) {
                                    for(int i = 0; i < FBLOKIRANJA_2; i++) {
                                        if(blok_p.parkinzi[i].id == OZNAKA_KRAJA_DATOTEKE) break;
                                        if(blok_p.parkinzi[i].reg_oznaka == reg) {
                                            broj_parkiranja++;
                                        }
                                    }
                                }
                            }
                            
                            // Ako postoje parkiranja, pitaj korisnika
                            if(broj_parkiranja > 0) {
                                printf("\nAutomobil sa oznakom %d ima %d povezanih parkiranja.\n", reg, broj_parkiranja);
                                printf("Da li zelite da obrisete i sva povezana parkiranja? (1-DA, 0-NE): ");
                                
                                int izbor;
                                scanf("%d", &izbor);
                                ocisti_buffer();
                                
                                if(izbor == 1 && pfile != NULL) {
                                    // Ponovo nadji sva parkiranja i obrisi ih
                                    rewind(pfile);
                                    BLOK_PARKING blok_p;
                                    int obrisano = 0;
                                    
                                    while(fread(&blok_p, sizeof(BLOK_PARKING), 1, pfile) == 1) {
                                        for(int i = 0; i < FBLOKIRANJA_2; i++) {
                                            if(blok_p.parkinzi[i].id == OZNAKA_KRAJA_DATOTEKE) break;
                                            if(blok_p.parkinzi[i].reg_oznaka == reg) {
                                                int id_za_brisanje = blok_p.parkinzi[i].id;
                                                // Vrati se na pocetak bloka da obrises
                                                fseek(pfile, -sizeof(BLOK_PARKING), SEEK_CUR);
                                                obrisi_parking(pfile, id_za_brisanje);
                                                obrisano++;
                                                // Vrati se na nastavak citanja
                                                fseek(pfile, 0, SEEK_CUR);
                                            }
                                        }
                                    }
                                    printf("Obrisano %d povezanih parkiranja.\n", obrisano);
                                } else {
                                    printf("Parkiranja nisu obrisana. Samo auto ce biti obrisan.\n");
                                }
                            }
                            
                            // Tek sada obrisi auto iz auto datoteke
                            int rez = obrisi_auto_fizicki(afile, reg);
                            if(rez == 0) {
                                printf("Automobil sa oznakom %d je uspesno obrisan.\n", reg);
                            } else {
                                printf("Greska pri brisanju automobila!\n");
                            }
                            break;
                        }
                        case 7:{
                                printf("Unesite registarsku oznaku automobila za izmenu: ");
                                int reg;
                                scanf("%d", &reg);
                                ocisti_buffer();
                                izmeni_auto(afile, reg);
                                break;
                        }
                        
                        case 0:
                            printf("Povratak na glavni meni...\n");
                            break;
                            
                        default:
                            printf("Nepostojeca opcija!\n");
                    }
                } while(podOpcija != 0);
                break;
                
            case 2:
                do {
                    printf("\n--- MENI ZA PARKIRANJA ---\n");
                    printf("1. Aktiviraj datoteku\n");
                    printf("2. Napravi novu datoteku\n");
                    printf("3. Pretraga parkiranja po ID\n");
                    printf("4. Ispis svih parkiranja\n");
                    printf("5. Dodavanje parkiranja\n");
                    printf("6. Izmena parkiranja\n");
                    printf("7. Brisanje parkiranja\n");
                    printf("0. Povratak na glavni meni\n");
                    printf("Izaberite opciju: ");
                    scanf("%d", &podOpcija);
                    while(getchar() != '\n');
                    
                    switch(podOpcija) {
                        case 1:
                            if(pfile != NULL) {
                                fclose(pfile);
                                pfile = NULL;
                            }
                            printf("Unesite naziv datoteke za parkiranja: ");
                            scanf("%s", aktivna_dat);
                            while(getchar() != '\n');
                            pfile = postavi_aktivnu_datoteku(aktivna_dat);
                            if(pfile != NULL) {
                                printf("Datoteka '%s' uspesno aktivirana.\n", aktivna_dat);
                            } else {
                                printf("Greska pri otvaranju datoteke!\n");
                            }
                            break;
                            
                        case 2: {
                            char naz_dat[256];
                            printf("Unesite naziv nove datoteke za parkiranja: ");
                            scanf("%s", naz_dat);
                            while(getchar() != '\n');
                            
                            if(napravi_datoteku_parking(naz_dat) == 0) {
                                printf("Datoteka '%s' uspesno kreirana.\n", naz_dat);
                                if(pfile != NULL) fclose(pfile);
                                pfile = postavi_aktivnu_datoteku(naz_dat);
                            } else {
                                printf("Greska pri kreiranju datoteke!\n");
                            }
                            break;
                        }
                        
                        case 3: {
                            if(pfile == NULL) {
                                printf("Prvo aktivirajte datoteku!\n");
                                break;
                            }
                            
                            int id;
                            SLOG_PARKING slog;
                            printf("Unesite ID parkiranja za pretragu: ");
                            scanf("%d", &id);
                            while(getchar() != '\n');
                            
                            int rez = nadji_parking_serijski(pfile, id, &slog);
                            if(rez == 1) {
                                printf("\nParkiranje pronadjeno:\n");
                                zaglavlje_datoteke_parking();
                                ispisi_slog_parking(&slog);
                            } else if(rez == 0) {
                                printf("Parkiranje sa ID %d ne postoji.\n", id);
                            } else {
                                printf("Greska pri citanju datoteke!\n");
                            }
                            break;
                        }
                        
                        case 4:
                            if(pfile == NULL) {
                                printf("Prvo aktivirajte datoteku!\n");
                                break;
                            }
                            ispisi_datoteku_parking(pfile);
                            break;
                            
                        case 5: {
                            if(pfile == NULL || afile == NULL) {
                                printf("Morate aktivirati obe datoteke!\n");
                                break;
                            }
                            
                            SLOG_PARKING p;
                            int zona_unos;
                            
                            printf("\n=== UNOS NOVOG PARKIRANJA ===\n");
                            
                            do {
                                printf("Identifikator parkiranja: ");
                                scanf("%d", &p.id);
                                while(getchar() != '\n');
                                
                                if (parking_id_postoji(pfile, p.id)) {
                                    printf("Parkiranje sa ID %d vec postoji!\n", p.id);
                                } else {
                                    break;
                                }
                            } while (1);
                            
                            int reg_uneta = 0;
                            while (!reg_uneta) {
                                printf("Registarska oznaka automobila: ");
                                scanf("%d", &p.reg_oznaka);
                                while(getchar() != '\n');
                                
                                SLOG_AUTO pronadjen_auto;
                                int status_auto = nadji_auto_sek(afile, p.reg_oznaka, &pronadjen_auto);
                                
                                if (status_auto == 1) {
                                    printf("Automobil pronadjen: %s %s\n", 
                                           pronadjen_auto.marka, pronadjen_auto.model);
                                    reg_uneta = 1;
                                } else {
                                    printf("\nAutomobil sa oznakom %d ne postoji!\n", p.reg_oznaka);
                                    printf("1 - Unesi novog automobila\n");
                                    printf("2 - Ponovi unos oznake\n");
                                    printf("0 - Odustani\n");
                                    printf("Izbor: ");
                                    
                                    int izbor;
                                    scanf("%d", &izbor);
                                    while(getchar() != '\n');
                                    
                                    if (izbor == 1) {
                                        SLOG_AUTO a;
                                        
                                        printf("\n=== UNOS NOVOG AUTOMOBILA ===\n");
                                        printf("Registarska oznaka: %d\n", p.reg_oznaka);
                                        a.reg_oznaka = p.reg_oznaka;
                                        
                                        printf("Marka: ");
                                        scanf(" %30[^\n]", a.marka);
                                        while(getchar() != '\n');
                                        
                                        printf("Model: ");
                                        scanf(" %30[^\n]", a.model);
                                        while(getchar() != '\n');
                                        
                                        printf("Godina proizvodnje: ");
                                        scanf(" %4[^\n]", a.god_proizvodnje);
                                        while(getchar() != '\n');
                                        
                                        printf("Boja: ");
                                        scanf(" %20[^\n]", a.boja);
                                        while(getchar() != '\n');
                                        
                                        int status = dodaj_slog_sek(afile, &a);
                                        if (status == 1) {
                                            printf("Automobil uspesno dodat.\n");
                                            reg_uneta = 1;
                                        } else {
                                            printf("Greska pri dodavanju automobila!\n");
                                            break;
                                        }
                                    } else if (izbor == 2) {
                                        continue;
                                    } else {
                                        printf("Unos parkiranja otkazan.\n");
                                        break;
                                    }
                                }
                            }
                            
                            if (!reg_uneta) break;
                            
                            printf("Datum (DD.MM.YYYY): ");
                            scanf(" %10[^\n]", p.datum);
                            while(getchar() != '\n');
                            
                            printf("Sati parkiranja: ");
                            scanf("%d", &p.sati);
                            while(getchar() != '\n');
                            
                            do {
                                printf("Zona (0-PLAVA, 1-BELA, 2-CRVENA): ");
                                scanf("%d", &zona_unos);
                                while(getchar() != '\n');
                                
                                if (zona_unos >= 0 && zona_unos <= 2) {
                                    p.zona = (Zona)zona_unos;
                                    break;
                                } else {
                                    printf("Nepostojeca zona!\n");
                                }
                            } while (1);
                            
                            int status = dodaj_slog_park_serijski(pfile, &p);
                            if (status == 1) {
                                printf("Parkiranje uspesno dodato!\n");
                            } else {
                                printf("Greska pri dodavanju parkiranja!\n");
                            }
                            break;
                        }
                        
                        case 6: {
                            if(pfile == NULL || afile == NULL) {
                                printf("Morate aktivirati obe datoteke!\n");
                                break;
                            }
                            
                            int id;
                            printf("Unesite ID parkiranja za izmenu: ");
                            scanf("%d", &id);
                            while(getchar() != '\n');
                            
                            int rez = izmeni_parking(pfile, afile, id);
                            if(rez == 1) {
                                printf("Parkiranje uspesno izmenjeno.\n");
                            } else if(rez == 0) {
                                printf("Parkiranje sa ID %d ne postoji.\n", id);
                            } else {
                                printf("Greska pri izmeni!\n");
                            }
                            break;
                        }
                        
                        case 7: {
                            if(pfile == NULL) {
                                printf("Prvo aktivirajte datoteku!\n");
                                break;
                            }
                            
                            int id;
                            printf("Unesite ID parkiranja za brisanje: ");
                            scanf("%d", &id);
                            while(getchar() != '\n');
                            
                            int rez = obrisi_parking_fizicki(pfile, id);
                            if(rez == 0) {
                                printf("Parkiranje uspesno obrisano.\n");
                            } else if(rez == 1) {
                                printf("Parkiranje sa ID %d ne postoji.\n", id);
                            } else {
                                printf("Greska pri brisanju!\n");
                            }
                            break;
                        }
                        
                        case 0:
                            printf("Povratak na glavni meni...\n");
                            break;
                            
                        default:
                            printf("Nepostojeca opcija!\n");
                    }
                } while(podOpcija != 0);
                break;
                
            case 3:
                do {
                    printf("\n--- IZVESTAJI ---\n");
                    printf("1. Automobili i parkiranja sa duzinom boravka iznad proseka\n");
                    printf("2. Naplata parkiranja po automobilu\n");
                    printf("0. Povratak na glavni meni\n");
                    printf("Izaberite opciju: ");
                    scanf("%d", &podOpcija);
                    while(getchar() != '\n');
                    
                    switch(podOpcija) {
                        case 1:
                            if(afile == NULL || pfile == NULL) {
                                printf("Morate aktivirati obe datoteke!\n");
                                break;
                            }
                            prikazi_auto_sa_parkiranjima_iznad_proseka(afile, pfile);
                            break;
                            
                        case 2:
                            if(afile == NULL || pfile == NULL) {
                                printf("Morate aktivirati obe datoteke!\n");
                                break;
                            }
                            prikazi_naplatu_po_automobilu(afile, pfile);
                            break;
                            
                        case 0:
                            printf("Povratak na glavni meni...\n");
                            break;
                            
                        default:
                            printf("Nepostojeca opcija!\n");
                    }
                } while(podOpcija != 0);
                break;
            case 4:
                do {
                    printf("\n--- MENI RAD SA AGREGIRANOM (RASUTOM) DATOTEKOM ---\n");
                    printf("1. Aktiviraj postojecu agregiranu datoteku\n");
                    printf("2. Formiraj novu agregiranu datoteku (iz automobila i parkinga)\n");
                    printf("3. Prikazi sadrzaj agregirane datoteke\n");
                    printf("4. Ispis datoteke\n");
                    printf("5. Dodavanje sloga\n");
                    printf("6. Fizicko brisanje sloga\n");
                    printf("7. Modifikacija sloga rasute datoteke\n");
                    printf("8. Formiraj datoteku promena\n");
                    printf("9. Dodaj slog u datoteku promena\n");
                    printf("10. Ispis datoteke promena\n");
                    printf("11. Fizicko brisanje sloga iz datoteke promena\n");
                    printf("12. Logicko brisanje sloga iz rasute datoteke\n");
                    printf("13. Formiranje log datoteke\n");
                    printf("14. Prikaz izvestaja.\n");
                    printf("15. Propagacija iz dela 1.\n");
                    printf("16. Modifikacija sloga datoteke promena\n");
                    printf("0. Povratak na glavni meni\n");
                    printf("Izaberite opciju: ");
                    scanf("%d", &podOpcija);
                    //while(getchar() != '\n');
                    
                    switch(podOpcija) {
                        case 1:
                            {
                                if(agregat_file != NULL) {
                                    fclose(agregat_file);
                                    agregat_file = NULL;
                                }
                                
                                printf("Unesite naziv agregirane datoteke: ");
                                scanf("%s", aktivna_dat);
                                //while(getchar() != '\n');
                                
                                agregat_file = postavi_aktivnu_datoteku(aktivna_dat);
                                if(agregat_file != NULL) {
                                    printf("Agregirana datoteka '%s' uspesno aktivirana.\n", aktivna_dat);
                                    E = 0;  // inicijalizuj E na prvi baket
                                } else {
                                    printf("Greska pri otvaranju agregirane datoteke!\n");
                                }
                                break;
                            }
                        case 2:
                            {
                                if(afile == NULL || pfile == NULL) {
                                    printf("Morate prvo aktivirati datoteke automobila i parkinga!\n");
                                    break;
                                }
                                
                                char naz_dat[256];
                                printf("Unesite naziv nove agregirane datoteke: ");
                                scanf("%s", naz_dat);
                                while(getchar() != '\n');
                                
                                // Formiraj datoteku
                                if(formiraj_agregiranu_datoteku(naz_dat, afile, pfile) == 0) {
                                    printf("Agregirana datoteka '%s' uspesno kreirana.\n", naz_dat);
                                    
                                    // Zatvori staru ako je bila otvorena
                                    if(agregat_file != NULL) {
                                        fclose(agregat_file);
                                        agregat_file = NULL;
                                    }
                                    
                                    // Aktiviraj novu datoteku
                                    agregat_file = postavi_aktivnu_datoteku(naz_dat);
                                    if(agregat_file != NULL) {
                                        E = 0;  // inicijalizuj E
                                        printf("Agregirana datoteka '%s' je sada aktivna.\n", naz_dat);
                                    }
                                } else {
                                    printf("Greska pri kreiranju agregirane datoteke!\n");
                                }
                                break;
                            }
                        case 3:
                            printf("Opcija 3: Prikaz sadržaja agregirane datoteke\n");
                            printf("(jos nije implementirano)\n");
                            break;
                            
                        case 4:
                            {
                                if(agregat_file == NULL) {
                                    printf("Prvo aktivirajte agregiranu datoteku (opcija 1)!\n");
                                    break;
                                }
                                
                                ispisi_agregiranu_datoteku(agregat_file);
                                break;
                            }   
                        case 5:
                        {
                              if(agregat_file == NULL) {
                                printf("Prvo aktivirajte agregiranu datoteku (opcija 1)!\n");
                                break;
                             }
    
                            printf("\n--- UNOS NOVOG SLOGA U AGREGIRANU DATOTEKU ---\n");
                            
                            SLOG_AGREGAT novi;
                            
                            printf("Registarska oznaka: ");
                            scanf("%d", &novi.reg_oznaka);
                            ocisti_buffer();
                            
                            printf("Marka: ");
                            scanf("%s", novi.marka);
                            ocisti_buffer();
                            
                            printf("Model: ");
                            scanf("%s", novi.model);
                            ocisti_buffer();
                            
                            printf("Godina proizvodnje (4 cifre): ");
                            scanf("%s", novi.godina_proizvodnje);
                            ocisti_buffer();
                            
                            printf("Boja: ");
                            scanf("%s", novi.boja);
                            ocisti_buffer();
                            
                            printf("Ukupna duzina boravka u BELOJ zoni (sati): ");
                            scanf("%d", &novi.duz_bela);
                            ocisti_buffer();
                            
                            printf("Ukupna duzina boravka u CRVENOJ zoni (sati): ");
                            scanf("%d", &novi.duz_crvena);
                            ocisti_buffer();
                            
                            printf("Ukupna duzina boravka u PLAVOJ zoni (sati): ");
                            scanf("%d", &novi.duz_plava);
                            ocisti_buffer();
                            
                            novi.status = ' ';  // aktivan
                            
                            // Izračunaj matični baket
                            int maticni = hash_preklapanje(novi.reg_oznaka);
                            printf("Auto treba u baket %d\n", maticni);
                            
                            // Pokušaj da upišeš
                            int prvi_slobodan = -1; // Za sada ne koristimo
                            int rezultat = dodaj_slog_u_rasutu(agregat_file, &novi, &E);
                            
                            if(rezultat == 0) {
                                printf("Automobil uspesno dodat u agregiranu datoteku!Novi E=%d\n",E);
                            } else {
                                printf("Greska pri dodavanju automobila!\n");
                            }
                            break;
                        }   
                        case 6:
                        {
                            if(agregat_file == NULL) {
                                printf("Prvo aktivirajte agregiranu datoteku!\n");
                                break;
                            }
                            
                            int reg;
                            printf("Unesite registarsku oznaku za brisanje: ");
                            scanf("%d", &reg);
                            ocisti_buffer();
                            
                            printf("Da li ste sigurni? (1-DA, 0-NE): ");
                            int potvrda;
                            scanf("%d", &potvrda);
                            ocisti_buffer();
                            
                            if (potvrda == 1) {
                                int rez = fizicki_obrisi_iz_rasute(agregat_file, reg, &E);
                                if (rez == 0) {
                                    printf("Auto uspesno obrisan!\n");
                                } else {
                                    printf(" Greska pri brisanju!\n");
                                }
                            } else {
                                printf("Brisanje otkazano.\n");
                            }
                            break;
                        }
                        case 7:
                        {
                            if(agregat_file == NULL) {
                                    printf("Prvo aktivirajte agregiranu datoteku!\n");
                                    break;
                                }
                                
                                printf("\n--- AZURIRANJE SLOGA U RASUTOJ DATOTECI ---\n");
                                
                                int kljuc;
                                printf("Unesite registarsku oznaku automobila za izmenu: ");
                                scanf("%d", &kljuc);
                                while(getchar() != '\n');
                                
                                int rezultat = azuriraj_agregiranu(agregat_file, kljuc);
                                
                                if(rezultat == 1) {
                                    printf("Izmena uspesno izvrsena.\n");
                                } else if(rezultat == 0) {
                                    printf(" Automobil nije pronadjen.\n");
                                } else {
                                    printf("Greska pri izmeni.\n");
                                }
                                break;
                        }                           
                        case 8:
                        {
                            if(agregat_file == NULL) {
                                printf("Prvo aktivirajte agregiranu datoteku!\n");
                                break;
                            }
                            printf("Unesite naziv datoteke promena: ");
                            scanf("%s", aktivna_dat);
                            
                            int status_promena = formiraj_datoteku_promena(aktivna_dat);
                            if(status_promena == 0) {
                                printf("Datoteka promena '%s' uspesno formirana.\n", aktivna_dat);
                                if(fpromena != NULL) fclose(fpromena);
                                fpromena = fopen(aktivna_dat, "rb+");
                                if(fpromena != NULL) {
                                    printf("Datoteka promena je sada aktivna.\n");
                                }
                            } else {
                                printf("Greska pri formiranju datoteke promena!\n");
                            }
                            break;
                        }    
                        case 9:
                            {
                                if(fpromena == NULL) {
                                    printf("Prvo aktivirajte datoteku promena!\n");
                                    break;
                                }
                                
                                SLOG_PROMENA slog_promena;  // <-- OVO JE KLJUČNO!
                                
                                printf("Unesite kljuc automobila: ");
                                scanf("%d", &slog_promena.kljuc);
                                ocisti_buffer();
                                
                                printf("Unesite marku: ");
                                scanf("%s", slog_promena.slog.marka);
                                
                                printf("Unesite model: ");
                                scanf("%s", slog_promena.slog.model);
                                
                                printf("Unesite godinu proizvodnje: ");
                                scanf("%s", slog_promena.slog.godina_proizvodnje);
                                
                                printf("Unesite boju: ");
                                scanf("%s", slog_promena.slog.boja);
                                
                                printf("Unesite duzinu u beloj zoni: ");
                                scanf("%d", &slog_promena.slog.duz_bela);
                                
                                printf("Unesite duzinu u crvenoj zoni: ");
                                scanf("%d", &slog_promena.slog.duz_crvena);
                                
                                printf("Unesite duzinu u plavoj zoni: ");
                                scanf("%d", &slog_promena.slog.duz_plava);
                                
                                slog_promena.slog.reg_oznaka = slog_promena.kljuc;
                                slog_promena.slog.status = ' ';
                                
                                printf("Unesite operaciju (n-novi, m-modifikacija, b-brisanje): ");
                                scanf(" %c", &slog_promena.operacija);
                                ocisti_buffer();
                                
                                // Prosleđuješ CELU SLOG_PROMENA strukturu
                                dodaj_u_datoteku_promena(fpromena, &slog_promena);
                                
                                break;
                            }
                        case 10:
                        {
                            if(fpromena == NULL) {
                                printf("Prvo aktivirajte datoteku promena!\n");
                                break;
                            }
                            ispisi_datoteku_promena(fpromena);
                            break;
                        }
                        case 11:
                            {
                                if (fpromena == NULL) {
                                    printf("Prvo aktivirajte datoteku promena!\n");
                                    break;
                                }
                                
                                int kljuc;
                                printf("Unesite kljuc auta za brisanje iz datoteke promena: ");
                                scanf("%d", &kljuc);
                                ocisti_buffer();
                                
                                // POZOVI FUNKCIJU DIREKTNO – NE ZATVARAJ FAJL!
                                int rez = fizicki_obrisi_iz_promena(fpromena, kljuc);
                                
                                if (rez == 0) {
                                    printf("Slog sa kljucem %d uspesno obrisan.\n", kljuc);
                                } else if (rez == 2) {
                                    printf("Slog sa kljucem %d nije pronadjen.\n", kljuc);
                                } else {
                                    printf("Greska pri brisanju.\n");
                                }
                                break;
                            }
                        case 12: 
                                {
                                    if(agregat_file == NULL) {
                                        printf("Prvo aktivirajte agregiranu datoteku (opcija 1)!\n");
                                        break;
                                    }
                                    
                                    printf("\n--- LOGICKO BRISANJE IZ RASUTE DATOTEKE ---\n");
                                    
                                    int kljuc;
                                    printf("Unesite registarsku oznaku automobila za logicko brisanje: ");
                                    scanf("%d", &kljuc);
                                    ocisti_buffer();
                                    
                                    // Poziv funkcije za logičko brisanje
                                    int rezultat = logicki_obrisi_iz_rasute(agregat_file, kljuc);
                                    
                                    if(rezultat == 0) {
                                        printf("Automobil %d je uspešno logicki obrisan.\n", kljuc);
                                    } else {
                                        printf("Greska pri logickom brisanju automobila %d.\n", kljuc);
                                    }
                                    break;
                                }
                            case 13: 
                                 {
                                    char naziv[256];
                                     printf("Unesite naziv log datoteke: ");
                                     scanf("%s", naziv);
                                        
                                    int status = formiraj_log_datoteku(naziv);
                                    if(status == 0) {
                                        printf("Log datoteka '%s' uspesno formirana.\n", naziv);
                                            
                                         // Zatvori staru ako je bila otvorena
                                        if(flog != NULL) {
                                             fclose(flog);
                                             flog = NULL;
                                        }
                                            
                                            // Otvori novu
                                        flog = fopen(naziv, "rb+");
                                        if(flog != NULL) {
                                             printf("Log datoteka je sada aktivna.\n");
                                        }
                                        } else {
                                            printf("Greska pri formiranju log datoteke.\n");
                                    }
                                    break;
                                }
                        case 14: // Prikaz izveštaja iz log datoteke
                                {
                                    if(flog == NULL) {
                                        printf("Prvo aktivirajte log datoteku (opcija 13)!\n");
                                        break;
                                    }
                                    
                                    prikazi_log_izvestaj(flog);
                                    break;
                                }
                        case 15: // Propagacija iz dela 1
                            {
                                if(agregat_file == NULL || afile == NULL || pfile == NULL) {
                                    printf("Morate aktivirati sve tri datoteke (auto, parking, agregat)!\n");
                                    break;
                                }
                                
                                if(flog == NULL) {
                                    printf("Prvo aktivirajte log datoteku (opcija 13)!\n");
                                    break;
                                }
                                
                                propagiraj_iz_automobila(agregat_file, afile, pfile, flog);
                                break;
                            }
                        case 16: // Modifikacija sloga u datoteci promena
                            {
                                if(fpromena == NULL) {
                                    printf("Prvo aktivirajte datoteku promena (opcija 8)!\n");
                                    break;
                                }
                                
                                int kljuc;
                                printf("Unesite kljuc sloga za izmenu: ");
                                scanf("%d", &kljuc);
                                
                                SLOG_PROMENA novi;
                                novi.kljuc = kljuc;
                                
                                printf("Unesite novu operaciju (n/m/b): ");
                                scanf(" %c", &novi.operacija);
                                
                                printf("Unesite marku: ");
                                scanf("%s", novi.slog.marka);
                                
                                printf("Unesite model: ");
                                scanf("%s", novi.slog.model);
                                
                                printf("Unesite godinu proizvodnje: ");
                                scanf("%s", novi.slog.godina_proizvodnje);
                                
                                printf("Unesite boju: ");
                                scanf("%s", novi.slog.boja);
                                
                                printf("Unesite duzinu u beloj zoni: ");
                                scanf("%d", &novi.slog.duz_bela);
                                
                                printf("Unesite duzinu u crvenoj zoni: ");
                                scanf("%d", &novi.slog.duz_crvena);
                                
                                printf("Unesite duzinu u plavoj zoni: ");
                                scanf("%d", &novi.slog.duz_plava);
                                
                                novi.slog.reg_oznaka = kljuc;
                                novi.slog.status = ' ';
                                
                                int rez = izmeni_u_promenama(fpromena, kljuc, &novi);
                                
                                if(rez == 0) {
                                    printf("Slog uspesno izmenjen u datoteci promena.\n");
                                } else {
                                    printf("Greska pri izmeni sloga.\n");
                                }
                                break;
                            }
                            case 17: // DIREKTNA OBRADA
                                {
                                    if(agregat_file == NULL || fpromena == NULL) {
                                        printf("Prvo aktivirajte agregiranu datoteku i datoteku promena!\n");
                                        break;
                                    }
                                    
                                    // Otvori log ako nije
                                    if(flog == NULL) {
                                        char naziv[256];
                                        printf("Unesite naziv log datoteke: ");
                                        scanf("%s", naziv);
                                        flog = fopen(naziv, "rb+");
                                        if(flog == NULL) {
                                            formiraj_log_datoteku(naziv);
                                            flog = fopen(naziv, "rb+");
                                        }
                                    }
                                    
                                    int rez = direktna_obrada(agregat_file, fpromena, flog, &E);
                                    
                                    if(rez == 0) {
                                        printf("Direktna obrada uspesno zavrsena.\n");
                                    } else {
                                        printf("Greska pri direktnoj obradi.\n");
                                    }
                                    break;
                                }
                        case 0:
                            printf("Povratak na glavni meni...\n");
                            break;
                            
                        default:
                            printf("Nepostojeca opcija!\n");
                    }
                } while(podOpcija != 0);
                break;
                
            case 0:
                printf("Izlaz iz programa.\n");
                if(afile != NULL) fclose(afile);
                if(pfile != NULL) fclose(pfile);
                break;
                
            default:
                printf("Nepostojeca opcija!\n");
        }
        
    } while(glavnaOpcija != 0);
    
    return 0;
}