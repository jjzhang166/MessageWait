#include "AsyncWaitMsg.h"

#include "Poco/Timestamp.h"
#include "Poco/Timer.h"
#include "Poco/Event.h"
#include "Poco/Mutex.h"

#include <iostream>
#include <assert.h>
#include <algorithm>

using Poco::Timestamp;
using Poco::Timer;
using Poco::TimerCallback;
using Poco::FastMutex;
using namespace std;

namespace
{
	Poco::Timer s_timeoutTimer(0, 200);
	bool s_TimerStarted = false;
	bool s_waitEvent = false;
	Poco::Event s_event;
	Poco::FastMutex s_mutex;
}


MessageCenter::MessageCenter()
{

}


bool MessageCenter::AddListener(int msg, boost::shared_ptr<CallbackItemBase> pSrcItem)
{
	assert(pSrcItem);

	FastMutex::ScopedLock lock(s_mutex);
	if (_callbackMap.find(msg) != _callbackMap.end())
	{
		CallbackItemVec& callbackItemVec = _callbackMap[msg];

		bool find = false;
		for (CallbackItemVec::iterator it = callbackItemVec.begin(); it != callbackItemVec.end(); ++it)
		{
			if ((*it) == pSrcItem) 
			{
				find = true;
				break;
			}
		}

		// add if not exist.
		if (!find)
		{
			_callbackMap[msg].push_back(pSrcItem);
		}
	}
	else
	{
		CallbackItemVec newItemVec;
		newItemVec.push_back(pSrcItem);
		_callbackMap.insert(CallbackMap::value_type(msg, newItemVec));
	}

	return true;
}

bool MessageCenter::AddListener(boost::shared_ptr<NormalCallbackItem> pSrcItem)
{
	assert(pSrcItem);

	s_mutex.lock();
	for (TimeOutCallbackItemList::iterator it = _timeoutCallbackList.begin();
		it != _timeoutCallbackList.end(); ++it)
	{
		boost::shared_ptr<NormalCallbackItem> pNormalCallbackItem = boost::dynamic_pointer_cast<NormalCallbackItem>(*it); 

		if (pNormalCallbackItem)
		{
			if (pNormalCallbackItem->_msg == pSrcItem->_msg 
				&& pNormalCallbackItem->_callback.target<MsgCallback>() == pSrcItem->_callback.target<MsgCallback>()
				&& pNormalCallbackItem->_timeOutCallback.target<MsgTimeOutCallback>() == pSrcItem->_timeOutCallback.target<MsgTimeOutCallback>())
			{
				return false;	// Already exist,but _timeout may not equal.
			}
		}
		else
		{
			continue;
		}
	}
	s_mutex.unlock();

	bool succress = AddListener(pSrcItem->_msg, pSrcItem);

	if (succress && pSrcItem->_timeout != Infinity)
	{
		if (pSrcItem->_timeOutCallback != NULL)
		{
			RegMsgTimeOutCallback(pSrcItem->_msg, pSrcItem);
		}
	}
	else
	{
		return false;
	}

	return true;
}


bool MessageCenter::AddListener(boost::shared_ptr<MutiCallbackItem> pSrcItem)
{
	assert(pSrcItem);
	if (pSrcItem->_msgVec.empty())
		return false;

	s_mutex.lock();
	for (TimeOutCallbackItemList::iterator it = _timeoutCallbackList.begin();
		it != _timeoutCallbackList.end(); ++it)
	{
		boost::shared_ptr<MutiCallbackItem> pMutiCallbackItem = boost::dynamic_pointer_cast<MutiCallbackItem>(*it); 

		if (pMutiCallbackItem)
		{
			if (pMutiCallbackItem->_msgVec == pSrcItem->_msgVec 
				&& pMutiCallbackItem->_callback.target<MsgCallback>() == pSrcItem->_callback.target<MsgCallback>()
				&& pMutiCallbackItem->_timeOutCallback.target<MsgTimeOutCallback>() == pSrcItem->_timeOutCallback.target<MsgTimeOutCallback>())
			{
				return false;	// Already exist,but _timeout may not equal.
			}
		}
		else
		{
			continue;
		}
	}
	s_mutex.unlock();

	bool success = false;

	for (vector<int>::iterator it = pSrcItem->_msgVec.begin();
		it != pSrcItem->_msgVec.end(); ++it)
	{
		success = AddListener(*it, pSrcItem);

		if (!success)
		{
			cout << "Failed to add at least one" << endl;
			return false;
		}
	}

	if (success && pSrcItem->_timeout != Infinity)
	{
		if (pSrcItem->_timeOutCallback != NULL)
		{
			RegMsgTimeOutCallback(pSrcItem->_msgVec[0], pSrcItem);
		}
	}

	return true;
}

