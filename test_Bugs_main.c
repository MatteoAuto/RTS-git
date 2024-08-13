
#include "BugsLifeTest.h"


extern int nab;                //numero di bug attivi, la variabile è definita in BugsLife.c quindi è una global extern
                               //va richiamata fuori da tutte le funzioni




void main(void) {

    srand(time(NULL));   //inizializzazione del generatore di numeri casuali
    int i;
    init();
    wait_for_task_end(51);
    wait_for_task_end(52);
    
    
    nab = 0;

    for(i = 1; i < (nab + 1) ; i++){      
        
        printf("sta aspettato la fine del task %d per proseguire col main\n", i);
        wait_for_task_end(i);
        printf("task %d terminato\n", i);
    }
     
   
    allegro_exit();
    printf("Tutti i task sono terminati\n");



    
    
    
}

