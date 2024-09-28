#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <unistd.h>


/**************************  DEFINIZIONE STRUCT  ****************************************************************************************************/

/** Struct contenente il terreno di gioco: array bidimensionale per la mappa, larghezza e altezza di questa */
struct terrain_s {
    char **labyrinth;
    unsigned L;
    unsigned H;
};

/** Struct contenente le variabili di orientamento dello snake */
struct orientation_s {
    char direction;
    char facing;
};

/** Struct contenente le coordinate del personaggio */
struct coordinates_s {
    int heroPosY;
    int heroPosX;
    int oldPosY;
    int oldPosX;
};

/** Struct contenente le coordinate di ingresso e di uscita del labirinto */
struct doors_s {
    unsigned entranceY;
    unsigned entranceX;
    unsigned exitY;
    unsigned exitX;
};

/** Struct contenente tutti i contatori */
struct counters_s {
    long score;
    unsigned steps;
    unsigned bonusCounter;
    unsigned malusCounter;
    unsigned drillCounter;
    bool disableDrill;
    bool moved;
};

/** Struct che definisce una lista concatenata in cui ogni nodo contiene le coordinate del frammento di coda corrispondente */
struct list {
    int tailPosY;
    int tailPosX;
    struct list *next;
};

/** Struct che definisce la coda dello snake */
struct tail_s {
    struct list *head;
    bool hasTail;
    unsigned size;
};



/**************************  PROTOTIPI FUNZIONI  ****************************************************************************************************/

/** Questa funzione permette di scegliere che mappa utilizzare (da file / inserita manualmente)*/
int chooseMap();

/** Questa funzione permette di scegliere quale modalità giocare (Manuale / AI) */
int menu();

/** Questa funzione calcola l'attuale punteggio */
long points(unsigned bonusCounter, unsigned malusCounter, unsigned steps);

/** Questa funzione stampa a schermo il conto degli oggetti posseduti e il punteggio */
void gameInfo(struct counters_s *counters, int mode);

/** Questa funzione permette il movimento di Snake mediante la pressione dei tasti A, W, S, D */
char awsdMovement();


/** Questa funzione trova le coordinate di ingresso al labirinto */
void findEntrance(struct terrain_s terrain, struct doors_s *doors, struct orientation_s *orientation);

/** Questa funzione trova le coordinate di uscita dal labirinto */
void findExit(struct terrain_s terrain, struct doors_s *doors);

/** Questa funzione stampa a schermo il terreno di gioco */
void printLabyrinth(struct terrain_s *terrain, struct coordinates_s coordinates, struct tail_s tail);


/** Questa funzione permette il movimento di Snake */
void moveHero(struct terrain_s terrain, struct coordinates_s *coordinates, struct orientation_s orientation, struct counters_s *counters);

/** Questa funzione restituisce true se la casella desiderata va oltre il bordo del terreno di gioco, restituisce false altrimenti */
bool isBorder(struct terrain_s terrain, struct orientation_s orientation, struct coordinates_s *coordinates);

/** Questa funzione restituisce true se la casella desiderata è un muro, restituisce false altrimenti */
bool isWall(struct terrain_s terrain, struct orientation_s orientation, struct coordinates_s *coordinates);

/** Questa funzione controlla che cosa si trova nella casella desiderata, e modifica di conseguenza i valori dei contatori */
void checkStep(struct terrain_s *terrain, struct coordinates_s *coordinates, struct tail_s *tail, struct counters_s *counters);


/** Questa funzione crea il primo nodo della coda di Snake (head) */
void tailFirstNode(struct tail_s *tail, int y, int x);

/** Questa funzione aggiunge un nodo alla coda di Snake (il nodo viene aggiunto davanti alla head, e diventa la nuova head) */
void tailNewHead(struct tail_s *tail, int y, int x);

/** Questa funzione permette il movimento della coda con il corpo */
void tailFollow(struct tail_s *tail, struct coordinates_s *coordinates);

/** Questa funzione taglia la coda quando il corpo ci passa sopra, o quando viene raccolto un imprevisto */
void cutTail(struct tail_s *tail, int y, int x, struct terrain_s *terrain);


