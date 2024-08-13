
#include "BugsLifeTest.h"


//----------------------------------------------------------------------
//GLOBAL VARIABLES
//----------------------------------------------------------------------


BITMAP *buffergame;                         //buffer per il double buffering dello scarafaggio
BITMAP *bufferUI;
 
BITMAP *buggitr;
struct FONT *myfont;
//stringhe per la UI
char s[100];
char up[100];
char down[100];
char click[100];
char dm[100];                               //deadline miss

struct state bug[MAX_BUGS + 1];             //stato dei bug
struct food pane[MAX_FOOD + 1];             //stato del cibo
int end = 0;                                //flag di terminazione
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
 
//numero di bug attivi variabile esterna
int nab;                                    //numero di bug massimi mai stati in vita
int nlb = 0;                                //numero di bug alive
int naf = 0;                                //numero di cibi attivi
int dmc = 0;                                //deadline miss count
int totfood = 0;                            //numero totale di cibi creati
float xg1, yg1, xg2, yg2;                   //coordinate dei due bug genitori
float count [MAX_BUGS + 1 ][MAX_BUGS + 1 ];             //contatore per la riproduzione dei bug


//----------------------------------------------------------------------
//FUNCTIONS
//----------------------------------------------------------------------
 



//---------------------------------------------------------------------
//AUXILIARY FUNCTIONS
//---------------------------------------------------------------------

char get_scancode(){
    
    if(keypressed()) {
        return readkey() >> 8;
    }
    else return 0;
}

float frand(float xmi, float xma){

    //inizializza il generator edi numeri casuali altrimenti dà sempre il solito numero
    float r;
        r = rand() / (float)RAND_MAX;   //rand in [0,1)
        return xmi + (xma - xmi)* r;
}

//distanze tra due bug isa in modulo che su assi x e y

float distancex(int i, int j) {

    float dx, dy, d;
    dx = bug[j].x_c - bug[i].x_c;
    dy = bug[j].y_c - bug[i].y_c;
    d = dx;
    return d;
}

float distancey(int i, int j) {

    float dx, dy, d;
    dx = bug[j].x_c - bug[i].x_c;
    dy = bug[j].y_c - bug[i].y_c;
    d = dy;
    return d;
}

float distance(int i, int j) {

    float dx, dy, d;
    dx = bug[j].x_c - bug[i].x_c;
    dy = bug[j].y_c - bug[i].y_c;
    d = sqrt(dx*dx + dy*dy);
    return d;
}



//---------------------------------------------------------------------
//INITIALIZATION
//---------------------------------------------------------------------

void init_counter(void){

    int i, j;
    for (i = 0; i < 50; i++) {
        for (j = 0; j < 50; j++) {
            count[i][j] = 0;
        }
    }
}

void init_food(int i, float x, float y){


    
    pane[i].x = x;
    pane[i].y = y - SCREEN_HUI;

    pane[i].eaten = 0;
    pane[i].active = 1;
    pane[i].bmp = load_bitmap("pane.bmp", NULL);
    if(pane[i].bmp == NULL) {
        allegro_message("errore loading bitmap pane %d\n", i);
        destroy_bitmap(pane[i].bmp);
        exit(1);
    }
   
}

void preinit_food(int i) {
    pane[i].active = 0;
    pane[i].eaten = 0;
 }

void init(void) {

    int err_user;
    int err_display; 
    int i;
    //Inizializzo la grafica
    allegro_init();
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1024, 768, 0, 0);
    set_color_depth(16);
    clear_to_color(screen, 0);
    install_keyboard();
    buffergame = create_bitmap(SCREEN_W, SCREEN_H - SCREEN_HUI - 10 );
    clear_bitmap(buffergame); 
    clear_to_color(buffergame, makecol(0,0,0));
    bufferUI = create_bitmap(SCREEN_WUI, SCREEN_HUI);
    clear_bitmap(bufferUI);
    clear_to_color(bufferUI, makecol(0,0,0));
    

    init_counter();
    for( i = 0; i <= MAX_FOOD; i++) {
        preinit_food(i);
    }
    
    ptask_init(SCHED_FIFO);
    install_mouse();
    enable_hardware_cursor();
    


    err_user = task_create(user, 51, 40, 40, 80, ACT);   //c'è un solo task user, numero 51
    if(err_user == 0) printf("task user creato correttamente\n");
    else printf("task user non creato, il codice errore è %d\n", err_user);

    err_display = task_create(display, 52, 150, 150, 90, ACT); //c' è un solo task display, numero 52  
    if(err_display == 0) printf("task display creato correttamente\n");
    else printf("task display non creato, il codice errore è %d\n", err_display);

}

