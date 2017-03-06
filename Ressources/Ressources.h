#ifndef RESSOURCES_H
#define RESSOURCES_H

// Macros (type) pour les bateaux (à utiliser avec DessineBateau declaree ci-dessous)
#define CUIRASSE        5   // le code d'un bateau correspond egalement a sa longueur !!!
#define CROISEUR        4
#define DESTROYER       3
#define TORPILLEUR      2

// Macros (direction) pour les bateaux (à utiliser avec DessineBateau declaree ci-dessous)
#define HORIZONTAL      40000
#define VERTICAL        40001

// Macros (couleur) pour les boutons sous-marin (à utiliser avec les fonctions declarees ci-dessous)
#define VERT            40002 // pour BoutonSousMarin et Fleche
#define ORANGE          40003 // pour BoutonSousMarin et couleur d'explosion
#define BLEU            40004 // pour couleur d'explosion
#define ROUGE           40004 // pour Fleche

int  OuvertureFenetreGraphique(const char* who);
// Pour le serveur, who = "serveur"
// Pour le client, who = "client"
int  FermetureFenetreGraphique();

void DessineBateau(int l,int c,int type,int direction,int indice);
// (l,c) = position de la case d'indice 0 du bateau
// type = CUIRASSE ou ... ; direction = HORIZONTAL ou VERTICAL ; indice = 0,1,...,longueurBateau-1
void DessineCible(int l,int c);
void DessineCibleVerrouillee(int l,int c);
void DessineExplosion(int l,int c,int couleur);
void DessineBoutonSousMarin(int l,int c,int couleur);
void DessineFleche(int l,int c,int couleur);
void DessineChiffre(int l,int c,int chiffre);
void DessineMissiles(int l,int c,int nbMissiles);

// Pour effacer une case, utiliser la fonction EffaceCarre definie dans GrilleSDL.h

#endif
