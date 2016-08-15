/*
 * =====================================================================================
 *
 *       Filename:  myutils.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/12/16 20:10:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Normandie (sams), romalin99@gmail.com
 *   Organization:  |ORGANIZATION |
 *
 * =====================================================================================
 */
#include <chrono>
#include <functional>
#include <future>
#include <type_traits>
#include "include/easylogging++.h"
using namespace std::chrono;
/**
 *  @brief Give the elapsed time of any function
 *
 *  @param func_name Used in printf
 *  @param func The function that to be measured
 *  @param args Arguments of the given function
 *
 *  @return The return value of the given function
 */
template <class Function, class ... Args>
std::future<typename std::result_of<Function(Args...)>::type> runtime(const char * func_name, Function&& func, Args&&... args) 
{
    typedef typename std::result_of<Function(Args...)>::type return_type;
    typedef std::packaged_task<return_type()> task; // std::function
    auto t = std::make_shared<task>(std::bind(std::forward<Function>(func), std::forward<Args>(args)...));
    auto start = std::chrono::high_resolution_clock::now();
    auto ret = t->get_future();
    {
        (*t)();
    }
    auto end = std::chrono::high_resolution_clock::now();
	auto totalus = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    //printf("%s: %ldus\n", func_name, totalus);
	LOG(DEBUG)<<func_name<<" elapsed time: "<<totalus<<"us";
    return ret;
}

