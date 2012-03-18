#pragma once
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/phoenix.hpp>

namespace boost
{

class semaphore : boost::noncopyable
{
public:
	explicit semaphore(unsigned int initial_count = 0) : _count(initial_count) {}

	void post()
	{
		boost::mutex::scoped_lock lock(_mutex);
		++_count;
		_cond.notify_one(); 
	}
	void wait()
	{
		boost::mutex::scoped_lock lock(_mutex);
		_cond.wait(lock, [&]() { return _count > 0; });
		--_count;
	}
	bool timed_wait(int ms)
	{
		boost::mutex::scoped_lock lock(_mutex);
		if(!_cond.timed_wait(lock,boost::posix_time::millisec(ms), [&]() { return _count > 0; }))
			return false;
		--_count;
		return true;
	}
	bool timed_wait(boost::system_time const& wait_until)
    {
		boost::mutex::scoped_lock lock(_mutex);
		if(!_cond.timed_wait(lock, wait_until, [&]() { return _count > 0; }))
			return false;
		--_count;
		return true;
    }
    bool timed_wait(boost::xtime const& wait_until)
    {
		boost::mutex::scoped_lock lock(_mutex);
		if(!_cond.timed_wait(lock, wait_until, [&]() { return _count > 0; }))
			return false;
		--_count;
		return true;
    }
    template<typename duration_type>
    bool timed_wait(duration_type const& wait_duration)
    {
		boost::mutex::scoped_lock lock(_mutex);
		if(!_cond.timed_wait(lock, wait_duration, [&]() { return _count > 0; }))
			return false;
		--_count;
		return true;
    }
protected:
	unsigned int _count;
	boost::mutex _mutex;
	boost::condition_variable _cond;

};

}