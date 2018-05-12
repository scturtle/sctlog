#ifndef _SCTLOG_H_
#define _SCTLOG_H_

#include <iostream>
#include <limits.h>
#include <mutex>
#include <sstream>
#include <string>
#include <time.h>
#include <unistd.h>
#include <vector>

const int FATAL = -3;
const int ERROR = -2;
const int WARNING = -1;
const int INFO = 0;

namespace sl
{
using Severity = int;

class Sink
{
  public:
    virtual ~Sink() {}
    virtual void send(const char *message) = 0;
    virtual void until_sent() = 0;
};

extern int v;
extern std::mutex sinks_mutex;
extern std::vector<Sink *> sinks;

inline void add_sink(Sink *sink)
{
    std::lock_guard<std::mutex> lock(sinks_mutex);
    sinks.push_back(sink);
}

class Msg
{
  public:
    Msg(const char *file, int line, Severity severity)
        : severity_(severity)
    {
        std::string file_(file);
        size_t pos = file_.rfind('/');
        std::string filename_only_;
        if (pos != std::string::npos)
            filename_only_ = file_.substr(pos + 1);
        else
            filename_only_ = file_;

        time_t rawtime;
        time(&rawtime);
        struct tm t;
        localtime_r(&rawtime, &t);
        char buf[32];
        // clang-format off
        snprintf(buf, sizeof(buf), "%d%02d%02d %02d:%02d:%02d ",
                 t.tm_year + 1900, t.tm_mon + 1,
                 t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
        stream_ << buf;
        switch (severity)
        {
            case FATAL: stream_ << "F "; break;
            case ERROR: stream_ << "E "; break;
            case WARNING: stream_ << "W "; break;
            case INFO: stream_ << "I "; break;
            default: stream_ << "V(" << severity << ") ";
        }
        // clang-format on
        stream_ << filename_only_ << ":" << line << " ";
    }

    ~Msg()
    {
        stream_ << "\n";

        std::lock_guard<std::mutex> lock(sinks_mutex);
        std::vector<Sink *>::iterator iter;
        for (iter = sinks.begin(); iter != sinks.end(); ++iter)
            (*iter)->send(stream_.str().c_str());
        for (iter = sinks.begin(); iter != sinks.end(); ++iter)
            (*iter)->until_sent();

        if (severity_ == FATAL)
            abort();
    }

    std::stringstream &stream() { return stream_; }

  private:
    std::stringstream stream_;
    Severity severity_;
};

struct Voidify
{
    void operator&(const std::ostream &) {}
};

// clang-format off
#define LOG_IF_RAW(n, cond) \
    !(cond) ? void(0) : sl::Voidify() & sl::Msg((char *)__FILE__, __LINE__, n).stream()
#define LOG_IF(n, cond) LOG_IF_RAW(n, (n <= sl::v) && (cond))
#define LOG_IF_FALSE(n, cond) LOG_IF(n, !(cond))
#define LOG(n) LOG_IF(n, true)
#define CHECK(cond) LOG_IF_RAW(FATAL, !(cond)) << "Check failed: " #cond " "

#ifndef NDEBUG
#  define DLOG LOG
#  define DCHECK CHECK
#else
#  define DLOG(n) true ? void(0) : LOG(n)
#  define DCHECK(cond) true ? void(0) : CHECK(cond) 
#endif
// clang-format on

class FileSink : public Sink
{
  public:
    FileSink(const char *filename)
    {
        char buf[PATH_MAX];
        char *path = getcwd(buf, PATH_MAX);
        CHECK(path);
        char fn[PATH_MAX];
        snprintf(fn, PATH_MAX, "%s/%s", path, filename);
        f_ = fopen(fn, "w");
        LOG_IF(WARNING, !f_) << "Cannot create " << buf;
    }

    void send(const char *message) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        fprintf(f_, "%s", message);
    }

    void until_sent() override { fflush(f_); }

  private:
    FILE *f_;
    std::mutex mutex_;
};

inline void log_to_file(const char *filename) { add_sink(new FileSink(filename)); }

class StderrSink : public Sink
{
  public:
    void send(const char *message) override { fprintf(stderr, "%s", message); }
    void until_sent() override{};
};

inline void log_to_stderr() { add_sink(new StderrSink()); }

} // namespace sl

#endif // _SCTLOG_H_
