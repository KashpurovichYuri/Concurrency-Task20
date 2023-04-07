#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

template < typename T >
class Threadsafe_Queue
{
public:

	Threadsafe_Queue() = default;

	Threadsafe_Queue(const Threadsafe_Queue & other)
	{
		std::lock_guard < std::mutex > lock(other.m_mutex);
		m_queue = other.m_queue;
	}

	Threadsafe_Queue & operator=(const Threadsafe_Queue & other)
	{
		std::scoped_lock lock(m_mutex, other.m_mutex);
		m_queue = other.m_queue;
	}

public:

	bool push(T value);

	bool pop(T & value);

	std::shared_ptr < T > wait_and_pop();

	bool try_pop(T & value);

	std::shared_ptr < T > try_pop();

	bool empty() const;

private:

	std::queue < T >		m_queue;
	std::condition_variable m_condition_variable;
	mutable std::mutex      m_mutex;
};