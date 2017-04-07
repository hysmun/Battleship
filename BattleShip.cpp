/**********************************
*	Programmation UNIX - Threads
*	Equipe Brajkovic - Mauhin
*	HEPL 2016-2017
***********************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include "Ecran.h"
#include "GrilleSDL.h"
#include "Ressources.h"
#include "MessageQueue.h"

#include "protocole.h"
#include "utils.h"

// Dimensions de la grille de jeu
#define NB_LIGNES			10
#define NB_COLONNES 		10

#define NB_CROISEURS 2
#define NB_DESTOYERS 3
#define NB_CUIRASSES 1
#define NB_TORPILLEURS 4

#define NB_BATEAUX (NB_CROISEURS + NB_CUIRASSES + NB_DESTOYERS + NB_TORPILLEURS)

// Tableau de jeu (mer)
long tab[NB_LIGNES][NB_COLONNES]={{0}};
long tabTir[NB_LIGNES][NB_COLONNES]={{0}};
int lignes[NB_LIGNES]={0};
int colonnes[NB_COLONNES]={0};

pid_t pidServeur = 0;
pid_t pid=getpid();
pthread_t tidEvent=0;
pthread_t tidReception=0;
pthread_t tidScore = 0;
pthread_t tidIA=0;
pthread_t tidAmiral;
pthread_t tid;

int nbCuirasses=0, nbCroiseurs=0, nbDestoyers=0, nbTorpilleurs=0;
int nbBateaux=0;
int nbVerticaux =0, nbHozizontaux=0;

pthread_key_t cleBateau;
pthread_key_t cleComBateau;
ComBateau comBateau[NB_BATEAUX];

int flagSousMarin=1;

pthread_mutex_t mutexTabTir;
pthread_mutex_t mutexScore;
pthread_cond_t condScore;
pthread_mutex_t mutexBateau;
pthread_cond_t condBateaux;
pthread_mutex_t mutexComBateau[NB_BATEAUX];
pthread_mutex_t mutexMer;

int score = 0;
int MAJScore = 1;

MessageQueue  connexion;  // File de messages

void *fctThEvent(void *p);
void *fctThAffBateau(void *p);
void *fctThReception(void *p);
void *fctThAfficheBateauCoule(void *p);
void *fctThScore(void *p);
void *fctThAmiral(void *p);
void *fctThBateau(void *);
void *fctThIA(void *);

void HandlerSIGUSR1(int sig, siginfo_t *info, void *p);
void HandlerSIGUSR1(int sig);
void HandlerSIGUSR2(int sig, siginfo_t *info, void *p);

int searchPosBateau(Bateau *pBateau);
int DessineFullBateau(Bateau *pBateau, int opt);
int deplacementBateau(Bateau *pBateau);
int searchPosBateau2(Bateau *pBateau);



/***********************************
*
*		Section MAIN (_main)
*
************************************
*/
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
	Trace("(THREAD MAIN %ld) Ouverture de la fenetre graphique",pthread_self()); fflush(stdout);
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
	pthread_mutex_init(&mutexMer,NULL);
	pthread_mutex_init(&mutexBateau, NULL);	
	pthread_cond_init(&condBateaux, NULL);
	
	//clé variable spécifique
  	pthread_key_create(&cleBateau, NULL);
  	pthread_key_create(&cleComBateau, NULL);
	
	for(int i = 0; i< NB_BATEAUX; i++)
	{
		pthread_mutex_init(&mutexComBateau[i], NULL);
	}

	pthread_create(&tidEvent, NULL, fctThEvent, NULL );
	pthread_create(&tidReception, NULL, fctThReception, NULL);
	pthread_create(&tidScore, NULL, fctThScore, NULL);
	pthread_create(&tidAmiral, NULL, fctThAmiral, NULL);
	pthread_create(&tidIA, NULL, fctThIA, NULL);
	
	pthread_join(tidEvent, NULL);
	// Fermeture de la grille de jeu (SDL)
	Trace("(THREAD MAIN %ld) Fermeture de la fenetre graphique...",pthread_self()); fflush(stdout);
	FermetureFenetreGraphique();
	Trace("(THREAD MAIN %ld) OK Fin",pthread_self()); //fflush(stdout);

	exit(0);
}

