#ifndef _SYNC_WAIT_MSG_H_
#define _SYNC_WAIT_MSG_H_

#include "Poco/Event.h"

class CSyncWait
{
public:
	CSyncWait();
	~CSyncWait();

	bool waitMsg(int msg1, int msg2 = INVALID_WAIT_MSG, int waitTime = INFINITE);

	void msgCome(int msg);

	void reset();


private:
	int _msg1;
	int _msg2;
	Poco::Event _event;

	enum
	{
		INVALID_WAIT_MSG  = 0
	};
};

#endif