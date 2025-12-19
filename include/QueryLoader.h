#pragma once
#include <string>
#include <unordered_map>
class QueryLoader {
private:
    static std::unordered_map<std::string, std::string> queries;
    static bool loaded;
    static void loadQueries();
    static std::string loadFile(const std::string& filepath);
    static void parseQueries(const std::string& content);
public:
    static const std::string& getQuery(const std::string& name);
};
