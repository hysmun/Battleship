#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "Ecran.h"
#include "GrilleSDL.h"
#include "Ressources.h"
#include "MessageQueue.h"

#include "protocole.h"




int searchPosBateau(Bateau *pBateau, int ***tab, int *lignes, int *colonnes, int nbLignes, int nbColonnes)
{
	if(pBateau == NULL || tab == NULL || lignes == NULL || colonnes==NULL)
	{
		Trace("Erreur param searchBateau");
		pthread_exit(0);
	}
	int posOK=0;
	Trace("SeachPosBateau");
	for(int i=0; i<nbLignes*nbColonnes; i++)
	{
		Trace("pos = %d", posOK);
		if(pBateau->direction == HORIZONTAL)
		{
			//bateau horizontal
			if(i%nbColonnes == 0)
				posOK =0;
			Trace("OK");
			
			if(*((tab+(i/nbLignes))[i%nbColonnes]) == 0)
			{
				posOK++;
			}
			else
			{
				posOK=0;
			}
			
			Trace("OK");
			if(posOK == pBateau->type)
			{
				pBateau->L = (i/nbLignes);
				pBateau->C = (i%nbColonnes)-(pBateau->type-1);
				//lignes[i/nbLignes] = 1;
				Trace("Trouver !\n");
				i = nbColonnes*nbLignes;
			}
			Trace("OK");
		}
		else
		{
			//bateau vertical
			if(i%nbLignes == 0)
				posOK =0;
			if(*tab[i%nbLignes][i/nbColonnes] == 0)
			{
				posOK++;
			}
			else
			{
				posOK=0;
			}
			if(posOK == pBateau->type)
			{
				pBateau->L = (i%nbLignes)-(pBateau->type-1);
				pBateau->C = i/nbColonnes;
				//colonnes[i/nbColonnes] = 1;
				Trace("Trouver !\n");
				i = nbColonnes*nbLignes;
			}
		}
	}
	return posOK;
}

int DessineFullBateau(Bateau *pBateau, int *tab[])
{
	for(int i; i<pBateau->type; i++)
	{
		if(pBateau->direction == HORIZONTAL)
		{
			DessineBateau(pBateau->L, pBateau->C+i, pBateau->type, HORIZONTAL,i);
			tab[pBateau->L][pBateau->C+i] = pthread_self();
		}
		else
		{
			DessineBateau(pBateau->L+i, pBateau->C, pBateau->type, VERTICAL,i);
			tab[pBateau->L+i][pBateau->C] = pthread_self();
		}
	}
	return 1;
}































