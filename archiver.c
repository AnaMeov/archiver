// Voicu Ana-Nicoleta 312CA
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
struct tm tm;
time_t secunde;
// functie pentru padding cu 0 la stanga
void padleft(char sir[], int lungime) {
  int i, k;
  char temp[64];
  strcpy(temp, sir);
  for (i = 0; i < lungime; i++)
    sir[i] = '0';
  sir[lungime - 1] = '\0';
//  k serveste drept cursor pentru decorarea sirului in stanga luii \0
  k = lungime - 2;
  for (i = strlen(temp) - 1; i >= 0; i--) {
    sir[k] = temp[i];
    k--;
  }
}
// functie de transformare din octal in decimal
int octaltodecimal(int multiplu) {
  int i = 0, aux = 0, putere = 1;
  while (multiplu != 0) {
    aux = aux + (multiplu % 10) * putere;
    putere = putere * 8;
    multiplu = multiplu / 10;
  }
  multiplu = aux;
  return multiplu;
}
// funtie pentru valorile permisiunilor
int get_chmod_val(char c) {
  if (c == 'x')
    return 1;
  if (c == 'w')
    return 2;
  if (c == 'r')
    return 4;
  return 0;
}
// functie pentru calcularea permisiunilor
int get_chmod_digit(char *aux, int poz) {
  int nr = 0;
  nr = nr + get_chmod_val(aux[poz]);
  nr = nr + get_chmod_val(aux[poz + 1]);
  nr = nr + get_chmod_val(aux[poz + 2]);
  return nr;
}
union record {
  char charptr[512];
  struct header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[8];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
  } header;
} record;

int get_header_files(FILE *files) {
  char aux[512], aux2[512];
  int contor = 0, multiplu = 0;
  while (contor <= 8) {
    contor++;
    fscanf(files, "%s", aux);
    if (feof(files))
      break;
    switch (contor) {
    case 1: {
      // citim si calculam permisiunile
      char mode[6] = {0};
      mode[0] = '0';
      mode[1] = '0';
      mode[2] = '0';
      mode[3] = '0';
      int aux_mode = 0;
      int i;
      for (i = 0; i < 3; i++)
        aux_mode = aux_mode * 10 + get_chmod_digit(aux, 3 * i + 1);
      sprintf(mode + 4, "%d", aux_mode);
      strcpy(record.header.mode, mode);
      record.header.mode[strlen(record.header.mode)] = '\0';
      break;
    }
    case 2: {
      // citim linkurile
      break;
    }
    case 3: {
      // citim numele user-ului
      strcpy(record.header.uname, aux);
      record.header.uname[strlen(record.header.uname)] = '\0';
      break;
    }
    case 4: {
      // citim numele grupului
      strcpy(record.header.gname, aux);
      record.header.gname[strlen(record.header.gname)] = '\0';
      break;
    }
    case 5: {
      // citim dimensiunea fisierului
      multiplu = atoi(aux);
      sprintf(record.header.size, "%o", atoi(aux));
      padleft(record.header.size, 12);
      break;
    }
    case 6: {
      // salvam intr-un aux2 bucata curenta
      strcpy(aux2, aux);
      break;
    }
    case 7: {
      // concatenam bucata anterioara cu cea curenta
      strcat(aux2, " "); // adaugam spatiu intre cele 2 bucati
      strcat(aux2, aux);
      // calculam secundele de la unix time
      strptime(aux2, "%Y-%m-%d %H:%M:%S", &tm);
      secunde = mktime(&tm);
      // transformam din double in array
      sprintf(record.header.mtime, "%lo", secunde);
      break;
    }
    case 8: {
      // ignoram timezone-ul
      break;
    }
    case 9: {
      strcpy(record.header.name, aux);
      record.header.name[strlen(record.header.name)] = '\0';
      strcpy(record.header.linkname, aux);
      record.header.linkname[strlen(record.header.linkname)] = '\0';
      break;
    }
    }
  }
  return multiplu;
}

