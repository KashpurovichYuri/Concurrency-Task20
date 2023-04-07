#pragma once

#include <exception>
#include <memory>
#include <mutex>
#include <stack>
#include <stdexcept>

template < typename T >
class Threadsafe_Stack
{
public:

	Threadsafe_Stack() = default;

	Threadsafe_Stack(const Threadsafe_Stack & other)
	{
		std::lock_guard < std::mutex > lock(other.m_mutex);
		m_data = other.m_data;
	}

	Threadsafe_Stack& operator=(const Threadsafe_Stack &) = delete;

public:

	bool push(T value);

	std::shared_ptr < T > pop();

	bool pop(T & value);

	bool empty() const;

private:

	std::stack < T >   m_data;
	mutable std::mutex m_mutex;

};
