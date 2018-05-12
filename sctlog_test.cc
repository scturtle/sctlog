#include <sctlog.h>
#include <thread>

void f1() { while(1) LOG(INFO) << "f1"; }
void f2() { while(1) LOG(INFO) << "f2"; }

int main()
{
    // sl::log_to_stderr();
    sl::log_to_file("sct.log");
    CHECK(true) << "check";
    std::thread t1(f1);
    std::thread t2(f2);
    t2.join();
}
