#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <set>
#include <vector>
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
//    class CWordForm;
    class CEString;

    struct StWordParse
    {
        int iPosInLine;
        int iLineOffset;
        int iLength;
//        int iPosInTactGroup;        &&&& This doesn't work!!'
        ET_WordStressType eStressType;
        long long llLineDbId;
        long long llWordInLineDbId;
        long long llWordToWordFormId;
        CWordForm WordForm;

        StWordParse()
        {
            Reset();
        }

        void Reset()
        {
            iPosInLine = -1;
            iLineOffset = -1;
            iLength = -1;
//            iPosInTactGroup = -1;
            llLineDbId = -1;
            eStressType = WORD_STRESS_TYPE_UNDEFINED;
            llWordInLineDbId = -1;
            llWordToWordFormId = -1;
//            CWordForm WordForm;
        }

        // Needed to be used with STL set
        bool operator < (const StWordParse& stRhs) const
        {
            return iPosInLine < stRhs.iPosInLine;
        }
    };      // StWordParse

    using InvariantParses = set<shared_ptr<StWordParse>>;

    //  CREATE TABLE tact_group(id INTEGER PRIMARY KEY ASC, line_id INTEGER, first_word_position INTEGER, num_of_words INTEGER, 
    //  source TEXT, transcription TEXT, stressed_syllable INTEGER, reverse_stressed_syllable INTEGER, FOREIGN KEY(line_id) REFERENCES lines_in_text(id));
    struct StTactGroup
    {
        long long llLineId;
        int iFirstWordNum;
        int iMainWordPos;
        int iNumOfWords;
        int iNumOfSyllables;
        int iStressedSyllable;             // TODO: multiple stresses, dash-separated compounds etc
        int iReverseStressedSyllable;
        int iSecondaryStressedSyllable;
        CEString sSource;
        CEString sTranscription;
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
    public:
        CAnalytics();
        CAnalytics(shared_ptr<CSqlite>, shared_ptr<CParser>);
        ~CAnalytics();

        virtual ET_ReturnCode eParseText(const CEString& sTextName, const CEString& sMetadata, const CEString& sText, long long& llParsedTextId, bool bIsProse = false);

    private:
        ET_ReturnCode eInit();
        ET_ReturnCode eParseMetadata(const CEString& sMetadata);
        ET_ReturnCode eRegisterText();
        ET_ReturnCode eParseWord(const CEString& sWord, const CEString& sLine, int iNumInLine, 
                                 int iWordsInLine, long long llLineDbKey, vector<shared_ptr<StWordParse>>& vecParses);
        ET_ReturnCode eFindEquivalencies(const vector<shared_ptr<StWordParse>>&, vector<InvariantParses>&);
        ET_ReturnCode eGetStress(shared_ptr<StTactGroup>);
        ET_ReturnCode eTranscribe(shared_ptr<StTactGroup>);
        ET_ReturnCode eSaveLine(int iLineNum, int iTextOffset, int iLength, int iNumOfWords, const CEString& sText, long long& llDbKey);
        ET_ReturnCode eSaveWord(long long llLineDbId, int iWord, int iWordsInLine, int iLineOffset, int iSegmentLength, const CEString& sWord, long long& llWordDbKey);
        ET_ReturnCode eSaveWordParse(long long llSegmentId, long long llWordFormId, long long& llWordToWordFormId);
        ET_ReturnCode eSaveTactGroup(shared_ptr<StTactGroup>);
        ET_ReturnCode eClearTextData(long long llText);
        ET_WordStressType eGetStressType(CWordForm&);
        bool bArePhoneticallyIdentical(CWordForm& wf1, CWordForm& wf2);
        ET_ReturnCode eAddParsesToTactGroup(int iLineNum, int iWord);

    private:
        shared_ptr<CSqlite> m_spDb;
        shared_ptr<CParser> m_spParser;
        unique_ptr<CTranscriber> m_spTranscriber;
        CEString m_sTextName;
        CEString m_sTextTitle;
        CEString m_sTextMetaData;
        CEString m_sText;
        long long m_llTextDbId;

        shared_ptr<StTactGroup> m_spCurrentTactGroup;
        multimap<int, InvariantParses> m_mmapLinePosToHomophones;
        vector<pair<CEString, CEString>> m_vecMetadataKeyValPairs;
        vector<shared_ptr<StTactGroup>> m_vecTactGroupListHead;
        int m_iWordsInCurrentLine;

    };      //  class CAnalytics

}   // namespace Hlib

#endif  //  ANALYTICS_H