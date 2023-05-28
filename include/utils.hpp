#ifndef UTILS_HPP
#define UTILS_HPP

#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

std::string readParam(int argc, const std::vector<std::string> &argv, char key);
bool isNumber(const std::string &s);
bool verifyFile(const std::string &path);
bool createFolder(const std::string &path);
std::map<std::string, std::string>
parseRequest(const std::vector<std::string> &requestLines);
std::string joinVector(const std::vector<std::string> &v,
                       const std::string &delim);

#endif