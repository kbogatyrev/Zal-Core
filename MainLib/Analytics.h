#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <set>
#include <vector>
#include <array>
#include <map>
#include <memory>

#include "Enums.h"
#include "EString.h"
#include "WordForm.h"

using namespace std;

namespace Hlib
{
    class CSqlite;
    class CParser;
    class CTranscriber;
    class CEString;

    enum class ET_WordContext
    {
        WORD_CONTEXT_UNDEFINED,
        WORD_CONTEXT_COMPLETE,
        WORD_CONTEXT_INCOMPLETE,
        WORD_CONTEXT_BREAK,
        WORD_CONTEXT_COUNT
    };

    struct StIrregularWord
    {
        int64_t llDbId {-1};
        CEString sGramHash;
        bool bIsAlternative {false};
        map<int, ET_StressType> mapStress;

        void Reset() {
            llDbId = -1;
            sGramHash.Erase();
            mapStress.clear();
        }
    };

    struct StWordParse
    {
        int iPosInLine;
        int iLineOffset;
        int iLength;
        ET_WordStressType eStressType;
        int64_t llLineDbId;
        int64_t llWordInLineDbId;
        int64_t llWordToWordFormId;
        shared_ptr<CWordForm> spWordForm;

        StWordParse()
        {
            Reset();
        }

        void Reset()
        {
            iPosInLine = -1;
            iLineOffset = -1;
            iLength = -1;
            llLineDbId = -1;
            eStressType = WORD_STRESS_TYPE_UNDEFINED;
            llWordInLineDbId = -1;
            llWordToWordFormId = -1;
        }

        // Needed to be used with STL set
        bool operator < (const StWordParse& stRhs) const
        {
            return iPosInLine < stRhs.iPosInLine;
        }
    };      // StWordParse

/*
    struct StIrregularWord
    {
        int64_t llDbId;
        bool bIsAlternative;
        shared_ptr<CEString> spGramHash;
        shared_ptr<map<int, ET_StressType>> spStressMap;
    };
*/

    struct StWordContext
    {
        bool bIncomplete {false};
        bool bBreak {false};
        bool bIgnore {false};
        int64_t llSegmentId;
        int iSeqNum;
        int64_t llWordFormId {-1};
        int64_t llIrregularFormId {-1};
        CEString sWord;
        CEString sGramHash;

        StWordContext() : llSegmentId(-1), iSeqNum(-1)
        {}

        void Reset()
        {
            bIncomplete = false;
            bBreak = false;
            bIgnore = false;
            llSegmentId = -1;
            iSeqNum = -1;
            sWord.Erase();
        }
    };

    using InvariantParses = set<shared_ptr<StWordParse>>;

    //  CREATE TABLE tact_group(id INTEGER PRIMARY KEY ASC, line_id INTEGER, first_word_position INTEGER, num_of_words INTEGER, 
    //  source TEXT, transcription TEXT, stressed_syllable INTEGER, reverse_stressed_syllable INTEGER, FOREIGN KEY(line_id) REFERENCES lines_in_text(id));
    struct StTactGroup
    {
        int64_t llLineId;
        int iFirstWordNum;
        int iMainWordPos;
        int iNumOfWords;
        int iNumOfSyllables;
        int iStressedSyllable;             // TODO: multiple stresses, dash-separated compounds etc
        int iReverseStressedSyllable;
        int iSecondaryStressedSyllable;
        CEString sSource;
        CEString sTranscription;
        CEString sPhonemicSequence;
        vector<InvariantParses> m_vecParses;
        vector<shared_ptr<StTactGroup>> m_vecNext;

        StTactGroup() : llLineId(0), iFirstWordNum(0), iMainWordPos(0),
            iNumOfWords(0), iNumOfSyllables(0), iStressedSyllable(0),
            iReverseStressedSyllable(0), iSecondaryStressedSyllable(0)
        {}

        void Reset()
        {
            llLineId = 0;
            iFirstWordNum = 0;
            iMainWordPos = 0;
            iNumOfWords = 0;
            iNumOfSyllables = 0;
            iStressedSyllable = 0;
            iReverseStressedSyllable = 0;
            iSecondaryStressedSyllable = 0;
            sSource.Erase();
            sTranscription.Erase();
            sPhonemicSequence.Erase();
        }

        int iWordNumFromTextPos(int iTextPos)
        {
            return sSource.uiGetTokenNum(sSource.stGetTokenFromOffset(iTextPos));
        }

        void AddWord(InvariantParses setParse, const CEString& sWord)
        {
            ++iNumOfWords;
            m_vecParses.push_back(setParse);
            if (sSource.uiLength() > 0)
            { 
                sSource += L' ';
            }
            sSource += sWord;
        }

    };      //  StTactGroup

    class CAnalytics
    {
    using ArrMetadataValues= array<CEString, Hlib::TEXT_METADATA_COUNT>;

