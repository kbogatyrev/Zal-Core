﻿#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <set>
#include <vector>
#include <map>
#include "Logging.h"
#include "Exception.h"
#include "SqliteWrapper.h"
#include "WordForm.h"
#include "Parser.h"
#include "Transcriber.h"
#include "Analytics.h"

using namespace Hlib;

CAnalytics::CAnalytics() : m_spDb(nullptr), m_llTextDbId(-1) 
{
    eInit();
}

CAnalytics::CAnalytics(shared_ptr<CSqlite> pDb, shared_ptr<CParser> pParser) : m_spDb(pDb), m_spParser(pParser), m_llTextDbId(-1)
{
    eInit();
}

CAnalytics::~CAnalytics()
{}

ET_ReturnCode CAnalytics::eInit()
{
    m_spTranscriber = make_unique<CTranscriber>(m_spDb);
    if (nullptr == m_spTranscriber)
    {
        return H_ERROR_POINTER;
    }

    eLoadIrregularForms();

    return H_NO_ERROR;
}

ET_ReturnCode CAnalytics::eHandleDbException(CException& e, CEString& sMsg)
{
    sMsg = e.szGetDescription();
    CEString sError;
    try
    {
        m_spDb->GetLastError(sError);
        sMsg += CEString(L", error: ");
        sMsg += sError;
    }
    catch (...)
    {
        sMsg = L"Apparent DB error ";
    }
    return H_NO_ERROR;
}

void CAnalytics::SetAuthor(const CEString sAuthor)
{
    m_sAuthor = sAuthor;
}

void CAnalytics::SetBookTitle(const CEString sBookTitle)
{
    m_sBookTitle = sBookTitle;
}

ET_ReturnCode CAnalytics::eParseText(const CEString& sMetadata, const CEString& sText, [[maybe_unused]]int64_t llFirstLineNum, bool bIsProse)
{
    ET_ReturnCode eRet = H_NO_ERROR;

    m_sTextMetaData = sMetadata;
    m_sText = sText;                // used to split text into lines

    if (bIsProse)
    {
        const_cast<CEString&>(sText).SetBreakChars(L".");
    }
    else
    {
        const_cast<CEString&>(sText).SetBreakChars(L"\r\n");
    }
    const_cast<CEString&>(sText).SetTabs(L"");
    const_cast<CEString&>(sText).SetPunctuation(L"");
    const_cast<CEString&>(sText).SetEscapeChars(L"");

    CEString sQuery;

/*
*   Table structures:
* 
*    CREATE TABLE lines_in_text(id INTEGER PRIMARY KEY ASC, text_id INTEGER, line_number INTEGER, text_offset INTEGER, length INTEGER, number_of_words INTEGER, source TEXT, FOREIGN KEY(text_id) REFERENCES text(id) ON DELETE CASCADE);
*
*    --word_text redundant, needed for testing
*        CREATE TABLE words_in_line(id INTEGER PRIMARY KEY ASC, line_id INTEGER, word_position INTEGER, line_offset INTEGER, word_length INTEGER, word_text TEXT, FOREIGN KEY(line_id) REFERENCES lines_in_text(id) ON DELETE CASCADE);
*
*    CREATE TABLE word_to_wordform(id INTEGER PRIMARY KEY ASC, word_in_line_id INTEGER, wordform_id INTEGER, FOREIGN KEY(word_in_line_id) REFERENCES words_in_line(id), FOREIGN KEY(wordform_id) REFERENCES wordforms(id) ON DELETE CASCADE);
*
*    -- this table is added to simplify retrieval of transcription and stress positions
*        CREATE TABLE tact_group(id INTEGER PRIMARY KEY ASC, line_id INTEGER, first_word_position INTEGER, num_of_words INTEGER, source TEXT, transcription TEXT, stressed_syllable INTEGER, reverse_stressed_syllable INTEGER, FOREIGN KEY(line_id) REFERENCES lines_in_text(id) ON DELETE CASCADE);
*
*    CREATE TABLE word_to_tact_group(id INTEGER PRIMARY KEY ASC, word_to_wordform_id INTEGER, tact_group_id INTEGER, position_in_tact_group INTEGER, FOREIGN KEY(word_to_wordform_id) REFERENCES word_to_word_form(id) ON DELETE CASCADE);
*/

    if (nullptr == m_spDb)
    {
        ERROR_LOG(L"No database access.");
        return H_ERROR_POINTER;
    }

    if (nullptr == m_spParser)
    {
        ERROR_LOG(L"No parser object.");
        return H_ERROR_POINTER;
    }

    m_spDb->BeginTransaction();

    eRet = eRegisterText();
    if (eRet != H_NO_ERROR)
    {
        m_spDb->RollbackTransaction();
        ERROR_LOG(L"Unable to register text");
        return eRet;
    }

    //
    // Parse text word by word
    //
    int iNLines = const_cast<CEString&>(sText).uiNFields();
    if (iNLines < 1)
    {
        m_spDb->RollbackTransaction();
        return H_FALSE;
    }

    for (int iLine = 0; iLine < iNLines; ++iLine)
    {
        int iTextOffset = const_cast<CEString&>(sText).uiGetFieldOffset(iLine);
        
        CEString sLine = const_cast<CEString&>(sText).sGetField(iLine);     // use to split line into words
        sLine.EnableBreaks();
        sLine.EnablePunctuation();
        sLine.EnableEscapeChars();
        sLine.EnableTabs();
        sLine.EnableVowels();

        m_iWordsInCurrentLine = sLine.uiNFields();
        if (m_iWordsInCurrentLine < 1)
        {
            continue;
        }

        int64_t llLineDbId = -1;
        eRet = eSaveLineDescriptor(iLine, iTextOffset, sLine.uiLength(), sLine.uiNFields(), sLine, llLineDbId);
        if (eRet != H_NO_ERROR)
        {
            continue;
        }

        m_mmapLinePosToHomophones.clear();

        for (int iField = 0; iField < m_iWordsInCurrentLine; ++iField)
        {
            vector<shared_ptr<StWordParse>> vecParses;
            CEString sWord = sLine.sGetField(iField);
            sWord.ToLower();
            eRet = eParseWord(sWord, sLine, iField, (int)sLine.uiNFields(), llLineDbId, vecParses);
            unsigned int uiStartPos = 0;
            auto uiAt = sWord.uiFind(L"е", uiStartPos);
            while (uiAt != Hlib::ecNotFound)
            {
                sWord[uiAt] = L'ё';
                eRet = eParseWord(sWord, sLine, iField, (int)sLine.uiNFields(), llLineDbId, vecParses);
                sWord[uiAt] = L'е';
                uiStartPos = uiAt + 1;
                if (uiStartPos >= sWord.uiLength())
                {
                    break;
                }
                uiAt = sWord.uiFind(L"е", uiStartPos);
            }

            vector<InvariantParses> vecPhoneticVariants;
            eRet = eFindEquivalencies(vecParses, vecPhoneticVariants);
            if (eRet != H_NO_ERROR)
            {
                CEString sMsg(L"Failed fo find equivalencies; line: ");
                sMsg += sLine;
                ERROR_LOG(sMsg);
                continue;
            }

            for (auto& setInvariants : vecPhoneticVariants)
            {
                m_mmapLinePosToHomophones.insert(make_pair(iField, setInvariants));
            }

//            m_spParser->ClearResults();

        }       // for (int iField ...)

//        m_vecTactGroupListHeads.clear();        // TODO -- verify that it is empty first?

//        auto spCurrentTactGroup = make_shared<StTactGroup>();
//        spCurrentTactGroup->llLineId = llLineDbId;
//        eRet = eAddParsesToTactGroup(sLine, llLineDbId, iLine, 0, spCurrentTactGroup);

//        spCurrentTactGroup->iNumOfSyllables = spCurrentTactGroup->sSource.uiNSyllables();

//        for (auto spTg : m_vecTactGroupListHeads) {
//            eSaveLineParses(spTg);
//        }
    }       //  for (int iLine = 0 ...)

    m_spDb->CommitTransaction();

//    m_spParser->ClearResults();

//    llParsedTextId = m_llTextDbId;

    return H_NO_ERROR;

}       // eParseText()