void init_bug(int i) {
    
    if(bug[i].son == 1) {
        bug[i].x = (xg1 + xg2) / 2;
        bug[i].y = (yg1 + yg2) / 2;
        bug[i].vx = 0;
        bug[i].vy = 0;
    }
    else {
        bug[i].x = frand(200 , SCREEN_W);
        bug[i].y = frand(5, SCREEN_H);
        bug[i].vx = 0;
        bug[i].vy = 0;
        bug[i].son = 0;
    }

    bug[i].ax = 0;
    bug[i].ay = 0;
    
    bug[i].cr = frand(100, 150);
    bug[i].cg = frand(1, 253);
    bug[i].cb = frand(1, 253);
    bug[i].color = makecol(bug[i].cr, bug[i].cg, bug[i].cb);
    //printf("Il colore del bug %d è %f %f %f\n", i, bug[i].cr, bug[i].cg, bug[i].cb);
    bug[i].r_vis = VIS_L;               //pixel
    bug[i].a_vis = VIS_A;               //angolo di visione in radianti 2.094 = 120 gradi
    bug[i].theta = atan2(bug[i].vy, bug[i].vx); //angolo in radianti
    //calcolo i vertici della hitbox
    bug[i].x1 = bug[i].x_c + 50;
    bug[i].y1 = bug[i].y_c - 50;
    bug[i].x2 = bug[i].x_c + 50;
    bug[i].y2 = bug[i].y_c + 50;
    bug[i].x3 = bug[i].x_c -50;
    bug[i].y3 = bug[i].y_c +50;
    //calcolo il centro della hitbox
    bug[i].x_c = bug[i].x + 50;
    bug[i].y_c = bug[i].y + 50;
    bug[i].bmp = load_bitmap("buggi.bmp", NULL);
    bug[i].new = 1;
    bug[i].alive = 1;
   
    bug[i].gender = rand() % 2;
    bug[i].food = 50;
    bug[i].age = 0;
    
    bug[i].repr = 0;
    bug[i].eat = 0;
    bug[i].vis = 0;
    bug[i].ingombro = 0;
}  

//---------------------------------------------------------------------
//GRAPHIC FUNCTIONS
//---------------------------------------------------------------------

void draw_bug(int i) { 

    int j;
    int angle;                                      //angolo in [0,256]
    int grad;                                      //angolo in gradi              
    grad = (bug[i].theta * 360 / (2 * PI));           //angolo in gradi da rad, tolgo 45 per come è inclinto il bitmap bug2.bmp         
    angle = 64 - grad*(float)256/360;               //angolo in fix da grad
    int rot = itofix(angle);                        //converte un int in un fixed point
    float energy;

    if(bug[i].bmp == NULL || buffergame == NULL){
        allegro_message("errore loading bitmap\n");
        destroy_bitmap(bug[i].bmp);
       
        exit(1);

    }
    else{
    
        
        //white_back_buggi();

        
        rotate_sprite(buffergame,bug[i].bmp,bug[i].x,bug[i].y,rot);


        //disegno un pallino blu nel centro della hitbox se maschio, rosa se femmina
        if(bug[i].gender == 1){//maschio

            if(bug[i].age < 0.2 * MAX_AGE) circlefill(buffergame, bug[i].x_c, bug[i].y_c,10, makecol(178,255,255)); //giovane
            else if(bug[i].age > 0.8 * MAX_AGE) circlefill(buffergame, bug[i].x_c, bug[i].y_c,10, makecol(0,0,0)); //anziano
            else circlefill(buffergame, bug[i].x_c, bug[i].y_c,10, makecol(0,0,200)); //adulto
        }

        else{//femmina
            if(bug[i].age < 0.2 * MAX_AGE) circlefill(buffergame, bug[i].x_c, bug[i].y_c,10, makecol(255,255,255));//giovane femmina
            else if(bug[i].age > 0.8 * MAX_AGE) circlefill(buffergame, bug[i].x_c, bug[i].y_c,10, makecol(128,0,128));//femmina anziana
            else circlefill(buffergame, bug[i].x_c, bug[i].y_c,10, makecol(255,106,180));//femmina adulta
            
        }

        //disegno la barra dell' energia
        energy = bug[i].food / MAX_FOOD * 90;   //energia del bug normalizzata tra 0 e 85
        rectfill(buffergame, bug[i].x + 25, bug[i].y, bug[i].x + energy, bug[i].y + 5, makecol(255,2165,0));

        //disegno il vettore velocità
        line(buffergame, bug[i].x_c, bug[i].y_c, (bug[i].x_c+4*bug[i].vx), (bug[i].y_c+4*bug[i].vy), makecol(200,0,0)); 
        
        //disegno il campo visivo
        if(bug[i].vis == 1) {   
            circle(buffergame, bug[i].x_c, bug[i].y_c, bug[i].r_vis, bug[i].color); 
            line(buffergame, bug[i].x_c, bug[i].y_c, (bug[i].x_c + bug[i].r_vis * cos(bug[i].theta + bug[i].a_vis)), (bug[i].y_c - bug[i].r_vis * sin(bug[i].theta + bug[i].a_vis)), bug[i].color);
            line(buffergame, bug[i].x_c, bug[i].y_c, (bug[i].x_c + bug[i].r_vis * cos(bug[i].theta - bug[i].a_vis)), (bug[i].y_c - bug[i].r_vis * sin(bug[i].theta - bug[i].a_vis)), bug[i].color);
        }
 
        //disegno l' ingombro del bug
        if(bug[i].ingombro == 1) {
            circle(buffergame, bug[i].x_c, bug[i].y_c, INGOMBRO, makecol(bug[i].cr, bug[i].cg, bug[i].cb));
        }
        
        //Disegna qualcosa se è appena nato, diverso per maschi e femmine, se creato da utente
        if(bug[i].new == 1 && bug[i].son == 1){
            
            circlefill(buffergame, bug[i].x_c, bug[i].y_c, 100, makecol(200,0,0));
            bug[i].new = 0;

        }

        //Disegna un pallino rosso se il nuovo bug è figlio
        if(bug[i].new == 1 && bug[i].son == 0){ 
            if(bug[i].gender == 1) {
                circlefill(buffergame, bug[i].x_c, bug[i].y_c, 100, makecol(0,0,200));
            }
            else{
                circlefill(buffergame, bug[i].x_c, bug[i].y_c, 100, makecol(255,105,180));
            }
            
            bug[i].new = 0;
        }

       
       
        
    }

    
   

}

