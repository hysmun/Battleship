#include "MessageQueue.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <pwd.h>

//**************************************************************************************
//***** Structure interne des messages *************************************************
//**************************************************************************************
struct MS
{
  long  type;
  pid_t expediteur;
  int   requete;
  char  data[1];
};

//**************************************************************************************
//***** Constructeurs et Destructeur ***************************************************
//**************************************************************************************
MessageQueue::MessageQueue()
{
  #ifdef TRACE
  cerr << "MessageQueue: constructeur par defaut" << endl;
  #endif

  id = -1;
  createur = false;
}

//**************************************************************************************
MessageQueue::MessageQueue(int key) throw (MessageQueueException)
{
  #ifdef TRACE
  cerr << "MessageQueue: constructeur d'initialisation (key=" << key << ")" << endl;
  #endif

  if ((id = msgget((key_t)key,IPC_CREAT | IPC_EXCL | 0777)) == -1)
  {
    // La file de messages existe deja, on tente de recuperer son id
    createur = false;
    if ((id = msgget((key_t)key,0)) == -1)
      throw MessageQueueException("Erreur de recuperation de la file de message (msgget)");

    #ifdef TRACE
    cerr << "MessageQueue: Recuperation File de messages OK (key=" << key << ")" << endl;
    #endif
  }
  else 
  {
    #ifdef TRACE
    cerr << "MessageQueue: Creation File de message OK (key=" << key << ")" << endl;
    #endif

    createur = true;
  }
}

//**************************************************************************************
MessageQueue::MessageQueue(const MessageQueue& mq)
{
  #ifdef TRACE
  cerr << "MessageQueue: constructeur de copie" << endl;
  #endif

  id = mq.id;
  createur = false;
}

//**************************************************************************************
MessageQueue::~MessageQueue() throw (MessageQueueException)
{
  #ifdef TRACE
  cerr << "MessageQueue: destructeur" << endl;
  #endif

  if (createur && id!=-1) // C'est le processus qui cree la file de message qui la supprime
  {
    if (msgctl(id,IPC_RMID,0))
      throw MessageQueueException("Erreur de ~MessageQueue (msgctl)");

    #ifdef TRACE
    cerr << "MessageQueue: Suppression File de message OK" << endl;
    #endif
  }
}

//**************************************************************************************
//***** Surcharge d'operateurs *********************************************************
//**************************************************************************************
MessageQueue& MessageQueue::operator=(const MessageQueue& mq)
{
  id = mq.id;
  createur = false;
}

//**************************************************************************************
//***** Methodes d'instance ************************************************************
//**************************************************************************************
void MessageQueue::init(int key) throw (MessageQueueException)
{
  if (id != -1)
    throw MessageQueueException("Erreur de MessageQueue.init: File deja initialisee");

  if ((id = msgget((key_t)key,IPC_CREAT | IPC_EXCL | 0777)) == -1)
    throw MessageQueueException("Erreur de MessageQueue.init (msgget)");

  #ifdef TRACE
  cerr << "MessageQueue.init: Creation File de messages OK (key=" << key << ")" << endl;
  #endif

  createur = true;
}

//**************************************************************************************
void MessageQueue::close() throw (MessageQueueException)
{
  if (createur && id!=-1) // C'est le processus qui cree la file de message qui la supprime
  {
    if (msgctl(id,IPC_RMID,0))
      throw MessageQueueException("Erreur de MessageQueue.close (msgctl)");

    #ifdef TRACE
    cerr << "MessageQueue.close: Suppression File de messages OK" << endl;
    #endif
  }

  id = -1;
  createur = false;

  #ifdef TRACE
  cerr << "MessageQueue.close: Deconnexion OK" << endl;
  #endif
}

