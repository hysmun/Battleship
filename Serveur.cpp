#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <limits.h>
#include "Ecran.h"
#include "GrilleSDL.h"
#include "Ressources.h"
#include "MessageQueue.h"

#include "protocole.h"
#include "utils.h"

// Dimensions de la grille de jeu
#define NB_LIGNES   10
#define NB_COLONNES 10


#define NB_CROISEURS 2
#define NB_DESTOYERS 3
#define NB_CUIRASSES 1
#define NB_TORPILLEURS 4

#define NB_BATEAUX (NB_CROISEURS + NB_CUIRASSES + NB_DESTOYERS + NB_TORPILLEURS)

MessageQueue connexion;  // File de message

void HandlerSIGINT(int s); // Fin propre du serveur

//thread
void *fctThBateau(void *);
void *fctThAmiral(void *);
void *fctThRequete(void *);

//autre fct
int searchPosBateau(Bateau *pBateau);
int DessineFullBateau(Bateau *pBateau, int opt);
int deplacementBateau(Bateau *pBateau);
void HandlerSIGUSR1(int sig, siginfo_t *info, void *p);
void HandlerSIGUSR1(int sig);
void HandlerSIGUSR2(int sig, siginfo_t *info, void *p);
void AfficheMer(void);

// Tableau de jeu (mer)
int tab[NB_LIGNES][NB_COLONNES]={{0}};
int lignes[NB_LIGNES]={0};
int colonnes[NB_COLONNES]={0};
pthread_t tidAmiral;
pthread_t tid;
int nbCuirasses=0, nbCroiseurs=0, nbDestoyers=0, nbTorpilleurs=0;
int nbBateaux=0;
int nbVerticaux =0, nbHozizontaux=0;

pid_t pid=getpid();
pid_t joueurs[10]={0};

pthread_mutex_t mutexMer;
pthread_mutex_t mutexBateau;
pthread_mutex_t mutexJoueurs;
pthread_mutex_t mutexComBateau[NB_BATEAUX];
pthread_cond_t condBateaux;
pthread_key_t cleBateau;
pthread_key_t cleComBateau;

pthread_mutex_t mutexCible[NB_LIGNES][NB_COLONNES];

ComBateau comBateau[NB_BATEAUX];

//**************************************************************
int main(int argc,char* argv[])
{
  srand((unsigned)time(NULL));
  Trace("(THREAD MAIN %d) Pid  = %d",tidSelf(),getpid());

  // Creation file de messages
  connexion.init(1000);

	// Ouverture de la fenetre graphique
	Trace("(THREAD MAIN %d) Ouverture de la fenetre graphique",tidSelf()); fflush(stdout);
	if(argc > 1 && argc < 3 && argv[1][0] == '-' && argv[1][1] == 's')
	{
		Trace("Sans fenetre graphique !");
	}
	else
	{
		Trace("Avec fenetre graphique !!");
		if (OuvertureFenetreGraphique("serveur") < 0)
		{
			Trace("Erreur de OuvrirGrilleSDL\n");
			exit(1);
		}
	}

  // Armement des signaux
  struct sigaction sigAct;

  sigAct.sa_handler = HandlerSIGINT;
  sigAct.sa_flags = 0;
  sigemptyset(&sigAct.sa_mask);
  sigaction(SIGINT,&sigAct,NULL); 
  
  //setMask
 	sigset_t setMask, oldMask;
 	sigfillset(&setMask);
 	sigdelset(&setMask, SIGINT);
  	sigprocmask(SIG_SETMASK, &setMask, &oldMask);
  	
  	//clé variable spécifique
  	pthread_key_create(&cleBateau, NULL);
  	pthread_key_create(&cleComBateau, NULL);
  
  // creation mutex et variable condition
	pthread_mutex_init(&mutexMer, NULL);
	pthread_mutex_init(&mutexBateau, NULL);
	pthread_mutex_init(&mutexJoueurs, NULL);
	pthread_cond_init(&condBateaux, NULL);
	
	for(int i = 0; i< NB_BATEAUX; i++)
	{
		pthread_mutex_init(&mutexComBateau[i], NULL);
	}
	
	for(int i = 0; i< NB_LIGNES; i++)
	{
		for(int j = 0; j < NB_COLONNES; j++)
		{
			pthread_mutex_init(&mutexCible[i][j], NULL);
		}
	}

  // Juste pour avoir un bateau 
	pthread_create(&tidAmiral, NULL, fctThAmiral, NULL);
  

	

  // Mise en boucle du serveur
  Message requete,reponse;
  while(1)
  {
    try
    {
      Trace("(THREAD MAIN %d) Attente d'une requete...",tidSelf());
      requete = connexion.ReceiveData(1);
      Trace("(THREAD MAIN %d) Message Recu de : %d",tidSelf(),requete.getExpediteur());   
      pthread_create(&tid, NULL, fctThRequete, (void *)&requete);

      
    }
    catch(MessageQueueException e)
    {
      Trace("Erreur de reception : %s",e.getMessage());
      break;
    }
  }

  Trace("(THREAD MAIN %d) Serveur se termine");
  return 0;
}

