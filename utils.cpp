#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <limits.h>
#include "Ecran.h"
#include "GrilleSDL.h"
#include "Ressources.h"
#include "MessageQueue.h"

#include "protocole.h"


int waitTime(int sec, long nsec)
{
	if(sec < 0 || nsec < 0 || nsec > 999999999)
	{
		Trace("Erreur nanosleep\n");
		return -1;
	}

	struct timespec time;
	time.tv_sec = sec;
	time.tv_nsec = nsec;
	return nanosleep(&time, NULL);
}

int waitRand(long min, long max)
{
	long number = (rand()%(max-min))+min;
	
	return waitTime(number/1000000000, number%1000000000);
}

pthread_t tidSelf()
{
	return pthread_self()/*%INT_MAX*/;
}
































