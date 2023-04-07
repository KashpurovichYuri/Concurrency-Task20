#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <iostream>
#include <algorithm>

template < 
	typename T,
	typename Container = std::vector < T >,
	typename Compare = std::less< typename Container::value_type >
	>
class Threadsafe_Priority_Queue
{
public:

	Threadsafe_Priority_Queue() = default;

	Threadsafe_Priority_Queue(const Threadsafe_Priority_Queue & other)
	{
		std::scoped_lock lock(other.m_mutex);
		m_priority_queue = other.m_priority_queue;
	}

	Threadsafe_Priority_Queue & operator=(const Threadsafe_Priority_Queue & other)
	{
		std::scoped_lock lock(m_mutex, other.m_mutex);
		m_priority_queue = other.m_priority_queue;
	}

	Threadsafe_Priority_Queue(Threadsafe_Priority_Queue && other)
	{
		std::scoped_lock lock(other.m_mutex);
		m_priority_queue = std::move(other.m_priority_queue);
		m_condition_variable = std::move(other.m_condition_variable);
		m_mutex = std::move(other.m_mutex);
	}

	Threadsafe_Priority_Queue & operator= (Threadsafe_Priority_Queue && other)
	{
		std::scoped_lock lock(other.m_mutex);
		m_priority_queue = std::move(other.m_priority_queue);
		m_condition_variable = std::move(other.m_condition_variable);
		m_mutex = std::move(other.m_mutex);
	}

public:

	auto push(T value)
	{
		std::scoped_lock lock(m_mutex);
		m_priority_queue.push(value);
		m_condition_variable.notify_one();
	}

	
	template < typename... Args > 
	auto emplace(Args... args)
	{
		std::scoped_lock lock(m_mutex);
		m_priority_queue.emplace(args...);
		m_condition_variable.notify_one();
	}

	/*
	template < typename Type >
	auto emplace(Type value)
	{
		std::scoped_lock lock(m_recursive_mutex);
		m_priority_queue.emplace(value);
		m_condition_variable.notify_one();
	}

	template < typename Type, typename... Args > 
	auto emplace(Type value, Args... args)
	{
		std::scoped_lock lock(m_recursive_mutex);
		m_priority_queue.emplace(value, args...);
		m_condition_variable.notify_one();
	}
	if we want to write an own emplace() method
	*/

	auto wait_and_pop(T & value)
	{
		std::unique_lock lock(m_mutex);

		m_condition_variable.wait(lock, [this] {return !m_priority_queue.empty(); });
		value = m_priority_queue.front();
		m_priority_queue.pop();
	}

	auto wait_and_pop()
	{
		std::unique_lock lock(m_mutex);
		
		m_condition_variable.wait(lock, [this] {return !m_priority_queue.empty(); });
		auto result = std::make_shared < T > (m_priority_queue.top());
		m_priority_queue.pop();
		
		return result;
	}

	auto try_pop(T & value)
	{
		std::scoped_lock lock(m_mutex);
		
		if (m_priority_queue.empty())
		{
			return false;
		}

		value = m_priority_queue.top();
		m_priority_queue.pop();

		return true;
	}

	auto try_pop()
	{
		std::scoped_lock < std::mutex > lock(m_mutex);
		
		if (m_priority_queue.empty())
		{
			return std::shared_ptr < T > ();
		}
			
		auto result = std::make_shared < T > (m_priority_queue.front());
		m_priority_queue.pop();
		
		return result;
	}

	auto swap(Threadsafe_Priority_Queue & other) noexcept
	{
		std::unique_lock lock(m_mutex);
		std::swap(this.m_mutex, other.m_mutex);
		std::swap(this.m_condition_variable, other.m_condition_variable);
		std::swap(this.m_priority_queue, other.m_priority_queue);
	}

	auto empty() const
	{
		std::scoped_lock lock(m_mutex);
		return m_priority_queue.empty();
	}

	auto size() const
	{
		std::scoped_lock lock(m_mutex);
		return m_priority_queue.size();
	}

private:

	std::priority_queue < T, Container, Compare > m_priority_queue;
	std::condition_variable                   m_condition_variable;
	mutable std::mutex m_mutex;
	// mutable std::recursive_mutex m_recursive_mutex; // for own emplace() method
};

int main()
{
	Threadsafe_Priority_Queue < int > priority_queue;

	priority_queue.push(42);

	auto ptr = priority_queue.wait_and_pop();

	int value;

	bool result = priority_queue.try_pop(value);

	priority_queue.emplace(49);
}