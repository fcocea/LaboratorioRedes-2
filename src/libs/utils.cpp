#include <iostream>
#include <numeric>
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
  // Follow the following format: <method>\s+<path>\s+HTTP/<version>
  // version like 1.1 / 1
  // Otherwhise it is a bad rqeuest (400)
  regex requestPattern(
      "([^\\s]+?)\\s+([^\\s]+?)\\s+HTTP\\/(\\d+|(\\d+\\.\\d+))$");
  if (!regex_match(requestLines[0], requestPattern)) {
    return requestMap;
  }
  const string request = regex_replace(requestLines[0], regex("\\s+"), " ");
  requestMap["Method"] = request.substr(0, request.find(' '));
  requestMap["Path"] = request.substr(
      request.find(' ') + 1, request.rfind(' ') - request.find(' ') - 1);
  requestMap["Version"] = request.substr(request.rfind(' ') + 1);
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

string joinVector(const vector<string> &v, const string &delim) {
  return accumulate(v.begin(), v.end(), string(),
                    [delim](const string &a, const string &b) {
                      return a.empty() ? b : a + delim + b;
                    });
}