#include "Ressources.h"
#include "GrilleSDL.h"

// Macros pour les sprites
#define S_CUIRASSE_0_V              300000
#define S_CUIRASSE_1_V              300001
#define S_CUIRASSE_2_V              300002
#define S_CUIRASSE_3_V              300003
#define S_CUIRASSE_4_V              300004
#define S_CUIRASSE_0_H              300005
#define S_CUIRASSE_1_H              300006
#define S_CUIRASSE_2_H              300007
#define S_CUIRASSE_3_H              300008
#define S_CUIRASSE_4_H              300009

#define S_CROISEUR_0_V              300010
#define S_CROISEUR_1_V              300011
#define S_CROISEUR_2_V              300012
#define S_CROISEUR_3_V              300013
#define S_CROISEUR_0_H              300014
#define S_CROISEUR_1_H              300015
#define S_CROISEUR_2_H              300016
#define S_CROISEUR_3_H              300017

#define S_DESTROYER_0_V             300018
#define S_DESTROYER_1_V             300019
#define S_DESTROYER_2_V             300020
#define S_DESTROYER_0_H             300021
#define S_DESTROYER_1_H             300022
#define S_DESTROYER_2_H             300023

#define S_TORPILLEUR_0_V            300024
#define S_TORPILLEUR_1_V            300025
#define S_TORPILLEUR_0_H            300026
#define S_TORPILLEUR_1_H            300027

#define S_CIBLE                     300030
#define S_CIBLE_VERROUILLEE         300031
#define S_EXPLOSION                 300032
#define S_EXPLOSION_BLEUE           300033

#define S_ZERO                      300070
#define S_UN                        300071
#define S_DEUX                      300072
#define S_TROIS                     300073
#define S_QUATRE                    300074
#define S_CINQ                      300075
#define S_SIX                       300076
#define S_SEPT                      300077
#define S_HUIT                      300078
#define S_NEUF                      300079

#define S_SOUSMARIN_VERT            300090
#define S_SOUSMARIN_ORANGE          300091
#define S_FLECHE_VERTE              300092
#define S_FLECHE_ROUGE              300093
#define S_MISSILE1                  300094
#define S_MISSILE2                  300095
#define S_MISSILE3                  300096

