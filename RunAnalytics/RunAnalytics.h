#ifndef RUN_ANALYTICS_H_INCLUDED
#define RUN_ANALYTICS_H_INCLUDED

#include "Enums.h"
#include "EString.h"
#include "RunAnalytics.h"
#include "Analytics.h"

namespace Hlib
{
    class CAnalyticsRunner
    {
        //    const CEString sMetadataTemplate{ L"author=#AUTHOR#|book=#BOOK#|title=#TITLE#|chapter=#CHAPTER#|dedication=#DEDICATION#|date=#DATE#|page=#PAGE#" };

    public:
        ET_ReturnCode eInit(const CEString& sDbPath, const CEString& sSourceTextPath);
        CEString sGetTextPath();
        ET_ReturnCode eAnalyze(CEString& sLine);
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

        ET_TextMetadata m_eLastTag{ TEXT_METADATA_UNDEFINED };

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

}

#endif      //  RUN_ANALYTICS_H_INCLUDED
