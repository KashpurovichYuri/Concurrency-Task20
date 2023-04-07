#pragma once

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <numeric>
#include <stack>
#include <queue>
#include <boost/lockfree/stack.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/atomic.hpp>
#include <boost/function.hpp>
#include <boost/bind/bind.hpp>
#include <boost/core/ref.hpp>
#include "../timer/timer.hpp"
#include "../compare_performance04/threadsafe_stack.hpp"
#include "../compare_performance04/threadsafe_queue.hpp"

template < typename Container >
void producer(Container & container, const unsigned int M, boost::atomic < bool > & flag)
{
    while (!flag.load());

    unsigned int i = 0;
    while (i < M)
    {
        auto value = 0;
        while (!container.push(value));
        ++i;
    }
}

template < typename Container >
void consumer(Container & container, const unsigned int M, boost::atomic < bool > & flag)
{
    while (!flag.load());
    while (container.empty())
        std::this_thread::yield();
    
    int value;
    unsigned int i = 0;
    while (i < M)
        if (container.pop(value))
            ++i;
}

int main()
{
    const auto N = 8;
    const unsigned int M = 1000000;
    std::vector < double > times;
    
    for (auto i = 0; i < 8; ++i)
    {
        boost::thread_group producer_threads, consumer_threads;

        boost::lockfree::queue < int > queue(128);

        boost::atomic < bool > flag = false;

        Timer timer{ "operations in lock-free containers", std::cout };
        for (auto i = 0; i < N; ++i)
            producer_threads.create_thread(boost::bind(
                producer < boost::lockfree::queue < int > >,
                boost::ref(queue),
                M,
                boost::ref(flag)));

        for (auto i = 0; i < N; ++i)
            consumer_threads.create_thread(boost::bind(
                consumer < boost::lockfree::queue < int > >,
                boost::ref(queue),
                M,
                boost::ref(flag)));

        flag = true;
        producer_threads.join_all();
        consumer_threads.join_all();
        timer.stop(); // before getting time...
        times.push_back(timer.get_time());
    }
    auto time1 = std::accumulate(std::begin(times), std::end(times), 0.0) / times.size();

    std::cout << N << '\n' << M << '\n' << time1 << " c\n" << std::endl;

    for (auto i = 0; i < 100; ++i)
    {
        boost::thread_group producer_threads, consumer_threads;

        Threadsafe_Queue < int > queue;

        boost::atomic < bool > flag = false;

        Timer timer{ "operations in lock-free containers", std::cout };
        for (auto i = 0; i < N; ++i)
            producer_threads.create_thread(boost::bind(
                producer < Threadsafe_Queue < int > >,
                boost::ref(queue),
                M,
                boost::ref(flag)));

        for (auto i = 0; i < N; ++i)
            consumer_threads.create_thread(boost::bind(
                consumer < Threadsafe_Queue < int > >,
                boost::ref(queue),
                M,
                boost::ref(flag)));

        flag = true;
        producer_threads.join_all();
        consumer_threads.join_all();
        timer.stop(); // before getting time...
        times.push_back(timer.get_time());
    }
    auto time2 = std::accumulate(std::begin(times), std::end(times), 0.0) / times.size();
    
    std::cout << N << '\n' << M << '\n' << time2 << " c\n" << std::endl;

    system("pause");

    return EXIT_SUCCESS;
}
