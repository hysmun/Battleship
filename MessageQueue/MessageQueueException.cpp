#include "MessageQueueException.h"

//***************************************************************************
//********** Constructeurs **************************************************
//***************************************************************************
MessageQueueException::MessageQueueException(void)
{
  //cout << ">>> MessageQueueException : constructeur par defaut <<<" << endl;
  message = NULL;
  setMessage("message");
}

MessageQueueException::MessageQueueException(const char* m)
{
  // cout << ">>> MessageQueueException : constructeur d'initialisation (" << m << ") <<<" << endl;
  message = NULL;
  setMessage(m);
}

MessageQueueException::MessageQueueException(const MessageQueueException &e)
{
  // cout << ">>> MessageQueueException : constructeur de copie <<<" << endl;
  message = NULL;
  setMessage(e.getMessage());
}

//***************************************************************************
//********** Desctructeur ***************************************************
//***************************************************************************
MessageQueueException::~MessageQueueException()
{
  // cout << ">>> MessageQueueException : destructeur (" << getNom() << ") <<<" << endl;
  if (message) delete message;
}

//***************************************************************************
//********** SETTERS ********************************************************
//***************************************************************************
void MessageQueueException::setMessage(const char* m)
{
  if (m == NULL) return;
  if (message) delete message;
  message = new char[strlen(m)+1];
  strcpy(message,m);
}


//***************************************************************************
//********** GETTERS ********************************************************
//***************************************************************************
char* MessageQueueException::getMessage() const { return message; }

//***************************************************************************
//********** Methodes d'instance ********************************************
//***************************************************************************

//***************************************************************************
//********** Surcharges d'operateurs ****************************************
//***************************************************************************
