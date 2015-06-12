#include "syncwaitmsg.h"

CSyncWaitMsg::CSyncWaitMsg()
{
	_msg1 = INVALID_WAIT_MSG;
	_msg2 = INVALID_WAIT_MSG;
}

CSyncWaitMsg::~CSyncWaitMsg()
{

}

bool CSyncWaitMsg::WaitMsg(int msg1, int msg2/* = INVALID_WAIT_MSG*/, int waitTime/* = INFINITE*/)
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

void CSyncWaitMsg::MsgCome(int msg)
{
	if(msg == _msg1 || msg == _msg1)
	{
		Reset();
	}
}

void CSyncWaitMsg::Reset()
{
	_msg1 = INVALID_WAIT_MSG;
	_msg2 = INVALID_WAIT_MSG;
	_event.set();
}

