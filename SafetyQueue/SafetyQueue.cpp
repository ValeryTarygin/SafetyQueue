#include <iostream>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <chrono>
#include <memory>

using namespace std;
template <typename T>
class ThreadSafetyQueue
{
    mutable mutex _mutex;
    queue<T> _queue;
    condition_variable _cv;
public:
    ThreadSafetyQueue() {};
    ThreadSafetyQueue(const ThreadSafetyQueue& other)
    {
        lock_guard lck(other._mutex);
        _queue = other._queue;
    }

    ThreadSafetyQueue& operator= (const ThreadSafetyQueue& other)
    {
        lock_guard lck(other._mutex);
        _queue = other._queue;
    }

    void push(const T& val)
    {
        lock_guard<mutex> lck(_mutex);
        _queue.push(val);
        _cv.notify_one();
    }

    void wait_or_pop(T& val)
    {
        unique_lock<mutex> lck(_mutex);
        _cv.wait(lck, [this]() { return !_queue.empty(); });
        val = _queue.front();
        _queue.pop();
    }

    shared_ptr<T> wait_or_pop()
    {
        unique_lock < mutex >lck(_mutex);
        _cv.wait(lck, [this]() { return !_queue.empty(); });
        shared_ptr<T> ret = make_shared<T>(_queue.front());
        _queue.pop();
        return ret;
    }

    shared_ptr<T> try_pop()
	{
		lock_guard<mutex> lck(_mutex);
        if (_queue.empty())
            return make_shared<T>();
        shared_ptr<T> ret = make_shared<T>(_queue.front());
		 _queue.pop();
        return ret;
	}

	bool try_pop(T& val)
	{
		lock_guard<mutex> lck(_mutex);
		if (_queue.empty())
			return false;
		val = _queue.front();
		_queue.pop();
		return true;
	}

    bool empty() const 
    {
        lock_guard<mutex> lck(_mutex);
        return _queue.empty();
    }
};


int main()
{   
    int flagExit_;
    ThreadSafetyQueue<int> queue_;
    vector<thread> vThread_;
    vThread_.push_back(thread([&flagExit_, &queue_]() {
        int newVal;
        while (true)
        {
            cin >> newVal;
            flagExit_ = newVal;
			if (flagExit_ == 10)
				break;  

            queue_.push(newVal);
            this_thread::sleep_for(chrono::milliseconds(20));
        }        
        }));


	vThread_.push_back(thread([&flagExit_, &queue_]() {
		int newVal;
		while (true)
		{
			if (flagExit_ == 10)
				break;

            cout << *queue_.wait_or_pop() << endl;
              this_thread::sleep_for(chrono::milliseconds(5000));
		}
		}));

    for (auto& it : vThread_)
        it.join();

    std::cout << "Hello World!\n";
}
