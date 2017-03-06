#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "MessageQueueException.h"
#include "Message.h"

class MessageQueue
{
  private:
    int id;
    bool createur;

  public:
    MessageQueue();
    // Objet File de message cree mais aucune IPC creee réellement --> utililsation de init()
    MessageQueue(int key) throw (MessageQueueException);
    // IPC creee avec la cle key
    MessageQueue(const MessageQueue& mq);
    // Creation d'un nouvel objet representant la meme IPC
    ~MessageQueue() throw (MessageQueueException);
    // Destruction de l'objet et suppression de l'IPC

    MessageQueue& operator=(const MessageQueue& mq);
    // l'objet de gauche represente la meme IPC que l'objet de droite

    void init(int key) throw (MessageQueueException);
    // Cree l'IPC de cle key pour l'objet courant
    void close() throw (MessageQueueException);
    // Supprimer l'IPC pour l'objet courant
    void connect(int key) throw (MessageQueueException);
    // Recupere une IPC existante --> a utiliser par le client qui se connecte
    int  getKey() const throw (MessageQueueException);
    // Retourne la cle de l'IPC
    bool isCreator() const throw (MessageQueueException);
    // Retourne true si c'est l'objet courant qui a cree l'IPC
    void info() const throw (MessageQueueException);
    // Affiche les infos de l'IPC

    void    SendData(const Message& m) throw (MessageQueueException);
    // Envoie un message sur la file de message (le type doit etre specifie dans m)
    Message ReceiveData(int type) throw (MessageQueueException);
    // Attend et lit un message sur la file de message
};

#endif

