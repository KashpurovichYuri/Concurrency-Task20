#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

#include "threadsafe_queue.hpp"

template < typename T >
bool Threadsafe_Queue < T >::push(T value)
{
	std::lock_guard < std::mutex > lock(m_mutex);
	m_queue.push(value);
	m_condition_variable.notify_one();

	return true;
}

template < typename T >
bool Threadsafe_Queue < T >::pop(T & value)
{
	std::unique_lock < std::mutex > lock(m_mutex);
		
	m_condition_variable.wait(lock, [this] {return !m_queue.empty(); });
	value = m_queue.front();
	m_queue.pop();

	return true;
}

template < typename T >
std::shared_ptr < T > Threadsafe_Queue < T >::wait_and_pop()
{
	std::unique_lock < std::mutex > lock(m_mutex);
		
	m_condition_variable.wait(lock, [this] {return !m_queue.empty(); });
	auto result = std::make_shared < T > (m_queue.front());
	m_queue.pop();
		
	return result;
}

template < typename T >
bool Threadsafe_Queue < T >::try_pop(T & value)
{
	std::lock_guard < std::mutex > lock(m_mutex);
		
	if (m_queue.empty())
	{
		return false;
	}

	value = m_queue.front();
	m_queue.pop();

	return true;
}

template < typename T >
std::shared_ptr < T > Threadsafe_Queue < T >::try_pop()
{
	std::lock_guard < std::mutex > lock(m_mutex);
		
	if (m_queue.empty())
	{
		return std::shared_ptr < T > ();
	}
			
	auto result = std::make_shared < T > (m_queue.front());
	m_queue.pop();
		
	return result;
}

template < typename T >
bool Threadsafe_Queue < T >::empty() const
{
	std::lock_guard < std::mutex > lock(m_mutex);
	return m_queue.empty();
}