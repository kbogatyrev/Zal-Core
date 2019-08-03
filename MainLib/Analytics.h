#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <set>
#include <vector>
#include <map>

#include "IAnalytics.h"

namespace Hlib
{
    class CSqlite;
    class CParser;
    struct CWordForm;
    class CEString;

    class CAnalytics : public IAnalytics
    {
    public:
        struct StWordParse
        {
            int iNumber;
            int iLineOffset;
            int iLength;
            int iPosInTactGroup;
            long long llLineDbId;
            long long llWordInLIneDbId;
            long long llWordToWordFormId;
            CWordForm WordForm;

            StWordParse()
            {
                Reset();
            }

            void Reset()
            {
                int iNumber = -1;
                int iLineOffset = -1;
                int iLength = -1;
                int iPosInTactGroup = -1;
                long long llLineDbId = -1;
                long long llWordInLIneDbId = -1;
                long long llWordToWordFormId = -1;
                CWordForm WordForm;
            }

            // Needed to be used with STL set
            bool operator < (const StWordParse& stRhs) const
            {
                return iNumber < stRhs.iNumber;
            }

        };      // StWordParse

        //  CREATE TABLE tact_group(id INTEGER PRIMARY KEY ASC, line_id INTEGER, first_word_position INTEGER, num_of_words INTEGER, 
        //  source TEXT, transcription TEXT, stressed_syllable INTEGER, reverse_stressed_syllable INTEGER, FOREIGN KEY(line_id) REFERENCES lines_in_text(id));
        struct StTactGroup
        {
            long long llLineId;
            int iFirstWordNum;
            int iNumOfWords;
            int iStressPos;
            int iReverseStressPos;
            int iSecondaryStressPos;
            CEString sSource;
            CEString sTranscription;
            vector<StWordParse> vecWords;

            StTactGroup()
            {
                Reset();
            }

            void Reset()
            {
                llLineId = -1;
                iFirstWordNum = -1;
                iNumOfWords = 0;
                iStressPos = -1;
                iReverseStressPos = -1;
                iSecondaryStressPos = -1;
                sSource.Erase();
                sTranscription.Erase();
                vecWords.clear();
            }
        };      //  StTactGroup

    public:
        CAnalytics();
        CAnalytics(CSqlite*, CParser*);
        ~CAnalytics();

        virtual ET_ReturnCode eParseText(const CEString& sTextName, const CEString& sMetadata, const CEString& sText);
        virtual void ClearResults();

    private:
        ET_ReturnCode eRegisterText(const CEString& sTextName, const CEString sTextMetadata, const CEString& sText);
        ET_ReturnCode eParseWord(const CEString& sWord, int iLine, int iNumInLine, long long llLineDbKey);
        ET_ReturnCode eFindEquivalencies(CEString& sLine);
        ET_ReturnCode eAssembleTactGroups(CEString& sLine);
        ET_ReturnCode eSaveLine(long long llTextId, int iLineNum, int iTextOffset, int iLength, int iNumOfWords, const CEString& sText, long long& llDbKey);
        ET_ReturnCode eSaveWord(long long llLineDbId, int iLine, int iWord, int iLineOffset, int iSegmentLength, const CEString& sWord, long long& llWordDbKey);
        ET_ReturnCode eSaveWordParse(long long llSegmentId, long long llWordFormId, long long& llWordToWordFormId);
        ET_ReturnCode eSaveTactGroups(vector<StTactGroup>&);
        ET_ReturnCode eClearTextData(long long llText);
        bool bIsProclitic(const CWordForm&);
        bool bIsEnclitic(const CWordForm&);
        bool bArePhoneticallyIdentical(CWordForm& wf1, CWordForm& wf2);

    private:
        CSqlite* m_pDb;
        CParser* m_pParser;
        CEString m_sTextName;
        CEString m_sTextTitle;
        CEString m_sTextMetaData;
        CEString m_sText;
        long long m_llTextDbId;

        multimap<int, StWordParse> m_mmapWordParses;
        multimap<int, vector<StWordParse>> m_mmapEquivalencies;  // word # --> phonetic invariant sets
        multimap<int, StTactGroup> m_mapTactGroups;   // 1st word # --> tact group

    };      //  class CAnalytics

}   // namespace Hlib

#endif  //  ANALYTICS_H