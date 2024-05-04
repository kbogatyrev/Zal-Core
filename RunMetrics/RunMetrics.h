#ifndef RUN_METRICS_H_INCLUDED
#define RUN_METRICS_H_INCLUDED

#include <map>
#include <vector>

#include "Enums.h"
#include "EString.h"
#include "SqliteWrapper.h"
#include "Singleton.h"
#include "Dictionary.h"

namespace Hlib
{
    class CMetrics
    {
    public:
        ET_ReturnCode eInit(const CEString& sDbPath);

    private:
        ET_ReturnCode eLoadLineIds();
        ET_ReturnCode eLoadSegment(int64_t llLineId);
        ET_ReturnCode eSaveTextData(int64_t llTextId);
        ET_ReturnCode eHandleDbException(CException&&, CEString& sMsg);

    private:
        shared_ptr<CSqlite> m_spDb;
        multimap<int64_t, int64_t> m_mmapTextIdToLineIds;
        vector<CEString> m_vecSourceWords;
        vector<CEString> m_vecParsedWords;

        CEString m_sCurrentSequence;
    };
}

#endif      // RUN_METRICS_H_INCLUDED