//**************************************************************************************
void MessageQueue::connect(int key) throw (MessageQueueException)
{
  if (id != -1)
    throw MessageQueueException("Erreur de MessageQueue.connect: File deja initialisee");

  if ((id = msgget((key_t)key,0)) == -1)
    throw MessageQueueException("Erreur de MessageQueue.connect (msgget)");

  createur = false;

  #ifdef TRACE
  cerr << "MessageQueue.connect: Recuperation File de message OK (key=" << key << ")" << endl;
  #endif
}

//**************************************************************************************
int MessageQueue::getKey() const throw (MessageQueueException)
{
  if (id==-1)
    throw MessageQueueException("Erreur de MessageQueue.getKey: File non initialisee");

  struct msqid_ds StructMsg;
  if (msgctl(id,IPC_STAT,&StructMsg) == -1)
    throw MessageQueueException("Erreur de MessageQueue.getKey (msgctl)");

  return (int)StructMsg.msg_perm.key;
}

//**************************************************************************************
bool MessageQueue::isCreator() const throw (MessageQueueException)
{
  if (id==-1)
    throw MessageQueueException("Erreur de MessageQueue.isCreator: File non initialisee");

  return createur;
}

//**************************************************************************************
void MessageQueue::info() const throw (MessageQueueException)
{
  if (id == -1)
    throw MessageQueueException("Erreur de MessageQueue.info: File non initialisee");

  struct msqid_ds StructMsg;
  if (msgctl(id,IPC_STAT,&StructMsg) == -1)
    throw MessageQueueException("Erreur de MessageQueue.info (msgctl)");

  cout << "--- MessageQueue.info ---" << endl;
  cout << "Key: " <<  (int)StructMsg.msg_perm.key << endl;
  cout << "Createur: ";
  if (createur) cout << "oui" << endl;
  else cout << "non" << endl;
  struct passwd StructPass;
  memcpy(&StructPass,getpwuid(StructMsg.msg_perm.uid),sizeof(struct passwd));
  cout << "Proprietaire: " << StructPass.pw_name << endl;
  cout << "Nb Messages: " << StructMsg.msg_qnum << endl;
  cout << "PID dernier emetteur: " << StructMsg.msg_lspid << endl;
  cout << "PID dernier recepteur: " << StructMsg.msg_lrpid << endl;
  cout << "-------------------------" << endl;
}

//**************************************************************************************
//***** Emission / Reception ***********************************************************
//**************************************************************************************
void MessageQueue::SendData(const Message& m) throw (MessageQueueException)
{
  if (id == -1)
    throw MessageQueueException("Erreur de MessageQueue.SendData: non connecte");
  MS *p;
  char *d;
  int taille = m.getNbBytes() + sizeof(long) + sizeof(pid_t) + sizeof(int);
  d = new char[taille];
  p = (MS*)d;
  p->type = m.getType();
  p->expediteur = getpid();
  p->requete = m.getRequete();
  if (m.getNbBytes() > 0) memcpy(p->data,m.getData(),m.getNbBytes());
  if (msgsnd(id,p,taille-sizeof(long),0) < 0)
    throw MessageQueueException("Erreur de MessageQueue.SendData (msgsnd)");
  delete p;
}

//**************************************************************************************
Message MessageQueue::ReceiveData(int type) throw (MessageQueueException)
{
  if (id == -1)
    throw MessageQueueException("Erreur de MessageQueue.ReceiveData: non connecte");
  int rc;
  MS *p;
  char *d;
  d = new char[200]; // taille maximale d'un message
  p = (MS*)d;
  if ((rc = msgrcv(id,p,200,type,0)) == -1)
  {
    delete d;
    throw MessageQueueException("Erreur de MessageQueue.ReceiveData (msgrcv)");
  }
  Message m;
  m.setType(p->type);
  m.expediteur = p->expediteur;
  int nbBytes = rc - sizeof(pid_t) - sizeof(int);
  if (nbBytes > 0) m.setData(p->data,nbBytes);
  else m.setData(NULL,0);
  m.setRequete(p->requete);
  delete d;
  return m; 
}
