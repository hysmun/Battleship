all:	ServeurMQ ClientMQ

ServeurMQ:	ServeurMQ.cpp MessageQueue.o Message.o MessageQueueException.o
		g++ ServeurMQ.cpp MessageQueue.o Message.o MessageQueueException.o -o ServeurMQ

ClientMQ:	ClientMQ.cpp MessageQueue.o Message.o MessageQueueException.o
		g++ ClientMQ.cpp MessageQueue.o Message.o MessageQueueException.o -o ClientMQ

MessageQueue.o:	MessageQueue.cpp MessageQueue.h
		g++ MessageQueue.cpp -c -DTRACE

Message.o:	Message.cpp Message.h
		g++ Message.cpp -c

MessageQueueException.o:	MessageQueueException.cpp MessageQueueException.h
				g++ MessageQueueException.cpp -c
