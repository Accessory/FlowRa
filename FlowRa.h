#pragma once

#include <string>
#include <vector>
#include <map>
#include <FlowUtils/FlowLog.h>
#include <boost/dll/import.hpp>
#include <FlowUtils/FlowInterface.h>
#include <FlowUtils/FlowParser.h>
#include <FlowUtils/FlowFile.h>
#include "FlowAction.h"
#include <FlowUtils/PriorityThreadPool.h>

class FlowRa {
public:

    FlowRa(const std::string &file, const size_t &threads, const std::vector<std::string> &plugins) {
        addFlowBinding = bind(&FlowRa::addFlow, this, std::placeholders::_1, std::placeholders::_2);
        createFlowBinding = bind(&FlowRa::createFlow, this, std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3);
        loadPlugins(plugins);
        loadLayer(file);
        threadPool = std::make_unique<PriorityThreadPool>(threads);
    }

    void run() {
        std::shared_ptr<std::function<void()>> toAdd = std::make_shared<std::function<void()>>(
                createFlowFunction(std::map<std::string, std::vector<std::string>>(), 0));
        threadPool->addFunction(toAdd, 0);
        threadPool->start();
        threadPool->join();
    }

    void addFlow(std::map<std::string, std::vector<std::string>> externalArguments, size_t layerIdx) {
        size_t newlayerdeep = layerIdx + 1;
        if (newlayerdeep < layer.size()) {
            std::shared_ptr<std::function<void()>> toAdd = std::make_shared<std::function<void()>>(
                    createFlowFunction(externalArguments, newlayerdeep));
            threadPool->addFunction(toAdd, newlayerdeep);
            threadPool->start();
        }
    }

    void createFlow(std::string flowName, std::vector<std::string> externalArguments,
                    std::map<std::string, std::vector<std::string>> internalArguments) {

    }

private:
    std::unique_ptr<PriorityThreadPool> threadPool;
    std::vector<FlowAction> layer;
    std::vector<boost::dll::shared_library> libraries;
    std::vector<std::shared_ptr<FlowInterface>> plugins;

    std::function<void(std::map<std::string, std::vector<std::string>>, size_t)> addFlowBinding;

    std::function<void(std::string, std::vector<std::string>, std::map<std::string, std::vector<std::string>>,
                       size_t)> createFlowBinding;
    std::map<std::string, std::function<void(std::vector<std::string>,
                                             std::map<std::string, std::vector<std::string>>, size_t)>>
            actionMap;


    void loadLayer(const std::string &file) {
        using namespace FlowParser;

        if (!FlowFile::fileExist(file)) {
            LOG_FATAL << "File " << file << " not found!";
            return;
        }
        LOG_INFO << "Run file " << file;

        auto content = FlowFile::fileToString(file);
        size_t pos = 0;
        size_t layerIdx = 0;
        while (pos < content.size()) {
            gotoNextNonWhite(content, pos);
            if (pos > content.size()) {
                break;
            }
            if (content.at(pos) == '#') {
                goToNewLine(content, pos);
                if (pos < content.length())
                    goToNextLine(content, pos);
                continue;
            }
            auto name = goTo(content, ":", pos);
            ++pos;
            std::vector<std::string> arguments;
            while (pos < content.size() && content.at(pos) != ';' && content.at(pos) != '\n' &&
                   content.at(pos) != '\r') {
                arguments.emplace_back(goToOne(content, ",;\n\r", pos));
                if (content.at(pos) == ',') {
                    ++pos;
                    continue;
                }
                if (pos < content.size())
                    goToNewLine(content, pos);
                if (pos < content.size())
                    goToNextLine(content, pos);
                break;
            }
            layer.emplace_back(name, arguments, layerIdx++);
        }

    }


    void loadPlugins(const std::vector<std::string> plugins) {
        for (auto &plugin : plugins)
            loadDll(plugin);
    }

    void loadDll(const std::string &plugin) {
        LOG_INFO << "Load plugin: " << plugin;
        try {
            libraries.emplace_back(plugin,
                                   boost::dll::load_mode::default_mode
                                   | boost::dll::load_mode::append_decorations
                                   | boost::dll::load_mode::search_system_folders
            );
            auto creator =
                    libraries.back().get_alias<std::shared_ptr<FlowInterface>(
                            std::function<void(std::map<std::string, std::vector<std::string>>, size_t)>,
                            std::function<void(std::string, std::vector<std::string>,
                                               std::map<std::string, std::vector<std::string>>, size_t)>)>(
                            "create_plugin");
            auto plugin = creator(addFlowBinding, createFlowBinding);
            plugins.push_back(plugin);

            auto logLevel = libraries.back().get_alias<void(logging::severity)>("setLogLevel");
            logLevel(logging::getConfig()->getLevel());
            actionMap.insert(plugin->actionMap.begin(), plugin->actionMap.end());
        } catch (std::exception &e) {
            LOG_WARNING << "Could not load " << plugin << " Reason:" << e.what();
        }
    }

    std::function<void()>
    createFlowFunction(std::map<std::string, std::vector<std::string>> internalArguments, size_t layerIdx) {
        auto flowAction = layer[layerIdx];
        std::function<void()> rtn;
        try {
            auto flowFunction = actionMap.find(flowAction.name);
            if (flowFunction == actionMap.end()) throw "";
            LOG_TRACE << "Create Function: " << flowAction.name << " Layer#: " << layerIdx;
            rtn = [flowAction, flowFunction, internalArguments, layerIdx] {
                flowFunction->second(flowAction.arguments, internalArguments, layerIdx);
            };
        } catch (...) {
            LOG_ERROR << "Cannot find " << flowAction.name;
            throw "\"Cannot find\" << flowAction.name";
        }
        return rtn;
    }
};