/***********************************
*
*		Section EVENT (_event)
*
************************************
*/
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
					Trace("Demande de tir !");
					pthread_mutex_lock(&mutexTabTir);
					if(tabTir[event.ligne-11][event.colonne] != 0)
					{
						//on essaye deja de tirer ici
						Trace("Position deja verrouillee");
					}
					else
					{
						//on peut tirer ici 
						Trace("Position verrouillee");
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

/***********************************
*
*		Section AfficheBATEAU (_affbat)
*
************************************
*/
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
			EffaceCarre(pBateau->L%NB_LIGNES+11, (pBateau->C+i)%NB_COLONNES);
			DessineBateau(pBateau->L%NB_LIGNES+11, (pBateau->C+i)%NB_COLONNES, pBateau->type, HORIZONTAL,i);
			
			if(tabTir[pBateau->L%NB_LIGNES][pBateau->C] == TOUCHE)
				DessineExplosion(pBateau->L%NB_LIGNES+11, pBateau->C, ORANGE);
			if(tabTir[pBateau->L%NB_LIGNES][pBateau->C] == DEJA_TOUCHE)
				DessineExplosion(pBateau->L%NB_LIGNES+11, pBateau->C, BLEU);
		}
		else
		{
			EffaceCarre(pBateau->L%NB_LIGNES+11, (pBateau->C+i)%NB_COLONNES);
			DessineBateau((pBateau->L+i)%NB_LIGNES+11, pBateau->C%NB_COLONNES, pBateau->type, VERTICAL,i);
			
			if(tabTir[(pBateau->L+i)%NB_LIGNES][pBateau->C] == TOUCHE)
				DessineExplosion((pBateau->L+i)%NB_LIGNES+11, pBateau->C, ORANGE);
			if(tabTir[pBateau->L][pBateau->C] == DEJA_TOUCHE)
				DessineExplosion((pBateau->L+i)%NB_LIGNES+11, pBateau->C, BLEU);
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
			
			if(tabTir[pBateau->L%NB_LIGNES][pBateau->C] == TOUCHE)
				DessineExplosion(pBateau->L%NB_LIGNES+11, pBateau->C, ORANGE);
			if(tabTir[pBateau->L%NB_LIGNES][pBateau->C] == DEJA_TOUCHE)
				DessineExplosion(pBateau->L%NB_LIGNES+11, pBateau->C, BLEU);
		}
		else
		{
			EffaceCarre((pBateau->L+i)%NB_LIGNES+11, pBateau->C%NB_COLONNES);
			
			if(tabTir[(pBateau->L+i)%NB_LIGNES][pBateau->C] == TOUCHE)
				DessineExplosion((pBateau->L+i)%NB_LIGNES+11, pBateau->C, ORANGE);
			if(tabTir[pBateau->L][pBateau->C] == DEJA_TOUCHE)
				DessineExplosion((pBateau->L+i)%NB_LIGNES+11, pBateau->C, BLEU);
		}
		
	}
	waitTime(30, 0);
	flagSousMarin = 1;
	pthread_exit(0);
}

