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
    CREATE TABLE lines_in_text(id INTEGER PRIMARY KEY ASC, text_id INTEGER, line_number INTEGER, text_offset INTEGER, length INTEGER, number_of_words INTEGER, source TEXT, FOREIGN KEY(text_id) REFERENCES text(id) ON DELETE CASCADE);

    --word_text redundant, needed for testing
        CREATE TABLE words_in_line(id INTEGER PRIMARY KEY ASC, line_id INTEGER, word_position INTEGER, line_offset INTEGER, word_length INTEGER, word_text TEXT, FOREIGN KEY(line_id) REFERENCES lines_in_text(id) ON DELETE CASCADE);

    CREATE TABLE word_to_wordform(id INTEGER PRIMARY KEY ASC, word_in_line_id INTEGER, wordform_id INTEGER, FOREIGN KEY(word_in_line_id) REFERENCES words_in_line(id), FOREIGN KEY(wordform_id) REFERENCES wordforms(id) ON DELETE CASCADE);

    -- this table is added to simplify retrieval of transcription and stress positions
        CREATE TABLE tact_group(id INTEGER PRIMARY KEY ASC, line_id INTEGER, first_word_position INTEGER, num_of_words INTEGER, source TEXT, transcription TEXT, stressed_syllable INTEGER, reverse_stressed_syllable INTEGER, FOREIGN KEY(line_id) REFERENCES lines_in_text(id) ON DELETE CASCADE);

    CREATE TABLE word_to_tact_group(id INTEGER PRIMARY KEY ASC, word_to_wordform_id INTEGER, tact_group_id INTEGER, position_in_tact_group INTEGER, FOREIGN KEY(word_to_wordform_id) REFERENCES word_to_word_form(id) ON DELETE CASCADE);

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

        int iNWords = sLine.uiNFields();
        if (iNWords < 1)
        {
            continue;
        }

        long long llLineDbId = -1;
        eRet = eSaveLine(iLine, iTextOffset, sLine.uiLength(), sLine.uiNFields(), sLine, llLineDbId);
        if (eRet != H_NO_ERROR)
        {
            continue;
        }

        m_mmapWordParses.clear();

        for (int iField = 0; iField < (int)sLine.uiNFields(); ++iField)
        {
            CEString sWord = sLine.sGetField(iField);
            sWord.ToLower();
            eRet = eParseWord(sWord, sLine, iField, (int)sLine.uiNFields(), llLineDbId);
            unsigned int uiStartPos = 0;
            auto uiAt = sWord.uiFind(L"е", uiStartPos);
            while (uiAt != Hlib::ecNotFound)
            {
                sWord[uiAt] = L'ё';
                eRet = eParseWord(sWord, sLine, iField, (int)sLine.uiNFields(), llLineDbId);
                sWord[uiAt] = L'е';
                uiStartPos = uiAt + 1;
                if (uiStartPos >= sWord.uiLength())
                {
                    break;
                }
                uiAt = sWord.uiFind(L"е", uiStartPos);
            }
        }

        eRet = eFindEquivalencies(sLine);
        if (eRet != H_NO_ERROR)
        {
            CEString sMsg(L"Failed fo find equivalencies; line: ");
            sMsg += sLine;
            ERROR_LOG(sMsg);
            continue;
        }

        eRet = eAssembleTactGroups(sLine);

        for (auto itTg = m_mapTactGroups.begin(); itTg != m_mapTactGroups.end(); ++itTg)
        {
            eRet = eSaveTactGroup((*itTg).second);
        }

        m_mapTactGroups.clear();

    }       //  for (int iLine = 0; iLine < iNLines; ++iLine)

    m_pDb->CommitTransaction();

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

ET_ReturnCode CAnalytics::eParseWord(const CEString& sWord, const CEString& sLine, int iNumInLine, int iWordsInLine, long long llLineDbKey)
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
        stParse.iNumber = iNumInLine;
        stParse.iLineOffset = iOffset;
        stParse.iLength = sWord.uiLength();

        if (bIsProclitic(*pWf))
        {
            stParse.eStressType = WORD_STRESS_TYPE_PROCLITIC;
        }
        else if (bIsEnclitic(*pWf))
        {
            stParse.eStressType = WORD_STRESS_TYPE_ENCLITIC;
        }
        else
        {
            stParse.eStressType = WORD_STRESS_TYPE_AUTONOMOUS;
        }

        stParse.llLineDbId = llLineDbKey;
        stParse.llWordInLineDbId = llWordInLineDbKey;
        stParse.llWordToWordFormId = llWordToWordFormId;
        stParse.iPosInTactGroup = 0;
        stParse.WordForm = *pWf;
        m_mmapWordParses.insert(make_pair(iNumInLine, stParse));

        eRet = m_pParser->eGetNextWordForm(pWf);
    
    }       //  while...

    m_pParser->ClearResults();

    return H_NO_ERROR;

}       //  eParseWord()

