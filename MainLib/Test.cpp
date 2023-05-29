#include <iostream>
#include <memory>
//#include "MainLib.h"
#include "Singleton.h"
#include "Dictionary.h"

using namespace Hlib;

int main() {
    std::cout << "Hello world!" << std::endl;
     
     auto pSingleton = Hlib::Singleton::pGetInstance();

    shared_ptr<CDictionary> spDictionary;
    auto rc = pSingleton->eGetDictionary(spDictionary);

#ifdef WIN32
    rc = spDictionary->eSetDbPath(L"..\\..\\..\\..\\Zal-Data\\ZalData\\ZalData_TEST.db3");
#else
    spDictionary->eSetDbPath(L"/home/konstantin/zal/ZalData_demo.db3");
#endif
    rc = spDictionary->eGetLexemesByInitialForm(L"мама");
    if (rc != H_NO_ERROR && rc != H_NO_MORE) {
        std::cout << "Error " << rc << std::endl;
    }
    std::cout << "Bye world!" << std::endl;
    return 0;
}