//************************************************************************************
void HandlerSIGINT(int s)
{
  Trace("(THREAD MAIN %d) Reception SIGINT",tidSelf());

  // Fermeture de la grille de jeu (SDL)
  Trace("(THREAD MAIN %d) Fermeture de la fenetre graphique...",tidSelf()); fflush(stdout);
  FermetureFenetreGraphique();
  Trace("(THREAD MAIN %d) OK Fin",tidSelf()); //fflush(stdout);

  // Suppression de la file de messages
  connexion.close();
}

//************************************************************************************

/*
*
*			fctThRequete
*
*/
void *fctThRequete(void *p)
{
	// Bloquer les signaux
	sigset_t maskAll;
	sigfillset(&maskAll);
	sigprocmask(SIG_SETMASK, &maskAll,NULL);
	
	Message requete=(Message)*((Message *)p), reponse;
	switch(requete.getRequete())
	{
		case CONNECT:
		{
			Trace("Une nouvelle personne s'est connectée : %d",requete.getExpediteur());
			// Chercher une place libre dans le tableau de joueurs
			pthread_mutex_lock(&mutexJoueurs);
			for(int i = 0;i<10;i++)
			{
				if(joueurs[i] != 0)
				{
					joueurs[i] = requete.getExpediteur();
					i = 10;
				}
			}
			pthread_mutex_unlock(&mutexJoueurs);
			reponse.setType(requete.getExpediteur());
			reponse.setRequete(CONNECT);
			connexion.SendData(reponse);
			break;
		}
		case DECONNECT:
		{
			Trace("Deconnection joueur : %d", requete.getExpediteur());
			pthread_mutex_lock(&mutexJoueurs);
			for(int i = 0;i<10;i++)
			{
				if(joueurs[i] != requete.getExpediteur())
				{
					joueurs[i] = 0;
					i = 10;
				}
			}
			pthread_mutex_unlock(&mutexJoueurs);
			break;
		}
		case TIR:
		{
			AfficheMer();
			// Recuperation charge utile requete
			RequeteTir reqTir;
			ReponseTir repTir;
			memcpy(&reqTir,requete.getData(),sizeof(RequeteTir)); // on recupere le contenu du message
			reponse.setType(requete.getExpediteur()); // Retour a l'expediteur
			reponse.setRequete(TIR); // pour prevnir que c'est une reponse a une requete de tir
			repTir.status = PLOUF;
			if(pthread_mutex_trylock(&mutexCible[reqTir.L][reqTir.C]) == 0)
			{
				pthread_mutex_lock(&mutexCible[reqTir.L][reqTir.C]);
				waitTime(5, 0);
				// Preparation de la reponse
				repTir.L = reqTir.L;
				repTir.C = reqTir.C;
				pthread_mutex_lock(&mutexMer);
				if (tab[reqTir.L][reqTir.C] != 0) 
				{
					if(tab[reqTir.L][reqTir.C] > 0)
					{
						Trace("Bateau touche");
						int ShipFound = 0;
						for(int i = 0;(i<NB_BATEAUX) && (ShipFound != 1);i++)
						{
							if(tab[reqTir.L][reqTir.C] == comBateau[i].tidBateau)
							{
								ShipFound = 1;
								pthread_mutex_lock(&mutexComBateau[i]);
								memcpy(&comBateau[i].Requete[comBateau[i].indEcriture],&requete,sizeof(RequeteTir));
								comBateau[i].indEcriture++;
								pthread_cond_signal(&comBateau[i].cond);
								pthread_mutex_unlock(&mutexComBateau[i]);
							}
						}
						pthread_kill(tab[reqTir.L][reqTir.C],SIGUSR2);
						//DessineExplosion(reqTir.L,reqTir.C,ORANGE);
						//pthread_kill(tab[reqTir.L][reqTir.C], SIGUSR2);
						tab[reqTir.L][reqTir.C] = -tab[reqTir.L][reqTir.C];
					}
					else
					{
						Trace("Bateau deja toucher");
						repTir.status = DEJA_TOUCHE;
					}
				}
				else 
				{
					Trace("Plouf !!");
					repTir.status = PLOUF;
				}
				pthread_mutex_unlock(&mutexMer);
			}
			else
			{
				//mutex deja pris a cette position 
				Trace("Locked");
				repTir.L = reqTir.L;
				repTir.C = reqTir.C;
				repTir.status = LOCKED;
			}
			pthread_mutex_unlock(&mutexCible[reqTir.L][reqTir.C]);
			
			reponse.setData((char*)&repTir,sizeof(ReponseTir)); // Charge utile du message
			// Envoi de la reponse
			connexion.SendData(reponse);
			break;
		}
		default:
		{
			Trace("!!!!!     requete non traiter     !!!!!");
		}
   }
   pthread_exit(0);
}