    public:
        CAnalytics();
        CAnalytics(shared_ptr<CSqlite>, shared_ptr<CParser>);
        ~CAnalytics();

//        virtual ET_ReturnCode eParseText(const CEString& sTextName, const CEString& sMetadata, const CEString& sText, int64_t& llParsedTextId, bool bIsProse = false);
        void SetAuthor(const CEString sAuthor);
        void SetBookTitle(const CEString sBookTitle);
        ET_ReturnCode eParseText(const CEString& sMetadata, const CEString& sText, int64_t llFirstLineNum, bool bIsProse = false);
        ET_ReturnCode eLoadIrregularForms();
        ET_ReturnCode eGetFirstSegment(vector<StWordContext>&, int64_t llStartAt=0);
        ET_ReturnCode eGetNextSegment(vector<StWordContext>&);

    private:
        ET_ReturnCode eInit();
        ET_ReturnCode eHandleDbException(CException&, CEString& sMsg);
        ET_ReturnCode eParseMetadata(const CEString&, ArrMetadataValues&);
        ET_ReturnCode eRegisterText();
        ET_ReturnCode eParseWord(const CEString& sWord, const CEString& sLine, int iNumInLine, 
                                 int iWordsInLine, int64_t llLineDbKey, vector<shared_ptr<StWordParse>>& vecParses);
        ET_ReturnCode eFindEquivalencies(const vector<shared_ptr<StWordParse>>&, vector<InvariantParses>&);
        ET_ReturnCode eGetStress(shared_ptr<StTactGroup>);
        ET_ReturnCode eTranscribe(shared_ptr<StTactGroup>);
        ET_ReturnCode eSaveLineDescriptor(int iLineNum, int iTextOffset, int iLength, int iNumOfWords, const CEString& sText, int64_t& llDbKey);
        ET_ReturnCode eSaveLineParses(shared_ptr<StTactGroup> spTactGroupListHead);
        ET_ReturnCode eSaveWord(int64_t llLineDbId, int iWord, int iWordsInLine, int iLineOffset, int iSegmentLength, const CEString& sWord, int64_t& llWordDbKey);
        ET_ReturnCode eSaveWordParse(int64_t llSegmentId, int64_t llWordFormId, int64_t& llWordToWordFormId);
        ET_ReturnCode eSaveTactGroup(shared_ptr<StTactGroup>);
        ET_ReturnCode eClearTextData(int64_t llText);
        ET_WordStressType eGetStressType(CWordForm&);
        bool bArePhoneticallyIdentical(shared_ptr<CWordForm>, shared_ptr<CWordForm>);
        ET_ReturnCode eAddParsesToTactGroup(int64_t llLineDbId, int iLineNum, int iWord, shared_ptr<StTactGroup>);

        ET_ReturnCode eGetSegment(vector<StWordContext>&);
        ET_ReturnCode eAssembleParsedSegment(vector<StWordContext>&);
        ET_ReturnCode eAddStressMark(CEString&, int, ET_StressType);


    private:
        shared_ptr<CSqlite> m_spDb;
        shared_ptr<CParser> m_spParser;
        unique_ptr<CTranscriber> m_spTranscriber;
        CEString m_sAuthor;
        CEString m_sBookTitle;
        CEString m_sTextName;
        CEString m_sTextTitle;
        CEString m_sTextMetaData;
        CEString m_sText;
        int64_t m_llTextDbId;

        multimap<int, InvariantParses> m_mmapLinePosToHomophones;
        vector<shared_ptr<StTactGroup>> m_vecTactGroupListHeads;
        int64_t m_llCurrentSegmentId{-1};
        CEString m_sCurrentTitle;
        CEString m_sCurrentSegment;
        vector<StToken> m_vecCurrentSourceTokens;
        vector<CEString> m_vecCurrentSourceWords;
        int m_iWordsInCurrentLine;
        int m_iCurrentPos {-1};

        struct StCEStringComparator {
            bool operator()(const shared_ptr<CEString> lhs, const shared_ptr<CEString> rhs) const {
                return *lhs < *rhs;
            }
        };
        multimap<shared_ptr<CEString>, shared_ptr<StIrregularWord>, StCEStringComparator> m_mmapWordToIrregForm;

        multimap<int, int64_t> m_mmapWordPosToFormIds;                              // 1 to many, multiple hypotheses
        multimap<int, int64_t> m_mmapWordPosToIrregIds;                             // 1 to many, multiple hypotheses
        multimap<int64_t, pair<int, ET_StressType>> m_mmapFormIdToStressPositions;  // 1 to many, e.g. priimary + secondary stress
        map<int64_t, CEString> m_mapFormIdToGramHashes;                             // 1 to 1?
        map<int, CEString> m_mapWordPosToWord;

        map<CEString, int> m_mapKeyToColumn
        {
            { L"author",        1 },
            { L"book",          2 },
            { L"page",          3 },
            { L"title",         4 },
            { L"dedication",    5 },
            { L"chapter",       6 },
            { L"footnote_ref",  7 },
            { L"footnone_text", 8 },
            { L"date",          9 }
        };
    };      //  class CAnalytics

}   // namespace Hlib

#endif  //  ANALYTICS_H