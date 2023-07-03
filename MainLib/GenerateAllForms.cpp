#include <iostream>
#include <memory>
//#include "MainLib.h"
#include "Singleton.h"
#include "Dictionary.h"

using namespace Hlib;

int main() {

//    _setmode(_fileno(stdout), _O_U16TEXT);

//    std::cout << L"Hello world!" << std::endl;
     
    auto pSingleton = Hlib::Singleton::pGetInstance();

    shared_ptr<CDictionary> spDictionary;
    auto rc = pSingleton->eGetDictionary(spDictionary);

#ifdef WIN32
    rc = spDictionary->eSetDbPath(L"..\\..\\..\\..\\Zal-Data\\ZalData\\ZalData_Master.db3");
#else
    spDictionary->eSetDbPath(L"/home/konstantin/zal/ZalData_demo.db3");
#endif
    rc = spDictionary->eGenerateAllForms();
    if (rc != H_NO_ERROR && rc != H_NO_MORE) {
        std::cout << "Error " << rc << std::endl;
    }
//    std::cout << L"Bye world!" << std::endl;
    return 0;
}