/***********************************
*
*		Section RECEPTION (_recep)
*
************************************
*/
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
				//Trace("Reponse TIR");
				memcpy(&tmpRepTir, requete.getData(), sizeof(ReponseTir));
				switch(tmpRepTir.status)
				{
					case PLOUF:
						Trace("Plouf !!  ");
						pthread_mutex_lock(&mutexTabTir);
						EffaceCarre(tmpRepTir.L+11,tmpRepTir.C);
						tabTir[tmpRepTir.L][tmpRepTir.C] = 0;
						pthread_mutex_unlock(&mutexTabTir);
						break;
					case TOUCHE:
						Trace("Touche  pos :   %d   --   %d  ",tmpRepTir.L+11, tmpRepTir.C );
						EffaceCarre(tmpRepTir.L+11, tmpRepTir.C);
						DessineExplosion(tmpRepTir.L+11, tmpRepTir.C, ORANGE);
						tabTir[tmpRepTir.L][tmpRepTir.C] = TOUCHE;
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
						tabTir[tmpRepTir.L][tmpRepTir.C] = DEJA_TOUCHE;
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
			case BATEAU_COULE:
				Trace("Bateau coule !!");
				memcpy(&tmpRepTir, requete.getData(), sizeof(ReponseTir));
				pthread_create(&tid, NULL, fctThAfficheBateauCoule, &(tmpRepTir.bateau));
				break;
			default:
				Trace("Erreur switch requete bateau");
				break;
		}
	}
	pthread_exit(0);
}

/***********************************
*
*		Section AfficheBatCOULE (_affbatcoule)
*
************************************
*/
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
	waitTime(3,0);
	pthread_mutex_lock(&mutexTabTir);
	for(i=0; i< pBateau.type; i++)
	{
		if(pBateau.direction == HORIZONTAL)
		{
			tabTir[(pBateau.L%NB_LIGNES)][(pBateau.C+i)%NB_COLONNES] = 0;
			EffaceCarre((pBateau.L%NB_LIGNES)+11,(pBateau.C+i)%NB_COLONNES);
		}
		else
		{
			tabTir[((pBateau.L+i)%NB_LIGNES)][pBateau.C%NB_COLONNES] = 0;
			EffaceCarre(((pBateau.L+i)%NB_LIGNES)+11,pBateau.C%NB_COLONNES);
		}
	}
	pthread_mutex_unlock(&mutexTabTir);
	pthread_exit(0);
}

/***********************************
*
*		Section SCORE (_score)
*
************************************
*/
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
		//Trace("Maj score %d", score);
		for(int i =0; i<3; i++)
		{
			EffaceCarre(10, 7+i);
			DessineChiffre(10, 7+i, score == 0 ? 0 : ((int)(score/modulo))%10);
			modulo /= 10;
		}
		modulo = 100;
		MAJScore = 0;
		//Trace("fin maj score");
		pthread_mutex_unlock(&mutexScore);
	}
	
	pthread_exit(0);
}

/***********************************
*
*		Section AMIRAL (_ami)
*
************************************
*/
void *fctThAmiral(void *p)
{
	Bateau *pBateau;
	
	// Bloquer tous les signaux
	sigset_t maskAll;
	sigfillset(&maskAll);
	sigprocmask(SIG_SETMASK, &maskAll,NULL);
	
	
	pthread_mutex_lock(&mutexBateau);
	while(nbBateaux < NB_BATEAUX)
	{
		//Trace("Amiral cree un bateau !");
		pBateau = (Bateau *)malloc(sizeof(Bateau));
		if(nbCroiseurs < NB_CROISEURS)
		{
			pBateau->type = CROISEUR;
			nbCroiseurs++;
		}
		else if(nbCuirasses < NB_CUIRASSES)
		{
			pBateau->type = CUIRASSE;
			nbCuirasses++;
		}
		else if(nbDestoyers < NB_DESTOYERS)
		{
			pBateau->type = DESTROYER;
			nbDestoyers++;
		}
		else if(nbTorpilleurs < NB_TORPILLEURS)
		{
			pBateau->type = TORPILLEUR;
			nbTorpilleurs++;
		}
		
		if(nbVerticaux > nbHozizontaux)
		{
			pBateau->direction = HORIZONTAL;
			nbHozizontaux++;
		}
		else
		{
			pBateau->direction = VERTICAL;
			nbVerticaux++;
		}
		nbBateaux++;
		pthread_create(&tid, NULL, fctThBateau, pBateau);
		pthread_mutex_unlock(&mutexBateau);
	}
	
	pthread_mutex_lock(&mutexBateau);
	while(nbBateaux > 0)
		pthread_cond_wait(&condBateaux, &mutexBateau);
	setTitreGrilleSDL("GAME OVER !!!");
	pthread_mutex_unlock(&mutexBateau);
	return NULL;
}