void ChargementImages(const char* who)
{
  // Definition de l'image de fond
  if (strcmp(who,"serveur") == 0) DessineImageFond("./images/fond32.bmp");
  else DessineImageFond("./images/fondDouble32.bmp");

  // Sprites
  AjouteSpriteAFondTransparent(S_CUIRASSE_0_V,"./images/cuirasse0_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CUIRASSE_1_V,"./images/cuirasse1_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CUIRASSE_2_V,"./images/cuirasse2_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CUIRASSE_3_V,"./images/cuirasse3_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CUIRASSE_4_V,"./images/cuirasse4_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CUIRASSE_0_H,"./images/cuirasse0_h.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CUIRASSE_1_H,"./images/cuirasse1_h.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CUIRASSE_2_H,"./images/cuirasse2_h.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CUIRASSE_3_H,"./images/cuirasse3_h.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CUIRASSE_4_H,"./images/cuirasse4_h.bmp",255,255,255);

  AjouteSpriteAFondTransparent(S_CROISEUR_0_V,"./images/croiseur0_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CROISEUR_1_V,"./images/croiseur1_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CROISEUR_2_V,"./images/croiseur2_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CROISEUR_3_V,"./images/croiseur3_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CROISEUR_0_H,"./images/croiseur0_h.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CROISEUR_1_H,"./images/croiseur1_h.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CROISEUR_2_H,"./images/croiseur2_h.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CROISEUR_3_H,"./images/croiseur3_h.bmp",255,255,255);

  AjouteSpriteAFondTransparent(S_DESTROYER_0_V,"./images/destroyer0_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_DESTROYER_1_V,"./images/destroyer1_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_DESTROYER_2_V,"./images/destroyer2_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_DESTROYER_0_H,"./images/destroyer0_h.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_DESTROYER_1_H,"./images/destroyer1_h.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_DESTROYER_2_H,"./images/destroyer2_h.bmp",255,255,255);

  AjouteSpriteAFondTransparent(S_TORPILLEUR_0_V,"./images/torpilleur0_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_TORPILLEUR_1_V,"./images/torpilleur1_v.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_TORPILLEUR_0_H,"./images/torpilleur0_h.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_TORPILLEUR_1_H,"./images/torpilleur1_h.bmp",255,255,255);

  AjouteSpriteAFondTransparent(S_EXPLOSION,"./images/explosion32.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_EXPLOSION_BLEUE,"./images/explosionBleue32.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CIBLE,"./images/cible32.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CIBLE_VERROUILLEE,"./images/cibleVerrouillee32.bmp",255,255,255);

  AjouteSpriteAFondTransparent(S_ZERO,"./images/Zero32.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_UN,"./images/Un32.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_DEUX,"./images/Deux32.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_TROIS,"./images/Trois32.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_QUATRE,"./images/Quatre32.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_CINQ,"./images/Cinq32.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_SIX,"./images/Six32.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_SEPT,"./images/Sept32.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_HUIT,"./images/Huit32.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_NEUF,"./images/Neuf32.bmp",255,255,255);

  AjouteSpriteAFondTransparent(S_SOUSMARIN_VERT,"./images/boutonSousMarinVert.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_SOUSMARIN_ORANGE,"./images/boutonSousMarinOrange.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_FLECHE_ROUGE,"./images/flecheRouge.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_FLECHE_VERTE,"./images/flecheVerte.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_MISSILE1,"./images/missiles1.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_MISSILE2,"./images/missiles2.bmp",255,255,255);
  AjouteSpriteAFondTransparent(S_MISSILE3,"./images/missiles3.bmp",255,255,255);
}

// Fonctions Ressources
int OuvertureFenetreGraphique(const char* who)
{
  if (strcmp(who,"serveur") == 0)
  {
    if (OuvrirGrilleSDL(10,10,32,"BATTLESHIP (S)","./images/IconeFenetre256.bmp") < 0)
      return -1;
  }
  else
  {
    if (OuvrirGrilleSDL(21,10,32,"BATTLESHIP","./images/IconeFenetre256.bmp") < 0)
      return -1;
  }
 
  ChargementImages(who);
 
  return 0;
}

int FermetureFenetreGraphique()
{
  FermerGrilleSDL();
  return 0;
}

void DessineBateau(int l,int c,int type,int direction,int indice)
{
  if (indice >= type) return;

  if (direction == VERTICAL)
  {
    switch(type)
    {
      case CUIRASSE: if (indice == 0) DessineSprite(l,c,S_CUIRASSE_0_V);
                     if (indice == 1) DessineSprite(l,c,S_CUIRASSE_1_V);
                     if (indice == 2) DessineSprite(l,c,S_CUIRASSE_2_V);
                     if (indice == 3) DessineSprite(l,c,S_CUIRASSE_3_V);
                     if (indice == 4) DessineSprite(l,c,S_CUIRASSE_4_V); 
                     break; 

      case CROISEUR: if (indice == 0) DessineSprite(l,c,S_CROISEUR_0_V);
                     if (indice == 1) DessineSprite(l,c,S_CROISEUR_1_V);
                     if (indice == 2) DessineSprite(l,c,S_CROISEUR_2_V);
                     if (indice == 3) DessineSprite(l,c,S_CROISEUR_3_V);
                     break; 


      case DESTROYER: if (indice == 0) DessineSprite(l,c,S_DESTROYER_0_V);
                      if (indice == 1) DessineSprite(l,c,S_DESTROYER_1_V);
                      if (indice == 2) DessineSprite(l,c,S_DESTROYER_2_V);
                      break;

      case TORPILLEUR: if (indice == 0) DessineSprite(l,c,S_TORPILLEUR_0_V);
                       if (indice == 1) DessineSprite(l,c,S_TORPILLEUR_1_V);
                       break;
    }
  }
  else
  {
    switch(type)
    {
      case CUIRASSE: if (indice == 0) DessineSprite(l,c,S_CUIRASSE_0_H);
                     if (indice == 1) DessineSprite(l,c,S_CUIRASSE_1_H);
                     if (indice == 2) DessineSprite(l,c,S_CUIRASSE_2_H);
                     if (indice == 3) DessineSprite(l,c,S_CUIRASSE_3_H);
                     if (indice == 4) DessineSprite(l,c,S_CUIRASSE_4_H); 
                     break; 

      case CROISEUR: if (indice == 0) DessineSprite(l,c,S_CROISEUR_0_H);
                     if (indice == 1) DessineSprite(l,c,S_CROISEUR_1_H);
                     if (indice == 2) DessineSprite(l,c,S_CROISEUR_2_H);
                     if (indice == 3) DessineSprite(l,c,S_CROISEUR_3_H);
                     break; 


      case DESTROYER: if (indice == 0) DessineSprite(l,c,S_DESTROYER_0_H);
                      if (indice == 1) DessineSprite(l,c,S_DESTROYER_1_H);
                      if (indice == 2) DessineSprite(l,c,S_DESTROYER_2_H);
                      break;

      case TORPILLEUR: if (indice == 0) DessineSprite(l,c,S_TORPILLEUR_0_H);
                       if (indice == 1) DessineSprite(l,c,S_TORPILLEUR_1_H);
                       break;
    }
  }
}

void DessineCible(int l,int c)
{
  DessineSprite(l,c,S_CIBLE);
}

void DessineCibleVerrouillee(int l,int c)
{
  DessineSprite(l,c,S_CIBLE_VERROUILLEE);
}

void DessineExplosion(int l,int c,int couleur)
{
  if (couleur == BLEU) DessineSprite(l,c,S_EXPLOSION_BLEUE); 
  else DessineSprite(l,c,S_EXPLOSION);
}

void DessineChiffre(int l,int c,int chiffre)
{
  switch(chiffre)
  {
    case 0 : DessineSprite(l,c,S_ZERO); break;
    case 1 : DessineSprite(l,c,S_UN); break;
    case 2 : DessineSprite(l,c,S_DEUX); break;
    case 3 : DessineSprite(l,c,S_TROIS); break;
    case 4 : DessineSprite(l,c,S_QUATRE); break;
    case 5 : DessineSprite(l,c,S_CINQ); break;
    case 6 : DessineSprite(l,c,S_SIX); break;
    case 7 : DessineSprite(l,c,S_SEPT); break;
    case 8 : DessineSprite(l,c,S_HUIT); break;
    case 9 : DessineSprite(l,c,S_NEUF); break;
    default : DessineSprite(l,c,S_ZERO); break;    
  }
}

void DessineBoutonSousMarin(int l,int c,int couleur)
{
  if (couleur == VERT) DessineSprite(l,c,S_SOUSMARIN_VERT);
  else DessineSprite(l,c,S_SOUSMARIN_ORANGE);
}

void DessineFleche(int l,int c,int couleur)
{
  if (couleur == VERT) DessineSprite(l,c,S_FLECHE_VERTE);
  else DessineSprite(l,c,S_FLECHE_ROUGE);
}

void DessineMissiles(int l,int c,int nbMissiles)
{
  switch(nbMissiles)
  {
    case 1 : DessineSprite(l,c,S_MISSILE1); break;
    case 2 : DessineSprite(l,c,S_MISSILE2); break;
    case 3 : DessineSprite(l,c,S_MISSILE3); break;
    default : EffaceCarre(l,c); break;    
  }
}



