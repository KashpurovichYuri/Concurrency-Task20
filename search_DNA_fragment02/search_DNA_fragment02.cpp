#include <algorithm>
#include <future>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <functional>
#include <set>

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

auto search_substr(
	const std::string & str,
	const std::string & substr,
	const size_t left,
	const size_t right,
	std::set < size_t > & ind,
	std::mutex & mutex)
{
	size_t pos = left;

	while (pos < right)
	{
		pos = str.find(substr, pos);
		if (pos != std::string::npos)
		{
			std::scoped_lock lock(mutex); // we also may use just mutex.lock() and mutex.unlock() to block emplace operation
			ind.emplace(pos);
			pos += 1;
		}
		else
			pos = right;
		// we have also correctly process edges of string because find will search substr even out of right boundary (but with beginning till it)...
	}
}

auto par_search_subseq(
	const std::string & str,
	const std::string & substr,
	std::set < size_t > & ind)
{
	const std::size_t min_per_thread = 25;
	const std::size_t max_threads =
		(str.size() + min_per_thread - 1) / min_per_thread;

	const std::size_t hardware_threads =
		std::thread::hardware_concurrency();

	const std::size_t num_threads =
		std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

	const std::size_t block_size = str.size() / num_threads;

	std::vector < std::future < void > > futures(num_threads - 1);
	std::vector < std::thread >          threads(num_threads - 1);

	Threads_Guard guard(threads);

	size_t block_start = 0;
	size_t block_end = block_start + block_size;
	std::mutex mutex;

	for (auto i = 0; i < (num_threads - 1); ++i)
	{
		std::packaged_task < void (
			const std::string &,
			const std::string &,
			const size_t,
			const size_t,
			std::set < size_t > &,
			std::mutex &) >	task{ search_substr };

		futures[i] = task.get_future();
		threads[i] = std::thread(std::move(task), std::ref(str), std::ref(substr), block_start, block_end, std::ref(ind), std::ref(mutex));

		block_start += block_size;
		block_end += block_size;
	}

	search_substr(str, substr, block_start, block_end, ind, mutex);

	for (std::size_t i = 0; i < (num_threads - 1); ++i)
		futures[i].get();
}


int main()
{
	//auto DNA = "AAA";
	auto DNA = "AGTCGGTATCTGATCGTTAGCTCGCGCTTGTTAGAGTCGCGATAGCGCCGCTGTATAGGAGATCTCTCTAGGAAACACTTTGCGATATAGCTCGTAGCTGATCGTCGGCTCGAGGCTAAGCTGAAAACAGGTACGGCAGATGCA";
	//auto DNA = "GTAGTAGTAGTAGTAGTAGTA";
	std::string subseq;
	std::cin >> subseq;

	std::set < size_t > ind;
	par_search_subseq(DNA, subseq, ind);
	for (auto index : ind)
		std::cout << index << " ";
}