#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include "Ecran.h"
#include "GrilleSDL.h"
#include "Ressources.h"
#include "MessageQueue.h"

#include "protocole.h"
#include "utils.h"

// Dimensions de la grille de jeu
#define NB_LIGNES			10
#define NB_COLONNES 		10

// Tableau de jeu (mer)
int   tab[NB_LIGNES][NB_COLONNES]={{0}};
int   tabTir[NB_LIGNES][NB_COLONNES]={{0}};

pid_t pidServeur = 0;
pid_t pid=getpid();
pthread_t tidEvent=0;
pthread_t tidReception=0;
pthread_t tidScore = 0;

int flagSousMarin=1;

pthread_mutex_t mutexTabTir;
pthread_mutex_t mutexScore;
pthread_cond_t condScore;

int score = 0;
int MAJScore = 1;

MessageQueue  connexion;  // File de messages

void *fctThEvent(void *p);
void *fctThAffBateau(void *p);
void *fctThReception(void *p);
void *fctThAfficheBateauCoule(void *p);
void *fctThScore(void *p);

int DessineFullBateau(Bateau *pBateau, int opt);




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
	DessineBoutonSousMarin(10, 0, VERT);
	//Init mutexTabTir
	pthread_mutex_init(&mutexTabTir,NULL);
	pthread_mutex_init(&mutexScore, NULL);	
	pthread_cond_init(&condScore, NULL);

	pthread_create(&tidEvent, NULL, fctThEvent, NULL );
	pthread_create(&tidReception, NULL, fctThReception, NULL);
	pthread_create(&tidScore, NULL, fctThScore, NULL);
	
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
				pthread_mutex_lock(&mutexScore);
				if(event.colonne < 3 && event.ligne == 10 && flagSousMarin == 1 && score > 0)
				{
					flagSousMarin = 0;
					Trace("Demande sous marrin\n");
					kill(pidServeur, SIGUSR1);
					DessineBoutonSousMarin(10, 0, ORANGE);
					score -= 1;
					MAJScore=1;
				}
				pthread_mutex_unlock(&mutexScore);
				pthread_cond_signal(&condScore);
				if(event.ligne > 10)
				{
					Trace("demande de tir !\n");
					pthread_mutex_lock(&mutexTabTir);
					if(tabTir[event.ligne-11][event.colonne] != 0)
					{
						//on essaye deja de tirer ici
						Trace("Position deja verouiller");
					}
					else
					{
						//on peut tirer ici 
						Trace("Position verouiller");
						tabTir[event.ligne-11][event.colonne] = 1;
						DessineCible(event.ligne, event.colonne);
						RequeteTir reqTir;
						reqTir.L = event.ligne - 11;
						reqTir.C = event.colonne; 
						Message requete(1,TIR,(char*)&reqTir,sizeof(RequeteTir)); // type = 1 --> a destination du Serveur
						connexion.SendData(requete);
						
					}
					pthread_mutex_unlock(&mutexTabTir);
				}
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

void *fctThAffBateau(void *p)
{
	sigset_t blockSet;
	//construction set
	sigfillset(&blockSet);
	sigprocmask(SIG_SETMASK, &blockSet, NULL);
	
	Bateau *pBateau = (Bateau *)p;
	
	waitTime(1, 0);
	//affichage bateau
	for(int i=0; i<pBateau->type; i++)
	{
		if(pBateau->direction == HORIZONTAL)
		{
			DessineBateau(pBateau->L%NB_LIGNES+11, (pBateau->C+i)%NB_COLONNES, pBateau->type, HORIZONTAL,i);
		}
		else
		{
			DessineBateau((pBateau->L+i)%NB_LIGNES+11, pBateau->C%NB_COLONNES, pBateau->type, VERTICAL,i);
		}
	}
	//attente 4 second
	waitTime(4, 0);
	//enleve bateau
	for(int i=0; i<pBateau->type; i++)
	{
		if(pBateau->direction == HORIZONTAL)
		{
			EffaceCarre(pBateau->L%NB_LIGNES+11, (pBateau->C+i)%NB_COLONNES);
		}
		else
		{
			EffaceCarre((pBateau->L+i)%NB_LIGNES+11, pBateau->C%NB_COLONNES);
		}
	}
	waitTime(30, 0);
	flagSousMarin = 1;
	pthread_exit(0);
}