ET_ReturnCode CAnalytics::eFindEquivalencies(CEString& sLine)
{
    m_mmapEquivalencies.clear();

    for (int iField = 0; iField < (int)sLine.uiNFields(); ++iField)
    {
        auto it = m_mmapWordParses.find(iField);
        if (m_mmapWordParses.end() == it)
        {
            CEString sMsg(L"Can't find parse for the word number ");
            sMsg += CEString::sToString(iField);
            sMsg += L" in '" + sLine;
            sMsg += L"'";
            ERROR_LOG(sMsg);
            continue;
        }

        // All parses for this word
        auto pairParses = m_mmapWordParses.equal_range(iField);
        if (pairParses.first == pairParses.second)
        {
            ERROR_LOG(L"Empty parse list.");
            continue;
        }

        // Find phonetic invariants for each word parse
        for (auto itParse = pairParses.first; itParse != pairParses.second; ++itParse)
        {
            bool bIsInvariant = false;

            auto pairEquivalentParses = m_mmapEquivalencies.equal_range((*itParse).first);
            for (auto itEquivalency = pairEquivalentParses.first;
                itEquivalency != pairEquivalentParses.second;
                ++itEquivalency)
            {
                StWordParse& stLeft = (*itParse).second;
                vector<StWordParse>& setEquivalencies = (*itEquivalency).second;
                if (setEquivalencies.empty())
                {
                    ERROR_LOG(L"Empty phonetical invariants list.");
                    continue;
                }

                StWordParse stRight = *(*itEquivalency).second.begin();      // any element will do
                if (bArePhoneticallyIdentical(stLeft.WordForm, stRight.WordForm))
                {
                    (*itEquivalency).second.push_back(stLeft);
                    bIsInvariant = true;
                    break;
                }
            }               //  for (auto itEquivalency...)

            if (!bIsInvariant)
            {
                vector<StWordParse> s{ (*itParse).second };
                m_mmapEquivalencies.insert(make_pair((*itParse).first, s));
            }
        }       //  for...
    }

    return H_NO_ERROR;

}       //  eFindEquivalencies()

