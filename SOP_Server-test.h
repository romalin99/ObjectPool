#include <stdlib.h>
#include <iostream>
#include <memory>
#include <assert.h>
#include <map>
#include <unordered_map>
#include <stack>
#include <stdexcept>
using namespace std;
struct RequestEvent{
	std::string n1;
	std::map<std::string,std::string> mkv;
} g_REreq; 
struct ResponseEvent { };

struct RequestSegment{
	std::string n1;
	std::map<std::string,std::string> mkv;
} g_RSreq; 
struct ResponseAttr {};

struct RequestAttr{
	std::string n1;
	std::map<std::string,std::string> mkv;
} g_RAreq; 
struct ResponseSegment{  };

class A{
	size_t i_;
public:
	A(){}
	A(size_t n):i_(n) { cout<<"A constructor ...\n"; }
	~A(){ cout<<"A deconstructor ...\n"; }
	void execute() {  	cout<<"tid = "<<std::this_thread::get_id() << " ,i = "<<i_<<'\n'; }
};
