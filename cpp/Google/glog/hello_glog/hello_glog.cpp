#include <vector>

#include <glog/logging.h>
#include <glog/stl_logging.h>

#ifdef _MSC_VER
# include <direct.h>
# define mkdir(p) _mkdir((p))
#elif defined(__linux__)
# include <sys/stat.h>
# include <sys/types.h>
# define mkdir(p) mkdir((p), 0700)
#endif


int main(int argc, char *argv[])
{
    FLAGS_log_dir = "./Logs";

    mkdir(FLAGS_log_dir.c_str());

    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    //google::SetLogFilenameExtension(".log");
    //google::SetLogDestination(0, "hello_glog");
    //google::SetLogDestination(google::GLOG_WARNING, "");

    FLAGS_alsologtostderr = 1;
    FLAGS_colorlogtostderr = 1;

    DLOG(INFO) << "Debug mode!";
    DLOG(INFO) << __FILE__;

    LOG(INFO) << "Hello, world!";
    LOG(WARNING) << "This is a test code. This is a real life.";
    LOG(ERROR) << "You can not see me!";

    std::vector<int> fibs { 1, 1, 2, 3, 5, 8, 13, 21, 34, 55 };
    LOG(INFO) << fibs;

    VLOG(1) << "I'm printed when you run the program with --v=1 or higher";
    VLOG(2) << "I'm printed when you run the program with --v=2 or higher";

    LOG(FATAL) << "Fatal error!";

    return 0;
}

// References:
// https://stackoverflow.com/questions/20016036/glog-no-output-file-log-is-created
// https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
