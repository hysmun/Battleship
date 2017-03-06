#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "Ecran.h"
#include "GrilleSDL.h"
#include "Ressources.h"
#include "MessageQueue.h"

#include "protocole.h"

// Dimensions de la grille de jeu
#define NB_LIGNES   10
#define NB_COLONNES 10

MessageQueue connexion;  // File de message

void HandlerSIGINT(int s); // Fin propre du serveur

// Tableau de jeu (mer)
int   tab[NB_LIGNES][NB_COLONNES];

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

  // Juste pour avoir un bateau --> a supprimer
  int L=2,C=3;
  for (int i=0 ; i<CUIRASSE ; i++) 
  {
    tab[L][C+i] = pthread_self();
    DessineBateau(L,C+i,CUIRASSE,HORIZONTAL,i);
  }

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