ET_ReturnCode CAnalytics::eParseMetadata(const CEString& sConstMetadata, ArrMetadataValues& arrValues)
{
    CEString sMetadata(sConstMetadata);
    sMetadata.ResetSeparators();
    sMetadata.SetBreakChars(L"|");

    if (sMetadata.uiNFields() < 1)
    {
        CEString sMsg(L"Unable to parse text metadata: ");
        ERROR_LOG(sMsg + sMetadata);
        return ET_ReturnCode(H_ERROR_INVALID_ARG);
    }

    for (int iKeyValPair = 0; iKeyValPair < (int)sMetadata.uiNFields(); ++iKeyValPair)
    {
        auto sKeyValPair = sMetadata.sGetField(iKeyValPair);
        sKeyValPair.ResetSeparators();
        sKeyValPair.SetBreakChars(L"=");
        if (sKeyValPair.uiNFields() < 2)
        {
            continue;       // empty values are allowed
        }

        auto&& sKey = sKeyValPair.sGetField(0);
        auto&& sValue = sKeyValPair.sGetField(1);
        if (m_mapKeyToColumn.find(sKey) == m_mapKeyToColumn.end())
        {
            CEString sMsg(L"Unknown metadata key ");
            sMsg += sKey;
            ERROR_LOG(sMsg);
            return H_ERROR_UNEXPECTED;
        }

        auto iCol = m_mapKeyToColumn[sKey];
        arrValues.at(iCol) = move(sValue);

//        auto sKey = sKeyValPair.sGetField(0);
//        sKey.Trim(L" ");
//        auto sValue = sKeyValPair.sGetField(1);
//        sValue.Trim(L" ");
//        m_vecMetadataKeyValPairs.push_back(make_pair(sKey, sValue));
    }
    return H_NO_ERROR;
}

//  CREATE TABLE text(id INTEGER PRIMARY KEY ASC, contents TEXT);
ET_ReturnCode CAnalytics::eRegisterText()
{
    ET_ReturnCode eRet = H_NO_ERROR;

    ArrMetadataValues arrValues;
    eRet = eParseMetadata(m_sTextMetaData, arrValues);
    if (eRet != H_NO_ERROR)
    {
        ERROR_LOG(L"Unable to parse text metadata.");
        return eRet;
    }

    if (m_sTextMetaData.bIsEmpty())
    {
        ERROR_LOG(L"No text or text descriptor.");
        return H_ERROR_UNEXPECTED;
    }

    try
    {
        m_spDb->PrepareForInsert(L"text", 1);
        m_spDb->Bind(1, m_sText);
        m_spDb->InsertRow();
        m_spDb->Finalize();

        m_llTextDbId = m_spDb->llGetLastKey();
    }
    catch (CException& e)
    {
        m_llTextDbId = -1;

        CEString sMsg;
        eHandleDbException(e, sMsg);
        ERROR_LOG(sMsg);
        return H_ERROR_DB;
    }

    //                                                            0                1           2          3           4             5
    //  CREATE TABLE text_metadata (id INTEGER PRIMARY KEY ASC, text_id INTEGER, author TEXT, book TEXT, page TEXT, title TEXT, dedication TEXT, 
    //    6              7                   8               9           
    // chapter TEXT, footnote_ref TEXT, footnote_text TEXT, date TEXT, FOREIGN KEY (text_id) REFERENCES text(id));

    try
    {
        m_spDb->PrepareForInsert(L"text_metadata", 10);
        m_spDb->Bind(1, (int64_t)m_llTextDbId);
        for (int iColumn = 1; iColumn < (int)arrValues.size(); ++iColumn)
        {
            m_spDb->Bind(iColumn+1, arrValues[iColumn]);
        }
        m_spDb->InsertRow();
        m_spDb->Finalize();
    }
    catch (CException& e)
    {
        CEString sMsg;
        eHandleDbException(e, sMsg);
        ERROR_LOG(sMsg);
        return H_ERROR_DB;
    }

    return eRet;

}       //  eRegisterText()

ET_ReturnCode CAnalytics::eParseWord(const CEString& sWord, 
                                     const CEString& sLine, 
                                     int iNumInLine, 
                                     int iWordsInLine, 
                                     int64_t llLineDbKey, 
                                     vector<shared_ptr<StWordParse>>& vecParses)
{
    if (nullptr == m_spParser)
    {
        ERROR_LOG(L"Parser not available.");
        return H_ERROR_POINTER;
    }

    ET_ReturnCode eRet = H_NO_ERROR;

    eRet = m_spParser->eParseWord(sWord);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    int iOffset = const_cast<CEString&>(sLine).uiGetFieldOffset(iNumInLine);
    int64_t llWordInLineDbKey = -1;
    eRet = eSaveWord(llLineDbKey, iNumInLine, iWordsInLine, iOffset, sWord.uiLength(), sWord, llWordInLineDbKey);   // words_in_line
    if (eRet != H_NO_ERROR)
    {
        CEString sMsg(L"Unable to save a word '");
        sMsg += sWord + L"'";
        ERROR_LOG(L"Unable to save a word.");
        return eRet;
    }

    shared_ptr<CWordForm> spWf;
    eRet = m_spParser->eGetFirstWordForm(spWf);
    while (H_NO_ERROR == eRet)
    {
        int64_t llWordToWordFormId = -1;
        eRet = eSaveWordParse(llWordInLineDbKey, spWf->llDbKey(), llWordToWordFormId);   //  word_to_wordform
        if (eRet != H_NO_ERROR)
        {
            CEString sMsg(L"Unable to save a word parse, word: ");
            sMsg += spWf->sWordForm();
            ERROR_LOG(sMsg);
            continue;
        }

        StWordParse stParse;
        stParse.iPosInLine = iNumInLine;
        stParse.iLineOffset = iOffset;
        stParse.iLength = sWord.uiLength();
        stParse.eStressType = eGetStressType(*spWf);
        stParse.llLineDbId = llLineDbKey;
        stParse.llWordInLineDbId = llWordInLineDbKey;
        stParse.llWordToWordFormId = llWordToWordFormId;
        stParse.spWordForm = spWf;

        vecParses.push_back(make_shared<StWordParse>(stParse));

        eRet = m_spParser->eGetNextWordForm(spWf);
    
    }       //  while...

    m_spParser->ClearResults();

    return H_NO_ERROR;

}       //  eParseWord()

ET_ReturnCode CAnalytics::eFindEquivalencies(const vector<shared_ptr<StWordParse>>& vecParses, 
                                             vector<InvariantParses>& vecVariants)
{
    if (!vecVariants.empty())
    {
        ERROR_LOG(L"Second argument must be an empty vector.");
        return H_ERROR_UNEXPECTED;
    }

    // Find phonetically identical parses, i.e. same stress
    for (auto& pstParse : vecParses)
    {
        bool bIsHomophone = false;
        // Any phonetically identical parses for this wordform?
        for (auto& setInvariants : vecVariants)
        {
            if (setInvariants.empty())
            {
                ERROR_LOG(L"Empty invariant set.");
                return H_ERROR_UNEXPECTED;
            }
            auto pstRhs = *(setInvariants.begin());
            if (bArePhoneticallyIdentical(pstParse->spWordForm, pstRhs->spWordForm))
            {
                bIsHomophone = true;
                setInvariants.insert(pstParse);
                break;
            }
        }

        if (!bIsHomophone)
        {
            vecVariants.emplace_back(InvariantParses{pstParse});
        }
    }

    return H_NO_ERROR;

}       //  eFindEquivalencies()

