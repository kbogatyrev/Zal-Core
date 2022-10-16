#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <set>
#include <vector>

#include "Logging.h"
#include "SqliteWrapper.h"
#include "WordForm.h"
#include "Parser.h"
#include "Transcriber.h"
#include "Analytics.h"

using namespace Hlib;

CAnalytics::CAnalytics() : m_pDb(nullptr), m_llTextDbId(-1) 
{
    eInit();
}

CAnalytics::CAnalytics(shared_ptr<CSqlite> pDb, shared_ptr<CParser> pParser) : m_pDb(pDb), m_pParser(pParser), m_llTextDbId(-1)
{
    eInit();
}

CAnalytics::~CAnalytics()
{}

ET_ReturnCode CAnalytics::eInit()
{
    m_spTranscriber = make_unique<CTranscriber>(m_pDb);
    if (nullptr == m_spTranscriber)
    {
        return H_ERROR_POINTER;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CAnalytics::eParseText(const CEString& sTextName, const CEString& sMetadata, const CEString& sText, long long& llParsedTextId, bool bIsProse)
{
    ET_ReturnCode eRet = H_NO_ERROR;

    m_sTextName = sTextName;
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

    if (NULL == m_pDb)
    {
        ERROR_LOG(L"No database access.");
        return H_ERROR_POINTER;
    }

    if (NULL == m_pParser)
    {
        ERROR_LOG(L"No parser object.");
        return H_ERROR_POINTER;
    }

    m_pDb->BeginTransaction();

    eRet = eRegisterText();
    if (eRet != H_NO_ERROR)
    {
        ERROR_LOG(L"Unable to register text");
        return eRet;
    }

    //
    // Parse text word by word
    //
    int iNLines = const_cast<CEString&>(sText).uiNFields();
    if (iNLines < 1)
    {
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

        long long llLineDbId = -1;
        eRet = eSaveLine(iLine, iTextOffset, sLine.uiLength(), sLine.uiNFields(), sLine, llLineDbId);
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
        }       // for (int iField ...)
    }       //  for (int iLine = 0 ...)

    m_pTactGroupListHead = make_shared<StTactGroup>();

    m_pDb->CommitTransaction();

    m_pParser->ClearResults();

    llParsedTextId = m_llTextDbId;

    return H_NO_ERROR;

}       // eParseText()

ET_ReturnCode CAnalytics::eParseMetadata(const CEString& sConstMetadata)
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
            CEString sMsg(L"Bad key/value pair: ");
            ERROR_LOG(sMsg + sKeyValPair);
            continue;
        }

        auto sKey = sKeyValPair.sGetField(0);
        sKey.Trim(L" ");
        auto sValue = sKeyValPair.sGetField(1);
        sValue.Trim(L" ");

        m_vecMetadataKeyValPairs.push_back(make_pair(sKey, sValue));
    }

    return H_NO_ERROR;
}