ET_ReturnCode CAnalytics::eGetStress(StTactGroup& stTg)
{
    ET_ReturnCode eRet = H_NO_ERROR;

    vector<int> vecPrimary;
    vector<int> vecSecondary;

    for (int iWord = stTg.iFirstWordNum, iCount = 0; iCount < stTg.iNumOfWords; ++iWord, ++iCount)
    {
        StWordParse& stWp = *(stTg.vecWords.begin());      // we can use any of the words
        if (WORD_STRESS_TYPE_AUTONOMOUS == stWp.eStressType)
        {
            int iPos = -1;
            ET_StressType eType = STRESS_TYPE_UNDEFINED;
            eRet = const_cast<CWordForm&>(stWp.WordForm).eGetFirstStressPos(iPos, eType);
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
                eRet = const_cast<CWordForm&>(stWp.WordForm).eGetNextStressPos(iPos, eType);
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
                CEString sWord = stWp.WordForm.sWordForm();

                try
                {
                    stTg.iStressedSyllable = sWord.uiGetSyllableFromVowelPos(iStressedCharPosInWord);
                }
                catch (...)
                {
                    return H_EXCEPTION;
                }

                // If this word is not the first word in the tact group, add syllable count from that word
                for (auto iProclitic = stTg.iFirstWordNum; iProclitic < iWord; ++iProclitic)
                {
                    StWordParse& stWpProclitic = *(stTg.vecWords.begin());      // we can use any of the words
                    stTg.iStressedSyllable += stWpProclitic.WordForm.sWordForm().uiNSyllables();
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
            CEString sWord = stWp.WordForm.sWordForm();

            try
            {
                stTg.iSecondaryStressedSyllable = sWord.uiGetSyllableFromVowelPos(iSecondaryStressedCharPosInWord);
            }
            catch (CException& ex)
            {
                CEString sMsg(L"Unable to get secondary stress: ");
                sMsg += ex.szGetDescription();
                return H_EXCEPTION;
            }

            // If this word is not the first word in the tact group, add syllable count from that word
            for (auto iProclitic = stTg.iFirstWordNum; iProclitic < iWord; ++iProclitic)
            {
                StWordParse& stWpProclitic = *(stTg.vecWords.begin());      // we can use any of the words
                stTg.iSecondaryStressedSyllable += stWpProclitic.WordForm.sWordForm().uiNSyllables();
            }

            eRet = H_NO_ERROR;
        }
        else
        {
            eRet = H_FALSE;
        }

    }       //  for (auto iWord = stTg.iFirstWordNum; ...

    int iStressPosInTactGroup = stTg.sSource.uiGetVowelPos(stTg.iStressedSyllable);
    stTg.sSource = stTg.sSource.sInsert(iStressPosInTactGroup + 1, CEString::g_chrCombiningAcuteAccent);

    auto iTotalSyllables = stTg.sSource.uiNSyllables();

    if (stTg.iStressedSyllable >= 0)
    {
        stTg.iReverseStressedSyllable = iTotalSyllables - stTg.iStressedSyllable - 1;
    }

    return eRet;

}       //  eGetStress()

ET_ReturnCode CAnalytics::eTranscribe(StTactGroup& stTg)
{
    if (nullptr == m_spTranscriber)
    {
        ERROR_LOG(L"m_spTranscriber is null.");
        return H_ERROR_POINTER;
    }

    auto eRet = m_spTranscriber->eTranscribeTactGroup(stTg);

    return eRet;
}

ET_ReturnCode CAnalytics::eAssembleTactGroups(CEString& sLine)
{
    ET_ReturnCode eRet = H_NO_ERROR;

    for (int iField = 0; iField < (int)sLine.uiNFields(); ++iField)
    {
        auto pairInvariants = m_mmapEquivalencies.equal_range(iField);
        for (auto itWordToParses = pairInvariants.first; itWordToParses != pairInvariants.second; ++itWordToParses)
        {
            vector<StWordParse>& vecVariants = (*itWordToParses).second;
            const StWordParse& stRandomVariant = *(vecVariants.begin());
            CWordForm wordForm = stRandomVariant.WordForm;

            StTactGroup stTg;
            stTg.iFirstWordNum = iField;
            stTg.iNumOfWords = 0;
            stTg.iMainWordPos = 0;
//            stTg.sSource = const_cast<CWordForm&>(wordForm).sWordForm();
            stTg.llLineId = stRandomVariant.llLineDbId;
            stTg.vecWords = vecVariants;

//            int iPosInGroup = 0;
            if (bIsProclitic(wordForm))
            {
                // insert before 1st autonomously stressed word, adjust iFirstWordNum
                stTg.sSource = wordForm.m_sWordForm;
                ++stTg.iMainWordPos;
                ++stTg.iNumOfWords;

            }
            else if (bIsEnclitic(wordForm)) // TODO: do we really need to distinguish btw enclitic and main word?
            {
                if (stTg.sSource.uiLength() > 0)
                {
                    stTg.sSource += L" ";
                }
                stTg.sSource += wordForm.m_sWordForm;
                ++stTg.iNumOfWords;
            }
            else
            {
                if (stTg.sSource.uiLength() > 0)
                {
                    stTg.sSource += L" ";
                }
                stTg.sSource += wordForm.m_sWordForm;
                ++stTg.iNumOfWords;
            }

            try
            {
                stTg.iNumOfSyllables = stTg.sSource.uiNSyllables();

                eRet = eGetStress(stTg);
                if (eRet != H_NO_ERROR && eRet != H_FALSE && eRet != H_NO_MORE)
                {
                    CEString sMsg(L"Unable to obtain stress position: ");
                    sMsg += stTg.sSource;
                    ERROR_LOG(sMsg);
                }

                eRet = eTranscribe(stTg);
                if (eRet != H_NO_ERROR)
                {
                    CEString sMsg(L"Unable to transcribe: ");
                    sMsg += stTg.sSource;
                    ERROR_LOG(sMsg);
                }
            }
            catch (CException& ex)
            {
                CEString sMsg(L"Exception: ");
                sMsg += ex.szGetDescription();
                ERROR_LOG(sMsg);
            }

            m_mapTactGroups.insert(make_pair(iField, stTg));
        
        }       // for (auto itWordToParses = ...
    }       //  for (int iField = 0; ...

    return H_NO_ERROR;

}       //  eAssembleTactGroups()

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

bool CAnalytics::bIsProclitic([[maybe_unused]]CWordForm& wordForm)
{
//    MESSAGE_LOG(wordForm.sWordForm());
    return false;       // TODO: implement
}

bool CAnalytics::bIsEnclitic([[maybe_unused]]CWordForm& wordForm)
{
//    MESSAGE_LOG(wordForm.sWordForm());
    return false;       // TODO: implement
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
//        assert(0);
        CEString sMsg(L"Error getting 1st stress position, words: ");
        sMsg += wf1.sWordForm();
        ERROR_LOG(sMsg);
        return false;
    }

    if (eRet2 != H_NO_ERROR && eRet2 != H_FALSE)
    {
//        assert(0);
        CEString sMsg(L"Error getting 2nd stress position, words: ");
        sMsg += wf2.sWordForm();
        ERROR_LOG(sMsg);
        return false;
    }

    if (eStressType1 != eStressType2 || iStressPos1 != iStressPos2)
    {
        return false;
    }

    for (int iCount = 0; iCount < 100; ++iCount)        // arbitraty max to avoid infinite loop
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
//            assert(0);
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

//    assert(0);
    ERROR_LOG(L"Error getting next stress position.");

    return false;

}       //  bArePhoneticallyIdentical()

