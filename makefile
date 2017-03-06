.SILENT:

GRILLESDL=GrilleSDL
RESSOURCES=Ressources
ECRAN=Ecran
MQ=MessageQueue

CC = g++ -m64 -DSUN -DTRACE -I$(MQ) -I$(ECRAN) -I$(GRILLESDL) -I$(RESSOURCES) 
OBJS = $(MQ)/MessageQueue.o $(MQ)/Message.o $(MQ)/MessageQueueException.o $(GRILLESDL)/GrilleSDL.o $(RESSOURCES)/Ressources.o $(ECRAN)/Ecran.o
PROGRAMS = BattleShip Serveur

ALL: $(PROGRAMS)

BattleShip:	BattleShip.cpp protocole.h $(OBJS)
	echo Creation de BattleShip...
	$(CC) BattleShip.cpp -o BattleShip $(OBJS) -lrt -lpthread -lSDL

Serveur:	Serveur.cpp protocole.h $(OBJS)
		echo Creation de Serveur...
		$(CC) Serveur.cpp -o Serveur $(OBJS) -lrt -lpthread -lSDL

$(GRILLESDL)/GrilleSDL.o:	$(GRILLESDL)/GrilleSDL.c $(GRILLESDL)/GrilleSDL.h
		echo Creation de GrilleSDL.o ...
		$(CC) -c $(GRILLESDL)/GrilleSDL.c
		mv GrilleSDL.o $(GRILLESDL)

$(RESSOURCES)/Ressources.o:	$(RESSOURCES)/Ressources.c $(RESSOURCES)/Ressources.h
		echo Creation de Ressources.o ...
		$(CC) -c $(RESSOURCES)/Ressources.c
		mv Ressources.o $(RESSOURCES)

$(ECRAN)/Ecran.o:	$(ECRAN)/Ecran.cpp $(ECRAN)/Ecran.h
		$(CC) $(ECRAN)/Ecran.cpp -c
		mv Ecran.o $(ECRAN)/Ecran.o

$(MQ)/MessageQueue.o:	$(MQ)/MessageQueue.cpp $(MQ)/MessageQueue.h
			$(CC) $(MQ)/MessageQueue.cpp -c
			mv MessageQueue.o $(MQ)/MessageQueue.o

$(MQ)/Message.o:	$(MQ)/Message.cpp $(MQ)/Message.h
			$(CC) $(MQ)/Message.cpp -c
			mv Message.o $(MQ)/Message.o

$(MQ)/MessageQueueException.o:	$(MQ)/MessageQueueException.cpp $(MQ)/MessageQueueException.h
				$(CC) $(MQ)/MessageQueueException.cpp -c
				mv MessageQueueException.o $(MQ)/MessageQueueException.o

clean:
	@rm -f $(OBJS) core

clobber:	clean
	@rm -f tags $(PROGRAMS)
