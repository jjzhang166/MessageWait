

#ifndef AsyncWaitMsg_INCLUDED
#define AsyncWaitMsg_INCLUDED

#include <map>
#include <vector>
#include <list>
#include "Poco/Timer.h"
#include "boost/function.hpp"
#include "boost/make_shared.hpp"


typedef boost::function<void (int msg, int param1, int param2)> MsgCallback;
typedef boost::function<void (int msg)> MsgTimeOutCallback;


enum CallbackType
{
	Always,
	Once
};

const long Infinity = 0xffffffff;

class CallbackItemBase
{
public:
	CallbackItemBase() 
	:_callback(NULL), _timeOutCallback(NULL), _timeout(1000)
	{}
	virtual ~CallbackItemBase() {}
	MsgCallback _callback;
	MsgTimeOutCallback _timeOutCallback;
	signed long _timeout;	// will be changed in RegMsgTimeOutCallback function.
};

class NormalCallbackItem :public CallbackItemBase
{
public:
	NormalCallbackItem() 
	:_type(Always), _msg(0)
	{}
	virtual ~NormalCallbackItem() {}
	CallbackType _type;
	int _msg;
};

class MutiCallbackItem :public CallbackItemBase
{
public:
	MutiCallbackItem() {}
	virtual ~MutiCallbackItem() {}

	std::vector<int> _msgVec;
};

class AsyncWaitMsg
{
public:
	static AsyncWaitMsg& instance()
	{
		static AsyncWaitMsg mc;
		return mc;
	}

	bool RemoveListener(int msg, boost::shared_ptr<CallbackItemBase>);

	void RegMsgTimeOutCallback(int msg, boost::shared_ptr<CallbackItemBase> pCallbackItem);

	void Dispatch(int msg, int param1, int param2);

	void MsgTimeOutChecking(Poco::Timer& timer);

	bool AddListener(boost::shared_ptr<MutiCallbackItem>);

	bool AddListener(boost::shared_ptr<NormalCallbackItem>);

	void Reset();
	void Stop();

private:
	AsyncWaitMsg();
	AsyncWaitMsg(const AsyncWaitMsg&);
	AsyncWaitMsg& operator= (const AsyncWaitMsg&);
	bool AddListener(int msg, boost::shared_ptr<CallbackItemBase>);

	typedef std::vector<boost::shared_ptr<CallbackItemBase> > CallbackItemVec;
	typedef std::map<int, CallbackItemVec> CallbackMap;
	CallbackMap _callbackMap;   // message dispatch map

	typedef std::list<boost::shared_ptr<CallbackItemBase> > TimeOutCallbackItemList;
	TimeOutCallbackItemList _timeoutCallbackList;	// timeout callback function list
};



#endif  // AsyncWaitMsg_INCLUDED
