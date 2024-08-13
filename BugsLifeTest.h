//----------------------------------------------------------------------
//Librerie necessarie per BugsLife
//----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

#include <allegro.h>
#include "Ptask.h"

//----------------------------------------------------------------------
// COSTANTI
//----------------------------------------------------------------------

//Costanti fisiche
#define G0 9.8
#define TSCALE 10
#define PI 3.14
#define LATOFOOD 1007
#define REPULSION 1000
#define ATTRACTION 100
#define MAXDELTATHETA 0.0212 //max variazione di theta ogni step e.g: PI/4 al secondo è PI/(4*25) ogni step di 1/25 sec
#define FOOD_ATTRACTION 10
#define MAX_SPEED 4

//Costanti bugs
#define MAX_BUGS 30
#define VIS_A 2.094         //radianti  2.094 = 120°
#define VIS_L 250           //pixel
#define BASE 30             //base del triangolo bug
#define HEIGHT 120          //altezza del triangolo bug
#define INTERACTION 0.5     //interazione tra i bug
#define INGOMBRO 50         //ingombro del bug
#define DUMP 0.8            //damping   
#define REPROD_TIME 1       //tempo di riproduzione
#define MAX_AGE 30          //età massima, secondi
#define MAX_FOOD 100        //cibo massimo per un bug
#define MAX_NUM_FOOD 300    //numero massimo di cibi

//Costanti grafiche
 
#define SCREEN_WUI 1024
#define SCREEN_HUI 200
#define SCREEN_WG 1024
#define SCREEN_HG 558

//Structure definition

struct state {
    float age;
    int alive;      // 1 for alive, 0 for dead
    float food;     //level of food in the bug, range 0-100
    int gender;     // 1 for man 0 for women
    float x;        //x coordinate alto sx
    float y;        //y coordinate
    float x1;       //coordinate alto dx
    float y1;
    float x2;       //coordinate basso dx
    float y2;       
    float x3;       //coordinate basso sx
    float y3;
    float x_c;      //coordinate centro hitbox
    float y_c;
    float vx;       //x velocity
    float vy;       //y velocity
    float ax;       //x acceleration
    float ay;       //y acceleration
    float color;
    float cr;        //color Red
    float cg;       //color Green
    float cb;       //color Blue
    float r_vis;    //Raggio del campo visivo
    float a_vis;    //Angolo del campo visivo
    float theta;    //angolo di orientazione del bug rispetto all' asse x positivo
    BITMAP *bmp;                        //bitmap per il disegno
    int new;                            //flag se il bug è appena nato
    int son;                            //0 se è un figlio, 1 se è della stirpe originale
    int repr;                           //1 se si sta riproducendo, 0 in caso negativo
    int eat;                            //1 se sta mangiando, 0 in caso negativo
    int vis;                            //1 sattiva la visualizzazione del camo visivo, 0 la disattiva
    int ingombro;                       //1 visualizzo l' ingombro del bug
};

struct food {
    float x;
    float y;
    int eaten;              //1 se è stato mangiato, 0 se è ancora presente
    int active;             //1 se è stato creato, 0 se è non è stato mai creato
    BITMAP *bmp;
    
};



//Function prototypes

void init_bug(int i);               //inizializza il bug i
void draw_bug(int i);               //disegna il bug i
void init(void);                    //inizializza il programma
float frand(float max, float min);  //genera un numero casuale tra xmi e xma
void bugs_interactions(int i);      //gestisce le collisioni con gli altri bug
char get_scancode();                //prende il codice della tastiera
void white_back_buggi(void);        //rende lo sfondo trasparente
void white_back_pane(void);        
void handle_boundary(int i);        //gestisce le collisioni con i bordi
void counter(int i);                //aggiorna la matrice di interazione tra i bug i e j
int inside(int i, int j);           //controlla se il bug i è dentro il campo visivo del bug j
void reproduction(int i);           //gestisce la riproduzione tra i bug i e j
void init_counter(void);            //inizializza la matrice di interazione
void age(int i);                    //aggiorna l'età del bug i
void death(int i);                  //gestisce la morte del bug i
void food(int i);                   //gestisce il cibo del bug i
void init_food(int i,float x, float y);              //inizializza il cibo con le coordinate del mouse x,y
void draw_food(void);                      //disegna il cibo
void display_repr(int i, int j);          //disegna la riproduzione
void bug_rand_move(int i, float dt);          //muove il bug i in modo casuale: dt è il passo di integrazione
void bug_repulsion(int i, float dt);          //gestisce la repulsione tra i bug
float distance(int i, int j);                 //calcola la distanza tra i bug i e j
float distancex(int i, int j);                //calcola la distanza in x tra i bug i e j
float distancey(int i, int j);                //calcola la distanza in y tra i bug i e j
void update_bug(int i, float dt);               //aggiorna la posizione del bug i
void apply_force(int i, float dt);              //applica le forze al bug i
float delta_theta(int i,float da);              //calcola la variazione di theta
void pacman(int i);                             //gestisce l' effetto pacman
void preinit_food(int i);                       //pre inizializza tutti i cibi come inattivi
int insideb_f(int i, int j);                    //calcola la distanza tra il bug i e il cibo j
void attract_to_food(int i);                    //attrazione verso il cibo
int getActiveFoodCount();                       //ritorna il numero di cibi attivi
void draw_VE();                                 //disegna l' ambiente virtuale
void draw_legend();                             //disegna la legenda    

//Thread functions prototypes
void* user(void *arg);
void* bugs_task(void * arg);
void* display(void *arg);



//Extern global variables definition

