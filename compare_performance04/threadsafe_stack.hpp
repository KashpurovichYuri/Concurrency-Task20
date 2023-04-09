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

	Threadsafe_Stack(const Threadsafe_Stack& other)
	{
		std::lock_guard < std::mutex > lock(other.m_mutex);
		m_data = other.m_data;
	}

	Threadsafe_Stack& operator=(const Threadsafe_Stack&) = delete;

public:

	bool push(T value)
	{
		std::lock_guard < std::mutex > lock(m_mutex);
		m_data.push(value);

		return true;
	}

	std::shared_ptr < T > pop()
	{
		std::lock_guard < std::mutex > lock(m_mutex);

		if (m_data.empty())
		{
			throw std::range_error("empty stack");
		}

		const auto result = std::make_shared < T >(m_data.top());
		m_data.pop();

		return result;
	}

	bool pop(T & value)
	{
		std::lock_guard < std::mutex > lock(m_mutex);

		if (m_data.empty())
		{
			return false;
		}

		value = m_data.top();
		m_data.pop();

		return true;
	}

	bool empty() const
	{
		std::lock_guard < std::mutex > lock(m_mutex);
		return m_data.empty();
	}

private:

	std::stack < T > m_data;

private:

	mutable std::mutex m_mutex;
};