/*
*
*			fctThAmiral
*
*/
void *fctThAmiral(void *p)
{
	Bateau *pBateau;
	
	// Bloquer tous les signaux
	sigset_t maskAll;
	sigfillset(&maskAll);
	sigprocmask(SIG_SETMASK, &maskAll,NULL);
	AfficheMer();
	while(1)
	{
		pthread_mutex_lock(&mutexBateau);
		while(nbBateaux >= NB_BATEAUX)
			pthread_cond_wait(&condBateaux, &mutexBateau);
		//mutex pris
		Trace("Amiral cree un bateau !");
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
	return NULL;
}


/*
*
*			fctThBateau
*
*/
void *fctThBateau(void *p)
{
	sigset_t blockSet, unblockSet;	
	
	//construction set
	sigfillset(&blockSet);
	sigfillset(&unblockSet);
	sigdelset(&unblockSet, SIGUSR1);
	sigdelset(&unblockSet, SIGUSR2);
	
	//armement handler SIGUSR1
	struct sigaction sigAct;
	sigAct.sa_sigaction = HandlerSIGUSR1;
	sigAct.sa_flags = SA_SIGINFO;
	sigemptyset(&sigAct.sa_mask);
	sigaction(SIGUSR1, &sigAct, NULL);
	
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
		if(comBateau[i].tidBateau != 0)
		{
			comBateau[i].tidBateau = tidSelf();
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
	
	if(searchPosBateau(pBateau) == 0)
	{
		Trace("Erreur search pos bateau !!");
		pthread_exit(0);
	}
	DessineFullBateau(pBateau, DRAW);
	Trace("Bateau dessine !  %d", tidSelf());
	
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
	Trace("Fin bateau !!  %d", tidSelf());
	pthread_exit(0);
}

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

int DessineFullBateau(Bateau *pBateau, int opt)
{
	if(opt == DRAW)
	{
		for(int i=0; i<pBateau->type; i++)
		{
			if(pBateau->direction == HORIZONTAL)
			{
				DessineBateau(pBateau->L%NB_LIGNES, (pBateau->C+i)%NB_COLONNES, pBateau->type, HORIZONTAL,i);
				tab[pBateau->L%NB_LIGNES][(pBateau->C+i)%NB_COLONNES] = tidSelf();
			}
			else
			{
				DessineBateau((pBateau->L+i)%NB_LIGNES, pBateau->C%NB_COLONNES, pBateau->type, VERTICAL,i);
				tab[(pBateau->L+i)%NB_LIGNES][pBateau->C%NB_COLONNES] = tidSelf();
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

void HandlerSIGUSR1(int sig, siginfo_t *info, void *p) 
{
	Trace("pid emetteur : %d",info->si_pid);
	Bateau *pBateau = (Bateau *)pthread_getspecific(cleBateau);
	Message envois(info->si_pid, SOUSMARIN, (char *)pBateau, sizeof(Bateau));
	connexion.SendData(envois);
}

void HandlerSIGUSR2(int sig, siginfo_t *info,void *p)
{
	Message resultTir;
	Bateau *pBateau = (Bateau *)pthread_getspecific(cleBateau);
	ComBateau *comBateau = (ComBateau *)pthread_getspecific(cleComBateau);
	pthread_mutex_lock(&comBateau->mutex);
	while(comBateau->indLecture == comBateau->indEcriture)
		pthread_cond_wait(&comBateau->cond,&comBateau->mutex);
	memcpy(&resultTir,comBateau->Requete[comBateau->indLecture],sizeof(Message));
	comBateau->indLecture ++;
	DessineExplosion(resultTir.)
	
	
	pthread_mutex_unlock(&comBateau->mutex);
}

void AfficheMer(void)
{
	pthread_mutex_lock(&mutexMer);
	for(int i = 0; i < NB_LIGNES; i++)
	{
		for(int j = 0; j < NB_COLONNES; j++)
		{
			if(tab[i][j] != 0)
			{
				if(tab[i][j] > 0)
					printf("1 ");
				else
					printf("2 ");
			}
			else
			printf("0 ");
		}
		printf("\n");
	}
	pthread_mutex_unlock(&mutexMer);
}
