/***********************************
*
*		Section BATEAU (_bat)
*
************************************
*/
void *fctThBateau(void *p)
{
	sigset_t blockSet, unblockSet;	
	
	//construction set
	sigfillset(&blockSet);
	sigfillset(&unblockSet);
	sigdelset(&unblockSet, SIGUSR2);
	
	// armement handler SIGUSR2
	struct sigaction sigAct2;
	sigAct2.sa_sigaction = HandlerSIGUSR2;
	sigAct2.sa_flags = SA_SIGINFO;
	sigemptyset(&sigAct2.sa_mask);
	sigaction(SIGUSR2, &sigAct2, NULL);
	
	//Allocation place ComBateau[]
	int BatPose = 0;
	int Place = 0;
	for(int i = 0;(i<NB_BATEAUX) && (BatPose != 1);i++)
	{
		pthread_mutex_lock(&mutexComBateau[i]);
		if(comBateau[i].tidBateau == 0)
		{
			//Trace("Remplis la struc bateau %d", i);
			comBateau[i].tidBateau = pthread_self();
			comBateau[i].indEcriture= 0;
			comBateau[i].indLecture =0;
			pthread_mutex_init(&comBateau[i].mutex, NULL);
			pthread_cond_init(&comBateau[i].cond, NULL);
			BatPose = 1;
			Place = i;
		}
		pthread_mutex_unlock(&mutexComBateau[i]);
	}
	pthread_setspecific(cleComBateau,&comBateau[Place]);
	
	//blockSet
	sigprocmask(SIG_SETMASK, &blockSet, NULL);
	//lock
	pthread_mutex_lock(&mutexMer);
	
	//on trouve une place + dessin
	Bateau *pBateau = (Bateau *)p;
	pthread_setspecific(cleBateau, (void*)pBateau);
	
	if(searchPosBateau2(pBateau) == 0)
	{
		Trace("Erreur search pos bateau !!");
		pthread_exit(0);
	}
	DessineFullBateau(pBateau, DRAW);
	Trace("Bateau dessine !  %ld", (long)pthread_self());
	
	//unlock
	pthread_mutex_unlock(&mutexMer);
	
	//AfficheMer();
	
	while(1)
	{
		
		waitRand(1000000000, 3999999999);
		//unblockSet
		sigprocmask(SIG_SETMASK, &unblockSet, NULL);
		//libre pour les signaux	
		//blockSet
		sigprocmask(SIG_SETMASK, &blockSet, NULL);
		//lock
		pthread_mutex_lock(&mutexMer);
		deplacementBateau(pBateau);
		//unlock
		pthread_mutex_unlock(&mutexMer);
	}
	Trace("Fin bateau !!  %ld", pthread_self());
	pthread_exit(0);
}

