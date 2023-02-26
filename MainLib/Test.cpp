#include <iostream>
#include <memory>
#include "MainLib.h"
#include "Dictionary.h"

using namespace Hlib;

//extern "C"
//{
//    ET_ReturnCode GetDictionary(Hlib::IDictionary*&);        // the only external function defined in MainLib
//}

int main() {
    std::cout << "Hello world!" << std::endl;
     
    Hlib::Singleton singleton;

    shared_ptr<CDictionary> spDictionary;
    auto rc = singleton.GetDictionary(spDictionary);

#ifdef WIN32
    spDictionary->eSetDbPath(L"C:\\dev_win\\Zal\\Zal-Data\\ZalData\\ZalData_Master.db3");
#else
    spDictionary->eSetDbPath(L"/home/konstantin/zal/ZalData_demo.db3");
#endif
    rc = spDictionary->eGetLexemesByInitialForm(L"мама");
    if (rc != H_NO_ERROR && rc != H_NO_MORE) {
        std::cout << "Error " << rc << std::endl;
    }
    std::cout << "Bye world!" << std::endl;
}
