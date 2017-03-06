#include <unistd.h>
#include "MessageQueue.h"

MessageQueue connexion;

typedef struct
{
  char msg[20];
  int  a;
} REQ;

int main()
{
  try
  {
    //MessageQueue connexion(100);
    connexion.init(100);
    connexion.info();
    sleep(10);
    connexion.info();
  
    Message m;
    m = connexion.ReceiveData(-8);
    printf("Message recu :\n");
    printf("Type = %d\n",m.getType());
    printf("requete = %d\n",m.getRequete());
    printf("expediteur = %d\n",m.getExpediteur());
/*    char data[200];
    memcpy(data,m.getData(),m.getNbBytes());
    REQ *pReq = (REQ*)data;
    printf("msg = %s\n",pReq->msg);
    printf("a = %d\n",pReq->a);
*/

    if (m.getNbBytes() == 0) cout << "Taille donnee nulle" << endl;
    if (m.getData() == NULL) cout << "Pointeur donnee NULL" << endl;

    connexion.close();
  }
  catch(MessageQueueException e)
  {
    cout << "ERREUR: " << e.getMessage() << endl;
  }
  
  return 0;
}
