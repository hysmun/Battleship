#ifndef MESSAGE_QUEUE_EXCEPTION_H
#define MESSAGE_QUEUE_EXCEPTION_H

#include <iostream>
#include <stdlib.h>
#include <string.h>

using namespace std;

class MessageQueueException
{
  private :
    char* message;

  public:
    MessageQueueException();
    MessageQueueException(const char* m);
    MessageQueueException(const MessageQueueException &e);
    ~MessageQueueException();

    void  setMessage(const char* m);
    char* getMessage() const;
};

#endif