/***********************************
*
*		Section IA (_ia)
*
************************************
*/
void *fctThIA(void *)
{
	// Bloquer tous les signaux
	sigset_t maskAll;
	sigfillset(&maskAll);
	sigprocmask(SIG_SETMASK, &maskAll,NULL);
	Trace("Debut thread IA    %ld", pthread_self());
	Message repMes;
	ReponseTir repTir;
	int selectedI, selectedJ;
	int tentatives = 0;
	int targetForTermination = 0;
	while(1)
	{
		waitTime(4, 0);
		pthread_mutex_lock(&mutexMer);
		//selection case tir
		int L = 0, C = 0;
		if(targetForTermination != 0)
		{
			for(L = 0;L<NB_LIGNES;L++)
			{
				for(C = 0;C<NB_COLONNES;C++)
				{
					if(tab[L][C] == targetForTermination)
					{
						selectedI = L;
						selectedJ = C;
						DessineCible(selectedI,selectedJ);
					}
				}
			}
			if((L == NB_LIGNES) && (C == NB_COLONNES))
				targetForTermination = 0;
		}
		if((tentatives < 20) && (targetForTermination == 0))
		{
			selectedI = rand()%(NB_LIGNES);
			selectedJ = rand()%(NB_COLONNES);
			while(tab[selectedI][selectedJ] < 0)
			{
				selectedI = rand()%(NB_LIGNES);
				selectedJ = rand()%(NB_COLONNES);
			}
			DessineCible(selectedI, selectedJ);
		}
		else if(tentatives == 20)
		{
			for(int tirAutoL = 0;tirAutoL<NB_LIGNES;tirAutoL++)
			{
				for(int tirAutoC = 0;tirAutoC<NB_COLONNES;tirAutoC++)
				{
					if(tab[tirAutoL][tirAutoC] > 0)
					{
						selectedI = tirAutoL;
						selectedJ = tirAutoC;
						DessineCible(selectedI,selectedJ);
					}
				}
			}
		}
		//attente 1 seconde
		waitTime(1, 0);
		
		if(tab[selectedI][selectedJ] == 0)
		{
			//pas toucher
			Trace(" IA Missed");
			EffaceCarre(selectedI, selectedJ);
			tentatives++;
		}
		if(tab[selectedI][selectedJ] > 0)
		{
			//prevenir bateau
			Trace("IA Hit ");
			repTir.status = TOUCHE;
			tentatives = 0;
			repTir.L = selectedI;
			repTir.C = selectedJ;
			repMes.setType(getpid());
			repMes.setData((char *)&repTir, sizeof(RequeteTir));
			int ShipFound = 0;
			pthread_mutex_lock(&mutexBateau);
			int i;
			for( i = 0;(i<NB_BATEAUX) && (ShipFound != 1);i++)
			{
				//Trace("test %d", i);
				if(tab[selectedI][selectedJ] == (long)comBateau[i].tidBateau)
				{
					ShipFound = 1;
					pthread_mutex_lock(&mutexComBateau[i]);
					comBateau[i].Requete[comBateau[i].indEcriture] = repMes;
					comBateau[i].indEcriture++;
					targetForTermination = comBateau[i].tidBateau;
					pthread_mutex_unlock(&mutexComBateau[i]);
					pthread_cond_signal(&comBateau[i].cond);
					//Trace("fin test %d", tab[reqTir.L][reqTir.C]);
				}
			}
			if(ShipFound == 1)
			{
				Trace("envois signal  %ld", comBateau[i-1].tidBateau);
				pthread_kill(comBateau[i-1].tidBateau,SIGUSR2);
				//DessineExplosion(reqTir.L,reqTir.C,ORANGE);
				//pthread_kill(tab[reqTir.L][reqTir.C], SIGUSR2);
				//Trace("test");
				tab[selectedI][selectedJ] = -tab[selectedI][selectedJ];
			}
			pthread_mutex_unlock(&mutexBateau);
		}
		pthread_mutex_unlock(&mutexMer);
	}
}

