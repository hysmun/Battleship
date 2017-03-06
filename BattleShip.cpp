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

// Dimensions de la grille de jeu
#define NB_LIGNES   10
#define NB_COLONNES 10

// Tableau de jeu (mer)
int   tab[NB_LIGNES][NB_COLONNES];
int   tabTir[NB_LIGNES][NB_COLONNES];

MessageQueue  connexion;  // File de messages

//************************************************************************************************
int main(int argc,char* argv[])
{
  srand((unsigned)time(NULL));

  // Connexion au serveur
  try
  {
    connexion.connect(1000);
  }
  catch(MessageQueueException e)
  {
    printf("Serveur non en ligne...\n");
    exit(1);
  }

  // Ouverture de la fenetre graphique
  Trace("(THREAD MAIN %d) Ouverture de la fenetre graphique",pthread_self()); fflush(stdout);
  if (OuvertureFenetreGraphique("client") < 0)
  {
    Trace("Erreur de OuvrirGrilleSDL\n");
    exit(1);
  }

  // Exemple d'utilisations des libriaires --> code à supprimer
  EVENT_GRILLE_SDL event;
  bool ok = false;
  while(!ok)
  {
    event = ReadEvent();
    if (event.type == CROIX) ok = 1;
    if (event.type == CLAVIER && event.touche == 'q') ok = 1;
    if (event.type == CLIC_GAUCHE) 
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

      if (repTir.status == TOUCHE) DessineExplosion(repTir.L+11,repTir.C,ORANGE);
      else DessineCible(event.ligne,event.colonne);
    }
  }

  // Fermeture de la grille de jeu (SDL)
  Trace("(THREAD MAIN %d) Fermeture de la fenetre graphique...",pthread_self()); fflush(stdout);
  FermetureFenetreGraphique();
  Trace("(THREAD MAIN %d) OK Fin",pthread_self()); //fflush(stdout);

  exit(0);
}


