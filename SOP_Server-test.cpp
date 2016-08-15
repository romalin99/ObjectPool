#include <stdlib.h>
#include <iostream>
#include <memory>
#include <assert.h>
#include <map>
#include <unordered_map>
#include <stack>
#include <stdexcept>
#include "myutils.h"
#include "AsyncObjectPool.h"
#include "SOP_Server-test.h"
using namespace std;
using namespace std::placeholders;
INITIALIZE_EASYLOGGINGPP
void Load_Logging() {
	el::Configurations conf("./conf/mdsp-logger.conf");
	el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
	el::Loggers::reconfigureAllLoggers(conf);
}
void Init_Resource(){ Load_Logging(); }
struct KHbase{
    KHbase(int num) : num_(num) {}
    void print_add(int i) const { cout<<num_ +i<<endl;}
	void ExCmd(ResponseEvent& res, const RequestEvent& req) { cout<<"Event handle ...\n";}
    void ExCmd(ResponseAttr& res, const RequestAttr& req) {cout<<"Attr handle ...\n"; }
	void ExCmd(ResponseSegment& res, const RequestSegment& req) { cout<<"Segment handle ...\n"; }
    int num_;
};
KHbase  hobj(1000);
struct ConObj { 
	RequestEvent m_reReq;
	RequestAttr  m_raReq;
	RequestSegment m_rsReq;
	bool fre = false;// RequestEvent
	bool fra = false;// RequestAttr
	bool frs = false;// RequestSegment
	ConObj(){}
	ConObj(RequestEvent&   req) :m_reReq(req), fre(true) { cout<<"ConObj construct RequestEvent ...... \n"; } 
	ConObj(RequestAttr&    req) :m_raReq(req), fra(true) { cout<<"ConObj construct RequestAttr ...... \n"; } 
	ConObj(RequestSegment& req) :m_rsReq(req), frs(true) { cout<<"ConObj construct RequestSegment ...... \n"; } 
	virtual ~ConObj() { 
		if(fre) cout<<"ConObj RequestEvent deconstruct ...... \n"; 
		if(fra) cout<<"ConObj RequestAttr  deconstruct ...... \n"; 
		if(frs) cout<<"ConObj RequestSegment deconstruct ...... \n"; 
	}
	//std::function<int(int )> m_cb;   A(std::function<int(int)> f ) : m_cb(f) {}
	std::function<void(KHbase&, int)> 						     f_add = &KHbase::print_add;
	std::function<void(int)>                                    f_add2 = std::bind(&KHbase::print_add, &hobj, _1);
	//std::function<void(ResponseEvent&,   const RequestEvent&)> execE   = &KHbase::ExCmd;
	//std::function<void(ResponseAttr&,    const RequestAttr&)> execA    = std::bind(&KHbase::ExCmd, &hobj, _1,_2);
	//std::function<void(ResponseSegment&, const RequestSegment&)> execS = std::bind(&KHbase::ExCmd, &hobj, _1,_2);
	void dis1() { f_add(hobj, 23); }
	void dis2() { f_add2(1001); }
	int operator()(int a){  //std::cout<<a<<' '<<m_cb(a)<<std::endl;
		ResponseEvent resRE; ResponseAttr resRA; ResponseSegment resRS;
		if(fre) hobj.ExCmd(resRE, m_reReq);
		if(fra) hobj.ExCmd(resRA, m_raReq);
		if(frs) hobj.ExCmd(resRS, m_rsReq);
		std::this_thread::sleep_for(std::chrono::microseconds(30));
		return a+100;
	}
	void execute() {   (*this)(10020); }

	template <class Function, class ... Args>
	std::future<typename std::result_of<Function(Args...)>::type> run(Function&& func, Args&&... args)
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
		LOG(DEBUG)<<"elapsed time: "<<totalus<<"us";
		return ret;
	}

};
void test1()
{
	g_REreq.mkv.emplace(std::make_pair("ddsd","12222"));
	g_REreq.mkv.emplace(std::make_pair("kwg","uuuuuuuu"));
	cout<<"sizeof(ConObj) = "<<sizeof(ConObj)<<'\n';
	ConObj a;
	std::function<void(int)> fun1 = a;
	{	a.dis1(); a.dis2(); fun1(334); }
	AsyncObjectPool<ConObj> pool;
	//std::queue<std::unique_ptr<A> > vpool;
	{
		for(int i = 0; i < 200000; ++i) 
			pool.enqueue(std::unique_ptr<ConObj>(new ConObj(g_REreq)));
		pool.enqueue(std::unique_ptr<ConObj>(new ConObj(g_RSreq)));
		pool.enqueue(std::unique_ptr<ConObj>(new ConObj(g_RAreq)));
	}
	cout<<"currently pool size = "<<pool.size()<<endl;
	int dd=102;
}
void test3() {
	cout<<"sizeof(A) = "<<sizeof(A)<<'\n';
	//AsyncObjectPool<A> pool(std::thread::hardware_concurrency());
	AsyncObjectPool<A> pool;
	//std::queue<std::unique_ptr<A> > vpool;
	for(int i = 0; i < 2100000; ++i) 
		pool.enqueue(std::unique_ptr<A>(new A(i*100+1)));
}
int main(int argc, char** argv) {
	Init_Resource();
	//test1();
	runtime("test1 ---> ",test1);
	std::this_thread::sleep_for(std::chrono::seconds(10000));
    cout << endl;
	return 0;
}