void draw_food(){
    int i;
    for(i = 1; i <= totfood; i++){
        if(pane[i].eaten == 0 && pane[i].active == 1){
           
           draw_sprite( buffergame, pane[i].bmp, pane[i].x, pane[i].y );
           //printf("Il cibo %d è stato disegnato\n", i);

        }
    }
}

void draw_VE (void) {                              

    rect(bufferUI, 5, 5, SCREEN_WUI - 5, SCREEN_HUI - 5, makecol(255,0,0));
    rect(buffergame,5, 5 , SCREEN_WUI - 5 , SCREEN_H - SCREEN_HUI - 11, makecol(255,0,0));

}
void draw_legend(void) {

        sprintf(s, "Push space to create a bug");
        sprintf(up, "Push Key-UP to visualize bugs' field of view");
        sprintf(down, "Push Key-DOWN to visualize bugs' hitbox");
        sprintf(click, "Click on the screen to create a piece of food");
        sprintf(dm, "Deadline miss since simulation start : %d", dmc);
        printf("Deadline miss since simulation start : %d\n", dmc);
        
        textout_ex(bufferUI, font, s,300, 40, makecol(255,255,255), -1);
        textout_ex(bufferUI, font, up,300, 60, makecol(255,255,255), -1);
        textout_ex(bufferUI, font, down,300, 80, makecol(255,255,255), -1);
        textout_ex(bufferUI, font, click,300, 100, makecol(255,255,255), -1);
        textout_ex(bufferUI, font, dm,300, 120, makecol(255,255,255), -1);
}

void white_back_buggi(void){

   
    PALETTE pal;                // color palette


    int x, y, c;
    int pink, white;
    
    white = makecol(255, 255, 255);
    pink = makecol(255, 0, 255);

    bug[0].bmp = load_bitmap("bug2.bmp", NULL);
    buggitr = create_bitmap(bug[0].bmp->w, bug[0].bmp->h);

        for (x=0; x<bug[0].bmp->w; x++){

            for (y=0; y<bug[0].bmp->h; y++) {

                c = getpixel(bug[0].bmp, x, y);
                if (c == white) c = pink;
                putpixel(buggitr, x, y, c);

            }
        }
        get_palette(pal);
        save_bitmap("bug2.bmp", buggitr, pal);
        destroy_bitmap(buggitr);
}

void white_back_pane(void){

   
    PALETTE pal;                // color palette


    int x, y, c;
    int pink, white;
    
    white = makecol(255, 255, 255);
    pink = makecol(255, 0, 255);

    pane[0].bmp = load_bitmap("pane.bmp", NULL);
    buggitr = create_bitmap(pane[0].bmp->w, pane[0].bmp->h);

        for (x=0; x<pane[0].bmp->w; x++){

            for (y=0; y<pane[0].bmp->h; y++) {

                c = getpixel(pane[0].bmp, x, y);
                if (c == white) c = pink;
                putpixel(buggitr, x, y, c);

            }
        }
        get_palette(pal);
        save_bitmap("pane.bmp", buggitr, pal);
        destroy_bitmap(buggitr);
}

void handle_boundary(int i){

        //detection e gestione posizione e velocità in caso di collisione con i bordi
        
        if(bug[i].x <= 0 ){
            bug[i].x = 10;
            bug[i].vx = - DUMP * bug[i].vx;
            bug[i].theta = PI - bug[i].theta;
        }
        if(bug[i].x >= SCREEN_W - 100){
            bug[i].x =  SCREEN_W - 110;
            bug[i].vx = - DUMP * bug[i].vx;
            bug[i].theta = PI - bug[i].theta;
        }
        
        if(bug[i].y <= 0){
            bug[i].y = 10;
            bug[i].vy = - DUMP * bug[i].vy;
            bug[i].theta = -bug[i].theta;
        }
        if(bug[i].y >= SCREEN_H - 100){
            bug[i].y = SCREEN_H - 110;
            bug[i].vy = - DUMP * bug[i].vy;
            bug[i].theta = -bug[i].theta;
        }
        
}

//---------------------------------------------------------------------
//PHYSICS FUNCTIONS
//---------------------------------------------------------------------
 

