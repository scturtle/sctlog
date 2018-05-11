#include <sctlog.h>

namespace sl
{
int v = 0;
std::mutex sinks_mutex;
std::vector<Sink*> sinks;
}