/** Intelligenza Artificiale "sempre a destra": avanza mantenendo la mano destra sempre appoggiata al muro */
void AI_alwaysRight(struct terrain_s terrain, struct orientation_s *orientation, struct coordinates_s coordinates, unsigned drillCounter);

/** Intelligenza Artificiale "direzioni random" */
void AI_crazy(char *direction);

/** Intelligenza Artificiale "direzioni random" + cambio di colore ad ogni passo */
void AI_reallyCrazy(char *direction);



/**********************************  MAIN  ****************************************************************************************************/

int main()
{
 // CREAZIONE VARIABILI
    struct terrain_s terrain;
    struct doors_s doors;
    struct orientation_s orientation;
    struct coordinates_s coordinates;
    struct counters_s counters;
    struct tail_s tail;

    srand(time(NULL));

 // SCELTA DELLA MAPPA DI GIOCO
    int mapChoice;
    do {
        mapChoice = chooseMap();
    } while (mapChoice < 1 || mapChoice > 5);

    if (mapChoice >= 1  &&  mapChoice <= 4)   // mappe salvate in file di testo
    {
        FILE *map;

        if (mapChoice == 1) map = fopen("maps/map1.txt", "r");
        else if (mapChoice == 2) map = fopen("maps/map2.txt", "r");
        else if (mapChoice == 3) map = fopen("maps/map3.txt", "r");
        else if (mapChoice == 4) map = fopen("maps/map4.txt", "r");

        if (map == NULL)
        {
            printf("Il file map.txt non e' stato trovato\n");
            printf("\nPremi un tasto qualsiasi per uscire . . . ");
            getch();
            return 0;
        }

        fscanf(map, "%d", &terrain.L);  // inserimento larghezza e altezza
        fscanf(map, "%d", &terrain.H);

        terrain.labyrinth = (char**) malloc(terrain.H * terrain.L);  // creazione terreno di gioco
        for (int y=0; y<terrain.H; y++)
            terrain.labyrinth[y] = (char*) malloc(terrain.L);

        for (int y=0; y<terrain.H; y++)                     // inserimento caselle nel labirinto
            fscanf(map, " %[^\n]s", terrain.labyrinth[y]);

        fclose(map);
    }

    else if (mapChoice == 5)  // mappa inserita manualmente
    {
     // INSERIMENTO LARGHEZZA E ALTEZZA
        scanf("%d", &terrain.L);
        scanf("%d", &terrain.H);

     // CREAZIONE TERRENO DI GIOCO
        terrain.labyrinth = (char**) malloc(terrain.H * terrain.L);
        for (int y=0; y<terrain.H; y++)
            terrain.labyrinth[y] = (char*) malloc(terrain.L);

     // INSERIMENTO CASELLE LABIRINTO
        for (int y=0; y<terrain.H; y++)
            scanf(" %[^\n]s", terrain.labyrinth[y]);
    }

 // TROVA ENTRATA E USCITA
    findEntrance(terrain, &doors, &orientation);
    findExit(terrain, &doors);

 // ASSEGNAZIONI COORDINATE
    {
        coordinates.heroPosY = doors.entranceY;
        coordinates.heroPosX = doors.entranceX;
        coordinates.oldPosY = coordinates.heroPosY;
        coordinates.oldPosX = coordinates.heroPosX;
    }

 // ASSEGNAZIONI COUNTER
    {
        counters.steps = 0;
        counters.score = 1000;
        counters.bonusCounter = 0;
        counters.malusCounter = 0;
        counters.drillCounter = 0;
        counters.disableDrill = false;
    }

 // ASSEGNAZIONI CODA
    {
        tail.hasTail = false;
        tail.head = NULL;
        tail.size = 0;
    }

 // MENU
    int mode;
    do {
        mode = menu();
    } while (mode < 1 || mode > 4);

    if (mode != 1) counters.disableDrill = true;   // il trapano è disabilitato per le modalita AI

 // PRIMA STAMPA
    printLabyrinth(&terrain, coordinates, tail);

 // GIOCO VERO E PROPRIO
    while (coordinates.heroPosY != doors.exitY || coordinates.heroPosX != doors.exitX)
    {
        gameInfo(&counters, mode);
        
        if (mode == 1) orientation.direction = awsdMovement();

        else if (mode == 2) AI_alwaysRight(terrain, &orientation, coordinates, counters.drillCounter);

        else if (mode == 3) AI_crazy(&orientation.direction);

        else if (mode == 4) AI_reallyCrazy(&orientation.direction);
        
        moveHero(terrain, &coordinates, orientation, &counters);

        checkStep(&terrain, &coordinates, &tail, &counters);  // cosa c'è nella casella in cui sto andando?

        printLabyrinth(&terrain, coordinates, tail);
    }

    gameInfo(&counters, mode);

    printf("\n\tPremi un tasto qualsiasi per uscire . . . ");
    getch();

    return 0;
}



