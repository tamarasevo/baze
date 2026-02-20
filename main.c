#include <stdio.h>
#include <stdlib.h>
#include "auto.h"
#include "parkiranje.h"
#include "operacije.h"

int main(){

int glavnaOpcija;
int podOpcija;
FILE* file=NULL;

char aktivna_dat[16];
    
    do {
    // Glavni meni
        printf("\n=== GLAVNI MENI ===\n");
        printf("1. Rad sa datotekom automobila\n");
        printf("2. Rad sa datotekom parkiranja\n");
        printf("0. Izlaz iz programa\n");
        printf("Izaberite opciju: ");
        scanf("%d", &glavnaOpcija);
        
        switch(glavnaOpcija) {
            case 1: // Meni za automobile
                do {
                    printf("\n--- MENI ZA AUTOMOBILE ---\n");
                    printf("1. Aktiviraj datoteku\n");
                    printf("2. Napravi datoteku\n");
                    printf("3. Pretraga automobila\n");
                    printf("4. Izmena podataka o automobilu\n");
                    printf("5. Brisanje automobila\n");
                    printf("0. Povratak na glavni meni\n");
                    printf("Izaberite opciju: ");
                    scanf("%d", &podOpcija);
                    
                    switch(podOpcija) {
                        case 1:{
                            if(file!=NULL){
                                printf("Prvo zatvorite aktivnu datoteku!\n");
                                 break;
                            }

                            printf("Unesite naziv aktivne datoteke:\n");
                            scanf("%s",aktivna_dat);   
                            file=postavi_aktivnu_datoteku(aktivna_dat);
                            if(file!=NULL){
                                printf("Aktivna datoteka uspesno otvorena.\n");
                            }else{
                                printf("Doslo je do greske\n");
                                exit(1);
                            }
                            break;
                        }
                        case 2:{
                                char naz_dat[16];
                                printf("Unesite zeljeni naziv datoteke:\n");
                                scanf("%s",naz_dat);
                                napravi_datoteku_auto(naz_dat);
                                printf("Datoteka %s uspesno kreirana\n",naz_dat);
                                 file=postavi_aktivnu_datoteku(naz_dat);
                             break;
                        }
                        case 3:
                            printf("-> Pretraga automobila\n");
                            // Ovde ide kod za pretragu automobila
                            break;
                        case 4:
                            printf("-> Izmena podataka o automobilu\n");
                            // Ovde ide kod za izmenu automobila
                            break;
                        case 5:
                            printf("-> Brisanje automobila\n");
                            // Ovde ide kod za brisanje automobila
                            break;
                        case 0:
                            printf("Povratak na glavni meni...\n");
                            break;
                        default:
                            printf("Nepostojeca opcija! Pokusajte ponovo.\n");
                    }
                } while(podOpcija != 0);
                break;
                
            case 2: // Meni za parkiranja
                do {
                    printf("\n--- MENI ZA PARKIRANJA ---\n");
                    printf("1. Aktiviraj datoteku\n");
                    printf("2. Napravi datoteku\n");
                    printf("3. Pretraga parkiranja\n");
                    printf("4. Izmena podataka o parkiranju\n");
                    printf("5. Brisanje parkiranja\n");
                    printf("0. Povratak na glavni meni\n");
                    printf("Izaberite opciju: ");
                    scanf("%d", &podOpcija);
                    
                    switch(podOpcija) {
                        case 1:{
                            if(file!=NULL){
                                printf("Prvo zatvorite aktivnu datoteku!\n");
                                 break;
                            }

                            printf("Unesite naziv aktivne datoteke:\n");
                            scanf("%s",aktivna_dat);   
                            file=postavi_aktivnu_datoteku(aktivna_dat);
                            if(file!=NULL){
                                printf("Aktivna datoteka uspesno otvorena.\n");
                            }else{
                                printf("Doslo je do greske\n");
                                exit(1);
                            }
                            break;
                        }
                        case 2:{
                                char naz_dat[16];
                                printf("Unesite zeljeni naziv datoteke:\n");
                                scanf("%s",naz_dat);
                                napravi_datoteku_parking(naz_dat);
                                printf("Datoteka %s uspesno kreirana\n",naz_dat);
                                 file=postavi_aktivnu_datoteku(naz_dat);
                             break;
                        }
                        case 3:
                            printf("-> Pretraga parkiranja\n");
                            // Ovde ide kod za pretragu parkiranja
                            break;
                        case 4:
                            printf("-> Izmena podataka o parkiranju\n");
                            // Ovde ide kod za izmenu parkiranja
                            break;
                        case 5:
                            printf("-> Brisanje parkiranja\n");
                            // Ovde ide kod za brisanje parkiranja
                            break;
                        case 0:
                            printf("Povratak na glavni meni...\n");
                            break;
                        default:
                            printf("Nepostojeca opcija! Pokusajte ponovo.\n");
                    }
                } while(podOpcija != 0);
                break;
                
            case 0:
                printf("Izlaz iz programa.\n");
                break;
                
            default:
                printf("Nepostojeca opcija! Pokusajte ponovo.\n");
        }
        
    } while(glavnaOpcija != 0);

};