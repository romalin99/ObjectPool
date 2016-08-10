#include <stdlib.h>
#include <iostream>
#include <memory>
#include <assert.h>
#include <stack>
#include <stdexcept>
#include "AsyncObjectPool.h"
using namespace std;

INITIALIZE_EASYLOGGINGPP
void Load_Logging() {
	el::Configurations conf("./conf/mdsp-logger.conf");
	el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
	el::Loggers::reconfigureAllLoggers(conf);
}
void Init_Resource(){
	Load_Logging();
}
class A{
public:
	A(){}
	A(size_t n):i_(n) { cout<<"A constructor ...\n"; }
	~A(){ cout<<"A deconstructor ...\n"; }
	void print(int a) { cout<<"i = "<<i_<<" ,a="<<a<<endl; }
	size_t i_;
};
int main(int argc, char** argv) {
	Init_Resource();
	AsyncObjectPool<A> pool(std::thread::hardware_concurrency());
    std::vector< std::future<int> > results;

    for(int i = 0; i < 8; ++i) {
		pool.enqueue(std::unique_ptr<A>(new A(i*100+1)));
		std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    cout << endl;
	return 0;
}
