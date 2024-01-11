#include <iostream>
#include <fstream>
#include <memory>
#include <locale>
#include <codecvt>

#include <toml.hpp>

#include "Enums.h"
#include "EString.h"
#include "Singleton.h"
#include "Dictionary.h"
#include "Analytics.h"

using namespace Hlib;

class CAnalyticsRunner
{
//    const CEString sMetadataTemplate{ L"author=#AUTHOR#|book=#BOOK#|title=#TITLE#|chapter=#CHAPTER#|dedication=#DEDICATION#|date=#DATE#|page=#PAGE#" };

public:
    ET_ReturnCode eInit(const CEString& sDbPath, const CEString& sSourceTextPath);
    CEString sGetTextPath();
    ET_ReturnCode eCheckMetadata(CEString& sLine);
    ET_ReturnCode eAssembleMetadatString();
    void AddLine(const CEString&);
    ET_ReturnCode eParseText();

private:
    CEString m_sSourceTextPath;

    CEString m_sAuthor;
    CEString m_sBook;
    CEString m_sPage;
    CEString m_sTitle;
    CEString m_sChapter;
    CEString m_sFootnoteRef;
    CEString m_sFootnoteText;
    CEString m_sDate;
    CEString m_sDedication;

    CEString m_sText;
    CEString m_sMetadata;

    ET_TextMetadata m_eLastTag {TEXT_METADATA_UNDEFINED};

    map<CEString, ET_TextMetadata> m_mapTagToMetadataType{
        { L"author", TEXT_METADATA_AUTHOR },
        { L"book", TEXT_METADATA_BOOK },
        { L"page", TEXT_METADATA_PAGE },
        { L"title", TEXT_METADATA_TITLE },
        { L"chapter", TEXT_METADATA_CHAPTER },
        { L"date", TEXT_METADATA_DATE },
        { L"dedication", TEXT_METADATA_DEDICATION }
    };

    shared_ptr<CAnalytics> m_spAnalytics;
};

ET_ReturnCode CAnalyticsRunner::eInit(const CEString& sDbPath, const CEString& sSourceTextPath)
{
    auto pSingleton = Hlib::Singleton::pGetInstance();
    shared_ptr<CDictionary> spDictionary;
    auto rc = pSingleton->eGetDictionary(spDictionary);

#ifdef WIN32
    rc = spDictionary->eSetDbPath(sDbPath);
#else
    rc = spDictionary->eSetDbPath(sDbPath);
#endif

    m_sSourceTextPath = sSourceTextPath;

    rc = spDictionary->eGetAnalytics(m_spAnalytics);
    if (rc != H_NO_ERROR)
    {
        ERROR_LOG(L"Error getting analytics module.");
        return H_ERROR_GENERAL;
    }

    return H_NO_ERROR;
}

CEString CAnalyticsRunner::sGetTextPath()
{
    return m_sSourceTextPath;
}