/**************************  MENU, INFORMAZIONI, LOADING  **************************************************************************************************/

int chooseMap()
{
    system("cls");

    printf("\n\n");
    printf("\t.oOOOo.                o                  o              o                                  o    \n"
           "\to     o                O                 O              O                  o               O     \n"
           "\tO.                     o                 o              O                              O   o     \n"
           "\t `OOoo.                o                 o              o                             oOo  O     \n"
           "\t      `O 'OoOo. .oOoO' O  o  .oOo.       O       .oOoO' OoOo. O   o `OoOo. O  'OoOo.   o   OoOo. \n"
           "\t       o  o   O O   o  OoO   OooO'       O       O   o  O   o o   O  o     o   o   O   O   o   o \n"
           "\tO.    .O  O   o o   O  o  O  O           o     . o   O  o   O O   o  O     O   O   o   o   o   O \n"
           "\t `oooO'   o   O `OoO'o O   o `OoO'       OOoOooO `OoO'o `OoO' `OoOO  o     o'  o   O   `oO O   o \n"
           "\t                                                                  o                              \n"
           "\t                                                               OoO'                              \n");

    int mapChoice;
    printf("\n\tSeleziona la mappa:\n"
             "\t1. Mappa 1\n"
             "\t2. Mappa 2\n"
             "\t3. Mappa 3\n"
             "\t4. Mappa 4\n"
             "\t5. Mappa inserita da tastiera nel seguente formato: lunghezza_x <a capo> altezza_y <a capo> labirinto\n\n"
             "\tInserisci la scelta: ");
    scanf("%d", &mapChoice);

    system("cls");
    return mapChoice;
}

int menu()
{
    system("cls");

    printf("\n\n");
    printf("\t.oOOOo.                o                  o              o                                  o    \n"
           "\to     o                O                 O              O                  o               O     \n"
           "\tO.                     o                 o              O                              O   o     \n"
           "\t `OOoo.                o                 o              o                             oOo  O     \n"
           "\t      `O 'OoOo. .oOoO' O  o  .oOo.       O       .oOoO' OoOo. O   o `OoOo. O  'OoOo.   o   OoOo. \n"
           "\t       o  o   O O   o  OoO   OooO'       O       O   o  O   o o   O  o     o   o   O   O   o   o \n"
           "\tO.    .O  O   o o   O  o  O  O           o     . o   O  o   O O   o  O     O   O   o   o   o   O \n"
           "\t `oooO'   o   O `OoO'o O   o `OoO'       OOoOooO `OoO'o `OoO' `OoOO  o     o'  o   O   `oO O   o \n"
           "\t                                                                  o                              \n"
           "\t                                                               OoO'                              \n");

    int mode;
    printf("\n\tModalita' di gioco:\n"
             "\t1. Manuale\n"
             "\t2. IA - Sempre a destra\n"
             "\t3. IA - Random\n"
             "\t4. IA - Crazy (random)\n\n"
             "\tInserisci la modalita': ");
    scanf("%d", &mode);

    system("cls");
    return mode;
}

long points(unsigned bonusCounter, unsigned malusCounter, unsigned steps)
{
    return (long)bonusCounter*10 - (long)steps;
}

