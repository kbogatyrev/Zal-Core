#include <iostream>
#include "Dictionary.h"

using namespace Hlib;

extern "C"
{
    ET_ReturnCode GetDictionary(Hlib::IDictionary*&);        // the only external function defined in MainLib
}

int main() {
    std::cout << "Hello world!" << std::endl;
     
    IDictionary * d;
    auto rc = GetDictionary(d);

#ifdef WIN32
    d->eSetDbPath(L"C:\\dev_win\\Zal\\Zal-Data\\ZalData\\ZalData_Master.db3");
#else
    d->eSetDbPath(L"/home/konstantin/dev/Zal-Data/ZalData/ZalData_Master.db3");
#endif
    rc = d->eGetLexemesByInitialForm(L"мама");
    if (rc != H_NO_ERROR && rc != H_NO_MORE) {
        std::cout << "Error " << rc << std::endl;
    }

    delete d;

    std::cout << "Bye world!" << std::endl;

}
