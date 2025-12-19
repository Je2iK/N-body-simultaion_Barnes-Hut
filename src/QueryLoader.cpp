#include "QueryLoader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
std::unordered_map<std::string, std::string> QueryLoader::queries;
bool QueryLoader::loaded = false;
void QueryLoader::loadQueries() {
    if (loaded) return;
    std::string basePath = "database/queries/";
    for (const auto& file : {"users.sql", "benchmarks.sql"}) {
        std::string filepath = basePath + file;
        if (std::filesystem::exists(filepath)) {
            std::string content = loadFile(filepath);
            parseQueries(content);
        }
    }
    loaded = true;
}
std::string QueryLoader::loadFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open query file: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
void QueryLoader::parseQueries(const std::string& content) {
    std::istringstream stream(content);
    std::string line;
    std::string currentName;
    std::string currentQuery;
    while (std::getline(stream, line)) {
        if (line.starts_with("-- name: ")) {
            if (!currentName.empty()) {
                queries[currentName] = currentQuery;
            }
            currentName = line.substr(9);
            currentQuery.clear();
        } else if (!line.starts_with("--") && !line.empty()) {
            if (!currentQuery.empty()) currentQuery += " ";
            currentQuery += line;
        }
    }
    if (!currentName.empty()) {
        queries[currentName] = currentQuery;
    }
}
const std::string& QueryLoader::getQuery(const std::string& name) {
    loadQueries();
    auto it = queries.find(name);
    if (it == queries.end()) {
        throw std::runtime_error("Query not found: " + name);
    }
    return it->second;
}