void bugs_interactions(int i){ //gestione sovrapposivione tra bug
//----------------------------------------------------------------------
//Neighbors detection and interactions handling
//----------------------------------------------------------------------

    float dx, dy, d;                    //distanza tra i centri dei due bug
    int j;
    float aj, ai;                       //angolo del vettore velocità dei due bug con l' orizzontale
    float bji;
    float nx, ny,tx,ty,tr;              //versore normale al punto di contatto e traslazione per evitare la sovrapposizione
    float vni, vnj, vti, vtj,temp;
    

    for(j = 1; j <= nab; j++){

        if(i != j){                      //non considero il bug stesso (i = j)
            
          
            dx = bug[j].x_c - bug[i].x_c;
            dy = bug[j].y_c - bug[i].y_c;
            d = sqrt(pow(dx, 2) + pow(dy, 2));
           

            if(d < 2 * INGOMBRO) {    
                //interactions handling
                //prima cosa è riportare il bug[i] in una posizione in cui non si sovrapponga al bug[j]

                if(d != 0){
                    nx = dx /d;
                    ny = dy /d;
                }
                else{                       //serve perchè se due bug sono inizializzati nel solito punto evita il problema
                    nx = 1;
                    ny = 1;
                }
                tx = -ny;
                ty = nx;
                tr = ((2*INGOMBRO) - d)/2; //traslazione per evitare la sovrapposizione, considerando che i bug hanno tutti lo stesso raggio di visione
                bug[i].x_c = bug[i].x_c - (tr * nx);
                bug[i].y_c = bug[i].y_c - (tr * ny);
                bug[j].x_c = bug[j].x_c + (tr * nx);
                bug[j].y_c = bug[j].y_c + (tr * ny);
                bug[i].x = bug[i].x_c - 50;
                bug[i].y = bug[i].y_c - 50;
                dx = bug[j].x_c - bug[i].x_c;
                dy = bug[j].y_c - bug[i].y_c;
                d = sqrt(pow(dx, 2) + pow(dy, 2));
                if(d < (2 * INGOMBRO-0.1) || d > (2*INGOMBRO+0.1)) printf("errore di traslazione\n");
               
               
                
                //poi gestire le velocità in modo che rallentino e si allontanino lentamente
               
                vni = bug[i].vx * nx + bug[i].vy * ny;
                vti = bug[i].vx * tx + bug[i].vy * ty;
                vnj = bug[j].vx * nx + bug[j].vy * ny;
                vtj = bug[j].vx * tx + bug[j].vy * ty;
               
                //aggiorno le velocità come urto completamente elastico con un coefficiente di damping
                
                temp = vni;
                vni = DUMP * vnj ;//- INTERACTION * (vni-vnj);
                vnj = DUMP * temp ;//- INTERACTION * (vnj-vni);


                //velocities recombination
            
                if(sqrt(vni*vni + vti*vti) < 1){ 

                bug[i].vx = vti * tx + vni * nx;
                bug[i].vy = vni * ny + vti * ty;
                bug[j].vx = vtj * tx + vnj * nx;
                bug[j].vy = vnj * ny + vtj * ty;
                
                bug[i].theta += delta_theta(i, bug[i].theta);

                }
                else{//normalizzo il vettore velocità
                    float lenght = 1 * sqrt(vni*vni + vti*vti);
                    bug[i].vx = (vti * tx + vni * nx) / lenght;
                    bug[i].vy = (vni * ny + vti * ty) / lenght;
                    bug[i].theta += delta_theta(i, bug[i].theta);
                }
            

            }
        }
    }
}

float delta_theta(int i,float da ){//da è il delta theta desiserato, fornisce la massima variazione di theta possibile

    da = -atan2(bug[i].vy, bug[i].vx);

    // Calculate the difference between the desired angle and the current angle
    float delta_theta = da - bug[i].theta;

    // Adjust the difference to be within the range [-pi, pi]
    while (delta_theta < -PI) delta_theta += 2 * PI;
    while (delta_theta > PI) delta_theta -= 2 * PI;

    // Limit the difference to a maximum value
    float max_delta_theta = MAXDELTATHETA;
    if (delta_theta > max_delta_theta) {
        delta_theta = max_delta_theta;
    } else if (delta_theta < -max_delta_theta) {
        delta_theta = -max_delta_theta;
    }

    return delta_theta;

    
}

int inside(int i, int j) { //controlla se il bug i è dentro il campo visivo del bug j

    float dx, dy, d, bji, aj;            //distanza tra i centri dei due bug
    
    int inside;
   
    dx = bug[j].x_c - bug[i].x_c;
    dy = bug[j].y_c - bug[i].y_c;
    d = sqrt(pow(dx, 2) + pow(dy, 2));
    aj = atan2(dy, dx);
    bji = aj - bug[i].theta;            //angolo tra il vettore velocità del bug[i] e il vettore che congiunge i centri dei due bug
    if(bji > PI) bji = bji - 2*PI;
    if(bji < -PI) bji = bji + 2*PI;
    inside = (d < bug[i].r_vis + bug[j].r_vis) && (bji < bug[i].a_vis && bji > -bug[i].a_vis);//bug[i] finds as a neighbor bug[j]
    /*if(inside) {
        printf("Il bug %d vede il bug %d \n", i, j);
    }
    else printf("Il bug %d non vede il bug %d \n", i, j);*/
    return inside;

}

void counter(int i) { //aggiorna la matrice dei contatori quando un bug vede un altro bug

    int j;
    for( j = 1 ; j <= nab; j++){
        pthread_mutex_lock(&count_mutex);
        if( i != j) {
            if(inside(i,j)) {
            count[i][j] = count[i][j] + 1.0/25.0;
            }
            else count[i][j] = 0;
        }
        //printf("Il bug %d vede il bug %d da %f sec\n", i, j, count[i][j]);
        pthread_mutex_unlock(&count_mutex);
    }

  
}