/***********************************
*
*		Section POSBATEAU (_posbat)
*
************************************
*/
int searchPosBateau(Bateau *pBateau)
{
	if(pBateau == NULL)
	{
		Trace("Erreur param searchBateau");
		pthread_exit(0);
	}
	int posOK=0;
	for(int i=0; i<NB_LIGNES*NB_COLONNES; i++)
	{
		if(pBateau->direction == HORIZONTAL)
		{
			//bateau horizontal
			if(i%NB_COLONNES == 0)
				posOK =0;
			if(tab[i/NB_LIGNES][i%NB_COLONNES] == 0)
			{
				posOK++;
			}
			else
			{
				posOK=0;
			}
			if(lignes[i/NB_LIGNES] != 0)
			{
				posOK=0;
			}
			if(posOK == pBateau->type)
			{
				pBateau->L = (i/NB_LIGNES);
				pBateau->C = (i%NB_COLONNES)-(pBateau->type-1);
				lignes[i/NB_LIGNES] = 1;
				pBateau->sens = DROITE;
				i = NB_COLONNES*NB_LIGNES;
			}
		}
		else
		{
			//bateau vertical
			if(i%NB_LIGNES == 0)
				posOK =0;
			if(tab[i%NB_LIGNES][i/NB_COLONNES] == 0)
			{
				posOK++;
			}
			else
			{
				posOK=0;
			}
			if(colonnes[i/NB_COLONNES] != 0)
			{
				posOK =0;
			}
			if(posOK == pBateau->type)
			{
				pBateau->L = (i%NB_LIGNES)-(pBateau->type-1);
				pBateau->C = i/NB_COLONNES;
				colonnes[i/NB_COLONNES] = 1;
				pBateau->sens = BAS;
				i = NB_COLONNES*NB_LIGNES;
			}
		}
	}
	return posOK;
}

/***********************************
*
*		Section DessineFullBATEAU (_dfbat)
*
************************************
*/
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

/***********************************
*
*		Section DeplacementBATEAU (_depbat)
*
************************************
*/
int deplacementBateau(Bateau *pBateau)
{
	DessineFullBateau(pBateau, CLEAR);
	int tmp=0;
	if(pBateau->direction == HORIZONTAL)
	{
		if(pBateau->sens == DROITE)
			// Detection obstacle
			if(tab[pBateau->L][(pBateau->C + pBateau->type)%NB_COLONNES] != 0)
				pBateau->sens = GAUCHE;
			else
				pBateau->C = (pBateau->C + 1)%NB_COLONNES;
		else
		{
			if(pBateau->C - 1 <0)
				tmp = NB_COLONNES-1;
			else
				tmp = pBateau->C -1;
			if(tab[pBateau->L][(tmp)%NB_COLONNES] != 0)
				pBateau->sens = DROITE; 
			else
				pBateau->C = (tmp)%NB_COLONNES;
		}
	}
	else
	{
		if(pBateau->sens == BAS)
			if(tab[(pBateau->L + pBateau->type)%NB_LIGNES][pBateau->C] != 0)
				pBateau->sens = HAUT;
			else
				pBateau->L = (pBateau->L + 1)%NB_LIGNES;
		else
		{
			if(pBateau->L - 1 < 0)
				tmp = NB_LIGNES-1;
			else
				tmp = pBateau->L -1;
			if(tab[(tmp)%NB_LIGNES][pBateau->C] != 0)
				pBateau->sens = BAS;
			else
				pBateau->L = (tmp)%NB_LIGNES;
		}
	}
	DessineFullBateau(pBateau, DRAW);
	return 1;
}
/***********************************
*
*		Section HandlerSIGUSR2 (_sigusr2)
*
************************************
*/
void HandlerSIGUSR2(int sig, siginfo_t *info,void *p)
{
	try
	{
		Trace("Entree SIGUSR2 je suis %ld", pthread_self());
		Message resultTir,reponse;
		RequeteTir reqTir;
		Bateau *pBateau = (Bateau *)pthread_getspecific(cleBateau);
		ComBateau *comBateau = (ComBateau *)pthread_getspecific(cleComBateau);
		//Trace("Info : bateau %d    --    %d -- %d ", comBateau->tidBateau, comBateau->indLecture, comBateau->indEcriture);
		while(1)
		{
			pthread_mutex_lock(&comBateau->mutex);
			while(comBateau->indLecture == comBateau->indEcriture)
			pthread_cond_wait(&comBateau->cond,&comBateau->mutex);
			pthread_mutex_lock(&mutexMer);
			//memcpy(&resultTir,&comBateau->Requete[comBateau->indLecture],sizeof(Message));
			resultTir = comBateau->Requete[comBateau->indLecture];
			comBateau->indLecture ++;
			memcpy(&reqTir, resultTir.getData(), sizeof(RequeteTir));
			//Trace("Toucher ! envois a %d   pos %d -- %d", resultTir.getExpediteur(), reqTir.L, reqTir.C );
			EffaceCarre(reqTir.L, reqTir.C);
			DessineBateau(reqTir.L, reqTir.C,pBateau->type, pBateau->direction, pBateau->direction == VERTICAL ? reqTir.L - pBateau->L : reqTir.C - pBateau->C);
			//Trace("%d %d ")
			DessineExplosion(reqTir.L, reqTir.C, ORANGE);
			if(comBateau->indEcriture != pBateau->type)
			{
				pthread_mutex_unlock(&comBateau->mutex);
				pthread_mutex_unlock(&mutexMer);
				//Trace("Fin envois ");
			}
			else
			{
				Trace("Badaboum");
				pthread_mutex_lock(&mutexBateau);
	
				nbBateaux--;
	
				pthread_mutex_unlock(&mutexBateau);
				pthread_cond_signal(&condBateaux);
				pthread_exit(0);
			}
		}
		free(pBateau);
	}
	catch(MessageQueueException e)
	{
		Trace("MessageQueueException  : %s", e.getMessage());	
	}
	catch(...)
	{
		Trace("Erreur handler SIGUSR2");
		perror("error");	
	}
}


