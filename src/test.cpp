#include "AsyncWaitMsg.h"
#include "SyncWaitMsg.h"

#include <iostream>
#include <cstdlib>
#include <ctime>


#include "Poco/Exception.h"
#include "Poco/Thread.h"
#include "Poco/Runnable.h"

#include "boost/lambda/lambda.hpp"

using Poco::Thread;
using Poco::Runnable;

using namespace std;

namespace
{
	enum testMessage
	{
		testMessage1 = 1,
		testMessage2,
		testMessage3,
		testMessage4,
		testMessage5
	};

	SyncWaitMsg s_syncWait;
}



void Ontest1(int msg, int param1, int param2)
{
	cout << "ontest1" << endl;
}

void Ontest2(int msg, int param1, int param2)
{
	cout << "ontest2" << endl;
}

void TimeOutCallback1(int msg)
{
	cout << "TimeOutCallback1" << endl;
}

void TimeOutCallback2(int msg)
{
	cout << "TimeOutCallback2" << endl;
}
	
class SyncWaitMsg_Runnable: public Runnable
{
public:
	virtual void run()
	{
		Poco::Thread::sleep(100);
		s_syncWait.MsgCome(testMessage1);
	}
};


class AsyncWaitMsg_Runnable: public Runnable
{
public:
	virtual void run()
	{
		while (TRUE)
		{
			srand((unsigned int)time(0));
			int n = rand() % 4 + 1;

			cout << "AsyncWaitMsg_Runnable rand is: " << n << endl;

			AsyncWaitMsg::instance().Dispatch(n, 0, 0);
			Poco::Thread::sleep(1000);
		}
	}
};

void TestSyncWaitMsg()
{
	Thread thread1;
	SyncWaitMsg_Runnable r1;
	thread1.start(r1);

	bool ret = s_syncWait.WaitMsg(testMessage1, testMessage2);
	assert(true == ret);

	ret = s_syncWait.WaitMsg(testMessage3, testMessage4, 1000);

	assert(false == ret);
}

void TestAsyncWaitMsg()
{
	using namespace boost::lambda;

	// create receive thread
	Thread thread;
	AsyncWaitMsg_Runnable r;
	thread.start(r);
	assert (thread.isRunning());

	std::string local_string = "XXXXXXXXX";

	auto func1 = [local_string] (int msg, int param1, int param2) 
	{ 
		std::cout << "[TestAsyncWaitMsg] -- test1 " << msg << endl << local_string << endl; 
	}; 

	auto func2 = [local_string] (int msg, int param1, int param2) 
	{ 
		std::cout << "[TestAsyncWaitMsg] -- test2 " << msg << endl << local_string << endl; 
	}; 

	// NormalCallbackItem, Please pay attention to "_type"
	boost::shared_ptr<NormalCallbackItem> pItem1(new NormalCallbackItem());
	pItem1->_msg = testMessage1;
	//pItem1->_callback = Ontest1;	// this is ok!
	pItem1->_callback = func1;
	pItem1->_timeout = 2000;
	pItem1->_timeOutCallback = [] (int msg) { std::cout << "test timeout1! " << endl; };
	//pItem1->_type = Once;	// if you want response only one time.
	AsyncWaitMsg::instance().AddListener(pItem1);

	// MutiCallbackItem
 	boost::shared_ptr<MutiCallbackItem> pItem2(new MutiCallbackItem());
 	pItem2->_msgVec.push_back(testMessage2);
 	pItem2->_msgVec.push_back(testMessage3);
 	pItem2->_callback = func2;
 	pItem2->_timeout = 3000;
 	pItem2->_timeOutCallback =  [] (int msg) { std::cout << "test timeout2! " << endl; };
 	AsyncWaitMsg::instance().AddListener(pItem2);
}


int main()
{
	TestSyncWaitMsg();
	TestAsyncWaitMsg();
	
	char c(' ');
	while (c != 'q' && c != 'Q')
	{
		cout << "press q" << endl;
		cin >> c;	
	}

	AsyncWaitMsg::instance().Stop();

	return 0;
}