#include <chrono>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>

class timer_exception : public std::exception 
{
public:
	explicit timer_exception(const std::string & message) noexcept :
		what_arg (message.c_str())
	{}

	explicit timer_exception(const char * const message) noexcept :
		what_arg (message)
	{}

	virtual const char* what() const noexcept override
	{
		return what_arg;
	}

	~timer_exception() noexcept = default;

private:
	const char* what_arg;
};

class Timer
{
private:
			
	using clock_t = std::chrono::steady_clock;
	using time_point_t = clock_t::time_point;

public:

	template < typename Name, typename Enable = 
		std::enable_if_t < std::is_convertible_v < Name, std::string > > >
	explicit Timer(Name && name, std::ostream & stream) :
		m_name(std::forward < Name > (name)), m_stream(stream), m_begin(clock_t::now()), m_time(0)
	{
		initialize();
	}

	~Timer() noexcept
	{
		try
		{
			uninitialize();
		}
		catch (...)
		{
			// std::abort();
		}
}

private:

	void initialize();

	void uninitialize();

public:

	void stop() noexcept;

	void resume() noexcept;

	void elapsed();

	const auto get_time() noexcept
	{
		return static_cast < double > (m_time.count()) / 1'000'000.0;
	}

private:

	std::string m_name;
	time_point_t m_begin;
	std::chrono::microseconds m_time;

private:

	std::ostream & m_stream;
};
