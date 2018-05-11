```c++
#include <sctlog.h>

int main()
{
    sl::log_to_stderr();
    sl::log_to_file("sct.log");
    LOG(INFO) << "test";
    CHECK(true) << "check";
}
```
