#include <iostream>
#include <sys/stat.h>

#include "utils.hpp"

using namespace std;

string readParam(int argc, const vector<string> &argv, char key) {
  for (int i = 1; i < argc; i++) {
    const string &arg = argv[i];
    if (arg[0] != '-')
      continue;
    size_t argLen = arg.length();
    for (size_t j = 1; j < argLen; j++) {
      if (arg[j] == key) {
        return i + 1 < argc ? argv[i + 1] : "";
      }
    }
  }
  return "";
}

bool isNumber(const string &s) {
  for (char c : s) {
    if (!isdigit(c))
      return false;
  }
  return true;
}

bool verifyFile(const string &path) {
  struct stat fileData;
  return stat(path.c_str(), &fileData) == 0 && S_ISREG(fileData.st_mode);
}

map<string, string> parseRequest(const vector<string> &requestLines) {
  map<string, string> requestMap;
  regex headerPattern("([^:]+): *(.*)");
  regex requestPattern(
      "(GET|POST|PUT|DELETE|HEAD|OPTIONS|TRACE|CONNECT) (.*) HTTP/(.*)");
  if (regex_match(requestLines[0], requestPattern)) {
    requestMap["Method"] = requestLines[0].substr(0, requestLines[0].find(' '));
    requestMap["Path"] = requestLines[0].substr(
        requestLines[0].find(' ') + 1,
        requestLines[0].rfind(' ') - requestLines[0].find(' ') - 1);
    requestMap["Version"] =
        requestLines[0].substr(requestLines[0].rfind(' ') + 1);
  } else {
    return requestMap;
  }
  for (const auto &line : requestLines) {
    smatch match;
    if (regex_search(line, match, headerPattern)) {
      requestMap[match[1]] = match[2];
    }
  }

  return requestMap;
}

bool createFolder(const string &path) {
  struct stat st = {0};
  if (stat(path.c_str(), &st) == -1) {
    mkdir(path.c_str(), 0700);
    return true;
  }
  return false;
}