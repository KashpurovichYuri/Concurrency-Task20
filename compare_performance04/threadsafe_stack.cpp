#pragma once

#include <exception>
#include <memory>
#include <mutex>
#include <stack>
#include <stdexcept>

#include "threadsafe_stack.hpp"

template < typename T >
bool Threadsafe_Stack < T >::push(T value)
{
	std::lock_guard < std::mutex > lock(m_mutex);
	m_data.push(value);
	
	return true;
}

template < typename T >
std::shared_ptr < T > Threadsafe_Stack < T >::pop()
{
	std::lock_guard < std::mutex > lock(m_mutex);

	if (m_data.empty())
	{
		throw std::range_error("empty stack");
	}

	const auto result = std::make_shared < T > (m_data.top());
	m_data.pop();

	return result;
}

template < typename T >
bool Threadsafe_Stack < T >::pop(T & value)
{
	std::lock_guard < std::mutex > lock(m_mutex);

	if (m_data.empty())
	{
		throw std::range_error("empty stack");
	}

	value = m_data.top();
	m_data.pop();

	return true;
}

template < typename T >
bool Threadsafe_Stack < T >::empty() const
{
	std::lock_guard < std::mutex > lock(m_mutex);
	return m_data.empty();
}