ET_ReturnCode CAnalytics::eGetStress(shared_ptr<StTactGroup> pTg)
{
    ET_ReturnCode eRet = H_NO_ERROR;

    if (pTg->m_vecParses.size() < 1)
    {
        return H_FALSE;
    }

    vector<int> vecPrimary;
    vector<int> vecSecondary;

    pTg->iNumOfSyllables = pTg->sSource.uiNSyllables();

    for (int iWord = pTg->iFirstWordNum, iCount = 0; iCount < pTg->iNumOfWords; ++iWord, ++iCount)
    {
        if (pTg->iNumOfWords < (int)pTg->m_vecParses.size())
        {
            CEString sMsg(L"Size of the parse array does not match number of words in the tact group.");
            return H_ERROR_UNEXPECTED;
        }

        auto& setInvariants = pTg->m_vecParses[iCount];
        auto pParse = *setInvariants.begin();      // we can use any of the words
        if (WORD_STRESS_TYPE_AUTONOMOUS == pParse->eStressType)
        {
            int iPos = -1;
            ET_StressType eType = STRESS_TYPE_UNDEFINED;
            eRet = pParse->spWordForm->eGetFirstStressPos(iPos, eType);
            if (H_NO_ERROR == eRet)     //  NB: H_NO_MORE is the same as H_FALSE
            {
                if (STRESS_PRIMARY == eType)
                {
                    vecPrimary.push_back(iPos);
                }
                else if (STRESS_SECONDARY == eType)
                {
                    vecSecondary.push_back(eType);
                }
            }
            else
            {
                return eRet;
            }

            // Very unlikely:
            while (H_NO_ERROR == eRet)
            {
                eRet = pParse->spWordForm->eGetNextStressPos(iPos, eType);
                if (H_NO_ERROR == eRet)     //  NB: H_NO_MORE is the same as H_FALSE
                {
                    if (STRESS_PRIMARY == eType)
                    {
                        vecPrimary.push_back(iPos);
                    }
                    else if (STRESS_SECONDARY == eType)
                    {
                        vecSecondary.push_back(eType);
                    }
                }
                else
                {
                    break;
                }
            }       //  while (H_NO_ERROR == eRet)...

            if (H_NO_ERROR != eRet && H_NO_MORE != eRet && H_FALSE != eRet)
            {
                return eRet;
            }

            if (vecPrimary.size() == 1)
            {
                auto iStressedCharPosInWord = *vecPrimary.begin();
                CEString sWord = pParse->spWordForm->sWordForm();

                try
                {
                    pTg->iStressedSyllable = sWord.uiGetSyllableFromVowelPos(iStressedCharPosInWord);
                }
                catch (...)
                {
                    return H_EXCEPTION;
                }

                // If this word is not the first word in the tact group, add syllable count from that word
                for (auto iProclitic = pTg->iFirstWordNum; iProclitic < iWord; ++iProclitic)
                {
                    auto& setParses = pTg->m_vecParses[iProclitic - pTg->iFirstWordNum];
                    auto pWpProclitic = *setParses.begin();      // we can use any of the words
                    pTg->iStressedSyllable += pWpProclitic->spWordForm->sWordForm().uiNSyllables();
                }

                eRet = H_NO_ERROR;
            }
            else
            {
                eRet = H_FALSE; //  TODO: multiple stresses
            }
        }

        if (vecSecondary.size() == 1)
        {
            auto iSecondaryStressedCharPosInWord = *vecSecondary.begin();
            CEString sWord = pParse->spWordForm->sWordForm();

            try
            {
                pTg->iSecondaryStressedSyllable = sWord.uiGetSyllableFromVowelPos(iSecondaryStressedCharPosInWord);
            }
            catch (CException& ex)
            {
                CEString sMsg(L"Unable to get secondary stress: ");
                sMsg += ex.szGetDescription();
                return H_EXCEPTION;
            }

            // If this word is not the first word in the tact group, add syllable count from that word
            for (auto iProclitic = pTg->iFirstWordNum; iProclitic < iWord; ++iProclitic)
            {
                auto& setParses = pTg->m_vecParses[iWord];
                auto pWpProclitic = *setParses.begin();      // we can use any of the words
                pTg->iStressedSyllable += pWpProclitic->spWordForm->sWordForm().uiNSyllables();
            }
            for (auto iProclitic = pTg->iFirstWordNum; iProclitic < iWord; ++iProclitic)
            {
                auto& setParses = pTg->m_vecParses[iWord];
                auto pWpProclitic = *setParses.begin();      // we can use any of the words
                pTg->iSecondaryStressedSyllable += pWpProclitic->spWordForm->sWordForm().uiNSyllables();
            }

            eRet = H_NO_ERROR;
        }
//        else
//        {
//            eRet = H_FALSE;
//        }

    }       //  for (auto iWord = stTg.iFirstWordNum; ...

    int iStressPosInTactGroup = pTg->sSource.uiGetVowelPos(pTg->iStressedSyllable);
    pTg->sSource = pTg->sSource.sInsert(iStressPosInTactGroup + 1, CEString::g_chrCombiningAcuteAccent);

    auto iTotalSyllables = pTg->sSource.uiNSyllables();

    if (pTg->iStressedSyllable >= 0)
    {
        pTg->iReverseStressedSyllable = iTotalSyllables - pTg->iStressedSyllable - 1;
    }

    return eRet;

}       //  eGetStress()

ET_ReturnCode CAnalytics::eTranscribe(shared_ptr<StTactGroup> pTg)
{
    if (nullptr == m_spTranscriber)
    {
        ERROR_LOG(L"m_spTranscriber is nullptr.");
        return H_ERROR_POINTER;
    }

    auto eRet = m_spTranscriber->eTranscribeTactGroup(pTg);

    return eRet;
}