void gameInfo(struct counters_s *counters, int mode)
{
    if (mode == 1) printf("\n\tUsa A-W-S-D per muoverti\n");

    counters->score = 1000 + points(counters->bonusCounter, counters->malusCounter, counters->steps);

    printf("\n\tTrapani: %u\t\tMonete: %u\n", counters->drillCounter, counters->bonusCounter);
    
    printf("\n\tPassi: %u\t\tImprevisti: %u\n", counters->steps, counters->malusCounter);

    printf("\n\t\tPUNTEGGIO: %d\n\n", counters->score);
}

char awsdMovement()
{
    switch(getch())
    {
        case 'a':
        case 'A':
            return 'w';

        case 'w':
        case 'W':
            return 'n';

        case 's':
        case 'S':
            return 's';

        case 'd':
        case 'D':
            return 'e';
    }
    
    return 0;
}



/**************************  TROVA ENTRATA E USCITA  ****************************************************************************************************/

void findEntrance(struct terrain_s terrain, struct doors_s *doors, struct orientation_s *orientation)
{
    for (int y=0; y<terrain.H; y++)
        for (int x=0; x<terrain.L; x++)
            if (terrain.labyrinth[y][x] == 'o')
            {
                doors->entranceY = y;
                doors->entranceX = x;

                if (doors->entranceY == 0) orientation->facing = 's';                   // imposta la direzione dello sguardo
                                                                                        // serve per l'AI sempre a destra
                else if (doors->entranceY == terrain.H-1) orientation->facing = 'n';

                else if (doors->entranceX == 0) orientation->facing = 'e';

                else if (doors->entranceX == terrain.L-1) orientation->facing = 'w';

                return;
            }
}

void findExit(struct terrain_s terrain, struct doors_s *doors)
{
    for (int y=0; y<terrain.H; y++)
        for (int x=0; x<terrain.L; x++)
            if (terrain.labyrinth[y][x] == '_')
            {
                doors->exitY = y;
                doors->exitX = x;

                return;
            }
}



/**************************  STAMPA TERRENO  ****************************************************************************************************/

void printLabyrinth(struct terrain_s *terrain, struct coordinates_s coordinates, struct tail_s tail)
{
    terrain->labyrinth[coordinates.oldPosY][coordinates.oldPosX] = ' ';

    while (tail.head != NULL)
    {
        terrain->labyrinth[tail.head->tailPosY][tail.head->tailPosX] = '*';
        tail.head = tail.head->next;
    }

    terrain->labyrinth[coordinates.heroPosY][coordinates.heroPosX] = 'o';    // posizione di snake

    system("cls");

    printf("\n\n");                     // questo codice disegna il labirinto
    for (int y=0; y<terrain->H; y++)
    {
        printf("\t\t");

        for (int x=0; x<terrain->L; x++)
            printf("%c", terrain->labyrinth[y][x]);

        printf("\n");
    }
}



/**************************  MOVIMENTO PERSONAGGIO  ****************************************************************************************************/

void moveHero(struct terrain_s terrain, struct coordinates_s *coordinates, struct orientation_s orientation, struct counters_s *counters)
{
    counters->moved = false;    // la coda si muove solo se si muove il corpo (vedi funzione checkStep)

    if (orientation.direction == 'n' || orientation.direction == 'e' || orientation.direction == 's' || orientation.direction == 'w')
    {
        coordinates->oldPosY = coordinates->heroPosY;
        coordinates->oldPosX = coordinates->heroPosX;

        if (isBorder(terrain, orientation, coordinates) == false)
        {
            if ((counters->drillCounter <= 0  ||  counters->disableDrill == true)  &&  isWall(terrain, orientation, coordinates) == false)
            {
                counters->moved = true;                                              //  ^^^ se non ci sono trivelle o la trivella è disabilitata (AI)
                (counters->steps)++;  // aumenta i passi di uno                            

                if (orientation.direction == 'n') coordinates->heroPosY--;
                
                else if (orientation.direction == 'e') coordinates->heroPosX++;
                
                else if (orientation.direction == 's') coordinates->heroPosY++;
                
                else if (orientation.direction == 'w') coordinates->heroPosX--;
            }
            if (counters->drillCounter > 0  &&  counters->disableDrill == false)
            {
                counters->moved = true;
                (counters->steps)++;  // aumenta i passi di uno

                if (orientation.direction == 'n') coordinates->heroPosY--;
                
                else if (orientation.direction == 'e') coordinates->heroPosX++;
                
                else if (orientation.direction == 's') coordinates->heroPosY++;
                
                else if (orientation.direction == 'w') coordinates->heroPosX--;
            }
        }
    }
}

