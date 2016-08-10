#pragma once
#include <memory>
#include <stack>
#include <mutex>
#include <stdexcept>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <chrono>
#include <functional>
#include <stdexcept>
#include "include/easylogging++.h"
template <class T, class D = std::default_delete<T>>
class AsyncObjectPool
{
	std::shared_ptr<AsyncObjectPool<T, D>* > this_ptr_;
	std::queue<std::unique_ptr<T, D> > pool_;
	std::mutex m_object;

	std::vector< std::thread > workers_;
	std::condition_variable condition_;
	std::atomic_bool stop_;
private:
	struct RetToPoolDel_t {
		explicit RetToPoolDel_t(std::weak_ptr<AsyncObjectPool<T, D>* > pool) : pool_(pool) {}
		void operator()(T* ptr) {	delete ptr; 	}
		private:
			std::weak_ptr<AsyncObjectPool<T, D>* > pool_;
	};
public:
	using ptr_type = std::unique_ptr<T, RetToPoolDel_t>;

	AsyncObjectPool(size_t threads) : this_ptr_(new AsyncObjectPool<T, D>*(this)), stop_(false) {
		for(size_t i = 0;i<threads;++i)
        workers_.emplace_back(
            [this]
            {
                for(;;)
                {
                    {
                        std::unique_lock<std::mutex> lock{this->m_object};
                        this->condition_.wait(lock, [this]{ return this->stop_ || !this->pool_.empty(); });
                        if(this->stop_ && this->pool_.empty())  return;
						//LOG(DEBUG)<<std::this_thread::get_id()<<" ,size: "<< this->size();
						ptr_type tmp(pool_.front().release(), RetToPoolDel_t{std::weak_ptr<AsyncObjectPool<T, D>*>{this_ptr_}});
						//LOG(DEBUG)<<std::this_thread::get_id()<<" ,size1: "<< this->size();
						{
							tmp->print(10001);
							//std::this_thread::sleep_for(std::chrono::seconds(1));
						}
                        this->pool_.pop();
						//LOG(DEBUG)<<std::this_thread::get_id()<<" ,size2: "<< this->size();
                    }
                }
            }
		);
	}
	virtual ~AsyncObjectPool(){ 
		{
			std::unique_lock<std::mutex> lock{this->m_object};
			stop_ = true;
		}
		condition_.notify_all();
		for(auto& ele : this->workers_) ele.join();
	}

	void enqueue(std::unique_ptr<T, D> t) {
		std::lock_guard<std::mutex> lock{m_object};
		if(this->stop_){ 
			LOG(ERROR)<<"enqueue on stopped ThreadPool"; }
		else{
			pool_.push(std::move(t)); 
			condition_.notify_one();
			//condition_.notify_all();
		}
	}
	bool empty() const { return pool_.empty(); }
	size_t size() const { return pool_.size(); }
	ptr_type acquire() {
		std::lock_guard<std::mutex> lock{m_object};
		this->condition_.wait(lock, [this]{ return this->stop_ || !this->pool_.empty(); });
		if(this->stop_ && this->pool_.empty())  return ptr_type(std::unique_ptr<T>(new T()));
		//throw std::out_of_range("Cannot acquire object from an empty pool.");
		ptr_type tmp(pool_.front().release(), RetToPoolDel_t{std::weak_ptr<AsyncObjectPool<T, D>*>{this_ptr_}});
		pool_.pop();
		return std::move(tmp);
	}
};
