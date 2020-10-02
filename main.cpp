#include <iostream>
#include <FlowUtils/FlowArgParser.h>
#include "Version.h"
#include <FlowUtils/FlowLog.h>
#include <thread>
#include "FlowRa.h"
#include <chrono>

int main(int argc, char *argv[]) {
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    LOG_INFO << "Working dir: " << FlowFile::getCurrentDirectory();
    FlowArgParser fap;
    fap.addFlagOption("version", "v", "Print version");
    fap.addFlagOption("debug", "d", "Set logging to Debug");
    fap.addFlagOption("trace", "T", "Set logging to trace");
    fap.addParameterOption("threads", "t", "Number of Threads");
    fap.addParameterOption("log", "l", "Logfile");
    fap.addIndexOption("file", "Working file", true);
    fap.addIndexList("plugins", "Plugins to load");
    fap.markList("plugins");

    fap.parse(argc, argv);


    if (fap.hasFlag("version")) {
        LOG_INFO << "FlowRa Version " << VERSION_MAJOR << "." << VERSION_MINOR;
        return 0;
    }

    if (!fap.hasRequiredOptions()) {
        LOG_FATAL << "Wrong amount of arguments";
        return EXIT_FAILURE;
    }

    if (fap.hasFlag("trace")) {
        LOG_INFO << "Log Level: Trace";
        setLogLevel(logging::TRACE);
    } else if (fap.hasFlag("debug")) {
        LOG_INFO << "Log Level: Debug";
        setLogLevel(logging::DEBUG);
    }

    auto logFile = fap.getString("log");
    if (!logFile.empty()) {
        LOG_INFO << "Log to file: " << logFile;
        logging::setLogFile("log.txt");
    }


    auto threads = fap.getString("threads");
    size_t threadCount = threads.empty() ? std::thread::hardware_concurrency() : std::stoul(threads, nullptr, 10);


    LOG_INFO << "Threads used: " << threadCount;
    try {
        FlowRa flowRa(fap.getString("file"), threadCount, fap.getList("plugins"));
        flowRa.run();
        LOG_TRACE << "End Run";
    } catch (const std::system_error &e) {
        LOG_FATAL << "Code " << e.code()
                  << " meaning " << e.what() << '\n';
        return e.code().value();
    }
    LOG_INFO << "Done";

    end = std::chrono::system_clock::now();
    const auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>
            (end - start).count();

    std::cout << "elapsed time: " << elapsed_seconds << "s\n";

    return 0;
}