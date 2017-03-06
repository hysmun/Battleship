#include "MessageQueue.h"
#include "Message.h"

MessageQueue connexion;

typedef struct
{
  char msg[20];
  int  a;
} REQ;

int main()
{
  connexion.connect(100);
  printf("Cle File: %d\n",connexion.getKey());
  connexion.info();

  //Message m(5,10,"coucou",7),m2;
  //m2 = m;

/*  REQ req;
  strcpy(req.msg,"hello !");
  req.a = 17;
  Message m;
  m.setType(5);
  m.setRequete(10);
  m.setData((char*)&req,sizeof(REQ));
  connexion.SendData(m);
*/

  Message m3;
  m3.setType(5);
  m3.setRequete(10);
  connexion.SendData(m3);

}
