#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "Ecran.h"
#include "GrilleSDL.h"
#include "Ressources.h"
#include "MessageQueue.h"

#include "protocole.h"
#include "utils.h"

// Dimensions de la grille de jeu
#define NB_LIGNES   10
#define NB_COLONNES 10

// Tableau de jeu (mer)
int   tab[NB_LIGNES][NB_COLONNES]={{0}};
int   tabTir[NB_LIGNES][NB_COLONNES]={{0}};

pid_t pidServeur = 0;
pid_t pid=getpid();
pthread_t tidEvent=0;
pthread_t tidReception=0;

MessageQueue  connexion;  // File de messages

void *fctThEvent(void *p);
void *fctReception(void *p);

//************************************************************************************************
int main(int argc,char* argv[])
{
	srand((unsigned)time(NULL));
	Trace("Je suis joueur : %d", pid);
	// Connexion au serveur
	try
	{
		connexion.connect(1000);
	}
	catch(MessageQueueException e)
	{
		printf("Serveur hors-ligne\n");
		exit(1);
	}

	//requete connexion
	Message reception, envois;
	envois.setType(1);
	envois.setRequete(CONNECT);
	connexion.SendData(envois);

	//reception pid serveur
	reception = connexion.ReceiveData(pid);
	if(reception.getRequete() == CONNECT)
		pidServeur = reception.getExpediteur();
	else
	{
		Trace("Erreur connexion ");
		exit(0);
	}
	
	Trace("%d Serveur connecter : %d", pid,  pidServeur);

	// Ouverture de la fenetre graphique
	Trace("(THREAD MAIN %d) Ouverture de la fenetre graphique",pthread_self()); fflush(stdout);
	if (OuvertureFenetreGraphique("client") < 0)
	{
		Trace("Erreur de OuvrirGrilleSDL\n");
		exit(1);
	}

	pthread_create(&tidEvent, NULL, fctThEvent, NULL );
	pthread_create(&tidReception, NULL, fctReception, NULL);
	
	pthread_join(tidEvent, NULL);
	// Fermeture de la grille de jeu (SDL)
	Trace("(THREAD MAIN %d) Fermeture de la fenetre graphique...",pthread_self()); fflush(stdout);
	FermetureFenetreGraphique();
	Trace("(THREAD MAIN %d) OK Fin",pthread_self()); //fflush(stdout);

	exit(0);
}


void *fctThEvent(void *p)
{
	EVENT_GRILLE_SDL event;
	bool ok = false;
	while(!ok)
	{
		event = ReadEvent();
		switch(event.type)
		{
			case CROIX:
			{
				Trace("Fin joueur %d", pid);
				Message requete(1, DECONNECT, NULL, 0);
				connexion.SendData(requete);
				ok = 1;
				break;
			}
			case CLIC_GAUCHE:
			{
				// Envoi d'une requete au Serveur
				RequeteTir reqTir;
				reqTir.L = event.ligne - 11;
				reqTir.C = event.colonne; 
				Message requete(1,TIR,(char*)&reqTir,sizeof(RequeteTir)); // type = 1 --> a destination du Serveur
				connexion.SendData(requete);

				// Attente de la reponse du serveur
				Message reponse; 
				reponse = connexion.ReceiveData(getpid());
				ReponseTir repTir;
				memcpy(&repTir,reponse.getData(),sizeof(ReponseTir));

				if (repTir.status == TOUCHE) 
					DessineExplosion(repTir.L+11,repTir.C,ORANGE);
				else 
					DessineCible(event.ligne,event.colonne);
				break;
			}
			default:
			{
				//rien
			}
		}
	}
	pthread_exit(0);
}

void *fctReception(void *p)
{
	// Bloquer les signaux
	sigset_t maskAll;
	sigfillset(&maskAll);
	sigprocmask(SIG_SETMASK, &maskAll,NULL);
	
	Message requeteSousMarin;
	Bateau bSousMarin;
	pthread_t tidAffBateau;
	
	while(1)
	{
		requeteSousMarin = connexion.ReceiveData(getpid());
		switch(requeteSousMarin.getRequete())
		{
			case SOUSMARIN:
				memcpy(&bSousMarin,requeteSousMarin.getData(),sizeof(Bateau));
				// Création  thread AfficheBateau avec bSousMarin en paramètre
				pthread_create(&tidAffBateau,NULL,fctThAffBateau,bSousMarin);
				break;
			default:
				break;
		}
	}
}



















