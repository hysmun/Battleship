#include <unistd.h>
#include <signal.h>
#include <pthread.h>
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
void *fctThBateau(void *);
void *fctThAmiral(void *);
int searchPosBateau(Bateau *pBateau);
int DessineFullBateau(Bateau *pBateau, int opt);
int deplacementBateau(Bateau *pBateau);

// Tableau de jeu (mer)
int tab[NB_LIGNES][NB_COLONNES]={{0}};
int lignes[NB_LIGNES]={0};
int colonnes[NB_COLONNES]={0};
pthread_t tidAmiral;
pthread_t tid;
int nbCuirasses=0, nbCroiseurs=0, nbDestoyers=0, nbTorpilleurs=0;
int nbBateaux=0;
int nbVerticaux =0, nbHozizontaux=0;

pthread_mutex_t mutexMer;
pthread_mutex_t mutexBateau;
pthread_cond_t condBateaux;

//**************************************************************
int main(int argc,char* argv[])
{
  srand((unsigned)time(NULL));
  Trace("(THREAD MAIN %d) Pid  = %d",pthread_self(),getpid());

  // Creation file de messages
  connexion.init(1000);

  // Ouverture de la fenetre graphique
  Trace("(THREAD MAIN %d) Ouverture de la fenetre graphique",pthread_self()); fflush(stdout);
  if (OuvertureFenetreGraphique("serveur") < 0)
  {
    Trace("Erreur de OuvrirGrilleSDL\n");
    exit(1);
  }

  // Armement des signaux
  struct sigaction sigAct;

  sigAct.sa_handler = HandlerSIGINT;
  sigAct.sa_flags = 0;
  sigemptyset(&sigAct.sa_mask);
  sigaction(SIGINT,&sigAct,NULL); 
  
  // creation mutex et variable condition
	pthread_mutex_init(&mutexMer, NULL);
	pthread_mutex_init(&mutexBateau, NULL);
	pthread_cond_init(&condBateaux, NULL);

  // Juste pour avoir un bateau --> a supprimer
	pthread_create(&tidAmiral, NULL, fctThAmiral, NULL);
  

  // Mise en boucle du serveur --> à modifier !!!
  Message requete,reponse;
  while(1)
  {
    try
    {
      Trace("(THREAD MAIN %d) Attente d'une requete...",pthread_self());
      requete = connexion.ReceiveData(1);
      Trace("(THREAD MAIN %d) Message Recu de : %d",pthread_self(),requete.getExpediteur());   

      if (requete.getRequete() == TIR)
      {
        // Recuperation charge utile requete
        RequeteTir reqTir;
        memcpy(&reqTir,requete.getData(),sizeof(RequeteTir)); // on recupere le contenu du message

        // Preparation de la reponse
        ReponseTir repTir;
        reponse.setType(requete.getExpediteur()); // Retour a l'expediteur
        reponse.setRequete(TIR); // pour prevnir que c'est une reponse a une requete de tir
        repTir.L = reqTir.L;
        repTir.C = reqTir.C;
        if (tab[reqTir.L][reqTir.C] != 0) 
        {
          repTir.status = TOUCHE;
          DessineExplosion(reqTir.L,reqTir.C,ORANGE);
        }
        else 
        {
          repTir.status = PLOUF;
          DessineCible(reqTir.L,reqTir.C);
        }
        reponse.setData((char*)&repTir,sizeof(ReponseTir)); // Charge utile du message

        // Envoi de la reponse
        connexion.SendData(reponse);
      }
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
  Trace("(THREAD MAIN %d) Reception SIGINT",pthread_self());

  // Fermeture de la grille de jeu (SDL)
  Trace("(THREAD MAIN %d) Fermeture de la fenetre graphique...",pthread_self()); fflush(stdout);
  FermetureFenetreGraphique();
  Trace("(THREAD MAIN %d) OK Fin",pthread_self()); //fflush(stdout);

  // Suppression de la file de messages
  connexion.close();
}

//************************************************************************************

void *fctThAmiral(void *)
{
	Bateau *pBateau;
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
}



void *fctThBateau(void *p)
{
	pthread_mutex_lock(&mutexMer);
	Bateau *pBateau = (Bateau *)p;
	printf("Bateau !!\n");
	if(searchPosBateau(pBateau) == 0)
	{
		Trace("Erreur search pos bateau !!");
		pthread_exit(0);
	}
	printf("truc \n");
	DessineFullBateau(pBateau, DRAW);
	printf("Bateau dessine !\n");
	
	
	while(1)
	{
		waitRand(1000000000, 3999999999);
		pthread_mutex_unlock(&mutexMer);
		//libre pour les signaux
		
		pthread_mutex_lock(&mutexMer);
		deplacementBateau(pBateau);

		
	}
	
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

int deplacementBateau(Bateau *pBateau)
{
	DessineFullBateau(pBateau, CLEAR);
	if(pBateau->direction == HORIZONTAL)
	{
		if(pBateau->sens == DROITE)
			pBateau->C = (pBateau->C + 1)%NB_COLONNES;
		else
			pBateau->C = (pBateau->C - 1)%NB_COLONNES;
	}
	else
	{
		if(pBateau->sens == BAS)
			pBateau->L = (pBateau->L + 1)%NB_LIGNES;
		else
			pBateau->L = (pBateau->L - 1)%NB_LIGNES;
	}
	DessineFullBateau(pBateau, DRAW);
	return 1;
}






