bool isBorder(struct terrain_s terrain, struct orientation_s orientation, struct coordinates_s *coordinates)
{
    if (orientation.direction == 'n'  &&  coordinates->heroPosY - 1 >= 0) return false;

    else if (orientation.direction == 'e'  &&  coordinates->heroPosX + 1 < terrain.L) return false;

    else if (orientation.direction == 's'  &&  coordinates->heroPosY + 1 < terrain.H) return false;

    else if (orientation.direction == 'w'  &&  coordinates->heroPosX - 1 >= 0) return false;

    return true;
}

bool isWall(struct terrain_s terrain, struct orientation_s orientation, struct coordinates_s *coordinates)
{
    if ((orientation.direction == 'n') && (terrain.labyrinth[coordinates->heroPosY-1][coordinates->heroPosX] != '#')) return false;
    
    else if ((orientation.direction == 'e') && (terrain.labyrinth[coordinates->heroPosY][coordinates->heroPosX+1] != '#')) return false;
    
    else if ((orientation.direction == 's') && (terrain.labyrinth[coordinates->heroPosY+1][coordinates->heroPosX] != '#')) return false;
    
    else if ((orientation.direction == 'w') && (terrain.labyrinth[coordinates->heroPosY][coordinates->heroPosX-1] != '#')) return false;
    
    return true;
}

void checkStep(struct terrain_s *terrain, struct coordinates_s *coordinates, struct tail_s *tail, struct counters_s *counters)
{
    if (terrain->labyrinth[coordinates->heroPosY][coordinates->heroPosX] == '$')
    {
        (tail->size)++;
        
        if (tail->hasTail == true)
            tailNewHead(tail, coordinates->oldPosY, coordinates->oldPosX);
        else
            tailFirstNode(tail, coordinates->oldPosY, coordinates->oldPosX);
    }
    else 
    {
        if (tail->hasTail == true  &&  counters->moved == true) tailFollow(tail, coordinates);  // la coda si muove solo se si muove il corpo

        if (terrain->labyrinth[coordinates->heroPosY][coordinates->heroPosX] == '!')
        {
            (counters->malusCounter)++;
            
            tail->size /= 2;                            // trova il punto in cui tagliare la coda (divisione intera)
            struct list *index = tail->head;            // caso particolare se la coda ha solo l'elemento head (vedi funzione cutTail)
            for (unsigned i=0; i<tail->size; i++)
                index = index->next;
            cutTail(tail, index->tailPosY, index->tailPosX, terrain);
        }

        else if (terrain->labyrinth[coordinates->heroPosY][coordinates->heroPosX] == 'T') (counters->drillCounter) += 3;
        else if (terrain->labyrinth[coordinates->heroPosY][coordinates->heroPosX] == '#') (counters->drillCounter)--;

        else if (terrain->labyrinth[coordinates->heroPosY][coordinates->heroPosX] == '*')
            cutTail(tail, coordinates->heroPosY, coordinates->heroPosX, terrain);
    }

    counters->bonusCounter = tail->size;
}



/**************************  FUNZIONI DELLA CODA  ****************************************************************************************************/

void tailFirstNode(struct tail_s *tail, int y, int x)
{
    tail->hasTail = true;

    tail->head = (struct list*) malloc(sizeof(struct list));

    tail->head->tailPosY = y;
    tail->head->tailPosX = x;
    tail->head->next = NULL;
}

void tailNewHead(struct tail_s *tail, int y, int x)
{
    struct list *new = (struct list*) malloc(sizeof(struct list));

    new->tailPosY = y;
    new->tailPosX = x;
    new->next = tail->head;
    tail->head = new;
}

