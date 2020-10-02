#pragma once

#include <string>
#include <vector>
#include <map>
#include <FlowUtils/FlowLog.h>
#include <boost/dll/import.hpp>
#include <FlowUtils/FlowInterface.h>
#include <FlowUtils/FlowParser.h>
#include <FlowUtils/FlowFile.h>
#include <FlowUtils/FlowArgParser.h>
#include "FlowRa.h"
#include "Version.h"

namespace Flow_Ra_Init{
    inline static int init(const std::string& args) {
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

        fap.parse(args);


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

        return EXIT_SUCCESS;
    }
}