void get_header_usermap(FILE *usermap) {
  char aux_usermap[512];
  int j, ok = 0;
  fseek(usermap, 0, SEEK_SET);
  while (ok == 0) {
    fscanf(usermap, "%s", aux_usermap);
    char *sir;
    sir = strtok(aux_usermap, ": ");
    if (strcmp(sir, record.header.uname) == 0) {
      ok = 1;
      for (j = 0; j < 4; j++) {
        // luam al 3-lea si al 4-lea element
        if (j == 2) {
          sprintf(record.header.uid, "%o", atoi(sir));
          padleft(record.header.uid, 8);
        }
        if (j == 3) {
          sprintf(record.header.gid, "%o", atoi(sir));
          padleft(record.header.gid, 8);
        }
        sir = strtok(NULL, ": ");
      }
    }
  }
}

void create() {
  char nume_arhiva[512], nume_director[512];
  char path_fisier[512], vector_multiplu[512];
  int multiplu, l, j, i;
  // citim numele fisierului si a directorului
  scanf("%s", nume_arhiva);
  scanf("%s", nume_director);
  // deschidem arhiva
  FILE *arhiva = fopen(nume_arhiva, "wb");
  if (arhiva == NULL) {
    printf("Eroare deschidere din fisierul arhiva");
    exit(0);
  }
  // deschidem fisierul files.txt
  FILE *files = fopen("files.txt", "rt");
  if (files == NULL) {
    printf("Eroare citire din fisierul files");
    exit(0);
  }
  // deschidem fisierul usermap.txt
  FILE *usermap = fopen("usermap.txt", "rt");
  if (usermap == NULL) {
    printf("Eroare citire din fisierul usermap");
    exit(0);
  }
  // citim elementele din files.txt si le prelucram
  while (!feof(files)) {
    // atribuim lui multiplu valoarea returnata de functie,
    // ca apoi sa o folosim la determinarea marimii arhivei
    multiplu = get_header_files(files);
    if (feof(files))
      break;
    // devmajor/devminor trebuie sa fie 0
    memset(record.header.devminor, 0, 8);
    memset(record.header.devmajor, 0, 8);
    get_header_usermap(usermap);
    // umplem magic cu un string
    strcpy(record.header.magic, "GNUtar ");
    // punem spatii in suma
    strcpy(record.header.chksum, "        ");
    // typeflag trebuie sa fie 0
    record.header.typeflag = '\0';
    // calculam suma
    long suma = 0;
    for (l = 0; l < 512; l++) {
      suma = suma + record.charptr[l];
    }
    sprintf(record.header.chksum, "%lo", suma);
    padleft(record.header.chksum, 7);
    record.header.chksum[strlen(record.header.chksum)] = '\0';
    // punem cursorul la sfarsitul arhivei
    fseek(arhiva, 0, SEEK_END);
    // punem headerul in arhiva
    fwrite(record.charptr, sizeof(record), 1, arhiva);
    // deschidem fisierul sa citim din el
    strcpy(path_fisier, nume_director);
    strcat(path_fisier, record.header.name);
    int marime_fisier = multiplu;
    // facem multiplu de 512 in functie de size
    if (multiplu % 512 != 0 && marime_fisier > 0)
      multiplu = multiplu + 512 - multiplu % 512;
    // umplu un vector cu 0 pentru sfarsit
    for (i = 0; i < 512; i++) {
      vector_multiplu[i] = 0;
    }
    multiplu = multiplu / 512;
    for (i = 0; i < multiplu; i++)
      fwrite(vector_multiplu, sizeof(vector_multiplu), 1, arhiva);
    // muta cursorul la pozitia dinainte sa puna 0-uri
    fseek(arhiva, -multiplu * 512, SEEK_CUR);
    // deschidem fisierul din care copiem
    FILE *f = fopen(path_fisier, "rb");
    if (f == NULL) {
      printf("Eroare citire din fisieul care trebuie arhivat");
      exit(0);
    }
    // copiem continutul fisierului in arhiva
    char c;
    for (i = 0; i < marime_fisier; i++) {
      fread(&c, sizeof(char), 1, f);
      fwrite(&c, sizeof(char), 1, arhiva);
    }
    fclose(f);
    memset(record.charptr, 0, 512);
  }
  // punem la sfarsitul arhivei 512 zero-uri si inchidem fisierele folosite
  fseek(arhiva, 0, SEEK_END);
  fwrite(vector_multiplu, sizeof(vector_multiplu), 1, arhiva);
  fclose(files);
  fclose(usermap);
  fclose(arhiva);
  printf("> Done!\n");
}

