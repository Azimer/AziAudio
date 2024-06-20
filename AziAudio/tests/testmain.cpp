#include "testmain.h"
#include "tests.h"

typedef std::map<std::string, TestRunFunc*> TestContainer;

TestContainer tests = {
    {"Configuration Default Test", ConfigTestDefaults},
    {"Configuration Save Test"   , ConfigTestSave},
    {"Configuration Save Test Modified"   , ConfigTestSaveModified},
    {"Configuration Load Test"   , ConfigTestLoad},
    {"Configuration Load with no Rom Section while Running", ConfigTestLoadRomMissing}, 
    {"Configuration Load with Rom Section while Running", ConfigTestLoadRomPresent}, 
    {"Configuration Load with Partial Rom Section while Running", ConfigTestLoadRomPartial}
};

int main ()
{
    std::string testFailureReason;
    int ctr = 0;
    int successCnt = 0, failCnt = 0;
    int result = 0;
    std::cout << "Starting Tests for AziAudio..." << std::endl;

    for ( TestContainer::iterator itr = tests.begin(); itr != tests.end(); itr++ )
    {
        std::cout << "Beginning Test " << ++ctr << " of " << tests.size() << " : " << itr->first << "... ";
        result = itr->second(testFailureReason);
        if (result == 0)
        {
            std::cout << "success" << std::endl;
            successCnt++;
        }
        else
        {
            std::cout << "failed" << std::endl;
            std::cout << testFailureReason << std::endl;
            failCnt++;
            testFailureReason = "";
        }
    }
    return failCnt > 0 ? -1 : 0;
}