//  CREATE TABLE text(id INTEGER PRIMARY KEY ASC, name TEXT, metadata TEXT, contents TEXT);
ET_ReturnCode CAnalytics::eRegisterText()
{
    ET_ReturnCode eRet = H_NO_ERROR;

    if (m_sTextName.bIsEmpty() || m_sTextMetaData.bIsEmpty())
    {
        ERROR_LOG(L"No text or text descriptor.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sQuery = L"SELECT id FROM text WHERE name = '#NAME#';";
    sQuery = sQuery.sReplace(L"#NAME#", m_sTextName);

    vector<long long> vecTextIds;
    try
    {
        m_pDb->PrepareForSelect(sQuery);
        while (m_pDb->bGetRow())
        {
            int64_t llTextId = -1;
            m_pDb->GetData(0, llTextId);
            if (llTextId < 0)
            {
                ERROR_LOG(L"Illegal text id.");
                continue;
            }
            vecTextIds.push_back(llTextId);
        }
        m_pDb->Finalize();

        if (vecTextIds.size() > 1)
        {
            ERROR_LOG(L"Warning: multiple DB entries for the same text.");
        }

        for (long long llId : vecTextIds)
        {
            eRet = eClearTextData(llId);
            sQuery = L"DELETE FROM text_metadata WHERE text_id = ";
            sQuery += CEString::sToString(llId);
            m_pDb->Delete(sQuery);
            sQuery = L"DELETE FROM text WHERE id = ";
            sQuery += CEString::sToString(llId);
            m_pDb->Delete(sQuery);
        }
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            m_pDb->GetLastError(sError);
            sMsg += CEString(L", error: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

//        sMsg += CEString::sToString(m_pDb->iGetLastError());
        ERROR_LOG(sMsg);
    }

    eRet = eParseMetadata(m_sTextMetaData);
    if (eRet != H_NO_ERROR)
    {
        ERROR_LOG(L"Unable to parse text metadata.");
        return eRet;
    }

    //
    // Create DB entry for the text
    //
    try
    {
        m_pDb->PrepareForInsert(L"text", 2);

        m_pDb->Bind(1, m_sTextName);
        m_pDb->Bind(2, m_sText);

        m_pDb->InsertRow();
        m_pDb->Finalize();

        m_llTextDbId = m_pDb->llGetLastKey();
    }
    catch (CException& exc)
    {
        m_llTextDbId = -1;

        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            m_pDb->GetLastError(sError);
            sMsg += CEString(L", error: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(m_pDb->iGetLastError());
        ERROR_LOG(sMsg);

        return H_ERROR_DB;
    }

    //  CREATE TABLE text_metadata(id INTEGER PRIMARY KEY ASC, text_id INTEGER, category TEXT, content text, FOREIGN KEY(text_id) REFERENCES text(id))

    for (auto keyValPair : m_vecMetadataKeyValPairs)
    {
        //
        // Create DB entry for each key/value pair
        //
        try
        {
            m_pDb->PrepareForInsert(L"text_metadata", 3);

            m_pDb->Bind(1, (int64_t)m_llTextDbId);
            m_pDb->Bind(2, keyValPair.first);
            m_pDb->Bind(3, keyValPair.second);

            m_pDb->InsertRow();
            m_pDb->Finalize();

//            m_llTextDbId = m_pDb->llGetLastKey();
        }
        catch (CException & exc)
        {
            CEString sMsg(exc.szGetDescription());
            CEString sError;
            try
            {
                m_pDb->GetLastError(sError);
                sMsg += CEString(L", error: ");
                sMsg += sError;
            }
            catch (...)
            {
                sMsg = L"Apparent DB error ";
            }

            sMsg += CEString::sToString(m_pDb->iGetLastError());
            ERROR_LOG(sMsg);

            return H_ERROR_DB;
        }
    }

    m_vecMetadataKeyValPairs.clear();

    return eRet;

}       //  eRegisterText()

ET_ReturnCode CAnalytics::eParseWord(const CEString& sWord, 
                                     const CEString& sLine, 
                                     int iNumInLine, 
                                     int iWordsInLine, 
                                     long long llLineDbKey, 
                                     vector<shared_ptr<StWordParse>>& vecParses)
{
    if (nullptr == m_pParser)
    {
        ERROR_LOG(L"Parser not available.");
        return H_ERROR_POINTER;
    }

    ET_ReturnCode eRet = H_NO_ERROR;

    int iOffset = const_cast<CEString&>(sLine).uiGetFieldOffset(iNumInLine);
    long long llWordInLineDbKey = -1;
    eRet = eSaveWord(llLineDbKey, iNumInLine, iWordsInLine, iOffset, sWord.uiLength(), sWord, llWordInLineDbKey);   // words_in_line
    if (eRet != H_NO_ERROR)
    {
        CEString sMsg(L"Unable to save a word '");
        sMsg += sWord + L"'";
        ERROR_LOG(L"Unable to save a word.");
        return eRet;
    }

    eRet = m_pParser->eParseWord(sWord);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    CWordForm * pWf = nullptr;
    eRet = m_pParser->eGetFirstWordForm(pWf);
    while (H_NO_ERROR == eRet)
    {
        long long llWordToWordFormId = -1;
        eRet = eSaveWordParse(llWordInLineDbKey, pWf->llDbKey(), llWordToWordFormId);   //  word_to_wordform
        if (eRet != H_NO_ERROR)
        {
            CEString sMsg(L"Unable to save a word parse, word: ");
            sMsg += pWf->sWordForm();
            ERROR_LOG(sMsg);
            continue;
        }

        StWordParse stParse;
        stParse.iPosInLine = iNumInLine;
        stParse.iLineOffset = iOffset;
        stParse.iLength = sWord.uiLength();
        stParse.eStressType = eGetStressType(*pWf);
        stParse.llLineDbId = llLineDbKey;
        stParse.llWordInLineDbId = llWordInLineDbKey;
        stParse.llWordToWordFormId = llWordToWordFormId;
//        stParse.iPosInTactGroup = 0;
        stParse.WordForm = *pWf;
//        m_mmapLinePosToParses.insert(make_pair(iNumInLine, stParse));

        vecParses.push_back(make_shared<StWordParse>(stParse));

        eRet = m_pParser->eGetNextWordForm(pWf);
    
    }       //  while...

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
            if (bArePhoneticallyIdentical(pstParse->WordForm, pstRhs->WordForm))
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

    vector<int> vecPrimary;
    vector<int> vecSecondary;

    for (int iWord = pTg->iFirstWordNum, iCount = 0; iCount < pTg->iNumOfWords; ++iWord, ++iCount)
    {
        if (pTg->iNumOfWords < pTg->m_vecParses.size())
        {
            CEString sMsg(L"Size of the parse array does not match number of words in the tact group.");
            return H_ERROR_UNEXPECTED;
        }

        auto& setInvariants = pTg->m_vecParses[iWord];
        auto pParse = *setInvariants.begin();      // we can use any of the words
        if (WORD_STRESS_TYPE_AUTONOMOUS == pParse->eStressType)
        {
            int iPos = -1;
            ET_StressType eType = STRESS_TYPE_UNDEFINED;
            eRet = const_cast<CWordForm&>(pParse->WordForm).eGetFirstStressPos(iPos, eType);
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
                eRet = const_cast<CWordForm&>(pParse->WordForm).eGetNextStressPos(iPos, eType);
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
                CEString sWord = pParse->WordForm.sWordForm();

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
                    auto& setParses = pTg->m_vecParses[iWord];
                    auto pWpProclitic = *setParses.begin();      // we can use any of the words
                    pTg->iStressedSyllable += pWpProclitic->WordForm.sWordForm().uiNSyllables();
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
            CEString sWord = pParse->WordForm.sWordForm();

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
                pTg->iStressedSyllable += pWpProclitic->WordForm.sWordForm().uiNSyllables();
            }
            for (auto iProclitic = pTg->iFirstWordNum; iProclitic < iWord; ++iProclitic)
            {
                auto& setParses = pTg->m_vecParses[iWord];
                auto pWpProclitic = *setParses.begin();      // we can use any of the words
                pTg->iSecondaryStressedSyllable += pWpProclitic->WordForm.sWordForm().uiNSyllables();
            }

            eRet = H_NO_ERROR;
        }
        else
        {
            eRet = H_FALSE;
        }

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
        ERROR_LOG(L"m_spTranscriber is null.");
        return H_ERROR_POINTER;
    }

    auto eRet = m_spTranscriber->eTranscribeTactGroup(pTg);

    return eRet;
}

//  CREATE TABLE lines_in_text(id INTEGER PRIMARY KEY ASC, text_id INTEGER, line_number INTEGER, text_offset INTEGER, number_of_words INTEGER, 
//  source TEXT, FOREIGN KEY(text_id) REFERENCES text(id));
ET_ReturnCode CAnalytics::eSaveLine(int iLineNum, int iTextOffset, int iLength, int iNumOfWords, const CEString& sText, long long& llDbKey)
{
    try
    {
        if (NULL == m_pDb)
        {
            ERROR_LOG(L"No database access.");
            return H_ERROR_POINTER;
        }

        m_pDb->PrepareForInsert(L"lines_in_text", 6);

        m_pDb->Bind(1, (int64_t)m_llTextDbId);
        m_pDb->Bind(2, iLineNum);
        m_pDb->Bind(3, iTextOffset);
        m_pDb->Bind(4, iLength);
        m_pDb->Bind(5, iNumOfWords);
        m_pDb->Bind(6, sText);

        m_pDb->InsertRow();
        m_pDb->Finalize();

        llDbKey = m_pDb->llGetLastKey();

    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            m_pDb->GetLastError(sError);
            sMsg += CEString(L", error: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(m_pDb->iGetLastError());
        ERROR_LOG(sMsg);

        return H_ERROR_GENERAL;
    }

    return H_NO_ERROR;
}

//  CREATE TABLE words_in_line(id INTEGER PRIMARY KEY ASC, line_id INTEGER, word_position INTEGER, reverse_word_position INTEGER, line_offset INTEGER, word_length INTEGER, 
//  word_text TEXT, FOREIGN KEY(line_id) REFERENCES lines_in_text(id));
ET_ReturnCode CAnalytics::eSaveWord(long long llLineDbId, int iWord, int iWordsInLine, int iLineOffset, int iSegmentLength, const CEString& sWord, long long& llWordDbKey)
{
    try
    {
        if (NULL == m_pDb)
        {
            ERROR_LOG(L"No database access.");
            return H_ERROR_POINTER;
        }

        if (m_sTextName.bIsEmpty() || m_sTextMetaData.bIsEmpty())
        {
            ERROR_LOG(L"No text or text descriptor.");
            return H_ERROR_UNEXPECTED;
        }

        m_pDb->PrepareForInsert(L"words_in_line", 6);

        m_pDb->Bind(1, (int64_t)llLineDbId);
        m_pDb->Bind(2, iWord);
        m_pDb->Bind(3, iWordsInLine-iWord-1);
        m_pDb->Bind(4, iLineOffset);
        m_pDb->Bind(5, iSegmentLength);
        m_pDb->Bind(6, sWord);

        m_pDb->InsertRow();
        m_pDb->Finalize();

        llWordDbKey = m_pDb->llGetLastKey();
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            m_pDb->GetLastError(sError);
            sMsg += CEString(L", error: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(m_pDb->iGetLastError());
        ERROR_LOG(sMsg);

        return H_ERROR_GENERAL;
    }

    return H_NO_ERROR;

}       // eSaveWord()

//  CREATE TABLE word_to_wordform(id INTEGER PRIMARY KEY ASC, word_in_line_id INTEGER, wordform_id INTEGER, 
//  FOREIGN KEY(word_in_line_id) REFERENCES words_in_line(id), FOREIGN KEY(wordform_id) REFERENCES wordforms(id));
ET_ReturnCode CAnalytics::eSaveWordParse(long long llWordId, long long llWordFormId, long long& llWordToWordFormId)
{
    if (NULL == m_pDb)
    {
        ERROR_LOG(L"No database access.");
        return H_ERROR_POINTER;
    }

    try
    {
        m_pDb->PrepareForInsert(L"word_to_wordform", 2);

        m_pDb->Bind(1, (int64_t)llWordId);
        m_pDb->Bind(2, (int64_t)llWordFormId);

        m_pDb->InsertRow();
        m_pDb->Finalize();

        llWordToWordFormId = m_pDb->llGetLastKey();
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            m_pDb->GetLastError(sError);
            sMsg += CEString(L", error: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(m_pDb->iGetLastError());
        ERROR_LOG(sMsg);

        return H_ERROR_GENERAL;
    }

    return H_NO_ERROR;

}       // eSaveWordParse()

ET_WordStressType CAnalytics::eGetStressType(CWordForm& wordForm)
{
    CEString sQuery = L"SELECT is_proclitic, is_enclitic FROM clitics WHERE headword_id = '#HEADWORD_ID#';";
    sQuery = sQuery.sReplace(L"#HEADWORD_ID#", CEString::sToString(wordForm.pLexeme()->llHeadwordId()));
    m_pDb->PrepareForSelect(sQuery);
    bool bProclitic {false}, bEnclitic {false};
    if (m_pDb->bGetRow())
    {
        m_pDb->GetData(0, bProclitic);
        m_pDb->GetData(0, bEnclitic);
    }
    m_pDb->Finalize();

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

bool CAnalytics::bArePhoneticallyIdentical(CWordForm& wf1, CWordForm& wf2)
{
    if (wf1.sWordForm() != wf2.sWordForm())
    {
        return false;
    }

    int iStressPos1, iStressPos2 = -1;
    ET_StressType eStressType1, eStressType2 = STRESS_TYPE_UNDEFINED;
    auto eRet1 = wf1.eGetFirstStressPos(iStressPos1, eStressType1);
    auto eRet2 = wf2.eGetFirstStressPos(iStressPos2, eStressType2);
    if (H_FALSE == eRet1 && H_FALSE == eRet2)       //  both are unstressed
    {
        return true;
    }

    if (eRet1 != H_NO_ERROR && eRet1 != H_FALSE)
    {
        CEString sMsg(L"Error getting 1st stress position, words: ");
        sMsg += wf1.sWordForm();
        ERROR_LOG(sMsg);
        return false;
    }

    if (eRet2 != H_NO_ERROR && eRet2 != H_FALSE)
    {
        CEString sMsg(L"Error getting 2nd stress position, words: ");
        sMsg += wf2.sWordForm();
        ERROR_LOG(sMsg);
        return false;
    }

    if (eStressType1 != eStressType2 || iStressPos1 != iStressPos2)
    {
        return false;
    }

    for (int iCount = 0; iCount < 5; ++iCount)
    {
        eRet1 = wf1.eGetNextStressPos(iStressPos1, eStressType1);
        eRet2 = wf2.eGetNextStressPos(iStressPos2, eStressType2);

        //  This is the most common scenario
        if (H_NO_MORE == eRet1 && H_NO_MORE == eRet2)
        {
            return true;
        }

        if ((eRet1 != H_NO_ERROR && eRet1 != H_NO_MORE) || (eRet2 != H_NO_ERROR && eRet2 != H_NO_MORE))
        {
            CEString sMsg(L"Error getting 1st stress position: '");
            sMsg += wf1.sWordForm() + L"', '";
            sMsg += wf2.sWordForm() + L"'.";
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

ET_ReturnCode CAnalytics::eAddParsesToTactGroup(shared_ptr<StTactGroup> pCurrentTactGroup, int iWordPos)
{
    if (iWordPos >= m_iWordsInCurrentLine)
    {
        return H_NO_ERROR;
    }

    if (nullptr == pCurrentTactGroup)
    {
        CEString sMsg(L"Null tact group, line pos. = ");
        sMsg += CEString::sToString(iWordPos) + L".";
        ERROR_LOG(sMsg);
        return H_ERROR_POINTER;
    }

    if (pCurrentTactGroup->m_vecParses.size() != pCurrentTactGroup->iNumOfWords)
    {
        CEString sMsg(L"Unexpected number of parses in tact group, word num ");
        sMsg += CEString::sToString(iWordPos) + L".";
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

    auto pairRange = m_mmapLinePosToHomophones.equal_range(iWordPos);
    if (pairRange.first == pairRange.second)
    {
        CEString sMsg(L"Empty variant vector for the word at position ");
        sMsg += CEString::sToString(iWordPos) + L".";
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

    // Loop over all distinct phonetic variants; invariant set is a set of all phonetically
    // identical parses
    for (auto itInvariantSet = pairRange.first; itInvariantSet != pairRange.second; ++itInvariantSet)
    {
        if (itInvariantSet->second.empty())
        {
            CEString sMsg(L"Empty invariant set for the word at position ");
            sMsg += CEString::sToString(iWordPos) + L".";
            ERROR_LOG(sMsg);
            return H_ERROR_UNEXPECTED;
        }

        auto pNextWordParse = *(itInvariantSet->second.begin());   // just take the first parse, they are phonetically identical

        if (ET_WordStressType::WORD_STRESS_TYPE_ENCLITIC == pNextWordParse->eStressType)
        {   //  Next word is enclitic: add to current tact group
            if (pCurrentTactGroup->iNumOfWords < 1)
            {
                CEString sMsg(L"No preceding tact group before enclitic, word num ");
                sMsg += CEString::sToString(iWordPos) + L".";
                ERROR_LOG(sMsg);
                continue;
            }
            pCurrentTactGroup->iNumOfWords++;
            pCurrentTactGroup->m_vecParses.push_back(itInvariantSet->second);
            pCurrentTactGroup->sSource += L" " + pNextWordParse->WordForm.sWordForm();
            auto eRc = eAddParsesToTactGroup(pCurrentTactGroup, iWordPos+1);
            // TODO handle return code
        }
        else if (ET_WordStressType::WORD_STRESS_TYPE_PROCLITIC == pNextWordParse->eStressType)
        {
            if (pCurrentTactGroup->iNumOfWords > 0)
            {   // Next word is proclitic preceded by a proclitic: add to current tact group
                auto& setParses = *pCurrentTactGroup->m_vecParses.begin();
                if ((*setParses.begin())->eStressType == ET_WordStressType::WORD_STRESS_TYPE_PROCLITIC)
                {
                    pCurrentTactGroup->iNumOfWords++;
                    pCurrentTactGroup->m_vecParses.push_back(itInvariantSet->second);
                    pCurrentTactGroup->sSource += L" " + pNextWordParse->WordForm.sWordForm();
                    auto eRc = eAddParsesToTactGroup(pCurrentTactGroup, iWordPos+1);
                }
            }
            else
            {   // Next word is proclitic preceded by anything other than a proclitic: start a new tact group
                shared_ptr<StTactGroup> pNextTactGroup = make_shared<StTactGroup>();
                pNextTactGroup->iFirstWordNum = iWordPos;
                pNextTactGroup->iNumOfWords = 1;
                pNextTactGroup->llLineId = pNextWordParse->llLineDbId;
                pNextTactGroup->m_vecParses.push_back(itInvariantSet->second);
                pCurrentTactGroup->sSource += pNextWordParse->WordForm.sWordForm();
                auto& setParses = *pCurrentTactGroup->m_vecParses.begin();
                if (ET_WordStressType::WORD_STRESS_TYPE_AUTONOMOUS == (*setParses.begin())->eStressType)
                {
                    pNextTactGroup->iMainWordPos = iWordPos;
                }
                pCurrentTactGroup->m_vecNext.push_back(pNextTactGroup);
                auto eRc = eAddParsesToTactGroup(pNextTactGroup, iWordPos+1);
            }
        }
        else if (ET_WordStressType::WORD_STRESS_TYPE_CLITIC == pNextWordParse->eStressType)
        {   // Next word is proclitic or enclitic: treat as enclitic if the preceding word is full-stressed.
            auto& setParses = *pCurrentTactGroup->m_vecParses.begin();
            if ((*setParses.begin())->eStressType == ET_WordStressType::WORD_STRESS_TYPE_AUTONOMOUS)
            {
                pCurrentTactGroup->iNumOfWords++;
                pCurrentTactGroup->m_vecParses.push_back(itInvariantSet->second);
                pCurrentTactGroup->sSource += L" " + pNextWordParse->WordForm.sWordForm();
                auto eRc = eAddParsesToTactGroup(pCurrentTactGroup, iWordPos+1);
            }
            else
            {
                shared_ptr<StTactGroup> pNextTactGroup = make_shared<StTactGroup>();
                pNextTactGroup->iFirstWordNum = iWordPos;
                pNextTactGroup->iNumOfWords = 1;
                pNextTactGroup->llLineId = pNextWordParse->llLineDbId;
                pNextTactGroup->m_vecParses.push_back(itInvariantSet->second);
                pCurrentTactGroup->sSource += pNextWordParse->WordForm.sWordForm();
                auto& setParses = *pCurrentTactGroup->m_vecParses.begin();
                pCurrentTactGroup->m_vecNext.push_back(pNextTactGroup);
                auto eRc = eAddParsesToTactGroup(pNextTactGroup, iWordPos + 1);
            }
        }
        else if (ET_WordStressType::WORD_STRESS_TYPE_AUTONOMOUS == pNextWordParse->eStressType)
        {   // Next word is full-stressed: if the preceding word is proclitic add to the current tact group,
            // otherwise, create a new tact group
            auto& setParses = *pCurrentTactGroup->m_vecParses.begin();
            if ((*setParses.begin())->eStressType == ET_WordStressType::WORD_STRESS_TYPE_PROCLITIC)
            {
                pCurrentTactGroup->iNumOfWords++;
                pCurrentTactGroup->m_vecParses.push_back(itInvariantSet->second);
                pCurrentTactGroup->sSource += L" " + pNextWordParse->WordForm.sWordForm();
                auto eRc = eAddParsesToTactGroup(pCurrentTactGroup, iWordPos+1);
            }
            else
            {
                shared_ptr<StTactGroup> pNextTactGroup = make_shared<StTactGroup>();
                pNextTactGroup->iFirstWordNum = iWordPos;
                pNextTactGroup->iNumOfWords = 1;
                pNextTactGroup->llLineId = pNextWordParse->llLineDbId;
                pNextTactGroup->m_vecParses.push_back(itInvariantSet->second);
                pCurrentTactGroup->sSource += pNextWordParse->WordForm.sWordForm();
                auto& setParses = *pCurrentTactGroup->m_vecParses.begin();
                pCurrentTactGroup->m_vecNext.push_back(pNextTactGroup);
                auto eRc = eAddParsesToTactGroup(pNextTactGroup, iWordPos+1);
            }
        }
    }           // for (auto itInvariantSet ...)

    return H_NO_ERROR;
}


// Save tact groups for a line
//  CREATE TABLE tact_group(id INTEGER PRIMARY KEY ASC, line_id INTEGER, first_word_position INTEGER, 
//  num_of_words INTEGER, source TEXT, transcription TEXT, num_of_syllables INTEGER, stressed_syllable INTEGER, 
//  reverse_stressed_syllable INTEGER, SECONDARY_STRESSED_SYLLABLE, FOREIGN KEY(line_id) 
//  REFERENCES lines_in_text(id));
ET_ReturnCode CAnalytics::eSaveTactGroup(shared_ptr<StTactGroup> spTg)
{
    if (NULL == m_pDb)
    {
        ERROR_LOG(L"No database access.");
        return H_ERROR_POINTER;
    }

    m_pDb->PrepareForInsert(L"tact_group", 10);

    m_pDb->Bind(1, (int64_t)spTg->llLineId);
    m_pDb->Bind(2, spTg->iFirstWordNum);
    m_pDb->Bind(3, spTg->iMainWordPos);
    m_pDb->Bind(4, spTg->iNumOfWords);
    m_pDb->Bind(5, spTg->sSource);
    //            m_pDb->Bind(6, wordParse.WordForm.sGramHash());
    m_pDb->Bind(6, spTg->sTranscription);
    m_pDb->Bind(7, spTg->iNumOfSyllables);
    m_pDb->Bind(8, spTg->iStressedSyllable);
    m_pDb->Bind(9, spTg->iReverseStressedSyllable);
    m_pDb->Bind(10, spTg->iSecondaryStressedSyllable);

    m_pDb->InsertRow();
    m_pDb->Finalize();

    long long llTactGroupId = m_pDb->llGetLastKey();

    int iPosInTactGroup = 0;
    for (auto setParses : spTg->m_vecParses)
    {
        try
        {
            // CREATE TABLE word_to_tact_group(id INTEGER PRIMARY KEY ASC, word_to_wordform_id INTEGER, 
            // tact_group_id INTEGER, position_in_tact_group INTEGER, FOREIGN KEY(word_to_wordform_id) 
            // REFERENCES word_to_word_form(id));

            sqlite3_stmt* pStmt = nullptr;
            m_pDb->uiPrepareForInsert(L"word_to_tact_group", 3, pStmt);
            auto llInsertHandle = (unsigned long long)pStmt;

            for (auto pstWordParse : setParses)
            {
                m_pDb->Bind(1, (int64_t)pstWordParse->llWordToWordFormId, (int64_t)llInsertHandle);
                m_pDb->Bind(2, (int64_t)llTactGroupId, (int64_t)llInsertHandle);
                m_pDb->Bind(3, iPosInTactGroup, (int64_t)llInsertHandle);
                m_pDb->InsertRow((int64_t)llInsertHandle);
            }
            m_pDb->Finalize((int64_t)llInsertHandle);

            ++iPosInTactGroup;

            // CREATE TABLE tact_group_to_gram_hash(id INTEGER PRIMARY KEY ASC, tact_group_id INTEGER, gram_hash TEXT)
/*
            pStmt = nullptr;
            m_pDb->uiPrepareForInsert(L"tact_group_to_gram_hash", 2, pStmt);
            llInsertHandle = (unsigned long long)pStmt;
            for (auto& pairParse : spTg->mapWordParses)
            {
                m_pDb->Bind(1, (int64_t)llTactGroupId, (int64_t)llInsertHandle);
                m_pDb->Bind(2, pairParse.second.WordForm.sGramHash(), (int64_t)llInsertHandle);
                m_pDb->InsertRow((int64_t)llInsertHandle);
            }
            m_pDb->Finalize(llInsertHandle);
*/
        }
        catch (CException & exc)
        {
            CEString sMsg(exc.szGetDescription());
            CEString sError;
            try
            {
                m_pDb->GetLastError(sError);
                sMsg += CEString(L", error: ");
                sMsg += sError;
            }
            catch (...)
            {
                sMsg = L"Apparent DB error ";
            }

            sMsg += CEString::sToString(m_pDb->iGetLastError());
            ERROR_LOG(sMsg);

            return H_ERROR_DB;
        }
    }

    return H_NO_ERROR;

}       //  eSaveTactGroups()

ET_ReturnCode CAnalytics::eClearTextData(long long llTextId)
{
    if (!m_pDb)
    {
        return H_ERROR_DB;
    }

//    ET_ReturnCode eRet = H_NO_ERROR;

    CEString sQuery;
    vector<long long> vecLineIds;

    // Get Line Id's
    sQuery = L"SELECT id FROM lines_in_text WHERE text_id = ";
    sQuery += CEString::sToString(llTextId);

    try
    {
        m_pDb->PrepareForSelect(sQuery);
        while (m_pDb->bGetRow())
        {
            int64_t iLineId = -1;
            m_pDb->GetData(0, iLineId);
            vecLineIds.push_back(iLineId);
        }
        m_pDb->Finalize();

        vector<long long> vecWordsInLineIds;
        for (auto llLineId : vecLineIds)
        {
            sQuery = L"SELECT id FROM words_in_line WHERE line_id = ";
            sQuery += CEString::sToString(llLineId);
            m_pDb->PrepareForSelect(sQuery);
            while (m_pDb->bGetRow())
            {
                int64_t iWordInLineId = -1;
                m_pDb->GetData(0, iWordInLineId);
                vecWordsInLineIds.push_back(iWordInLineId);
            }
            m_pDb->Finalize();
        }

        vector<long long> vecWordToWordFormIds;
        for (auto llWordInLineId : vecWordsInLineIds)
        {
            sQuery = L"SELECT id FROM word_to_wordform WHERE word_in_line_id = ";
            sQuery += CEString::sToString(llWordInLineId);
            m_pDb->PrepareForSelect(sQuery);
            while (m_pDb->bGetRow())
            {
                int64_t iWordToWordFormId = -1;
                m_pDb->GetData(0, iWordToWordFormId);
                vecWordToWordFormIds.push_back(iWordToWordFormId);
            }
            m_pDb->Finalize();
        }

        vector<long long> vecTactGroupIds;
        for (auto llLineId : vecLineIds)
        {
            sQuery = L"SELECT id FROM tact_group WHERE line_id = ";
            sQuery += CEString::sToString(llLineId);
            m_pDb->PrepareForSelect(sQuery);
            while (m_pDb->bGetRow())
            {
                int64_t iTactGroupId = -1;
                m_pDb->GetData(0, iTactGroupId);
                vecTactGroupIds.push_back(iTactGroupId);
            }
            m_pDb->Finalize();
        }

        vector<long long> vecWordToTactGroupIds;
        for (auto llTactGroupId : vecTactGroupIds)
        {
            sQuery = L"SELECT id FROM word_to_tact_group WHERE tact_group_id = ";
            sQuery += CEString::sToString(llTactGroupId);
            m_pDb->PrepareForSelect(sQuery);
            while (m_pDb->bGetRow())
            {
                int64_t iWordToTactGroupId = -1;
                m_pDb->GetData(0, iWordToTactGroupId);
                vecWordToTactGroupIds.push_back(iWordToTactGroupId);
            }
            m_pDb->Finalize();
        }

        for (auto llWordToTactGroupId : vecWordToTactGroupIds)
        {
            sQuery = L"DELETE FROM word_to_tact_group WHERE id = ";
            sQuery += CEString::sToString(llWordToTactGroupId);
            m_pDb->Delete(sQuery);
        }

        for (auto llTactGroupId : vecTactGroupIds)
        {
            sQuery = L"DELETE FROM tact_group WHERE id = ";
            sQuery += CEString::sToString(llTactGroupId);
            m_pDb->Delete(sQuery);
        }

        for (auto llWordFormId : vecWordToWordFormIds)
        {
            sQuery = L"DELETE FROM word_to_wordform WHERE id = ";
            sQuery += CEString::sToString(llWordFormId);
            m_pDb->Delete(sQuery);
        }

        for (auto llWordInLineId : vecWordsInLineIds)
        {
            sQuery = L"DELETE FROM words_in_line WHERE id = ";
            sQuery += CEString::sToString(llWordInLineId);
            m_pDb->Delete(sQuery);
        }

        for (auto llLineId : vecLineIds)
        {
            sQuery = L"DELETE FROM lines_in_text WHERE id = ";
            sQuery += CEString::sToString(llLineId);
            m_pDb->Delete(sQuery);
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