// Save tact groups for a line
//  CREATE TABLE tact_group(id INTEGER PRIMARY KEY ASC, line_id INTEGER, first_word_position INTEGER, 
//  num_of_words INTEGER, source TEXT, transcription TEXT, num_of_syllables INTEGER, stressed_syllable INTEGER, 
//  reverse_stressed_syllable INTEGER, SECONDARY_STRESSED_SYLLABLE, FOREIGN KEY(line_id) 
//  REFERENCES lines_in_text(id));
ET_ReturnCode CAnalytics::eSaveTactGroup(StTactGroup& stTg)
{
    if (NULL == m_pDb)
    {
        ERROR_LOG(L"No database access.");
        return H_ERROR_POINTER;
    }

    m_pDb->PrepareForInsert(L"tact_group", 10);

    m_pDb->Bind(1, (int64_t)stTg.llLineId);
    m_pDb->Bind(2, stTg.iFirstWordNum);
    m_pDb->Bind(3, stTg.iMainWordPos);
    m_pDb->Bind(4, stTg.iNumOfWords);
    m_pDb->Bind(5, stTg.sSource);
    //            m_pDb->Bind(6, wordParse.WordForm.sGramHash());
    m_pDb->Bind(6, stTg.sTranscription);
    m_pDb->Bind(7, stTg.iNumOfSyllables);
    m_pDb->Bind(8, stTg.iStressedSyllable);
    m_pDb->Bind(9, stTg.iReverseStressedSyllable);
    m_pDb->Bind(10, stTg.iSecondaryStressedSyllable);

    m_pDb->InsertRow();
    m_pDb->Finalize();

    long long llTactGroupId = m_pDb->llGetLastKey();

    for (auto wordParse : stTg.vecWords)
    {
        try
        {
/*
            m_pDb->PrepareForInsert(L"tact_group", 10);

            m_pDb->Bind(1, stTg.llLineId);
            m_pDb->Bind(2, stTg.iFirstWordNum);
            m_pDb->Bind(3, stTg.iMainWordPos);
            m_pDb->Bind(4, stTg.iNumOfWords);
            m_pDb->Bind(5, stTg.sSource);
//            m_pDb->Bind(6, wordParse.WordForm.sGramHash());
            m_pDb->Bind(6, stTg.sTranscription);
            m_pDb->Bind(7, stTg.iNumOfSyllables);
            m_pDb->Bind(8, stTg.iStressedSyllable);
            m_pDb->Bind(9, stTg.iReverseStressedSyllable);
            m_pDb->Bind(10, stTg.iSecondaryStressedSyllable);

            m_pDb->InsertRow();
            m_pDb->Finalize();

            long long llTactGroupId = m_pDb->llGetLastKey();
*/

            // CREATE TABLE word_to_tact_group(id INTEGER PRIMARY KEY ASC, word_to_wordform_id INTEGER, 
            // tact_group_id INTEGER, position_in_tact_group INTEGER, FOREIGN KEY(word_to_wordform_id) 
            // REFERENCES word_to_word_form(id));

            sqlite3_stmt* pStmt = nullptr;
            m_pDb->uiPrepareForInsert(L"word_to_tact_group", 3, pStmt);
            auto llInsertHandle = (unsigned long long)pStmt;

            for (StWordParse& stWordParse : stTg.vecWords)
            {
                m_pDb->Bind(1, (int64_t)stWordParse.llWordToWordFormId, (int64_t)llInsertHandle);
                m_pDb->Bind(2, (int64_t)llTactGroupId, (int64_t)llInsertHandle);
                m_pDb->Bind(3, stWordParse.iPosInTactGroup, (int64_t)llInsertHandle);
                m_pDb->InsertRow((int64_t)llInsertHandle);
            }
            m_pDb->Finalize((int64_t)llInsertHandle);

            // CREATE TABLE tact_group_to_gram_hash(id INTEGER PRIMARY KEY ASC, tact_group_id INTEGER, gram_hash TEXT)
            pStmt = nullptr;
            m_pDb->uiPrepareForInsert(L"tact_group_to_gram_hash", 2, pStmt);
            llInsertHandle = (unsigned long long)pStmt;
            for (StWordParse& stWordParse : stTg.vecWords)
            {
                m_pDb->Bind(1, (int64_t)llTactGroupId, (int64_t)llInsertHandle);
                m_pDb->Bind(2, stWordParse.WordForm.sGramHash(), (int64_t)llInsertHandle);
                m_pDb->InsertRow((int64_t)llInsertHandle);
            }
            m_pDb->Finalize(llInsertHandle);
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