void reproduction(int i) {  //gestisce la riproduzione tra i bug i e j, costa cibo ai bug

    

    
    int j;
    int k;
    int err_bugs;
    float dx;
    float dy;
    float d;
    int temp;
    
    for(j = 1; j <= nab; j++){

        dx = bug[j].x_c - bug[i].x_c;
        dy = bug[j].y_c - bug[i].y_c;
        d = sqrt(dx*dx + dy*dy);
        if ( (i != j) && (count[i][j] > REPROD_TIME) && (count[j][i] > REPROD_TIME) && 
             (d < 2 * INGOMBRO) && (bug[i].age < 0.8 * MAX_AGE) && 
             (bug[j].age < 0.8 * MAX_AGE) && (bug[i].age > 0.2 * MAX_AGE) && 
             (bug[j].age > 0.2 * MAX_AGE) && ( bug[i].food > 39) && (bug[j].food > 39) &&
             (bug[i].gender != bug[j].gender) ) {
            //new bug creation
            
            if( nlb < MAX_BUGS){
                bug[i].repr = 1;
                bug[j].repr = 1;
                xg1 = bug[i].x_c;
                yg1 = bug[i].y_c;
                xg2 = bug[j].x_c;
                yg2 = bug[j].y_c;
                
                temp = 0;
                    for(i = 1; i <= nlb ; i++) {
                        if(bug[i].alive == 0) {
                            printf("Il bug %d è morto\n", i);
                            temp = i;
                            break;
                             
                        }
                    }
                    if(temp == 0) { // se è 0 non è morto nessuno
                        temp = nlb + 1;
                        
                    }
                    err_bugs = task_create(bugs_task, temp, 40, 40, 80, ACT);
                    

                    if( err_bugs == 0 ) {
                        printf("task %d  creato correttamente\n", temp);

                        if( temp == nlb +1) {
                            nab++;
                            nlb++;
                        }
                        else {
                            nab++;
                        }
                    }
                
                
                
                bug[temp].son = 1;

                for(k = 1; k < nab + 1; k++){
                    //if(i =! k) {
                        count[i][k] = 0;
                        
                    //}
                    //if(j =! k) {
                        count[j][k] = 0;
                    //}
                }

                bug[i].food = bug[i].food - 30;
                bug[j].food = bug[j].food - 30;
                
                //printf("Il bug %d si è riprodotto con il bug %d\n", i, j);
                //printf("Il bug %d ha il livello di cibo a %f\n", i, bug[i].food);
                //printf("Il bug %d ha il livello di cibo a %f\n", j, bug[j].food);
                //printf("Il bug %d ha il livello di cibo a %d\n", i, bug[i].food);
                          
                
            }
        }
    }
 
}

void age(int i) {
    bug[i].age = bug[i].age + 1.0/25.0;
    //printf("L' età del bug %d è %f\n", i, bug[i].age);
}

void death(int i) {
    if(bug[i].age > MAX_AGE || bug[i].food > MAX_FOOD || bug[i].food <= 0) {
        nlb--;
        
        bug[i].alive = 0;
        if(bug[i].food > MAX_FOOD) printf("Il bug %d è morto di ingordigia\n", i);
        if(bug[i].food <= 0) printf("Il bug %d è morto di fame\n", i);
        if(bug[i].age > MAX_AGE) printf("Il bug %d è morto di vecchiaia\n", i);
        printf("Il numero di bug attivi è %d\n", nlb);
        
    }
}

int getActiveFoodCount() {  //ritorna il numero di cibi attivi
    int count = 0;
    for(int j = 1; j <= totfood; j++){
        if(pane[j].active == 1 && pane[j].eaten == 0){
            count++;
        }
    }
    
    return count;
}

void food(int i) {          //verifica quando un bug mangia un cibo
    float d,dx,dy;
    int j;
    int k;
    k = 0;
    bug[i].food = bug[i].food - 1.0/25.0;
    //printf("Il livello di cibo del bug %d è %f\n",i,bug[i].food);
    int temp;
    temp = totfood;
    for( j = 1; j <= temp; j++){
            //printf("l' indice j è %d\n", j);
            dx = bug[i].x_c - (pane[j].x + 50);
            dy = bug[i].y_c - (pane[j].y + 50);
            d = sqrt(dx*dx + dy*dy) ; 
            if(d < INGOMBRO + 25) { //se il bug è sopra il cibo, pane mangiato
                if(pane[j].eaten == 0 && pane[j].active == 1 ){
                    printf("Il bug %d sta mangiando il cibo %d\n", i, j);
                    bug[i].food = bug[i].food + 30;
                    printf("Il bug %d ha il livello di cibo a %f\n", i,bug[i].food);
                    
                    pane[j].eaten = 1;
                    pane[j].active = 1;
                    printf("Il cibo %d è stato mangiato dal bug %d\n", j, i);
                    k++;
                    
                }
            }
    }
    
    
}

