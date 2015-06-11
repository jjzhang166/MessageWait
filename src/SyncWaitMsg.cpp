#include "syncwaitmsg.h"

CSyncWait::CSyncWait()
{
	_msg1 = INVALID_WAIT_MSG;
	_msg2 = INVALID_WAIT_MSG;
}

CSyncWait::~CSyncWait()
{

}

bool CSyncWait::waitMsg(int msg1, int msg2/* = INVALID_WAIT_MSG*/, int waitTime/* = INFINITE*/)
{
	if (INVALID_WAIT_MSG == msg1)
		return false;

	_msg1 = msg1;
	_msg2 = msg2;

	if(INFINITE == waitTime)
	{
		_event.wait();
	}
	else
	{
		return _event.tryWait(waitTime);
	}

	return true;
}

void CSyncWait::msgCome(int msg)
{
	if(msg == _msg1 || msg == _msg1)
	{
		reset();
	}
}

void CSyncWait::reset()
{
	_msg1 = INVALID_WAIT_MSG;
	_msg2 = INVALID_WAIT_MSG;
	_event.set();
}

