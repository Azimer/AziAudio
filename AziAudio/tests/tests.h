#ifndef __TESTS_DOT_H__
#define __TESTS_DOT_H__

#include <iostream>
#include <map>

// Arg[0] = failureResult string to hold a list of fail results
// Return : 0 on success, non-zero on failure
typedef int TestRunFunc(std::string&);

TestRunFunc ConfigTestDefaults;
TestRunFunc ConfigTestSave;
TestRunFunc ConfigTestSaveModified;
TestRunFunc ConfigTestLoad, ConfigTestLoadRomMissing, ConfigTestLoadRomPresent, ConfigTestLoadRomPartial;

#endif //__TESTS_DOT_H__