void *fctThReception(void *p)
{
	// Bloquer les signaux
	sigset_t maskAll;
	sigfillset(&maskAll);
	sigprocmask(SIG_SETMASK, &maskAll,NULL);
	
	Message requete;
	Bateau bSousMarin;
	pthread_t tidAffBateau;
	ReponseTir tmpRepTir;
	pthread_t tid;
	
	while(1)
	{
		requete = connexion.ReceiveData(getpid());
		switch(requete.getRequete())
		{
			case SOUSMARIN:
				Trace("Reponse sous-marin");
				memcpy(&bSousMarin,requete.getData(),sizeof(Bateau));
				// Création  thread AfficheBateau avec bSousMarin en paramètre
				pthread_create(&tidAffBateau,NULL,fctThAffBateau,&bSousMarin);
				break;
			case TIR:
			{
				Trace("Reponse TIR");
				memcpy(&tmpRepTir, requete.getData(), sizeof(ReponseTir));
				switch(tmpRepTir.status)
				{
					case PLOUF:
						Trace("Plouf");
						pthread_mutex_lock(&mutexTabTir);
						tabTir[tmpRepTir.L][tmpRepTir.C] = 0;
						pthread_mutex_unlock(&mutexTabTir);
						break;
					case TOUCHE:
						Trace("Touche");
						EffaceCarre(tmpRepTir.L+11, tmpRepTir.C);
						DessineExplosion(tmpRepTir.L+11, tmpRepTir.C, ORANGE);
						pthread_mutex_lock(&mutexScore);
						score+=1;
						MAJScore=1;
						pthread_mutex_unlock(&mutexScore);
						pthread_cond_signal(&condScore);
						break;
					case DEJA_TOUCHE:
						Trace("Deja Touche");
						EffaceCarre(tmpRepTir.L+11, tmpRepTir.C);
						DessineExplosion(tmpRepTir.L+11, tmpRepTir.C, BLEU);
						break;
					case LOCKED:
						Trace("locked");
						EffaceCarre(tmpRepTir.L, tmpRepTir.C);
						DessineCibleVerrouillee(tmpRepTir.L+11, tmpRepTir.C);
						waitTime(0, 500000000);
						EffaceCarre(tmpRepTir.L+11, tmpRepTir.C);
						pthread_mutex_lock(&mutexTabTir);
						tabTir[tmpRepTir.L][tmpRepTir.C] = 0;
						pthread_mutex_unlock(&mutexTabTir);
						break;
					case COULE:
						Trace("Coule");
						pthread_create(&tid, NULL, fctThAfficheBateauCoule, &(tmpRepTir.bateau));
						pthread_mutex_lock(&mutexScore);
						score+=2;
						MAJScore=1;
						pthread_mutex_unlock(&mutexScore);
						pthread_cond_signal(&condScore);
						break;
					default:
						Trace("Erreur switch reponse tir");
						break;
				}
				break;
			}
			default:
				Trace("Erreur switch requete bateau");
				break;
		}
	}
	pthread_exit(0);
}

void *fctThAfficheBateauCoule(void *p)
{
	// Bloquer les signaux
	sigset_t maskAll;
	sigfillset(&maskAll);
	sigprocmask(SIG_SETMASK, &maskAll,NULL);
	
	
	Bateau pBateau;
	memcpy((void *)&pBateau, (Bateau *)p, sizeof(Bateau));
	int i;
	for(i=0; i< pBateau.type; i++)
	{
		if(pBateau.direction == HORIZONTAL)
		{
			DessineBateau(pBateau.L%NB_LIGNES+11, (pBateau.C+i)%NB_COLONNES, pBateau.type, HORIZONTAL,i);
			DessineExplosion(pBateau.L%NB_LIGNES+11, (pBateau.C+i)%NB_COLONNES, ORANGE);
		}
		else
		{
			DessineBateau((pBateau.L+i)%NB_LIGNES+11, pBateau.C%NB_COLONNES, pBateau.type, VERTICAL,i);
			DessineExplosion((pBateau.L+i)%NB_LIGNES+11, pBateau.C%NB_COLONNES,ORANGE);
		}
	}
	
	pthread_mutex_lock(&mutexTabTir);
	for(i=0; i< pBateau.type; i++)
	{
		if(pBateau.direction == HORIZONTAL)
		{
			tabTir[pBateau.L%NB_LIGNES+11][(pBateau.C+i)%NB_COLONNES] = 0;
		}
		else
		{
			tabTir[(pBateau.L+i)%NB_LIGNES+11][pBateau.C%NB_COLONNES] = 0;
		}
	}
	pthread_mutex_unlock(&mutexTabTir);
	pthread_exit(0);
}

void *fctThScore(void *p)
{
	// Bloquer les signaux
	sigset_t maskAll;
	sigfillset(&maskAll);
	sigprocmask(SIG_SETMASK, &maskAll,NULL);
	
	int modulo = 100;
	while(1)
	{
		pthread_mutex_lock(&mutexScore);
		while(!MAJScore)
			pthread_cond_wait(&condScore, &mutexScore);
		//score mis a jour
		for(int i =0; i<3; i++)
		{
			EffaceCarre(10, 7+i);
			DessineChiffre(10, 7+i, score == 0 ? 0 : (int)(score/modulo));
			modulo /= 10;
		}
		modulo = 100;
		MAJScore = 0;
		pthread_mutex_unlock(&mutexScore);
	}
	
	pthread_exit(0);
}


int DessineFullBateau(Bateau *pBateau, int opt)
{
	if(opt == DRAW)
	{
		for(int i=0; i<pBateau->type; i++)
		{
			if(pBateau->direction == HORIZONTAL)
			{
				DessineBateau(pBateau->L%NB_LIGNES, (pBateau->C+i)%NB_COLONNES, pBateau->type, HORIZONTAL,i);
				tab[pBateau->L%NB_LIGNES][(pBateau->C+i)%NB_COLONNES] = pthread_self();
			}
			else
			{
				DessineBateau((pBateau->L+i)%NB_LIGNES, pBateau->C%NB_COLONNES, pBateau->type, VERTICAL,i);
				tab[(pBateau->L+i)%NB_LIGNES][pBateau->C%NB_COLONNES] = pthread_self();
			}
		}
	}
	if(opt == CLEAR)
	{
		for(int i=0; i<pBateau->type; i++)
		{
			if(pBateau->direction == HORIZONTAL)
			{
				EffaceCarre(pBateau->L%NB_LIGNES, (pBateau->C+i)%NB_COLONNES);
				tab[pBateau->L%NB_LIGNES][(pBateau->C+i)%NB_COLONNES] = 0;
			}
			else
			{
				EffaceCarre((pBateau->L+i)%NB_LIGNES, pBateau->C%NB_COLONNES);
				tab[(pBateau->L+i)%NB_LIGNES][pBateau->C%NB_COLONNES] = 0;
			}
		}
	}
	
	return 1;
}

