ET_ReturnCode CAnalyticsRunner::eCheckMetadata(CEString& sLine)
{
    bool bRet = sLine.bRegexMatch(L"^<(\\w+?)\\s+(.+?)\\/(\\w+)>");
    if (!bRet)
    {
        return H_FALSE;
    }

    auto sStartTag = sLine.sGetRegexMatch(0);
    auto sValue = sLine.sGetRegexMatch(1);
    auto sEndTag = sLine.sGetRegexMatch(2);
    if (sStartTag != sEndTag)
    {
        CEString sMsg(L"Metadata start tag ");
        sMsg += sStartTag + CEString(L" does not match the end tag ");
        sMsg += sEndTag;
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

    if (m_mapTagToMetadataType.find(sStartTag) == m_mapTagToMetadataType.end())
    {
        CEString sMsg(L"Unknown metadata tag ");
        sMsg += sStartTag;
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

    auto eTag = m_mapTagToMetadataType[sStartTag];
    switch (eTag)
    {
    case TEXT_METADATA_AUTHOR:
        m_sAuthor = sValue;
        break;

    case TEXT_METADATA_BOOK:
        m_sBook = sValue;
        break;

    case TEXT_METADATA_PAGE:
        m_sPage = sValue;
        break;

    case TEXT_METADATA_TITLE:
        if (!m_sTitle.bIsEmpty() && !m_sText.bIsEmpty())
        {
            auto rc = eParseText();
            if (rc != H_NO_ERROR)
            {
                CEString sMsg(L"Error in text parser ");
                ERROR_LOG(sMsg);
                break;
            }
            m_sText.Erase();
        }
        m_sTitle = sValue;
        break;

    case TEXT_METADATA_CHAPTER:
        if (!m_sText.bIsEmpty())
        {
            auto rc = eParseText();
            if (rc != H_NO_ERROR)
            {
                CEString sMsg(L"Error in text parser ");
                ERROR_LOG(sMsg);
                break;
            }
        }
        m_sText.Erase();
        m_sChapter = sValue;
        break;

    case TEXT_METADATA_DEDICATION:
        if (m_sTitle.bIsEmpty() && m_sChapter.bIsEmpty())
        {
            CEString sMsg(L"Dedication tag with unknown title or chapter: ");
            sMsg += sStartTag;
            ERROR_LOG(sMsg);
            return H_ERROR_UNEXPECTED;
        }
        m_sDedication = sValue;
        break;

    case TEXT_METADATA_DATE:
        if (m_sText.bIsEmpty())
        {
            ERROR_LOG(L"Date tag appears with empty text.");
            return H_ERROR_UNEXPECTED;
        }
        if (m_eLastTag != TEXT_METADATA_UNDEFINED)
        {
            CEString sMsg(L"Date tag does not appear immediately after a text: ");
            sMsg += sStartTag;
            ERROR_LOG(sMsg);
            return H_ERROR_UNEXPECTED;
        }
        m_sDate = sValue;     // belongs to current text, expect new text
        break;

    default:
        CEString sMsg(L"Unknown metadata tag: ");
        sMsg += sStartTag;
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

    return H_TRUE;

}   //  eCheckMetadata()

ET_ReturnCode CAnalyticsRunner::eAssembleMetadatString()
{
    if (m_sTitle.bIsEmpty())
    {
        ERROR_LOG(L"No title.")
    }

    static const CEString sMetadataTemplate{ 
        L"author=#AUTHOR#|book=#BOOK#|title=#TITLE#|chapter=#CHAPTER#|dedication=#DEDICATION#|date=#DATE#|page=#PAGE#" 
    };
    m_sMetadata = sMetadataTemplate;
    m_sMetadata.sReplace(L"#AUTHOR#", m_sAuthor);
    m_sMetadata.sReplace(L"#BOOK#", m_sBook);
    m_sMetadata.sReplace(L"#TITLE#", m_sTitle);
    m_sMetadata.sReplace(L"#CHAPTER#", m_sChapter);
    m_sMetadata.sReplace(L"#DEDICATION#", m_sDedication);
    m_sMetadata.sReplace(L"#DATE#", m_sDate);
    m_sMetadata.sReplace(L"#PAGE#", m_sPage);

    m_sTitle.Erase();
    m_sChapter.Erase();
    m_sDedication.Erase();

    return H_NO_ERROR;
}

void CAnalyticsRunner::AddLine(const CEString& sLine)
{
    m_sText += sLine + L"\r\n";     // Stupid... There should be a better way... 
}

ET_ReturnCode CAnalyticsRunner::eParseText()
{
    if (!m_spAnalytics)
    {
        ERROR_LOG(L"Analytics module not available.");
        return H_ERROR_UNEXPECTED;
    }
    auto rc = eAssembleMetadatString();
    if (rc != H_NO_ERROR)
    {
        ERROR_LOG(L"Unable to assemble metadata string.");
    }

    rc = m_spAnalytics->eParseText(m_sMetadata, m_sText, 0, false);
    if (rc != H_NO_ERROR)
    {
        ERROR_LOG(L"Text parse failed.");
    }
    return rc;
    m_sText.Erase();

    return H_NO_ERROR;
}

//
//  --------------------------------------------------------------------------------------------
//

int main(int argc, char *argv[]) {

    [[maybe_unused]] ET_ReturnCode rc = H_NO_ERROR;

    if (argc < 2)
    {
        ERROR_LOG(L"Argument expected: \'ZalRunAnalytics analytics.toml\'");
        return H_ERROR_INVALID_ARG;
    }

    toml::table config;

    try
    {
        string path = argv[1];
        config = toml::parse_file(path);
    }
    catch (const toml::parse_error& err)
    {
        CEString sMsg(L"Unable to parse config.");
        sMsg += Hlib::CEString::sFromUtf8(err.description().data());
        ERROR_LOG(sMsg)
        return H_ERROR_UNEXPECTED;
    }

    auto utf8Author = config["metadata"]["author"].as_string()->get();
    auto utf8Book = config["metadata"]["book"].as_string()->get();

    CAnalyticsRunner Runner;
#ifdef WIN32
    auto utf8DbPath = config["paths"]["db_path_windows"].as_string()->get();
    auto utf8TextPath = config["paths"]["text_path_windows"].as_string()->get();
    Runner.eInit(Hlib::CEString::sFromUtf8(utf8DbPath), Hlib::CEString::sFromUtf8(utf8TextPath));
#else
    auto utf8DbPath = config["paths"]["db_path_linux"].as_string()->get();
    auto utf8TextPath = config["paths"]["text_path_linux"].as_string()->get();
    Runner.eInit(Hlib::CEString::sFromUtf8(utf8DbPath), Hlib::CEString::sFromUtf8(utf8TextPath));
#endif

    wifstream ioIn(Runner.sGetTextPath().stl_sToUtf8(), ios::binary);
    ioIn.imbue(std::locale(ioIn.getloc(), new std::codecvt_utf16<wchar_t, 0x10ffff, std::consume_header>));
//    if (!fin) {
//        std::cout << "!fin" << std::endl;
//        return 1;
//    }

    wstring cppLine;
    wchar_t pBuf[10000];
    CEString sFirstLine;
    CEString sLine;
    int iAbsLineNum {0};
    [[maybe_unused]] int iFirstLineNum {0};

    CEString sMetadataString;
    CEString sText;

    while (!ioIn.eof() && !ioIn.fail())
    {
        ioIn.getline(pBuf, 10000, L'\n');       // NB Windows only
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
        sLine.sErase(sLine.uiLength()-1);
        if (sLine.uiLength() < 1)
        {
//            ERROR_LOG(L"Empty string, CR expected.");
            continue;
        }

        auto rc = Runner.eCheckMetadata(sLine);
        if (H_TRUE == rc)
        {
            continue;
        }
        else if (H_FALSE)
        {
            Runner.AddLine(sLine);
        }
        else
        {
            ERROR_LOG(L"Unexpected return from CAnalyticsRunner::eCheckMetadata()");
            return H_ERROR_UNEXPECTED;
        }

        if (iAbsLineNum % 1000 == 0)
        {
            wcout << "********************** " << iAbsLineNum << endl;
        }
        ++iAbsLineNum;
    }
    //    std::cout << L"Bye world!" << std::endl;

    return 0;
}
