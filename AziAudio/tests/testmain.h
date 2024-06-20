#ifndef __TESTMAIN_DOT_H__
#define __TESTMAIN_DOT_H__
#include <string>

inline bool TestTrue(bool test, std::string testString, std::string &statusBuffer)
{
    if (!test)
    {
        statusBuffer += "ERROR: " + testString + "\n";
    }
    return test == true ? 0 : 1;
}


#endif // __TESTMAIN_DOT_H__