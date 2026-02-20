#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "operacije.h"

FILE* postavi_aktivnu_datoteku(char* naz_dat){
    FILE* file=fopen(naz_dat,"rb+");
    printf("Datoteka %s AKTIVIRANA",naz_dat);
    return file;
}

void napravi_datoteku_auto(char* naz_dat){
    BLOK_AUTO b;
    FILE* f=fopen(naz_dat,"wb");
    b.automobili[0].reg_oznaka=OZNAKA_KRAJA_DATOTEKE;
    fwrite(&b,sizeof(BLOK_AUTO),1,f);
    fclose(f);
}

void napravi_datoteku_parking(char* naz_dat){
    BLOK_PARKING b;
    FILE* f=fopen(naz_dat,"wb");
    b.automobili[0].id=OZNAKA_KRAJA_DATOTEKE;
    fwrite(&b,sizeof(BLOK_PARKING),1,f);
    fclose(f);
}