bool MessageCenter::RemoveListener(int msg, boost::shared_ptr<CallbackItemBase> pCallbackItem)
{
	FastMutex::ScopedLock lock(s_mutex);
	if (_callbackMap.find(msg) != _callbackMap.end())
	{
		CallbackItemVec& callbackItemVec= _callbackMap[msg];
		CallbackItemVec::iterator result = 
			find(callbackItemVec.begin(), callbackItemVec.end(), pCallbackItem);
		if (result != callbackItemVec.end())
		{
			callbackItemVec.erase(result);
			return true;
		}
	}
	return false;
}

// From small to large 
void MessageCenter::RegMsgTimeOutCallback(int msg, boost::shared_ptr<CallbackItemBase> pCallbackItem)
{
	assert(pCallbackItem);

	pCallbackItem->_timeout *= 1000;
	Timestamp now;
	pCallbackItem->_timeout += (signed long)now.epochMicroseconds();  // XXX

	FastMutex::ScopedLock lock(s_mutex);
	bool find = false;
	TimeOutCallbackItemList::iterator it = _timeoutCallbackList.begin();
	while (it != _timeoutCallbackList.end())
	{
		if ((*it)->_timeout > pCallbackItem->_timeout)
		{
			find = true;
		}
		++it;
	}

	if (find)
	{
		_timeoutCallbackList.insert(it, pCallbackItem);
	}
	else
	{
		_timeoutCallbackList.push_back(pCallbackItem);
	}

	if (!_timeoutCallbackList.empty())
	{
		if (!s_TimerStarted)
		{
			// open timer
			TimerCallback<MessageCenter> tc(*this, &MessageCenter::MsgTimeOutChecking);
			s_timeoutTimer.start(tc);
			s_TimerStarted = true;
			cout << "_timeoutTimer.start" << endl;
		}
		else if (s_waitEvent)
		{
			s_event.set();
			cout << "_event.set()" << endl;
			s_waitEvent = false;
		}
	}
}