int searchPosBateau2(Bateau *pBateau)
{
	if(pBateau == NULL)
	{
		Trace("Erreur param searchBateau");
		pthread_exit(0);
	}
	Position pos[NB_COLONNES*NB_LIGNES];
	int posOK=0, tmpRand;
	int i, j, k, posMax=0;
	if(pBateau->direction == HORIZONTAL)
	{	
		//horizontal
		rand()%2 ? pBateau->sens = DROITE: pBateau->sens = GAUCHE;
		for(i=0; i<NB_LIGNES; i++)
		{
			if(lignes[i] == 0)
			{
				for(j=0;j<NB_COLONNES; j++)
				{
					for(k=0; k<pBateau->type; k++)
					{
						//
						if(tab[i][(j+k)%NB_COLONNES] == 0)
							posOK +=1;
						else
						{
							posOK = 0;
							k = pBateau->type + 2;
						}
					}
					if(posOK == pBateau->type)
					{
						pos[posMax].L = i;
						pos[posMax].C = j;
						posMax++;
					}
					posOK = 0;	
				}	
			}
			posOK=0;
		}
	}
	if(pBateau->direction == VERTICAL)
	{	
		//vertical
		rand()%2 ? pBateau->sens = BAS: pBateau->sens = HAUT;
		for(i=0; i<NB_COLONNES; i++)
		{
			if(colonnes[i] == 0)
			{
				for(j=0;j<NB_LIGNES; j++)
				{
					for(k=0; k<pBateau->type; k++)
					{
						//
						if(tab[(j+k)%NB_LIGNES][i] == 0)
							posOK +=1;
						else
						{
							posOK=0;
							k = pBateau->type + 2 ;
						}
					}
					if(posOK == pBateau->type)
					{
						pos[posMax].L = j;
						pos[posMax].C = i;
						posMax++;
					}
					posOK = 0;	
				}
			}
			posOK =0;
		}
	}
	if(posMax < 1)
	{
		Trace("Erreur search pos!!");
		return 0;
	}
	
	tmpRand = (rand()%posMax);
	
	pBateau->L = pos[tmpRand].L;
	pBateau->C = pos[tmpRand].C;
	pBateau->direction == HORIZONTAL ? lignes[pBateau->L] = 1: colonnes[pBateau->C] = 1;
	Trace("Pos choisie %d - %d      %d", pBateau->L,	pBateau->C, posMax);
	return 1;
}












