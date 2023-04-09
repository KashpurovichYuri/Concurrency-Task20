#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <execution>
#include <thread>
#include <future>
#include <functional>
#include <mutex>

auto estimate_pi_seq_geom(const size_t N, size_t & Nin, size_t & Nout, std::mutex & mutex)
{
	std::random_device seed;
	std::uniform_real_distribution < double > distribution(-1, 1);
	std::mt19937 engine(seed());
	
	size_t in = 0;
	size_t out = 0;

	for (auto i = 0; i < N; ++i)
	{
		double x = distribution(engine);
		double y = distribution(engine);

		if (x * x + y * y < 1)
			++in;
		++out;
	}

	std::scoped_lock lock(mutex);
	Nin += in;
	Nout += out;
}

class Threads_Guard
{
public:

	explicit Threads_Guard(std::vector < std::thread > & threads) :
		m_threads(threads)
	{}

	Threads_Guard			(Threads_Guard const&) = delete;

	Threads_Guard& operator=(Threads_Guard const&) = delete;

	~Threads_Guard() noexcept
	{
		try
		{
			for (std::size_t i = 0; i < m_threads.size(); ++i)
			{
				if (m_threads[i].joinable())
				{
					m_threads[i].join();
				}
			}
		}
		catch (...)
		{
			// std::abort();
		}
	}

private:

	std::vector < std::thread > & m_threads;
};

double estimate_pi_par(const size_t N)
{
	const std::size_t min_per_thread = 25;
	const std::size_t max_threads =
		(N + min_per_thread - 1) / min_per_thread;

	const std::size_t hardware_threads =
		std::thread::hardware_concurrency();

	const std::size_t num_threads =
		std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

	const std::size_t block_size = N / num_threads;

	std::vector < std::future < void > > futures(num_threads - 1);
	std::vector < std::thread >          threads(num_threads - 1);

	Threads_Guard guard(threads);

	auto block_start = 0.0;
	size_t Nin = 0;
	size_t Nout = 0;
	std::mutex mutex;

	for (auto i = 0; i < (num_threads - 1); ++i)
	{
		block_start += block_size;

		std::packaged_task < void (const size_t, size_t &, size_t &, std::mutex &) > task{ estimate_pi_seq_geom };

		futures[i] = task.get_future();
		threads[i] = std::thread(std::move(task), block_start, std::ref(Nin), std::ref(Nout), std::ref(mutex));
	}

	estimate_pi_seq_geom(block_start, Nin, Nout, mutex);

	for (std::size_t i = 0; i < (num_threads - 1); ++i)
		futures[i].get();

	auto result = 4.0 * Nin / Nout;
	return result;
}

int main()
{
	const size_t N = 10000000;
	auto pi_par = estimate_pi_par(N);
	std::cout << pi_par << std::endl;
}
