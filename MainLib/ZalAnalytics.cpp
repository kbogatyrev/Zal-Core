#include <iostream>
#include <fstream>
#include <memory>
#include <locale>
#include <codecvt>

#include "EString.h"
#include "Singleton.h"
#include "Dictionary.h"
#include "Analytics.h"

//#include <stdlib.h>
//#include <crtdbg.h>

using namespace Hlib;

int main() {

    auto pSingleton = Hlib::Singleton::pGetInstance();

    shared_ptr<CDictionary> spDictionary;
    auto rc = pSingleton->eGetDictionary(spDictionary);

#ifdef WIN32
    rc = spDictionary->eSetDbPath(L"..\\..\\..\\..\\Zal-Data\\ZalData\\ZalData_Master_Tsvetaeva.db3");
    string text_path {"C:\\git-repos\\Zal\\Zal-Data\\ZalData\\Tsvetaeva_UTF-16_BOM.txt"};
#else
    rc = spDictionary->eSetDbPath(L"ZalData_Master_Tsvetaeva.db3");
    string text_path{ "/home/konstantin/.vs/Zal-Core/out/build/linux-debug/Tsvetaeva_UTF-16_BOM.txt" };
#endif


    shared_ptr<CAnalytics> spAnalytics;
    rc = spDictionary->eGetAnalytics(spAnalytics);
    if (rc != H_NO_ERROR)
    {
        wcout << L"Error getting analytics module.";
        return -1;
    }
//#else
//    spDictionary->eSetDbPath(L"/home/konstantin/zal/ZalData_demo.db3");
//#endif

    wifstream ioIn(text_path.c_str(), ios::binary);
    ioIn.imbue(std::locale(ioIn.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
//    if (!fin) {
//        std::cout << "!fin" << std::endl;
//        return 1;
//    }

    wstring cppLine;
    wchar_t pBuf[10000];
    CEString sLine;
    CEString sText;
    int iAbsLineNum {0};
    int iFirstLineNum {0};
    int iEmptyCount {0};
    while (!ioIn.eof() && !ioIn.fail())
    {
        ++iAbsLineNum;
        ioIn.getline(pBuf, 10000);
        if (ioIn.eof() || ioIn.fail())
        {
            if (ioIn.eof()) {
                wcout << L"--------------- EOF\n";
            }
            else
            {
                if (ioIn.fail()) {
                    wcout << L"--------------- FAIL\n";
                    auto err = strerror(errno);
                    wcout << err << "\n";
                }
            }
            wcout << L"              " << iAbsLineNum << endl;
            wcout << L"**************** EOF *****************\n";
            continue;
        }
        sLine = pBuf;
        if (sLine.uiLength() < 1)
        {
            wcout << L"***************** ERROR: empty string, CR expected\n";
            continue;
        }
        sLine.sErase(sLine.uiLength() - 1);

        if (sLine.bIsEmpty())
        {
            ++iEmptyCount;
        }
        else
        {
            iEmptyCount = 0;
        }

        if (iEmptyCount >= 2)
        {
//            auto bIsProse{ false };
            long long llParsedTextId{ 0 };
            rc = spAnalytics->eParseText(L"Tsvetaeva", L"Author=Tsvetaeva|Book=Collected Works", sText, (long long)iFirstLineNum, llParsedTextId);
            iFirstLineNum = iAbsLineNum;
            sText.Erase();
        }
        else
        {
            if (!sText.bIsEmpty())
            {
                sText += L"\r\n";
            }
            sText += sLine;
        }

        if (iAbsLineNum % 1000 == 0)
        {
            wcout << "********************** " << iAbsLineNum << endl;
        }
    }
    //    std::cout << L"Bye world!" << std::endl;

    return 0;
}
