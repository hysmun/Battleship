#include "Message.h"

//**************************************************************************************
//***** Constructeurs et Destructeur ***************************************************
//**************************************************************************************
Message::Message()
{
  type = 1;
  expediteur = -1;
  requete = 0;
  data = NULL;
  nbBytes = 0;
}

//**************************************************************************************
Message::Message(long t,int r,const char* d,unsigned int n) throw (MessageQueueException)
{
  setType(t);
  expediteur = -1;
  setRequete(r);
  data = NULL;
  setData(d,n);
}

//**************************************************************************************
Message::Message(const Message& m)
{
  setType(m.getType());
  expediteur = m.getExpediteur();
  setRequete(m.getRequete());
  data = NULL;
  setData(m.getData(),m.getNbBytes());
}

//**************************************************************************************
Message::~Message()
{
  if (data) delete data;
}

//**************************************************************************************
//***** Surcharge d'operateurs *********************************************************
//**************************************************************************************
Message& Message::operator=(const Message& m)
{
  setType(m.getType());
  expediteur = m.getExpediteur();
  setRequete(m.getRequete());
  setData(m.getData(),m.getNbBytes());
}

//**************************************************************************************
//***** Setters/Getters ****************************************************************
//**************************************************************************************
void Message::setType(long t) throw (MessageQueueException)
{
  if (t <= 0)
    throw MessageQueueException("Erreur de Message.setType: Type de message invalide");
  type = t;
}

void Message::setRequete(int r)
{
  requete = r;
}

void Message::setData(const char* d,unsigned int n) throw (MessageQueueException)
{ 
  if (d == NULL && n!=0)
    throw MessageQueueException("Erreur de Message.setData: Donnee invalide");
  if (n<0)
    throw MessageQueueException("Erreur de Message.setData: Taille de donnee invalide");

  if (data) delete data;
  if (n==0)
  {
    data = NULL;
    nbBytes = 0;
    return;
  }

  nbBytes = n;
  data = new char[nbBytes];
  memcpy(data,d,nbBytes);
}

long Message::getType() const
{
  return type;
}

pid_t Message::getExpediteur() const
{
  return expediteur;
}

int Message::getRequete() const 
{
  return requete;
}

const char* Message::getData() const 
{
  return data;
}

int Message::getNbBytes() const 
{
  return nbBytes;
}