//  CREATE TABLE lines_in_text(id INTEGER PRIMARY KEY ASC, text_id INTEGER, line_number INTEGER, text_offset INTEGER, number_of_words INTEGER, 
//  source TEXT, FOREIGN KEY(text_id) REFERENCES text(id));
ET_ReturnCode CAnalytics::eSaveLineDescriptor(int iLineNum, int iTextOffset, int iLength, int iNumOfWords, const CEString& sText, int64_t& llDbKey)
{
    try
    {
        if (nullptr == m_spDb)
        {
            ERROR_LOG(L"No database access.");
            return H_ERROR_POINTER;
        }

        m_spDb->PrepareForInsert(L"lines_in_text", 6);

        m_spDb->Bind(1, (int64_t)m_llTextDbId);
        m_spDb->Bind(2, iLineNum);
        m_spDb->Bind(3, iTextOffset);
        m_spDb->Bind(4, iLength);
        m_spDb->Bind(5, iNumOfWords);
        m_spDb->Bind(6, sText);

        m_spDb->InsertRow();
        m_spDb->Finalize();

        llDbKey = m_spDb->llGetLastKey();

    }
    catch (CException& e)
    {
        CEString sMsg;
        eHandleDbException(e, sMsg);
        return H_ERROR_GENERAL;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CAnalytics::eSaveLineParses(shared_ptr<StTactGroup> spTactGroup)
{
    if (spTactGroup->sSource.bIsEmpty())
    {
        ERROR_LOG(L"Empty tact group.");
        return H_ERROR_UNEXPECTED;
    }

    auto rc = eGetStress(spTactGroup);
    if (rc != H_NO_ERROR && rc != H_FALSE && rc != H_NO_ERROR)
    {
        ERROR_LOG(L"Unable to retrieve stress data.");
        return H_ERROR_UNEXPECTED;
    }

    if (H_NO_ERROR == rc)   // H_FALSE?
    {
        rc = eTranscribe(spTactGroup);
    }

    rc = eSaveTactGroup(spTactGroup);
    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    if (spTactGroup->m_vecNext.empty())
    {
        return H_NO_ERROR;
    }

    for (auto spNext : spTactGroup->m_vecNext)
    {
        eSaveLineParses(spNext);
    }

    return H_NO_ERROR;
}

//  CREATE TABLE words_in_line(id INTEGER PRIMARY KEY ASC, line_id INTEGER, word_position INTEGER, reverse_word_position INTEGER, line_offset INTEGER, word_length INTEGER, 
//  word_text TEXT, FOREIGN KEY(line_id) REFERENCES lines_in_text(id));
ET_ReturnCode CAnalytics::eSaveWord(int64_t llLineDbId, int iWord, int iWordsInLine, int iLineOffset, int iSegmentLength, const CEString& sWord, int64_t& llWordDbKey)
{
    try
    {
        if (nullptr == m_spDb)
        {
            ERROR_LOG(L"No database access.");
            return H_ERROR_POINTER;
        }

        if (m_sTextMetaData.bIsEmpty())
        {
            ERROR_LOG(L"No text or text descriptor.");
            return H_ERROR_UNEXPECTED;
        }

        m_spDb->PrepareForInsert(L"words_in_line", 6);

        m_spDb->Bind(1, (int64_t)llLineDbId);
        m_spDb->Bind(2, iWord);
        m_spDb->Bind(3, iWordsInLine-iWord-1);
        m_spDb->Bind(4, iLineOffset);
        m_spDb->Bind(5, iSegmentLength);
        m_spDb->Bind(6, sWord);

        m_spDb->InsertRow();
        m_spDb->Finalize();

        llWordDbKey = m_spDb->llGetLastKey();
    }
    catch (CException& e)
    {
        CEString sMsg;
        eHandleDbException(e, sMsg);
        return H_ERROR_GENERAL;
    }

    return H_NO_ERROR;

}       // eSaveWord()

//  CREATE TABLE word_to_wordform(id INTEGER PRIMARY KEY ASC, word_in_line_id INTEGER, wordform_id INTEGER, 
//  FOREIGN KEY(word_in_line_id) REFERENCES words_in_line(id), FOREIGN KEY(wordform_id) REFERENCES wordforms(id));
ET_ReturnCode CAnalytics::eSaveWordParse(int64_t llWordId, int64_t llWordFormId, int64_t& llWordToWordFormId)
{
    if (nullptr == m_spDb)
    {
        ERROR_LOG(L"No database access.");
        return H_ERROR_POINTER;
    }

    try
    {
        m_spDb->PrepareForInsert(L"word_to_wordform", 2);

        m_spDb->Bind(1, (int64_t)llWordId);
        m_spDb->Bind(2, (int64_t)llWordFormId);

        m_spDb->InsertRow();
        m_spDb->Finalize();

        llWordToWordFormId = m_spDb->llGetLastKey();
    }
    catch (CException& e)
    {
        CEString sMsg;
        eHandleDbException(e, sMsg);
        return H_ERROR_GENERAL;
    }

    return H_NO_ERROR;

}       // eSaveWordParse()

ET_WordStressType CAnalytics::eGetStressType(CWordForm& wordForm)
{
    CEString sQuery = L"SELECT is_proclitic, is_enclitic FROM clitics WHERE headword_id = '#HEADWORD_ID#';";
    sQuery = sQuery.sReplace(L"#HEADWORD_ID#", CEString::sToString(wordForm.pLexeme()->llHeadwordId()));
    m_spDb->PrepareForSelect(sQuery);
    bool bProclitic {false}, bEnclitic {false};
    if (m_spDb->bGetRow())
    {
        m_spDb->GetData(0, bProclitic);
        m_spDb->GetData(1, bEnclitic);
    }
    m_spDb->Finalize();

    if (!bProclitic && !bEnclitic)
    {
        return ET_WordStressType::WORD_STRESS_TYPE_AUTONOMOUS;
    }
    if (bProclitic && bEnclitic)
    {
        return ET_WordStressType::WORD_STRESS_TYPE_CLITIC;
    }
    if (bProclitic)
    {
        return ET_WordStressType::WORD_STRESS_TYPE_PROCLITIC;
    }
    if (bEnclitic)
    {
        return ET_WordStressType::WORD_STRESS_TYPE_ENCLITIC;
    }

    return ET_WordStressType::WORD_STRESS_TYPE_UNDEFINED;
}

bool CAnalytics::bArePhoneticallyIdentical(shared_ptr<CWordForm> spWf1, shared_ptr<CWordForm> spWf2)
{
    if (spWf1->sWordForm() != spWf2->sWordForm())
    {
        return false;
    }

    int iStressPos1, iStressPos2 = -1;
    ET_StressType eStressType1, eStressType2 = STRESS_TYPE_UNDEFINED;
    auto eRet1 = spWf1->eGetFirstStressPos(iStressPos1, eStressType1);
    auto eRet2 = spWf2->eGetFirstStressPos(iStressPos2, eStressType2);
    if (H_FALSE == eRet1 && H_FALSE == eRet2)       //  both are unstressed
    {
        return true;
    }

    if (eRet1 != H_NO_ERROR && eRet1 != H_FALSE)
    {
        CEString sMsg(L"Error getting 1st stress position, words: ");
        sMsg += spWf1->sWordForm();
        ERROR_LOG(sMsg);
        return false;
    }

    if (eRet2 != H_NO_ERROR && eRet2 != H_FALSE)
    {
        CEString sMsg(L"Error getting 2nd stress position, words: ");
        sMsg += spWf2->sWordForm();
        ERROR_LOG(sMsg);
        return false;
    }

    if (eStressType1 != eStressType2 || iStressPos1 != iStressPos2)
    {
        return false;
    }

    for (int iCount = 0; iCount < 5; ++iCount)
    {
        eRet1 = spWf1->eGetNextStressPos(iStressPos1, eStressType1);
        eRet2 = spWf2->eGetNextStressPos(iStressPos2, eStressType2);

        //  This is the most common scenario
        if (H_NO_MORE == eRet1 && H_NO_MORE == eRet2)
        {
            return true;
        }

        if ((eRet1 != H_NO_ERROR && eRet1 != H_NO_MORE) || (eRet2 != H_NO_ERROR && eRet2 != H_NO_MORE))
        {
            CEString sMsg(L"Error getting 1st stress position: '");
            sMsg += spWf1->sWordForm() + L"', '";
            sMsg += spWf2->sWordForm() + L"'.";
            ERROR_LOG(sMsg);
            return false;
        }

        if (H_NO_ERROR != eRet1 || H_NO_ERROR != eRet2)
        {
            return false;
        }

        if (eStressType1 != eStressType2 || iStressPos1 != iStressPos2)
        {
            return false;
        }
    }

    ERROR_LOG(L"Error getting next stress position.");

    return false;

}       //  bArePhoneticallyIdentical()

// Recursively create tact groups as a liniked list 
ET_ReturnCode CAnalytics::eAddParsesToTactGroup(CEString& sLine, int64_t llLineId, int iLine, int iPos, shared_ptr<StTactGroup> spCurrentTactGroup)
{
    if (iPos >= m_iWordsInCurrentLine)
    {
        return H_NO_ERROR;
    }

    if (m_mmapLinePosToHomophones.empty())
    {
        CEString sMsg(L"No parsing data for line ");
        sMsg += CEString::sToString(iLine) + L".";
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

    // Loop over all distinct phonetic variants; invariant set is a set of all phonetically
    // identical parses
    auto pairRange = m_mmapLinePosToHomophones.equal_range(iPos);
    if (pairRange.first == pairRange.second)
    {
        CEString sMsg(L"No invariant set for the word at position ");
        sMsg += CEString::sToString(iPos) + L" in line ";
        sMsg += CEString::sToString(iLine) + L".";
        ERROR_LOG(sMsg);
//        return H_ERROR_UNEXPECTED;

//        auto eRc = eAddParsesToTactGroup(llLineId, iLine, iPos + 1, spCurrentTactGroup);
//        if (eRc != H_NO_ERROR)
//        {
//            CEString sMsg(L"Unable to add parse to the tact group, word position is ");
//            sMsg += CEString::sToString(iPos) + L".";
//            ERROR_LOG(sMsg);
//            return H_FALSE;
//        }

        // Treat as an autonomous word without a parse
        shared_ptr<StTactGroup> spNextTactGroup = make_shared<StTactGroup>();
        spNextTactGroup->llLineId = llLineId;
        spNextTactGroup->iFirstWordNum = iPos;
        spNextTactGroup->iMainWordPos = 0;
        spNextTactGroup->iNumOfWords = 1;
        spNextTactGroup->iReverseStressedSyllable = -1;
        spNextTactGroup->iSecondaryStressedSyllable = -1;
        spNextTactGroup->iStressedSyllable = -1;
        spNextTactGroup->sSource = sLine.sGetField(iPos);
//        spNextTactGroup->AddWord(itInvariantSet->second, spWordParse->spWordForm->sWordForm());
        spCurrentTactGroup->m_vecNext.push_back(spNextTactGroup);
        spCurrentTactGroup = spNextTactGroup;
        auto eRc = eAddParsesToTactGroup(sLine, llLineId, iLine, iPos + 1, spCurrentTactGroup);
        if (eRc != H_NO_ERROR)
        {
            CEString sMsg(L"Unable to add parse to the tact group, word position is ");
            sMsg += CEString::sToString(iPos) + L".";
            ERROR_LOG(sMsg);
        }
    }

    for (auto itInvariantSet = pairRange.first; itInvariantSet != pairRange.second; ++itInvariantSet)
    {
        if (itInvariantSet->second.empty())
        {
            CEString sMsg(L"Empty invariant set for the word at position ");
            sMsg += CEString::sToString(iPos) + L" in line ";
            sMsg += CEString::sToString(iLine) + L".";
            ERROR_LOG(sMsg);
//            return H_ERROR_UNEXPECTED;
            auto eRc = eAddParsesToTactGroup(sLine, llLineId, iLine, iPos + 1, spCurrentTactGroup);
            if (eRc != H_NO_ERROR)
            {
                CEString sMsg(L"Unable to add parse to the tact group, word position is ");
                sMsg += CEString::sToString(iPos) + L".";
                ERROR_LOG(sMsg);
                continue;
            }
        }

        if (0 == iPos)
        {
            m_vecTactGroupListHeads.push_back(spCurrentTactGroup);
        }

        auto spWordParse = *(itInvariantSet->second.begin());   // just take the first parse, they are phonetically identical
        switch (spWordParse->eStressType)
        {

        case ET_WordStressType::WORD_STRESS_TYPE_ENCLITIC:  // add to current tg if it is not empty
            if (spCurrentTactGroup->iNumOfWords < 1)
            {
                CEString sMsg(L"No preceding tact group before enclitic, word num ");
                sMsg += CEString::sToString(iLine) + L".";
                ERROR_LOG(sMsg);
                continue;
            }
            spCurrentTactGroup->AddWord(itInvariantSet->second, spWordParse->spWordForm->sWordForm());
            break;

        case ET_WordStressType::WORD_STRESS_TYPE_PROCLITIC:     // may be first in tg or follow another proclitic
            if (spCurrentTactGroup->iNumOfWords < 1)
            {   // First word in TG
                spCurrentTactGroup->AddWord(itInvariantSet->second, spWordParse->spWordForm->sWordForm());
            }
            else
            {
                bool bWordAdded{ false };
                if (!spCurrentTactGroup->m_vecParses.empty())
                {
                    auto spLastParse = *spCurrentTactGroup->m_vecParses.back().begin();  // nevermind, just take the first 
                    if (spLastParse->eStressType == ET_WordStressType::WORD_STRESS_TYPE_PROCLITIC)
                    {
                        spCurrentTactGroup->AddWord(itInvariantSet->second, spWordParse->spWordForm->sWordForm());
                    }
                    bWordAdded = true;
                }
                else
                {
                    spCurrentTactGroup->llLineId = llLineId;
                    spCurrentTactGroup->iFirstWordNum = iPos;
                    spCurrentTactGroup->iMainWordPos = 0;
                    spCurrentTactGroup->iNumOfWords = 1;
                    spCurrentTactGroup->iReverseStressedSyllable = -1;
                    spCurrentTactGroup->iSecondaryStressedSyllable = -1;
                    spCurrentTactGroup->iStressedSyllable = -1;
                    spCurrentTactGroup->sSource = sLine.sGetField(iPos);
                    bWordAdded = true;
                }
                
                if (!bWordAdded)
                {   // Current word is proclitic preceded by anything other than a proclitic: start a new tact group
                    shared_ptr<StTactGroup> spNextTactGroup = make_shared<StTactGroup>();
                    spNextTactGroup->iFirstWordNum = iPos;
                    spNextTactGroup->llLineId = llLineId;
                    spNextTactGroup->AddWord(itInvariantSet->second, spWordParse->spWordForm->sWordForm());
                    spCurrentTactGroup->m_vecNext.push_back(spNextTactGroup);
                    spCurrentTactGroup = spNextTactGroup;
                }
            }
            break;

        case ET_WordStressType::WORD_STRESS_TYPE_CLITIC:
            break;

        case ET_WordStressType::WORD_STRESS_TYPE_AUTONOMOUS:
            if (spCurrentTactGroup->iNumOfWords < 1)
            {   // First word in TG
                spCurrentTactGroup->AddWord(itInvariantSet->second, spWordParse->spWordForm->sWordForm());
            }
            else
            {
                bool bWordAdded {false};
                if (!spCurrentTactGroup->m_vecParses.empty())
                {
                    auto spLastParse = *spCurrentTactGroup->m_vecParses.back().begin();  // nevermind, just take the first one
                    if (spLastParse->eStressType == ET_WordStressType::WORD_STRESS_TYPE_PROCLITIC)
                    {
                        spCurrentTactGroup->AddWord(itInvariantSet->second, spWordParse->spWordForm->sWordForm());
                        bWordAdded = true;
                    }
                }
                else
                {
                    spCurrentTactGroup->llLineId = llLineId;
                    spCurrentTactGroup->iFirstWordNum = iPos;
                    spCurrentTactGroup->iMainWordPos = 0;
                    spCurrentTactGroup->iNumOfWords = 1;
                    spCurrentTactGroup->iReverseStressedSyllable = -1;
                    spCurrentTactGroup->iSecondaryStressedSyllable = -1;
                    spCurrentTactGroup->iStressedSyllable = -1;
                    spCurrentTactGroup->sSource = sLine.sGetField(iPos);
                    bWordAdded = true;
                }

                if (!bWordAdded)
                {   // Current word is autonomous preceded by anything other than a proclitic: start a new tact group
                    shared_ptr<StTactGroup> spNextTactGroup = make_shared<StTactGroup>();
                    spNextTactGroup->llLineId = llLineId;
                    spNextTactGroup->iFirstWordNum = iPos;
                    spNextTactGroup->AddWord(itInvariantSet->second, spWordParse->spWordForm->sWordForm());
                    spCurrentTactGroup->m_vecNext.push_back(spNextTactGroup);
                    spCurrentTactGroup = spNextTactGroup;
                }
            }
            break;

        default:
            CEString sMsg(L"Unknown stress type ");
            sMsg += CEString::sToString(spWordParse->eStressType) + L".";
            ERROR_LOG(sMsg);
            continue;
        
        }       // switch...

        // Next word?
        auto eRc = eAddParsesToTactGroup(sLine, llLineId, iLine, iPos + 1, spCurrentTactGroup);
        if (eRc != H_NO_ERROR)
        {
            CEString sMsg(L"Unable to add parse to the tact group, word position is ");
            sMsg += CEString::sToString(iPos) + L".";
            ERROR_LOG(sMsg);
            continue;
        }

    }           // for (auto itInvariantSet ...)

    return H_NO_ERROR;

}   //  eAddParsesToTactGroup()

// Save tact groups for a line
//  CREATE TABLE tact_group(id INTEGER PRIMARY KEY ASC, line_id INTEGER, first_word_position INTEGER, 
//  num_of_words INTEGER, source TEXT, transcription TEXT, num_of_syllables INTEGER, stressed_syllable INTEGER, 
//  reverse_stressed_syllable INTEGER, SECONDARY_STRESSED_SYLLABLE, FOREIGN KEY(line_id) 
//  REFERENCES lines_in_text(id));
ET_ReturnCode CAnalytics::eSaveTactGroup(shared_ptr<StTactGroup> spTg)
{
    if (nullptr == m_spDb)
    {
        ERROR_LOG(L"No database access.");
        return H_ERROR_POINTER;
    }

    m_spDb->PrepareForInsert(L"tact_group", 11);

    m_spDb->Bind(1, (int64_t)spTg->llLineId);
    m_spDb->Bind(2, spTg->iFirstWordNum);
    m_spDb->Bind(3, spTg->iMainWordPos);
    m_spDb->Bind(4, spTg->iNumOfWords);
    m_spDb->Bind(5, spTg->sSource);
    //            m_spDb->Bind(6, wordParse.WordForm.sGramHash());
    m_spDb->Bind(6, spTg->sTranscription);
    m_spDb->Bind(7, spTg->sPhonemicSequence);
    m_spDb->Bind(8, spTg->iNumOfSyllables);
    m_spDb->Bind(9, spTg->iStressedSyllable);
    m_spDb->Bind(10, spTg->iReverseStressedSyllable);
    m_spDb->Bind(11, spTg->iSecondaryStressedSyllable);

    m_spDb->InsertRow();
    m_spDb->Finalize();

    int64_t llTactGroupId = m_spDb->llGetLastKey();

    int iPosInTactGroup = 0;
    for (auto setParses : spTg->m_vecParses)
    {
        try
        {
            // CREATE TABLE word_to_tact_group(id INTEGER PRIMARY KEY ASC, word_to_wordform_id INTEGER, 
            // tact_group_id INTEGER, position_in_tact_group INTEGER, FOREIGN KEY(word_to_wordform_id) 
            // REFERENCES word_to_word_form(id));

            sqlite3_stmt* pStmt = nullptr;
            m_spDb->uiPrepareForInsert(L"word_to_tact_group", 3, pStmt);
            auto llInsertHandle = (unsigned long long)pStmt;

            for (auto pstWordParse : setParses)
            {
                m_spDb->Bind(1, (int64_t)pstWordParse->llWordToWordFormId, (int64_t)llInsertHandle);
                m_spDb->Bind(2, (int64_t)llTactGroupId, (int64_t)llInsertHandle);
                m_spDb->Bind(3, iPosInTactGroup, (int64_t)llInsertHandle);
                m_spDb->InsertRow((int64_t)llInsertHandle);
            }
            m_spDb->Finalize((int64_t)llInsertHandle);

            ++iPosInTactGroup;

            // CREATE TABLE tact_group_to_gram_hash(id INTEGER PRIMARY KEY ASC, tact_group_id INTEGER, gram_hash TEXT)
/*
            pStmt = nullptr;
            m_spDb->uiPrepareForInsert(L"tact_group_to_gram_hash", 2, pStmt);
            llInsertHandle = (unsigned int64_t)pStmt;
            for (auto& pairParse : spTg->mapWordParses)
            {
                m_spDb->Bind(1, (int64_t)llTactGroupId, (int64_t)llInsertHandle);
                m_spDb->Bind(2, pairParse.second.WordForm.sGramHash(), (int64_t)llInsertHandle);
                m_spDb->InsertRow((int64_t)llInsertHandle);
            }
            m_spDb->Finalize(llInsertHandle);
*/
        }
        catch (CException & e)
        {
            CEString sMsg;
            eHandleDbException(e, sMsg);
            return H_ERROR_DB;
        }
    }

    return H_NO_ERROR;

}       //  eSaveTactGroups()

ET_ReturnCode CAnalytics::eClearTextData(int64_t llTextId)
{
    if (!m_spDb)
    {
        return H_ERROR_DB;
    }

//    ET_ReturnCode eRet = H_NO_ERROR;

    CEString sQuery;
    vector<int64_t> vecLineIds;

    // Get Line Id's
    sQuery = L"SELECT id FROM lines_in_text WHERE text_id = ";
    sQuery += CEString::sToString(llTextId);

    try
    {
        m_spDb->PrepareForSelect(sQuery);
        while (m_spDb->bGetRow())
        {
            int64_t iLineId = -1;
            m_spDb->GetData(0, iLineId);
            vecLineIds.push_back(iLineId);
        }
        m_spDb->Finalize();

        vector<int64_t> vecWordsInLineIds;
        for (auto llLineId : vecLineIds)
        {
            sQuery = L"SELECT id FROM words_in_line WHERE line_id = ";
            sQuery += CEString::sToString(llLineId);
            m_spDb->PrepareForSelect(sQuery);
            while (m_spDb->bGetRow())
            {
                int64_t iWordInLineId = -1;
                m_spDb->GetData(0, iWordInLineId);
                vecWordsInLineIds.push_back(iWordInLineId);
            }
            m_spDb->Finalize();
        }

        vector<int64_t> vecWordToWordFormIds;
        for (auto llWordInLineId : vecWordsInLineIds)
        {
            sQuery = L"SELECT id FROM word_to_wordform WHERE word_in_line_id = ";
            sQuery += CEString::sToString(llWordInLineId);
            m_spDb->PrepareForSelect(sQuery);
            while (m_spDb->bGetRow())
            {
                int64_t iWordToWordFormId = -1;
                m_spDb->GetData(0, iWordToWordFormId);
                vecWordToWordFormIds.push_back(iWordToWordFormId);
            }
            m_spDb->Finalize();
        }

        vector<int64_t> vecTactGroupIds;
        for (auto llLineId : vecLineIds)
        {
            sQuery = L"SELECT id FROM tact_group WHERE line_id = ";
            sQuery += CEString::sToString(llLineId);
            m_spDb->PrepareForSelect(sQuery);
            while (m_spDb->bGetRow())
            {
                int64_t iTactGroupId = -1;
                m_spDb->GetData(0, iTactGroupId);
                vecTactGroupIds.push_back(iTactGroupId);
            }
            m_spDb->Finalize();
        }

        vector<int64_t> vecWordToTactGroupIds;
        for (auto llTactGroupId : vecTactGroupIds)
        {
            sQuery = L"SELECT id FROM word_to_tact_group WHERE tact_group_id = ";
            sQuery += CEString::sToString(llTactGroupId);
            m_spDb->PrepareForSelect(sQuery);
            while (m_spDb->bGetRow())
            {
                int64_t iWordToTactGroupId = -1;
                m_spDb->GetData(0, iWordToTactGroupId);
                vecWordToTactGroupIds.push_back(iWordToTactGroupId);
            }
            m_spDb->Finalize();
        }

        for (auto llWordToTactGroupId : vecWordToTactGroupIds)
        {
            sQuery = L"DELETE FROM word_to_tact_group WHERE id = ";
            sQuery += CEString::sToString(llWordToTactGroupId);
            m_spDb->Delete(sQuery);
        }

        for (auto llTactGroupId : vecTactGroupIds)
        {
            sQuery = L"DELETE FROM tact_group WHERE id = ";
            sQuery += CEString::sToString(llTactGroupId);
            m_spDb->Delete(sQuery);
        }

        for (auto llWordFormId : vecWordToWordFormIds)
        {
            sQuery = L"DELETE FROM word_to_wordform WHERE id = ";
            sQuery += CEString::sToString(llWordFormId);
            m_spDb->Delete(sQuery);
        }

        for (auto llWordInLineId : vecWordsInLineIds)
        {
            sQuery = L"DELETE FROM words_in_line WHERE id = ";
            sQuery += CEString::sToString(llWordInLineId);
            m_spDb->Delete(sQuery);
        }

        for (auto llLineId : vecLineIds)
        {
            sQuery = L"DELETE FROM lines_in_text WHERE id = ";
            sQuery += CEString::sToString(llLineId);
            m_spDb->Delete(sQuery);
        }
    }
    catch (CException& ex)
    {
        CEString sMsg(L"Warning: Unable to read db, exception: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG(sMsg);
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eClearTextData()

//
//  Web interface, manual editing
//

static CEString sLineQuery { 
//                 0         1             2              3         4           5             6               7                 8              9
    L"SELECT tmd.title, lit.id, wil.word_position, lit.source, wf.id, sd.gram_hash, wil.word_text, stress.position, stress.is_primary, stress.is_variant \
      FROM lines_in_text AS lit INNER JOIN words_in_line AS wil ON wil.line_id = lit.id INNER JOIN word_to_wordform AS wtw ON wtw.word_in_line_id = wil.id \
      INNER JOIN wordforms AS wf ON wf.id = wtw.wordform_id INNER JOIN stem_data AS sd ON sd.id = wf.stem_data_id INNER JOIN stress_data AS stress \
      ON stress.form_id = wtw.wordform_id INNER JOIN text as t ON lit.text_id = t.id INNER JOIN text_metadata as tmd ON t.id = tmd.text_id; " };

//                                               0         1           2             3             4             5
static CEString sIrregularFormsQuery{ L"SELECT f.id, f.gram_hash, f.wordform, f.is_alternative, s.position, s.is_primary \
                                        FROM irregular_forms as f INNER JOIN irregular_stress as s ON f.id = s.form_id \
                                        ORDER BY f.id;" };

ET_ReturnCode CAnalytics::eLoadIrregularForms()
{
    if (nullptr == m_spDb)
    {
        ERROR_LOG(L"No database access.");
        return H_ERROR_POINTER;
    }

    int64_t llLastDbId = -1;
    CEString sGramHash;
    CEString sWordSource;
//    bool bIsAlternative {false};
    int iStressPos {-1};
    bool bIsPrimary {true};

    auto spIrregularForm = make_shared<StIrregularWord>();
    bool bMore {true};
    try
    {
        m_spDb->PrepareForSelect(sIrregularFormsQuery);
        while (bMore)
        {
            auto bRc = m_spDb->bGetRow();
            if (!bRc)
            {
                m_spDb->Finalize();
                bMore = false;
                continue;
            }

            int64_t llDbId {-1};
            m_spDb->GetData(0, llDbId);
            if (llLastDbId >= 0 && llLastDbId != llDbId)
            {
                m_mmapWordToIrregForm.insert({make_shared<CEString>(sWordSource), move(spIrregularForm)});
                spIrregularForm.reset();
                spIrregularForm = make_shared<StIrregularWord>();

                llLastDbId = llDbId;
            }
             
            spIrregularForm->llDbId = llLastDbId = llDbId;
            m_spDb->GetData(1, spIrregularForm->sGramHash);
            m_spDb->GetData(2, sWordSource);
            m_spDb->GetData(3, spIrregularForm->bIsAlternative);
            m_spDb->GetData(4, iStressPos);
            m_spDb->GetData(5, bIsPrimary);
            spIrregularForm->mapStress[iStressPos] = bIsPrimary
                ? ET_StressType::STRESS_PRIMARY 
                : ET_StressType::STRESS_SECONDARY;
        }
    }
    catch (CException& e)
    {
        CEString sMsg;
        eHandleDbException(e, sMsg);
        ERROR_LOG(sMsg);
        return H_EXCEPTION;
    }

    return H_NO_ERROR;
}

static CEString sCliticsQuery{ L"SELECT hw.source, c.is_proclitic, c.is_enclitic, FROM clitics AS c \
                                 INNER JOIN headword AS hw ON hw.id=c.headword_id \
                                 ORDER BY hw.source;" };

ET_ReturnCode CAnalytics::eLoadClitics()
{
    if (nullptr == m_spDb)
    {
        ERROR_LOG(L"No database access.");
        return H_ERROR_POINTER;
    }


    auto spIrregularForm = make_shared<StIrregularWord>();
    bool bMore{ true };
    try
    {
        m_spDb->PrepareForSelect(sCliticsQuery);
        while (bMore)
        {
            auto bRc = m_spDb->bGetRow();
            if (!bRc)
            {
                m_spDb->Finalize();
                bMore = false;
                continue;
            }

            CEString sSource;
            bool bIsProclitic{ false };
            bool bIsEnclitic{ false };
            m_spDb->GetData(0, sSource);
            m_spDb->GetData(1, bIsProclitic);
            m_spDb->GetData(2, bIsEnclitic);

            if (bIsProclitic)
            {
                m_setProclitics.insert(sSource);
            }
            if (bIsEnclitic)
            {
                m_setEnclitics.insert(sSource);
            }
        }
    }
    catch (CException& e)
    {
        CEString sMsg;
        eHandleDbException(e, sMsg);
        ERROR_LOG(sMsg);
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eLoadClitics()

ET_ReturnCode CAnalytics::eGetFirstSegment(vector<StWordContext>& vecParses, int64_t llStartAt)
{
    int iSegmentsSkipped = 0;
    try
    {
        if (nullptr == m_spDb)
        {
            ERROR_LOG(L"No database access.");
            return H_ERROR_POINTER;
        }

        m_sCurrentSegment.Erase();
        m_llCurrentSegmentId = llStartAt;

        m_spDb->PrepareForSelect(sLineQuery);
        auto bRc = m_spDb->bGetRow();
        if (!bRc)
        {
            m_spDb->Finalize();
            ERROR_LOG(L"WARNING: no records.");
            return H_NO_MORE;
        }

        while (iSegmentsSkipped < llStartAt)
        {
            auto rc = m_spDb->bGetRow();
            if (!rc)
            {
                m_spDb->Finalize();
                ERROR_LOG(L"WARNING: no records.");
                return H_NO_MORE;
            }
            ++iSegmentsSkipped;
        }

        m_spDb->GetData(1, m_llCurrentSegmentId);
        m_spDb->GetData(3, m_sCurrentSegment);
    }
    catch (CException& e)
    {
        CEString sMsg;
        eHandleDbException(e, sMsg);
        //        sMsg += CEString::sToString(m_spDb->iGetLastError());
        ERROR_LOG(sMsg);
    }

    auto rc = eGetSegment(vecParses);

    return rc;

}   //  eGetFirstSegment()

ET_ReturnCode CAnalytics::eGetNextSegment(vector<StWordContext>& vecParses)
{
    auto rc = eGetSegment(vecParses);
    return rc;

}  //  eGetNextSegment()

ET_ReturnCode CAnalytics::eGetSegment(vector<StWordContext>& vecParses)
{    
    if (nullptr == m_spDb)
    {
        ERROR_LOG(L"No database access.");
        return H_ERROR_POINTER;
    }

    m_mmapWordPosToFormIds.clear();
    m_mmapFormIdToStressPositions.clear();
    m_mapFormIdToGramHashes.clear();
    m_mapWordPosToWord.clear();

    try
    {
        CEString sTitle;
        int64_t llSegmentId {-1};
        CEString sSegment;
        bool bNextSegment {false};
        while (!bNextSegment)
        {
            m_spDb->GetData(0, sTitle);

            m_spDb->GetData(1, llSegmentId);        // lit.id
            if (llSegmentId < m_llCurrentSegmentId)
            {
                ERROR_LOG(L"Illegal segment ID.");
                return H_ERROR_UNEXPECTED;
            }

            if (m_llCurrentSegmentId < 0)
            {
                m_llCurrentSegmentId = llSegmentId;
            }

            if (llSegmentId > m_llCurrentSegmentId)
            {
                // Last word in line
                m_spDb->GetData(3, sSegment);
                m_iCurrentPos = -1;
                bNextSegment = true;    // exit loop
                continue;
            }

            int iPos {-1};
            m_spDb->GetData(2, iPos);            //  word_position
            if (iPos < m_iCurrentPos)
            {
                ERROR_LOG(L"Word numbering error.");
                return H_ERROR_UNEXPECTED;
            }

            if (iPos != m_iCurrentPos)
            {
                m_iCurrentPos = iPos;
            }

            int64_t llWordFormId = -1;
            m_spDb->GetData(4, llWordFormId);
            m_mmapWordPosToFormIds.insert(pair{iPos, llWordFormId});
//            if (llWordFormId != m_llCurrentFormId)
//            {
//                m_llCurrentFormId = llWordFormId;
//                m_mmapWordPosToFormIds.insert(pair{m_iCurrentPos, m_llCurrentFormId});
//            }

            CEString sHash;
            m_spDb->GetData(5, sHash);
            if (m_mapFormIdToGramHashes.find(llWordFormId) != m_mapFormIdToGramHashes.end())
            {
                CEString sMsg(L"Multiple gram hashes for a single wordform ID, word ID = ");
                sMsg += CEString::sToString(llWordFormId);
                ERROR_LOG(sMsg);
            }
            m_mapFormIdToGramHashes[llWordFormId] = sHash;

            CEString sWord;
            m_spDb->GetData(6, sWord);
            m_mapWordPosToWord[iPos] = sWord;

            int iStressSyll{-1};
            m_spDb->GetData(7, iStressSyll);
            bool bIsPrimary;
            m_spDb->GetData(8, bIsPrimary);
            ET_StressType eType = bIsPrimary ? ET_StressType::STRESS_PRIMARY : ET_StressType::STRESS_SECONDARY;
            m_mmapFormIdToStressPositions.insert(pair {llWordFormId, pair{iStressSyll, eType}});

            auto rc = m_spDb->bGetRow();
            if (!rc)
            {
                m_spDb->Finalize();
                ERROR_LOG(L"WARNING: no more records.");
                return H_NO_MORE;
            }       
        }       // while(!bNextSegment)
        //        m_spDb->Finalize();

        eAssembleParsedSegment(vecParses);
        m_llCurrentSegmentId = llSegmentId;
        m_sCurrentSegment = sSegment;
        m_sCurrentTitle = sTitle;
    }
    catch (CException& e)
    {
        CEString sMsg;
        eHandleDbException(e, sMsg);
        ERROR_LOG(L"Error getting line/paragraph context: " + sMsg);
    }

    return H_NO_ERROR;

}       // eGetSegment()

ET_ReturnCode CAnalytics::eAssembleParsedSegment(vector<StWordContext>& vecParses)
{
    vecParses.clear();

    multimap<int, StWordContext> mapWordContexts;        // Pos in segment --> StWordContext

    int iCurrentWordPos {0};

    // TODO --- check if m_mapWordPosToWord is empty

    auto nTokens = m_sCurrentSegment.uiGetNumOfTokens();
    for (int iAt = 0; iAt < (int)nTokens; ++iAt)
    {
        auto stCurrentToken = m_sCurrentSegment.stGetToken(iAt);
        switch (stCurrentToken.eType)
        {
            case ETokenType::ecTokenText:
            {
                // All parses for this word
                auto pairWfIdRange = m_mmapWordPosToFormIds.equal_range(iCurrentWordPos);
                StWordContext stCtx;

                if (pairWfIdRange.first == pairWfIdRange.second)        // no parse
                {
                    const CEString& sWord = CEString::sToLower(m_sCurrentSegment.sGetToken(iAt));
                    auto pairRange = m_mmapWordToIrregForm.equal_range(move(make_shared<CEString>(sWord)));
                    if (pairRange.first == pairRange.second)
                    {
                        stCtx.bIncomplete = true;
                        stCtx.sWord = m_sCurrentSegment.sGetToken(iAt);
                    }
                    else
                    {
                        auto pairWordToIrregPair = get<0>(pairRange);
                        auto spIrregularForm = pairWordToIrregPair->second;
                        stCtx.llIrregularFormId = spIrregularForm->llDbId;
                        stCtx.sGramHash = spIrregularForm->sGramHash;
//                        stCtx.sWord = sWord;
                        stCtx.sWord = m_sCurrentSegment.sGetToken(iAt);

                        for (auto pairStress : spIrregularForm->mapStress)
                        {
                            eAddStressMark(stCtx.sWord, pairStress.first, pairStress.second);
                        }
                    }
//                    stCtx.iSeqNum = iCurrentWordPos;
                }

                for (auto itWfId = pairWfIdRange.first; itWfId != pairWfIdRange.second; ++itWfId)
                {
                    stCtx.bIgnore = false;
                    stCtx.bBreak = false;
//                    stCtx.iSeqNum = iCurrentWordPos;
                    stCtx.llWordFormId = itWfId->second;
//                    stCtx.sWord = m_mapWordPosToWord[iCurrentWordPos];
                    stCtx.sWord = m_sCurrentSegment.sGetToken(iAt);
//                    stCtx.llSegmentId = m_llCurrentSegmentId;
                    auto pairStressRange = m_mmapFormIdToStressPositions.equal_range(stCtx.llWordFormId);
                    for (auto itStress = pairStressRange.first; itStress != pairStressRange.second; ++itStress)
                    {
                        auto pairStress = itStress->second;     // pair: < stress pos, stress type >
                        eAddStressMark(stCtx.sWord, stCtx.sWord.uiGetVowelPos(pairStress.first), pairStress.second);
                    }

                    if (m_mapFormIdToGramHashes.find(stCtx.llWordFormId) == m_mapFormIdToGramHashes.end())
                    {
                        CEString sMsg(L"No gram hash for wordform ID ");
                        sMsg += CEString::sToString(stCtx.llWordFormId);
                        ERROR_LOG(sMsg);
                    }
                    else
                    {
                        stCtx.sGramHash = m_mapFormIdToGramHashes[stCtx.llWordFormId];
                    }
                }
                stCtx.llSegmentId = m_llCurrentSegmentId;
                stCtx.iSeqNum = iCurrentWordPos;
                mapWordContexts.insert(pair{iCurrentWordPos++, stCtx});

                break;
            }
            case ecTokenSpace:
            case ecTokenTab:
            case ecTokenDiacritics:
            {
                break;
            }

            case ecTokenPunctuation:
            case ecTokenEndOfParagraph:
            {
//                StWordContext stCtx;
//                stCtx.iSeqNum = iCurrentWordPos++;

                break;
            }

            case ecTokenMeta:
            case ecTokenRegexMatch:
            default:
            {
                ERROR_LOG(L"Error assembling parsed segment: unknown token type.");
                return H_ERROR_UNEXPECTED;
            }
        }       // switch
    
    }       //  for (int iAt; iAt < nTokens; ++iAt)

    for (auto& pairWordCtx : mapWordContexts)
    {
        vecParses.push_back(pairWordCtx.second);
    }

    StWordContext stBreak;
    stBreak.bBreak = true;
    vecParses.push_back(stBreak);

    m_mmapWordPosToFormIds.clear();
    m_mmapFormIdToStressPositions.clear();
    m_mapFormIdToGramHashes.clear();
    m_mapWordPosToWord.clear();

    return H_NO_ERROR;

}   //  eAssembleParsedSegment()

ET_ReturnCode CAnalytics::eAddStressMark(CEString& sWord, int iPos, ET_StressType eType)
{
    sWord.SetVowels(CEString::g_szRusVowels);

    try
    {
        if (L'ё' == sWord[iPos])
        {
            return H_NO_ERROR;
        }
        if (eType == ET_StressType::STRESS_PRIMARY)
        {
            sWord.sInsert(iPos + 1, CEString::g_chrCombiningAcuteAccent);
        }
        else
        {
            sWord.sInsert(iPos + 1, CEString::g_chrCombiningGraveAccent);
        }
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        ERROR_LOG(sMsg);
        return H_EXCEPTION;
    }

    return H_NO_ERROR;
}
