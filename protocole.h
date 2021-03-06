#ifndef PROTOCOLE_H
#define PROTOCOLE_H

// Type de requete
#define CONNECT       1
#define DECONNECT     2
#define TIR           3
#define SOUSMARIN     4
#define BATEAU_COULE  5

// Sens de deplacement des bateaux
#define DROITE  1
#define GAUCHE  2
#define HAUT    3
#define BAS     4

struct Bateau
{
  int type;
  int L;
  int C;
  int direction;
  int sens;
};

struct RequeteTir
{
  int L;
  int C;
};

struct Position
{
  int L;
  int C;
};

struct ReponseTir
{
  int L;
  int C;
  int status;     // Resultat du tir : valeurs possibles ci-dessous 
  Bateau bateau;  // infos completes du bateau coule
};

struct ComBateau 
{
	pthread_t tidBateau ;// Identifiant du bateau qui utilise cette structure
	Message Requete[5] ; // Requetes transmises par les threads Requete
	int indEcriture ; // indice où doit écrire un Thread Requete
	int indLecture ; // indice où doit lire le thread Bateau
	pthread_mutex_t mutex ; // protège Requete[5], indEcriture, indLecture
	pthread_cond_t cond ; // synchronisation
};

// Valeurs possible de status
#define LOCKED       1  // si cible deja verrouillee par un autre joueur
#define PLOUF        2  // si aucun bateau touche
#define TOUCHE       3  // si bateau touche en (L,C) mais non coule
#define DEJA_TOUCHE  4  // si bateau deja touche en (L,C) par un autre joueur (et forcement pas coule)
#define COULE        5  // si bateau touche en (L,C) et coule
#define ERREUR 		100

#define DRAW 0
#define CLEAR 1

#endif
