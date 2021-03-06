#ifndef PARSINGTREE_H_INCLUDED
#define PARSINGTREE_H_INCLUDED

#include <set>
#include <map>
#include <memory>

#include "Enums.h"
#include "EString.h"

using namespace std;

//
// Endings parse tree
//
namespace Hlib
{
    //
    // Possible wordform division
    //
    struct StSplit
    {
        unsigned int uiEndingLength;
        vector<int64_t> vecEndingKeys;
    };

    //
    // Endings parser node
    //
    struct StNode
    {
        wchar_t cLetter;
        StNode * pPrevious;
        vector<StNode *> vecChildren;
    };

    struct StReverseComparisonFunctor
    {
        bool operator() (const CEString& sLeft, const CEString& sRight) const
        {
            int iLeftLength = sLeft.uiLength();
            int iRightLength = sRight.uiLength();
            if (iLeftLength < 1 && iRightLength < 1)
            {
                return false;
            }
            if (iLeftLength < 1)
            {
                return true;
            }
            if (iRightLength < 1)
            {
                return false;
            }

            if (iLeftLength > 10 || iRightLength > 10)
            {
                throw CException(H_ERROR_INVALID_ARG, L"Ending too long.");
            }

            int iLeftOffset = iLeftLength - 1;
            int iRightOffset = iRightLength - 1;
            for (; iLeftOffset >= 0 && iRightOffset >= 0; --iLeftOffset, --iRightOffset)
            {
                if (sLeft[iLeftOffset] < sRight[iRightOffset])
                {
                    return true;
                }
                else if (sLeft[iLeftOffset] > sRight[iRightOffset])
                {
                    return false;
                }
            }

            if (iLeftOffset >= 0)
            {
                return false;
            }
            else if (iRightOffset >= 0)
            {
                return true;;
            }

            return false;

        }   //  bool operator() (const CEString& sLeft, const CEString& sRight)

    };  //  struct StReverseComparisonFunctor

    class CParsingTree
    {
    public:
        CParsingTree(shared_ptr<CSqlite> pDb);
        ~CParsingTree();

    public:
        void eSplit(const CEString& sWord);
        ET_ReturnCode eGetFirstMatch(int& iEndingLength);
        ET_ReturnCode eGetNextMatch(int& iEndingLength);
        ET_ReturnCode eIsEmptyEnding(int64_t llEndingId);

    private:
        typedef std::map<CEString, vector<int64_t>, StReverseComparisonFunctor> MapEndingToDescriptors;

        MapEndingToDescriptors m_mapSortedEndingsList;
        StNode * m_pRoot;
        vector<int> m_vecMatches;    // length of ending
        vector<int>::iterator m_itCurrentMatch;
        set<CEString, StReverseComparisonFunctor> m_setEndingStrings;
        set<int64_t> m_setEmptyEndingIds;

    protected:
        CParsingTree() {};
        ET_ReturnCode eLoad(shared_ptr<CSqlite> pDb);
        void AddLevel(unsigned int uiOffset, StNode * pParent, vector<CEString> vecEndings);
        void Traverse(StNode * pRoot, const CEString& sWord);
        ET_ReturnCode eTraverseAndDelete(StNode * pRoot);

    private:

    };      // CParsingTree

} // namespace Hlib

#endif // PARSINGTREE_H_INCLUDED
