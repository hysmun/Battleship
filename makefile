.SILENT:

GRILLESDL=GrilleSDL
RESSOURCES=Ressources
ECRAN=Ecran
MQ=MessageQueue

CC = g++ -m64 -DLINUX -DTRACE -I$(MQ) -I$(ECRAN) -I$(GRILLESDL) -I$(RESSOURCES) -Wall
OBJS = $(MQ)/MessageQueue.o $(MQ)/Message.o $(MQ)/MessageQueueException.o $(GRILLESDL)/GrilleSDL.o $(RESSOURCES)/Ressources.o $(ECRAN)/Ecran.o utils.o
PROGRAMS = BattleShip Serveur
TMP = "default"

ALL:
	clear
	clear
	echo ALL ...
	make BattleShip
	make Serveur

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
	echo Creation de Ecran.o ...
	$(CC) $(ECRAN)/Ecran.cpp -c
	mv Ecran.o $(ECRAN)/Ecran.o

$(MQ)/MessageQueue.o:	$(MQ)/MessageQueue.cpp $(MQ)/MessageQueue.h
	echo Creation de MessageQueue.o ...
	$(CC) $(MQ)/MessageQueue.cpp -c
	mv MessageQueue.o $(MQ)/MessageQueue.o

$(MQ)/Message.o:	$(MQ)/Message.cpp $(MQ)/Message.h
	echo Creation de Message.o ...
	$(CC) $(MQ)/Message.cpp -c
	mv Message.o $(MQ)/Message.o

$(MQ)/MessageQueueException.o:	$(MQ)/MessageQueueException.cpp $(MQ)/MessageQueueException.h
	echo Creation de MessageQueueException.o ...
	$(CC) $(MQ)/MessageQueueException.cpp -c
	mv MessageQueueException.o $(MQ)/MessageQueueException.o

utils.o: utils.cpp utils.h
	echo Creation de utils.o ...
	$(CC) utils.cpp -c -o utils.o

clean:
	clear
	clear
	echo Clean ...
	@rm -f $(OBJS) core

clobber:	clean
	clear
	clear
	echo Clobber ...
	@rm -f tags $(PROGRAMS)

git:
	clear
	clear
	git add -A
	git commit -m "$(TMP)"
	git push
	
ipc:
	clear
	clear
	echo ipc ... 
	ipcrm -Q 0x3e8

