#name Bugs_main assigned to variable main
MAIN = test_Bugs_main
#dependencies and command for the executable

$(MAIN): 	$(MAIN).o BugsLifeTest.o
		gcc -o $(MAIN) $(MAIN).o BugsLifeTest.o Ptask.o `allegro-config --libs` -lm -lpthread
	   
#dependencies and command for the main programme

$(MAIN).o:	$(MAIN).c
		gcc -c $(MAIN).c
	
#dependencies and command for the library

BugsLife.o:	BugsLifeTest.c
		gcc -c BugsLifeTest.c
		
Ptask.o: Ptask.c
		gcc -c Ptask.c
			
