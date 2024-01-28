#ifndef RUN_PHONOTACTICS_H_INCLUDED
#define RUN_PHONOTACTICS_H_INCLUDED

#include <map>
#include <vector>
#include <set>
#include <tuple>

#include "Enums.h"
#include "EString.h"
#include "SqliteWrapper.h"
#include "Singleton.h"
#include "Dictionary.h"

namespace Hlib
{
    class CPhonotactics
    {
    public:
        ET_ReturnCode eInit(const CEString& sDbPath);
        ET_ReturnCode eCountSoundPairs();

    private:
        ET_ReturnCode eLoadPhonemicPairs();
        ET_ReturnCode eLoadLineIds();
        ET_ReturnCode eLoadSegment(int64_t llLineId);
        ET_ReturnCode eRegisterSoundPair(ET_Sound eLhs, ET_Sound eRhs, int64_t& llId);
        ET_ReturnCode eSaveTextData(int64_t llTextId);
        ET_ReturnCode eStringToSound(CEString&&, ET_Sound&);
        ET_ReturnCode eHandleDbException(CException&&, CEString& sMsg);

    private:
        shared_ptr<CSqlite> m_spDb;
        multimap<int64_t, int64_t> m_mmapTextIdToLineIds;
        map<pair<ET_Sound, ET_Sound>, int64_t> m_mapSoundPairToId;
        map<int64_t, int64_t> m_mapSoundPairIdToCount;
        set<tuple<int64_t, ET_Sound, ET_Sound>> m_setNewSoundPairs;
        CEString m_sCurrentSequence;
    };
}

#endif      // RUN_PHONOTACTICS_H_INCLUDED
