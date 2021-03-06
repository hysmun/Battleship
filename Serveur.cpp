/**********************************
*	Programmation UNIX - Threads
*	Equipe Brajkovic - Mauhin
*	HEPL 2016-2017
***********************************
*/

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
int searchPosBateau2(Bateau *pBateau);
int DessineFullBateau(Bateau *pBateau, int opt);
int deplacementBateau(Bateau *pBateau);
void HandlerSIGUSR1(int sig, siginfo_t *info, void *p);
void HandlerSIGUSR1(int sig);
void HandlerSIGUSR2(int sig, siginfo_t *info, void *p);
void AfficheMer(void);

// Tableau de jeu (mer)
long tab[NB_LIGNES][NB_COLONNES]={{0}};
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

/***********************************
*
*		Section MAIN (_main)
*
************************************
*/
int main(int argc,char* argv[])
{
  srand((unsigned)time(NULL));
  Trace("(THREAD MAIN %d) Pid  = %ld",pthread_self(),getpid());

  // Creation file de messages
  connexion.init(1000);

	// Ouverture de la fenetre graphique
	Trace("(THREAD MAIN %ld) Ouverture de la fenetre graphique",pthread_self()); fflush(stdout);
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
		comBateau[i].tidBateau = 0;
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
      Trace("(THREAD MAIN %ld) Attente d'une requete...",pthread_self());
      requete = connexion.ReceiveData(1);
      Trace("(THREAD MAIN %ld) Message Recu de : %d",pthread_self(),requete.getExpediteur());   
      pthread_create(&tid, NULL, fctThRequete, (void *)&requete);

      
    }
    catch(MessageQueueException e)
    {
      Trace("Erreur de reception : %s",e.getMessage());
      break;
    }
  }

  Trace("(THREAD MAIN ) Serveur se termine");
  return 0;
}

/***********************************
*
*		Section HandlerSIGINT (_sigint)
*
************************************
*/
void HandlerSIGINT(int s)
{
  Trace("(THREAD MAIN %ld) Reception SIGINT",pthread_self());

  // Fermeture de la grille de jeu (SDL)
  Trace("(THREAD MAIN %ld) Fermeture de la fenetre graphique...",pthread_self()); fflush(stdout);
  FermetureFenetreGraphique();
  Trace("(THREAD MAIN %ld) OK Fin",pthread_self()); //fflush(stdout);

  // Suppression de la file de messages
  connexion.close();
}