void tailFollow(struct tail_s *tail, struct coordinates_s *coordinates)
{
    struct list *index = tail->head;

    int tempY = index->tailPosY;
    int tempX = index->tailPosX;

    int tempX2, tempY2;

    index->tailPosY = coordinates->oldPosY;
    index->tailPosX = coordinates->oldPosX;

    while (index->next != NULL)
    {
        index = index->next;

        tempY2 = index->tailPosY;
        tempX2 = index->tailPosX;

        index->tailPosY = tempY;
        index->tailPosX = tempX;

        tempY = tempY2;
        tempX = tempX2;
    }

    coordinates->oldPosY = tempY;
    coordinates->oldPosX = tempX;
}

void cutTail(struct tail_s *tail, int y, int x, struct terrain_s *terrain)
{                                                                           // caso particolare in cui viene preso un imprevisto quando si ha 1 moneta
    if (tail->size == 0)                                                    // (vedi funzione checkStep)
    {                                                                       // è l'unico caso in cui viene persa anche la head della coda
        tail->hasTail = false;
        terrain->labyrinth[tail->head->tailPosY][tail->head->tailPosX] = ' ';
        tail->head = NULL;
        return;
    }

    bool found = false;
    unsigned tailSize = 0;

    struct list *index = tail->head;
    struct list *cutNode;

    while (index->next != NULL)
    {
        if (found == false)
            tailSize++;
        
        terrain->labyrinth[index->next->tailPosY][index->next->tailPosX] = ' ';  // cancella tutta la coda (viene comunque ristampata in printLabyrinth)

        if (y == index->next->tailPosY  &&  x == index->next->tailPosX)
        {
            found = true;
            cutNode = index;
        }

        index = index->next;
    }

    if (found == false) return;  // ciò si verifica se non si entra nel while, e quindi se la coda ha un solo elemento

    tail->size = tailSize;       // altrimenti imposta la nuova lunghezza e taglia
    cutNode->next = NULL;
}



/**************************  INTELLIGENZA ARTIFICIALE  ****************************************************************************************************/

void AI_alwaysRight(struct terrain_s terrain, struct orientation_s *orientation, struct coordinates_s coordinates, unsigned drillCounter)
{
    if (orientation->facing == 'n')
    {
        if (terrain.labyrinth[coordinates.heroPosY][coordinates.heroPosX+1] != '#') orientation->facing = 'e';       // se a destra non c'è un muro vai a destra
        else if (terrain.labyrinth[coordinates.heroPosY-1][coordinates.heroPosX] == '#') orientation->facing = 'w';  // se davanti c'è un muro vai a sinistra
    }

    else if (orientation->facing == 'e')
    {
        if (terrain.labyrinth[coordinates.heroPosY+1][coordinates.heroPosX] != '#') orientation->facing = 's';
        else if (terrain.labyrinth[coordinates.heroPosY][coordinates.heroPosX+1] == '#') orientation->facing = 'n';
    }

    else if (orientation->facing == 's')
    {
        if (terrain.labyrinth[coordinates.heroPosY][coordinates.heroPosX-1] != '#') orientation->facing = 'w';
        else if (terrain.labyrinth[coordinates.heroPosY+1][coordinates.heroPosX] == '#') orientation->facing = 'e';
    }

    else
    {
        if (terrain.labyrinth[coordinates.heroPosY-1][coordinates.heroPosX] != '#') orientation->facing = 'n';
        else if (terrain.labyrinth[coordinates.heroPosY][coordinates.heroPosX-1] == '#') orientation->facing = 's';
    }

    orientation->direction = orientation->facing;

    sleep(1);
}

void AI_crazy(char *direction)
{
    int numRan = rand() % 4;

    if (numRan == 0) *direction = 'n';
    else if (numRan == 1) *direction = 'e';
    else if (numRan == 2) *direction = 's';
    else *direction = 'w';
}

void AI_reallyCrazy(char *direction)
{
    int numRan = rand() % 4;

    if (numRan == 0) {
        *direction = 'n';
        system("color a");
    }
    else if (numRan == 1) {
        *direction = 'e';
        system("color b");
    }
    else if (numRan == 2) {
        *direction = 's';
        system("color c");
    }
    else {
        *direction = 'w';
        system("color d");
    }
}