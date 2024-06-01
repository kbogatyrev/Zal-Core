#include "EString.h"
#include "SqliteWrapper.h"
#include "Singleton.h"
#include "Dictionary.h"

int main()
{
    {
        Hlib::CEString s1{ L"abcdefghij" };
        Hlib::CEString s2{ L"0123456789" };

        auto s3 = s1 + s2;

        std::cout << s3.stl_sToUtf8() << std::endl;
    }

//    {
//        auto spDb = make_shared<Hlib::CSqlite>(L"..\\..\\..\\..\\Zal-Data\\ZalData\\ZalData_Master.db3");
//    }

    {
        auto pSingleton = Hlib::Singleton::pGetInstance();

        shared_ptr<Hlib::CDictionary> spDictionary;
        auto rc = pSingleton->eGetDictionary(spDictionary);
        if (rc != Hlib::H_NO_ERROR)
        {
            std::cout << "Error!\n";
        }

#ifdef WIN32
        rc = spDictionary->eSetDbPath(L"..\\..\\..\\..\\Zal-Data\\ZalData\\ZalData_Master.db3");
#else
        spDictionary->eSetDbPath(L"/home/konstantin/Leaks/ZalData_Master.db3");
#endif
        Hlib::CEString sWord{ L"мама" };
        rc = spDictionary->eGetLexemesByInitialForm(sWord);
        if (rc != Hlib::H_NO_ERROR && rc != Hlib::H_NO_MORE) {
            std::cout << "Error " << rc << std::endl;
        }
        //    std::cout << L"Bye world!" << std::endl;

        delete pSingleton;

        return 0;

    }

    std::cout << "Done!\n";
}