/***********************************
*
*		Section REQUETE (_req)
*
************************************
*/
void *fctThRequete(void *p)
{
	// Bloquer les signaux
	sigset_t maskAll;
	sigfillset(&maskAll);
	sigprocmask(SIG_SETMASK, &maskAll,NULL);
	Trace("Thread requete !!");
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
				if(joueurs[i] == 0)
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
				if(joueurs[i] == requete.getExpediteur())
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
			Trace("Tir ");
			RequeteTir reqTir;
			ReponseTir repTir;
			memcpy(&reqTir,requete.getData(),sizeof(RequeteTir)); // on recupere le contenu du message
			reponse.setType(requete.getExpediteur()); // Retour a l'expediteur
			reponse.setRequete(TIR); // pour prevnir que c'est une reponse a une requete de tir
			repTir.status = ERREUR;
			repTir.L = reqTir.L;
			repTir.C = reqTir.C;
			if(pthread_mutex_trylock(&mutexCible[reqTir.L][reqTir.C]) == 0)
			{
				waitTime(5, 0);
				// Preparation de la reponse
				repTir.L = reqTir.L;
				repTir.C = reqTir.C;
				int i;
				pthread_mutex_lock(&mutexMer);
				if (tab[reqTir.L][reqTir.C] != 0) 
				{
					if(tab[reqTir.L][reqTir.C] > 0)
					{
						Trace("Bateau touche pos %d -- %d",reqTir.L, reqTir.C );
						int ShipFound = 0;
						for( i = 0;(i<NB_BATEAUX) && (ShipFound != 1);i++)
						{
							//Trace("test %d", i);
							if(tab[reqTir.L][reqTir.C] == (long)comBateau[i].tidBateau)
							{
								ShipFound = 1;
								pthread_mutex_lock(&mutexComBateau[i]);
								comBateau[i].Requete[comBateau[i].indEcriture] = requete;
								comBateau[i].indEcriture++;
								
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
							tab[reqTir.L][reqTir.C] = -tab[reqTir.L][reqTir.C];
						}
						else
						{
							Trace("ERREUR !!");
						}
						//Trace("fin bateau tir");
						//Trace("Fin requete");
						pthread_mutex_unlock(&mutexCible[reqTir.L][reqTir.C]);
						pthread_mutex_unlock(&mutexMer);
						pthread_exit(0);
					}
					else
					{
						Trace("Bateau deja toucher");
						repTir.L = reqTir.L;
						repTir.C = reqTir.C;
						repTir.status = DEJA_TOUCHE;
					}
				}
				else 
				{
					Trace("Plouf !! ");
					repTir.L = reqTir.L;
					repTir.C = reqTir.C;
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
   Trace("Fin requete");
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
	//AfficheMer();
	while(1)
	{
		pthread_mutex_lock(&mutexBateau);
		while(nbBateaux >= NB_BATEAUX)
			pthread_cond_wait(&condBateaux, &mutexBateau);
		//mutex pris
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
*		Section POSBATEAU(_posbat)
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
				rand()%3 ? pBateau->sens = DROITE :pBateau->sens = GAUCHE;
			else
				pBateau->C = (pBateau->C + 1)%NB_COLONNES;
		else
		{
			if(pBateau->C - 1 <0)
				tmp = NB_COLONNES-1;
			else
				tmp = pBateau->C -1;
			if(tab[pBateau->L][(tmp)%NB_COLONNES] != 0)
				rand()%3 ? pBateau->sens = GAUCHE :pBateau->sens = DROITE;
			else
				pBateau->C = (tmp)%NB_COLONNES;
		}
	}
	else
	{
		if(pBateau->sens == BAS)
			if(tab[(pBateau->L + pBateau->type)%NB_LIGNES][pBateau->C] != 0)
				rand()%3 ? pBateau->sens = BAS :pBateau->sens = HAUT;
			else
				pBateau->L = (pBateau->L + 1)%NB_LIGNES;
		else
		{
			if(pBateau->L - 1 < 0)
				tmp = NB_LIGNES-1;
			else
				tmp = pBateau->L -1;
			if(tab[(tmp)%NB_LIGNES][pBateau->C] != 0)
				rand()%3 ? pBateau->sens = HAUT :pBateau->sens = BAS;
			else
				pBateau->L = (tmp)%NB_LIGNES;
		}
	}
	DessineFullBateau(pBateau, DRAW);
	return 1;
}

/***********************************
*
*		Section HandlerSIGUSR1 (_sigusr1)
*
************************************
*/
void HandlerSIGUSR1(int sig, siginfo_t *info, void *p) 
{
	Trace("pid emetteur : %d",info->si_pid);
	Bateau *pBateau = (Bateau *)pthread_getspecific(cleBateau);
	Message envois(info->si_pid, SOUSMARIN, (char *)pBateau, sizeof(Bateau));
	connexion.SendData(envois);
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
		ReponseTir *repTir = (ReponseTir *)malloc(sizeof(ReponseTir));
		Bateau *pBateau = (Bateau *)pthread_getspecific(cleBateau);
		ComBateau *comBateau = (ComBateau *)pthread_getspecific(cleComBateau);
		//Trace("Info : bateau %d    --    %d -- %d ", comBateau->tidBateau, comBateau->indLecture, comBateau->indEcriture);
		while(1)
		{
			pthread_mutex_lock(&comBateau->mutex);
			while(comBateau->indLecture == comBateau->indEcriture)
				pthread_cond_wait(&comBateau->cond,&comBateau->mutex);
			//memcpy(&resultTir,&comBateau->Requete[comBateau->indLecture],sizeof(Message));
			resultTir = comBateau->Requete[comBateau->indLecture];
			comBateau->indLecture ++;
			memcpy(&reqTir, resultTir.getData(), sizeof(RequeteTir));
			//Trace("Toucher ! envois a %d   pos %d -- %d", resultTir.getExpediteur(), reqTir.L, reqTir.C );
			DessineExplosion(reqTir.L, reqTir.C, ORANGE);
			if(comBateau->indEcriture != pBateau->type)
			{
			
				reponse.setType(resultTir.getExpediteur());
				reponse.setRequete(TIR);
				repTir->L = reqTir.L;
				repTir->C = reqTir.C;
				repTir->status = TOUCHE;
				memcpy(&repTir->bateau, pBateau, sizeof(Bateau));
				reponse.setData((char*)repTir,sizeof(ReponseTir));
				connexion.SendData(reponse);
				pthread_mutex_unlock(&comBateau->mutex);
				//Trace("Fin envois ");
			}
			else
			{
				Trace("Badaboum");
				reponse.setType(resultTir.getExpediteur());
				reponse.setRequete(TIR);
				repTir->L = reqTir.L;
				repTir->C = reqTir.C;
				repTir->status = COULE;
				memcpy(&repTir->bateau, pBateau, sizeof(Bateau));
				reponse.setData((char*)repTir,sizeof(ReponseTir));
				connexion.SendData(reponse);
				
				reponse.setRequete(BATEAU_COULE);
				for(int i = 0;i<10;i++)
				{
					//Sauf le joueur qui a coulé le bateau
					if((joueurs[i] != 0) && (joueurs[i] != resultTir.getExpediteur()))
					{
						reponse.setType(joueurs[i]);
						connexion.SendData(reponse);
					}
				}
				waitTime(3,0);
				for(int i = 0; i< pBateau->type; i++)
				{
					if(pBateau->direction == HORIZONTAL)
						pthread_mutex_unlock(&mutexCible[(pBateau->L)][(pBateau->C+i)%NB_COLONNES]);
					else
						pthread_mutex_unlock(&mutexCible[(pBateau->L+i)%NB_LIGNES][(pBateau->C)]);
				}
				
				DessineFullBateau(pBateau, CLEAR);
				//Si le bateau est vertical
				if(pBateau->direction == HORIZONTAL)
					lignes[pBateau->L] = 0;
				else
					colonnes[pBateau->C] = 0;
				comBateau->tidBateau = 0;
				comBateau->indEcriture = 0;
				comBateau->indLecture = 0;
				pthread_setspecific(cleComBateau,comBateau);
				nbBateaux--;
				switch(pBateau->type)
				{
					case TORPILLEUR:
						nbTorpilleurs--;
						break;
					case DESTROYER:
						nbDestoyers--;
						break;
					case CROISEUR:
						nbCroiseurs--;
						break;
					case CUIRASSE:
						nbCuirasses--;
						break;			
				}
				pthread_mutex_unlock(&comBateau->mutex);
				pthread_cond_signal(&condBateaux);
				break;
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

/***********************************
*
*		Section AfficheMER (_affmer)
*
************************************
*/
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