void bug_rand_move(int i, float dt){        //implementa una comonente casuale al movimento del bug

    float da, dv;
    float v;
    da = frand(-0.02, 0.02);                    //inizializzo la variazione dell'angolo rad
    dv = frand(-0.2, 0.2);                      //inizializzo la variazione del modulo della velocità m/ms
        
    //Aggiorno l' angolo theata e coerentemente il vettore velocità

    bug[i].theta = bug[i].theta + da;
    v = sqrt(pow(bug[i].vx, 2) + pow(bug[i].vy, 2));     //calcolo del modulo della velocità
        
    if(v + dv > 0 && v + dv <= MAX_SPEED) {
        v = v + dv;
    }
        
    //aggiorno la posizione del bug integrando la velocità con Forward Euler
    bug[i].x = bug[i].x + bug[i].vx * dt;
    bug[i].y = bug[i].y + bug[i].vy * dt;

    //aggiorno i vertici della hitbox
    bug[i].x1 = bug[i].x_c + 50;
    bug[i].y1 = bug[i].y_c - 50;
    bug[i].x2 = bug[i].x_c + 50;
    bug[i].y2 = bug[i].y_c + 50;
    bug[i].x3 = bug[i].x_c -50;
    bug[i].y3 = bug[i].y_c +50;
    bug[i].x_c = bug[i].x + 50;
    bug[i].y_c = bug[i].y + 50;

    //aggiorno la velocità
    bug[i].vx = v * cos(bug[i].theta);
    bug[i].vy = - v * sin(bug[i].theta);
    float velocity_angle = atan2(bug[i].vy, bug[i].vx);  //condizone di coerenza tra velocità e theta

}


void bug_repulsion(int i, float dt) {
    
    int j; 
    for(j = 1; j <= nab; j++){
        if(i != j) {

            float force;
            if(inside(i,j)) {
                if( bug[i].gender == bug[j].gender) {

                    force = -REPULSION / (distance(i, j) * distance(i, j));
                    bug[i].ax += force * distancex(i, j) / distance(i, j);
                    bug[i].ay += force * distancey(i, j) / distance(i, j);
                    bug[i].vx = bug[i].vx + bug[i].ax * dt;
                    bug[i].vy = bug[i].vy + bug[i].ay * dt;
                    
                    bug[i].x = bug[i].x + bug[i].vx * dt;
                    bug[i].y = bug[i].y + bug[i].vy * dt;
                    bug[i].x_c = bug[i].x + 50;
                    bug[i].y_c = bug[i].y + 50;
                    //bug[i].theta = atan2(bug[i].vy, bug[i].vx);
                    bug[i].ax = 0;
                    bug[i].ay = 0;

                }
                else {
                    force = ATTRACTION / (distance(i, j) * distance(i, j));
                    bug[i].ax += force * distancex(i, j) / distance(i, j);
                    bug[i].ay += force * distancey(i, j) / distance(i, j);
                    bug[i].vx = bug[i].vx + bug[i].ax * dt;
                    bug[i].vy = bug[i].vy + bug[i].ay * dt;
                    //bug[i].theta = atan2(bug[i].vy, bug[i].vx);
                    bug[i].x = bug[i].x + bug[i].vx * dt;
                    bug[i].y = bug[i].y + bug[i].vy * dt;
                    bug[i].x_c = bug[i].x + 50;
                    bug[i].y_c = bug[i].y + 50;
                    bug[i].ax = 0;
                    bug[i].ay = 0;
                }
            }
            
        }
    }
}

void update_bug(int i, float dt) {          //aggiorna la posizione e la velocità del bug dovuta alle forze
    float v;
    //aggiorno la posizione con la velocità al passo precedente
    bug[i].x = bug[i].x + bug[i].vx * dt;
    bug[i].y = bug[i].y + bug[i].vy * dt;
    bug[i].x_c = bug[i].x + 50;
    bug[i].y_c = bug[i].y + 50;

    v = sqrt(pow(bug[i].vx, 2) + pow(bug[i].vy, 2));     //calcolo del modulo della velocità
    if( v > MAX_SPEED) {
        bug[i].vx = (bug[i].vx + bug[i].ax * dt)/v * MAX_SPEED;
        bug[i].vy = (bug[i].vy + bug[i].ay * dt)/v * MAX_SPEED;
    }
    else {
        bug[i].vx = bug[i].vx + bug[i].ax * dt;
        bug[i].vy = bug[i].vy + bug[i].ay * dt;
    }
     
    //calcolare theta dopo affinchè si mantenga sempre coerente con la velocità
   

    bug[i].theta += delta_theta(i, bug[i].theta);//restituisce la max variaz. possibile di theta parallelo al vettore velocità
    //aggiorno l' angolo theta affinchè sia coerente
    //bug[i].theta = -atan2(bug[i].vy, bug[i].vx);
    //azzero le forze perchè verranno ricalcolate al prossimo passo
    bug[i].ax = 0;
    bug[i].ay = 0;
} 

void apply_force(int i, float dt) {         //applica le forze di attrazione e repulsione tra i bug

    float force;
    int j,k;
    for(j = 1; j <= nab; j++) {
        if(i != j && inside(i,j) && bug[j].alive == 1) {
           if(bug[i].gender == bug[j].gender) {
                force = -REPULSION / (distance(i, j) * distance(i, j));
                bug[i].ax += force * distancex(i, j) / distance(i, j);
                bug[i].ay += force * distancey(i, j) / distance(i, j);
            }
            
            else if( bug[i].gender != bug[j].gender && distance(i, j) > 2.5 * INGOMBRO) {
                force = -ATTRACTION / (distance(i, j) * distance(i, j));
                bug[i].ax += force * distancex(i, j) / distance(i, j);
                bug[i].ay += force * distancey(i, j) / distance(i, j);
            }

        }
       
    }
    
}
 
