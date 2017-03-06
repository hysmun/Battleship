#ifndef MESSAGE_H
#define MESSAGE_H

#include <unistd.h>

#include "MessageQueueException.h"

class Message
{
  private:
    int    type;
    pid_t  expediteur;
    int    requete;
    char   *data;
    int    nbBytes;
    
  public:
    Message();
    Message(long t,int r,const char* d,unsigned int n) throw (MessageQueueException);
    Message(const Message& m);
    ~Message();

    Message& operator=(const Message& m);

    void setType(long t) throw (MessageQueueException);
    void setRequete(int r);
    void setData(const char* d,unsigned int n) throw (MessageQueueException);
    // Mise en place de la charge utile du message vu comme un paquet de bytes
    // d = adresse du paquet de bytes
    // n = taille du paquet de bytes (en nombre de bytes)

    long getType() const;
    pid_t getExpediteur() const;
    int  getRequete() const;
    const char* getData() const;
    int  getNbBytes() const;

  friend class MessageQueue;
};

#endif

