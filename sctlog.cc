#include <sctlog.h>

namespace sl
{
Severity v = INFO;
std::mutex sinks_mutex;
std::vector<Sink*> sinks;
}