void attract_to_food(int i) {                           //applica la forza di attrazione tra il bug e il cibo

    int j;
    for(j = 1;j <= totfood;j++) {
        if( pane[j].eaten == 0 && pane[j].active == 1) {
            // Calcola il vettore direzione dal bug al cibo
            float dx = pane[j].x - bug[i].x_c;
            float dy = pane[j].y - bug[i].y_c;

            // Normalizza il vettore direzione (lo rende di lunghezza 1)
            float length = sqrt(dx*dx + dy*dy);
            if( insideb_f(i,j) ) {
                dx /= length;
                dy /= length;
    
                // Aggiungi una forza in direzione del cibo al bug
                float force_strength = FOOD_ATTRACTION;  
                bug[i].ax += force_strength * dx;
                bug[i].ay += force_strength * dy;
            }
        }
    }
}

int insideb_f(int i, int j) {
    
        
    float dx, dy, d, bji, aj;            //distanza tra i centri dei due bug
    
    int inside;
   
    dx = (pane[j].x + 50) - bug[i].x_c;
    dy = (pane[j].y + 40) - bug[i].y_c;
    d = sqrt(pow(dx, 2) + pow(dy, 2));
    aj = atan2(dy, dx);
    
    bji = aj - bug[i].theta;            //angolo tra il vettore velocità del bug[i] e il vettore che congiunge i centri dei due bug
    if(bji > PI) bji = bji - 2*PI;
    if(bji < -PI) bji = bji + 2*PI;
    inside = (d < bug[i].r_vis) && (bji < bug[i].a_vis/2.0 && bji > -bug[i].a_vis/2.0);//bug[i] finds pane[j]
    /*if(inside) {
        printf("Il bug %d vede il food %d \n", i, j);
    }
    else printf("Il bug %d non vede il food %d \n", i, j);*/
    return inside;

    
}

void pacman(int i) {                        //gestisce la collisione con i bordi del campo con effetto pacman

        if(bug[i].x < - 100) {
            bug[i].x = SCREEN_W + 100;
            bug[i].x_c = bug[i].x + 50;
            
        }
        if(bug[i].x > SCREEN_W  ) {
            bug[i].x = -100;
            bug[i].x_c = bug[i].x + 50;
        }
        if(bug[i].y < -100) { 
            bug[i].y = SCREEN_HG ;
            bug[i].y_c = bug[i].y + 50;
        }
        if(bug[i].y > SCREEN_HG ) {
            bug[i].y = - 100;
            bug[i].y_c = bug[i].y + 50;
        }
    
}

//----------------------------------------------------------------------
//THREAD FUNCTIONS
//----------------------------------------------------------------------

void* bugs_task(void *arg){

    int i;                                          //task index
    float dt;                                       //integration interval    
    i = get_task_index(arg);
    printf("bug task index is %d\n", i);
    wait_for_activation(i);
    init_bug(i);
    dt = TSCALE *(float)task_period(i)/1000;        //passo di integrazione in ms
    
    float da;
    float dv;
    float vx_fix;                                   //componente x velocità nel frame fisso
    float vy_fix;                                   //componente y velocità nel frame fisso
    float v;                                        //modulo velocità
   
    
    while( end == 0 && bug[i].alive == 1){
        
        bugs_interactions(i);                           //gestione collisioni con gli altri bug
        
        bug_rand_move(i, dt);

        //forze tra bug

        apply_force(i, dt);                             //repulsione o attrazione a seconda del genere
        attract_to_food(i);

        update_bug(i, dt);                              //aggiorna la velocità e la posizione del bug e l' accelerazione
       
        pacman(i);                                      //gestione collisioni con i bordi con effetto pacman
        //handle_boundary(i);                              //gestione collisioni con i bordi effetto pallina
        //printf("Il bug %d  è un figlio se è 1, è un figlio? %d\n", i, bug[i].son);
        int j; 
        for( j = 1; j <= nab; j++){
            if(i != j) {
                if(inside(i, j)) {
                    
                    //printf("Il bug %d vede il bug %d\n", i, j);
                    counter(i);   
                    //printf("Il bug %d vede il bug %d da %f sec\n", i, j, count[i][j]);
                }
                else{
                    counter(i);
                    //printf("Il bug %d non vede il bug %d, il counter è a  %f\n", i, j, count[i][j]);
                } 
                reproduction(i);
            }
        }

        food(i);                                       //gestione del cibo
        age(i);                                        //gestione dell' età
        death(i);                                      //gestione della morte
        

        if(deadline_miss(i)){
            printf("deadline miss del task %d\n", i);
            dmc++;
        }


        /*int j;
        for (i = 0; i < 50; i++) {
            for (j = 0; j < 50; j++) {
                printf("count[%d][%d] = %d\n", i, j, count[i][j]);
            }
        }*/
        

        wait_for_period(i); 

    }
    printf("Il task %d è terminato\n", i);
    
   
}