void list() {
  char nume_arhiva[512];
  int multiplu;
  // citim numele arhivei
  scanf("%s", nume_arhiva);
  // deschidem arhiva
  FILE *arhiva = fopen(nume_arhiva, "rb");
  if (arhiva == NULL) {
    printf("> File not found!");
  }
  while (!feof(arhiva)) {
    // citim header, afisam nume
    fread(record.charptr, sizeof(record.charptr), 1, arhiva);
    if (feof(arhiva))
      break;
    if (strlen(record.header.name) > 0)
      printf("> %s\n", record.header.name);
    // atribuim lui "multiplu" marimea fisierului, pt a sari peste el
    multiplu = atoi(record.header.size);
    // transformam marimea in zecimal
    multiplu = octaltodecimal(multiplu);
    // transformam marimea in urmatorul multiplu al lui 512
    if (multiplu > 0 && multiplu % 512 != 0)
      multiplu = multiplu + 512 - multiplu % 512;
    // sarim peste fisierul in sine
    fseek(arhiva, multiplu, SEEK_CUR);
    if (feof(arhiva))
      break;
  }
  fclose(arhiva);
}
void extract() {
  char nume_arhiva[512];
  char nume_fisier[512];
  char nume_extracted[512];
  int multiplu, ok = 0;
  // citim numele arhivei
  scanf("%s", nume_fisier);
  scanf("%s", nume_arhiva);
  strcpy(nume_extracted, "extracted_");
  strcat(nume_extracted, nume_fisier);
  // deschidem arhiva
  FILE *arhiva = fopen(nume_arhiva, "rb");
  if (arhiva == NULL) {
    printf("Eroare deschidere din fisierul arhiva");
    exit(0);
  }
  while (!feof(arhiva)) {
    // citim header
    fread(record.charptr, sizeof(record.charptr), 1, arhiva);
    if (feof(arhiva))
      break;
    if (strcmp(record.header.name, nume_fisier) == 0) {
      // am gasit fisierul
      ok = 1;
      // cream un fisier in care vom extrage
      FILE *fisier = fopen(nume_extracted, "wb");
      if (fisier == NULL) {
        printf("Eroare deschidere din fisierul arhiva");
        exit(0);
      }
      // atribuim lui "multiplu" marimea fisierului
      multiplu = atoi(record.header.size);
      // transformam marimea in zecimal
      multiplu = octaltodecimal(multiplu);

      // copiam "multiplu" caractere in fisierul nou-creat
      char c;
      int i;
      for (i = 0; i < multiplu; i++) {
        fread(&c, sizeof(char), 1, arhiva);
        fwrite(&c, sizeof(char), 1, fisier);
      }
    } else {
      // atribuim lui "multiplu" marimea fisierului, pt a sari peste el
      multiplu = atoi(record.header.size);
      // transformam marimea in zecimal
      multiplu = octaltodecimal(multiplu);
      // transformam marimea in urmatorul multiplu al lui 512
      if (multiplu > 0 && multiplu % 512 != 0)
        multiplu = multiplu + 512 - multiplu % 512;

      // sarim peste fisierul in sine
      fseek(arhiva, multiplu, SEEK_CUR);
      if (feof(arhiva))
        break;
    }
  }
  if (ok == 1)
    printf("> File extracted!\n");
  else
    printf("> File not found!\n");
}
int main() {
  char comanda[512] = {0};
  while (strcmp(comanda, "exit") != 0) {
    scanf("%s", comanda);
    if (strcmp(comanda, "create") == 0)
      create();
    else if (strcmp(comanda, "list") == 0)
      list();
    else if (strcmp(comanda, "extract") == 0)
      extract();
    else if (strcmp(comanda, "exit") != 0)
      printf("> Wrong command!\n");
  }
  return 0;
}