void MessageCenter::Dispatch(int msg, int param1, int param2)
{
	FastMutex::ScopedLock lock(s_mutex);
	// Erase the relational items of the message from the _timeoutCallbackList.
	for (TimeOutCallbackItemList::iterator it = _timeoutCallbackList.begin(); it != _timeoutCallbackList.end(); ++it)
	{
		boost::shared_ptr<NormalCallbackItem> pCallbackItem = boost::dynamic_pointer_cast<NormalCallbackItem>(*it); 
		if (pCallbackItem)
		{
			if (pCallbackItem->_msg == msg)
			{
				_timeoutCallbackList.erase(it);
				break;
			}
		}
		else
		{
			boost::shared_ptr<MutiCallbackItem> pCallbackItem = boost::dynamic_pointer_cast<MutiCallbackItem>(*it); 
			if (pCallbackItem)
			{
				vector<int>::iterator result = 
					find(pCallbackItem->_msgVec.begin(), pCallbackItem->_msgVec.end(), msg);
				if (result != pCallbackItem->_msgVec.end())
				{
					_timeoutCallbackList.erase(it);
					break;
				}
			}
		}
	}
		// Call back.And, erase the message from callbacklist if the callback vector of the message is empty.
		if (_callbackMap.find(msg) != _callbackMap.end())
		{
			CallbackItemVec& callbackItemVec = _callbackMap[msg];

			for (CallbackItemVec::iterator it = callbackItemVec.begin(); it != callbackItemVec.end();)
			{
				boost::shared_ptr<NormalCallbackItem> pCallbackItem = boost::dynamic_pointer_cast<NormalCallbackItem>(*it); 

				if (pCallbackItem)
				{
					if (pCallbackItem->_callback != NULL)
					{
						pCallbackItem->_callback(msg, param1, param2);
					}

					if (pCallbackItem->_type == Once)
					{
						it = callbackItemVec.erase(it);
					}
					else
					{
						++it;
					}
				}
				else
				{
					boost::shared_ptr<MutiCallbackItem> pCallbackItem = boost::dynamic_pointer_cast<MutiCallbackItem>(*it); 
					if (pCallbackItem)
					{
						if (pCallbackItem->_callback != NULL)
						{
							pCallbackItem->_callback(msg, param1, param2);
						}

						it = callbackItemVec.erase(it);

						// Since one of the messages in pCallbackItem->_msgVec is come,
						// we should erase other messages in pCallbackItem->_msgVec
						for (vector<int>::iterator it2 = pCallbackItem->_msgVec.begin(); 
							it2 != pCallbackItem->_msgVec.end(); ++it2)
						{
							if (_callbackMap.find(*it2) != _callbackMap.end() && msg != *it2)
							{
								CallbackItemVec& itemVec = _callbackMap[*it2];
								CallbackItemVec::iterator result = 
									find(itemVec.begin(), itemVec.end(), pCallbackItem); 
								if (result != itemVec.end())
								{
									itemVec.erase(result);

									// erase the message from callbacklist if the callback vector of the message is empty.
									if (itemVec.empty())
									{
										_callbackMap.erase(*it2);
									}
									continue;
								}
							}
						}
					}
				}
			}
			// erase the message from callbacklist if the callback vector of the message is empty.
			if (callbackItemVec.empty())
			{
				_callbackMap.erase(msg);
			}

		}
}

void MessageCenter::MsgTimeOutChecking(Poco::Timer& timer)
{
	FastMutex::ScopedLock lock(s_mutex);
	// stop timer if _timeoutCallbackList is empty
	if (_timeoutCallbackList.empty())
	{
		cout << "_event.wait()" << endl;
		s_waitEvent = true;
		s_mutex.unlock();
		s_event.wait();	
	}

	Timestamp now;
	signed long currentTicks = (signed long)now.epochMicroseconds(); // XXX System.DateTime.Now.Ticks;
	for (TimeOutCallbackItemList::iterator it =_timeoutCallbackList.begin(); it != _timeoutCallbackList.end();)
	{
		if ((*it)->_timeout <= currentTicks) // time out
		{
			if ((*it)->_timeOutCallback != NULL)
			{
				boost::shared_ptr<NormalCallbackItem> pCallbackItem = boost::dynamic_pointer_cast<NormalCallbackItem>(*it); 

				if (pCallbackItem)
				{
					pCallbackItem->_timeOutCallback(pCallbackItem->_msg);

					RemoveListener(pCallbackItem->_msg, *it);
				}
				else
				{
					boost::shared_ptr<MutiCallbackItem> pCallbackItem = boost::dynamic_pointer_cast<MutiCallbackItem>(*it); 
					if (pCallbackItem)
					{
						pCallbackItem->_timeOutCallback(pCallbackItem->_msgVec[0]);

						for (vector<int>::iterator itvec = pCallbackItem->_msgVec.begin(); 
							itvec != pCallbackItem->_msgVec.end(); ++itvec)
						{
							RemoveListener(*itvec, *it);
						}
					}
				}	
			}

			it = _timeoutCallbackList.erase(it);
		}
		else
		{
			//已按小到大排序
			break;
		}
	}
}

void MessageCenter::Reset()
{
	s_event.reset();
	s_waitEvent = true;
	s_timeoutTimer.restart();

	FastMutex::ScopedLock lock(s_mutex);
	_callbackMap.clear();
	_timeoutCallbackList.clear();
}

void MessageCenter::Stop()
{
	s_event.set();
	s_timeoutTimer.stop();
}