void* display(void *arg){

    int d;                      //task index del task display, è uno solo e dovrebbe essere il numero 52
    d = get_task_index(arg);
    int i;
    printf("display task index is %d\n", d);// dovrebbe essere 52
    wait_for_activation(d);
    struct timespec now;
    double timeprec = 0;

    rectfill(screen, 0, 0, SCREEN_W, SCREEN_H, makecol(0,0,0));
    clear_to_color(screen, makecol(0,0,0));
    white_back_pane();
    int cicd;
    cicd = 0;

    while( end == 0 ) {

        
        
        
     
        show_mouse(screen);
        
        
        rectfill(buffergame, 0, 0, SCREEN_W, SCREEN_H, makecol(0,0,0));
        
        //Disegnar i riquadri
        draw_VE();
        draw_legend();
        
        show_mouse(NULL); 
        for( i = 1; i < nab + 1 ; i++ ) {
            
            if(bug[i].alive == 1) {
                draw_bug(i);
             }
                    
        }

        for( i = 1; i < totfood + 1 ; i++ ) {
            if(pane[i].eaten == 0 && pane[i].active == 1) {
               
               draw_food();
            
            }

        }
        
        show_mouse(screen);
        
        blit(buffergame, screen, 5, 5, 5 ,SCREEN_HUI + 10 , buffergame->w - 8, buffergame->h - 5);  //double buffering
        blit(bufferUI, screen, 0,0,0, 0, bufferUI->w, bufferUI->h);

        if(deadline_miss(d)){
            printf("deadline miss del task %d\n", d);
            dmc++;
        }
       
        wait_for_period(d);
        //printf("Il numero di cicli di display è : %d\n", cicd);
        cicd++;
    }



    destroy_bitmap(buffergame);
    destroy_bitmap(bufferUI);
    show_mouse(NULL);
    
    for(i = 1; i <= nab; i++){
        destroy_bitmap(bug[i].bmp);
    }
    for(i = 1; i <= totfood; i++){
        if(pane[i].bmp != NULL) {
            destroy_bitmap(pane[i].bmp);
        }
    }
    

}



void* user(void *arg){
   
    int u;                                      //task index del task user, è uno solo e dovrebbe essere il numero 51
    u = get_task_index(arg);
    printf("index del task  user is %d\n", u);  // dovrebbe essere 51      
    printf(("User task period is %d\n"), task_period(u) );
    wait_for_activation(u);
    float timeprec = 0;

    struct timespec now;
    int j;                                      //indice per il for cibo
    int err_bugs;
    char scan;
    int temp;                                   //variabile temporanea per la gestione del numero di bug
    int i;                                      //indice per il for bug
    int tempf;                                  //variabile temporanea per inserire cibi
    tempf = 1;
    int cicu;
    cicu = 0;

    do{
        
        
        
        scan = get_scancode();
    
        switch (scan) {

            case KEY_SPACE:

            

                if( nlb < MAX_BUGS) {
                    
                    
                    temp = 0;
                    for(i = 1; i <= nlb ; i++) {
                        if(bug[i].alive == 0) {
                            printf("Il bug %d è morto\n", i);
                            temp = i;
                            break;
                             
                        }
                    }
                    if(temp == 0) {         // se è 0 non è morto nessuno
                        temp = nlb + 1;
                        
                    }
                    err_bugs = task_create(bugs_task, temp, 40, 40, 90, ACT);
                   

                    if( err_bugs == 0 ) {
                        printf("task %d  creato correttamente\n", temp);
                        //printf("numero di bug attivi è %d\n", nlb);
                        if( temp == nlb +1) {
                            nab++;
                            nlb++;
                        }
                        else {
                            nlb++;
                        }
                        
                        printf("numero di bugs attivi è %d\n", nlb);
                        
                        
                    }
                    else{
                        printf("task %d  non creato", temp);
                    }
                    
                }
                else printf("numero massimo di bugs raggiunto\n");

                break;

            case KEY_UP:

                for(i = 1; i <= nab; i++) {
                    if(bug[i].vis == 0) {

                    bug[i].vis = 1;

                    }
                    else bug[i].vis = 0;

                }
                

                break;
            
            case KEY_DOWN:

                 for(i = 1; i <= nab; i++) {

                    if(bug[i].ingombro == 0 ) bug[i].ingombro = 1;
                    else bug[i].ingombro = 0;
                    
                }
                
                break;
            
            default : break;


        }
       
        
        
        switch(mouse_b) {
            
            

            case 1:

                
                if(totfood < MAX_NUM_FOOD && tempf >= 2) {
                
                tempf = 0;

                init_food(totfood + 1, mouse_x, mouse_y);
                totfood++;
                printf("è stato creato il cibo %d, Il numero di cibi attivi è %d \n", totfood, getActiveFoodCount());

                
                
                //printf("é stato creato il cibo %d, numero di cibi attivi è %d\n",temp, naf);
                
                
                
                }

                else if(totfood >= MAX_NUM_FOOD) {
                    printf("numero massimo di cibo raggiunto\n");
                }
                
                
                else {

                    //printf("tempf vale %d\n",tempf);
                    printf("Il numero di cibi attivi è %d \n", getActiveFoodCount());
                
                }
                

                break;

            default: break;


        }
     
        

    tempf ++;
        
        
    if(deadline_miss(u)){
            printf("deadline miss del task %d\n", u);
            dmc++;
        }
   
    //printf("Il numero di cicli di user è : %d\n", cicu);
    cicu++;
    wait_for_period(u);

    } while ( scan != KEY_ESC);
         
    end = 1;

}

