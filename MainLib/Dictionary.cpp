// Dictionary.cpp : Implementation of CDictionary

#include "stdafx.h"

#include <ctime>

#include "WordForm.h"
#include "Lexeme.h"
#include "Dictionary.h"
#include "Parser.h"
#include "Verifier.h"

using namespace Hlib;

static CEString sQueryBase(L"SELECT \
                            headword.source, \
                            headword.id, \
                            headword.comment, \
                            descriptor.id, \
                            descriptor.graphic_stem, \
                            descriptor.is_variant, \
                            descriptor.main_symbol, \
                            descriptor.part_of_speech, \
                            descriptor.is_plural_of, \
                            descriptor.is_intransitive, \
                            descriptor.is_reflexive, \
                            descriptor.main_symbol_plural_of, \
                            descriptor.alt_main_symbol, \
                            descriptor.inflection_type, \
                            descriptor.comment, \
                            descriptor.alt_main_symbol_comment, \
                            descriptor.alt_inflection_comment, \
                            descriptor.verb_stem_alternation, \
                            descriptor.part_past_pass_zhd, \
                            descriptor.section, \
                            descriptor.no_comparative, \
                            descriptor.assumed_forms, \
                            descriptor.yo_alternation, \
                            descriptor.o_alternation, \
                            descriptor.second_genitive, \
                            descriptor.has_aspect_pair, \
                            descriptor.difficult_and_missing_forms, \
                            descriptor.has_irregular_forms, \
                            descriptor.has_deficiencies, \
                            descriptor.restricted_contexts, \
                            descriptor.contexts, \
                            descriptor.trailing_comment, \
                            inflection.id, \
                            inflection.is_primary, \
                            inflection.inflection_type, \
                            inflection.accent_type1, \
                            inflection.accent_type2, \
                            inflection.short_form_restrictions, \
                            inflection.past_part_restrictions, \
                            inflection.no_short_form, \
                            inflection.no_past_part, \
                            inflection.fleeting_vowel, \
                            inflection.stem_augment \
                            FROM \
                            headword INNER JOIN descriptor ON descriptor.word_id = headword.id ");

CDictionary::CDictionary() : m_pDb(NULL), m_pParser(NULL), m_pVerifier(NULL)
{}

CDictionary::~CDictionary()
{
    Clear();
    delete m_pDb;
    delete m_pVerifier;
}

ET_ReturnCode CDictionary::eSetDbPath(const CEString& sPath)
{
    m_sDbPath = sPath;
    try
    {
        delete m_pDb;   // needed for ref counting

        m_pDb = new CSqlite(m_sDbPath);
        if (!m_pDb)
        {
            return H_ERROR_DB;
        }
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  setDbPath (...)

CEString CDictionary::sGetDbPath()
{
    return m_sDbPath;
}

ET_ReturnCode CDictionary::eGetLexemeById(int iId)
{
    ET_ReturnCode rc = H_NO_ERROR;
    CEString sQuery(sQueryBase);
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id ";
    sQuery += L"WHERE descriptor.id = ";
    sQuery += CEString::sToString(iId);
    sQuery += L";";

    uint64_t uiQueryHandle = 0;
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }
    CLexeme * pLexeme = new CLexeme(this);
    if (NULL == pLexeme)
    {
        return H_ERROR_POINTER;
    }

    StLexemeProperties& stProperties = pLexeme->stGetPropertiesForWriteAccess();
    rc = eGetSingleRow(stProperties, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        delete pLexeme;
        pLexeme = NULL;
        ERROR_LOG(L"Error retrieving lexeme data.");
        return rc;
    }

    rc = eGetSingleRow(stProperties, uiQueryHandle);
    if (H_NO_MORE != rc)
    {
        delete pLexeme;
        pLexeme = NULL;
        ERROR_LOG(L"More than one row returned for a single lexeme ID or DB error.");
        return H_ERROR_UNEXPECTED;
    }

    m_pDb->Finalize(uiQueryHandle);

    m_vecLexemes.push_back(pLexeme);

    return rc;

}   //  eGetLexemeById (...)

ET_ReturnCode CDictionary::eGetLexemesByHash(const CEString& sMd5)
{
    ET_ReturnCode rc = H_NO_ERROR;
    CEString sQuery(sQueryBase);
    sQuery += L"INNER JOIN lexeme_hash_to_descriptor as lhd on lhd.descriptor_id=descriptor.id ";
    sQuery += L"LEFT OUTER JOIN inflection ON lhd.inflection_id = inflection.id ";
    sQuery += L"WHERE lexeme_hash = \"";
    sQuery += sMd5;
    sQuery += L"\";";

    uint64_t uiQueryHandle = 0;
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    while (H_NO_ERROR == rc)
    {
        CLexeme * pLexeme = new CLexeme(this);
        if (NULL == pLexeme)
        {
            return H_ERROR_POINTER;
        }

        rc = eGetSingleRow(pLexeme->stGetPropertiesForWriteAccess(), uiQueryHandle);
        if (H_NO_ERROR == rc)
        {
            m_vecLexemes.push_back(pLexeme);
        }
        if (H_NO_ERROR != rc && H_NO_MORE != rc)
        {
            delete pLexeme;
            pLexeme = NULL;
            ERROR_LOG(L"Error retrieving lexeme data.");
            return rc;
        }
    }

    m_pDb->Finalize(uiQueryHandle);

    return (H_NO_MORE == rc) ? H_NO_ERROR : rc;

}   //  eGetLexemesByMd5 (...)

ET_ReturnCode CDictionary::eGetLexemesByGraphicStem(const CEString& sHeadword)
{
    ET_ReturnCode rc = H_NO_ERROR;
    CEString sQuery(sQueryBase);
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id ";
    sQuery += L"WHERE descriptor.graphic_stem = \"";
    sQuery += sHeadword;
    sQuery += L"\";";

    uint64_t uiQueryHandle = 0;
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    while (H_NO_ERROR == rc)
    {
        CLexeme * pLexeme = new CLexeme(this);
        if (NULL == pLexeme)
        {
            Clear();
            return H_ERROR_POINTER;
        }

        rc = eGetSingleRow(pLexeme->stGetPropertiesForWriteAccess(), uiQueryHandle);
        if (H_NO_ERROR != rc && H_NO_MORE != rc)
        {
            Clear();
            delete pLexeme;
            pLexeme = NULL;
            ERROR_LOG(L"Error retrieving lexeme data.");
            return rc;
        }

        m_vecLexemes.push_back(pLexeme);
    }

    m_pDb->Finalize(uiQueryHandle);

    return (H_NO_ERROR == rc) ? H_NO_ERROR : rc;
}

ET_ReturnCode CDictionary::eGetLexemesByInitialForm(const CEString& sSource)
{
    ET_ReturnCode rc = H_NO_ERROR;
    CEString sQuery(sQueryBase);
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id ";
    sQuery += L"WHERE headword.source = \"";
    sQuery += sSource;
    sQuery += L"\";";

    uint64_t uiQueryHandle = 0;
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    while (H_NO_ERROR == rc)
    {
        CLexeme * pLexeme = new CLexeme(this);
        if (NULL == pLexeme)
        {
            return H_ERROR_POINTER;
        }

        rc = eGetSingleRow(pLexeme->stGetPropertiesForWriteAccess(), uiQueryHandle);
        if (H_NO_ERROR == rc)
        {
            m_vecLexemes.push_back(pLexeme);
        }
        if (H_NO_ERROR != rc && H_NO_MORE != rc)
        {
            Clear();
            delete pLexeme;
            pLexeme = NULL;
            ERROR_LOG(L"Error retrieving lexeme data.");
            return rc;
        }
    }

    m_pDb->Finalize(uiQueryHandle);

    return (H_NO_ERROR == rc) ? H_NO_ERROR : rc;
}

void CDictionary::Clear()
{
    vector<CLexeme *>::iterator itL = m_vecLexemes.begin();
    for (; itL != m_vecLexemes.end(); ++itL)
    {
        delete *itL;
    }
    m_vecLexemes.clear();
}

ET_ReturnCode CDictionary::Clear(ILexeme * pLexeme)
{
    vector<CLexeme *>::iterator itL = m_vecLexemes.begin();
    for (; itL != m_vecLexemes.end(); ++itL)
    {
        if (*itL == pLexeme)
        {
            delete *itL;
            m_vecLexemes.erase(itL);
            return H_NO_ERROR;
        }
    }

    return H_ERROR_UNEXPECTED;
}

ET_ReturnCode CDictionary::eGetParser(IParser *& pParser)
{
    if (NULL == m_pDb)
    {
        m_pVerifier = NULL;
        ERROR_LOG(L"Error retrieving IParser interface.");
        return H_ERROR_POINTER;
    }

    if (NULL == m_pParser)
    {
        m_pParser = new CParser(m_pDb);
    }

    pParser = m_pParser;

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eGetVerifier(IVerifier *& pVerifier)
{
    if (NULL == m_pDb)
    {
        m_pVerifier = NULL;
        ERROR_LOG(L"Error retrieving IVerifier interface.");
        return H_ERROR_POINTER;
    }

    if (NULL == m_pVerifier)
    {
        m_pVerifier = new CVerifier(this);
    }

    pVerifier = m_pVerifier;

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eExportTestData(const CEString& sPath, PROGRESS_CALLBACK_CLR pProgressCallback)
{
    if (NULL == m_pDb)
    {
        ERROR_LOG(L"Unable to obtain database handle.");
        return H_ERROR_POINTER;
    }

    bool bRet = true;

    vector<CEString> vecTables;
    vecTables.push_back(L"test_data");
    vecTables.push_back(L"test_data_stress");
    
    try
    {
        bRet = m_pDb->bExportTables(sPath, vecTables, pProgressCallback);
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return bRet ? H_NO_ERROR : H_ERROR_GENERAL;

}       //  eExportTestData()

ET_ReturnCode CDictionary::eImportTestData(const CEString& sPath, PROGRESS_CALLBACK_CLR pProgressCallback)
{
    if (NULL == m_pDb)
    {
        ERROR_LOG(L"Unable to obtain database handle.");
        return H_ERROR_POINTER;
    }

    bool bRet = true;

    vector<CEString> vecTables;
    vecTables.push_back(L"test_data");
    vecTables.push_back(L"test_data_stress");

    try
    {
        bool bAutoincrement = true;
        bRet = m_pDb->bImportTables(sPath, bAutoincrement, pProgressCallback);
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    if (!bRet)
    {
        return H_ERROR_GENERAL;
    }

    ET_ReturnCode rc = ePopulateHashToDescriptorTable(pProgressCallback);

    pProgressCallback(99, false);

    return H_NO_ERROR;

}       //  eImportTestData()

CSqlite * CDictionary::pGetDbHandle()
{
    if (m_pDb)
    {
        return m_pDb;
    }
    throw CException(H_ERROR_POINTER, L"Database handle not initialized.");
}

//
// Generate all wordforms for lexemes in m_vecLexemes
//
ET_ReturnCode CDictionary::eGenerateFormsForSelectedLexemes()
{
    if (!m_pDb)
    {
        return H_ERROR_DB;
    }

    ET_ReturnCode rc = H_NO_ERROR;


    m_pDb->BeginTransaction();

    vector<CLexeme *>::iterator itLexeme = m_vecLexemes.begin();
    for (; itLexeme != m_vecLexemes.end(); ++itLexeme)
    {
        //
        // Generate word forms
        //

        try
        {
            (*itLexeme)->eGenerateParadigm();

            //if (H_NO_ERROR != rc)
            //{
            //    ERROR_LOG(L"Error generating paradigm.")
            //}

            //
            // Save word forms
            //

            ET_ReturnCode eRet = (*itLexeme)->eSaveDataToDb();
            if (H_NO_ERROR != eRet)
            {
                ERROR_LOG(L"Error saving word froms to database.")
            }

        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
            //            return H_EXCEPTION;
        }

    }       //  for (int iRow = 0; iRow < iLexemes; ++iRow)

    m_pDb->CommitTransaction();

    return (H_NO_MORE == rc) ? H_NO_ERROR : rc;

}    //  eGenerateAllForms()

//
// Generate all word forms and store them in DB
//
ET_ReturnCode CDictionary::eGenerateAllForms()
{
    if (!m_pDb)
    {
        return H_ERROR_DB;
    }

    ET_ReturnCode rc = H_NO_ERROR;

    rc = ePopulateStemsTable();
    rc = ePopulateWordFormDataTables();

    return rc;

}       //  eGenerateAllForms()

ET_ReturnCode CDictionary::ePopulateStemsTable()
{
    if (!m_pDb)
    {
        return H_ERROR_DB;
    }

    ET_ReturnCode rc = H_NO_ERROR;

    uint64_t uiQueryHandle = 0;
    CEString sQuery(sQueryBase);
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id;";
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    int iRows = 0;

    m_pDb->BeginTransaction();

    int iPercentDone = 0;
    bool bMoreData = true;
    for (int iRow = 0; bMoreData; ++iRow)
    {
        CLexeme lexeme(this);
        try
        {
            rc = eGetSingleRow(lexeme.stGetPropertiesForWriteAccess(), uiQueryHandle);
            if (H_NO_ERROR != rc)
            {
                if (H_NO_MORE == rc)
                {
                    bMoreData = false;
                }
                else
                {
                    CEString sMsg(L"Error retrieving lexeme data: ");
                    sMsg += lexeme.sSourceForm();
                    sMsg += L".";
                    ERROR_LOG(sMsg);
                    //                return rc;
                }
                continue;
            }
        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
        }

        //
        // Generate word forms
        //

        try
        {
            rc = lexeme.eGenerateParadigm();
            if (H_NO_ERROR != rc)
            {
                ERROR_LOG(L"Error generating paradigm.")
            }

            ET_ReturnCode eRet = lexeme.eSaveStemsToDb();
        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
            //            return H_EXCEPTION;
        }

        //
        // Update progress
        //
        if ((H_NO_ERROR == rc) && (iRow > 0) && (iRow % 1000 == 0))
        {
            m_pDb->CommitTransaction();
            m_pDb->BeginTransaction();
            CEString sMsg = CEString::sToString(iRow);
            sMsg += L" rows";
            ERROR_LOG(sMsg);
            std::wcout << wstring(sMsg) << std::endl;
        }

    }       //  for (...)

    m_pDb->CommitTransaction();

//    CLexeme lexeme(this);

    return (H_NO_MORE == rc) ? H_NO_ERROR : rc;

}       //  ePopulateStemsTable()

ET_ReturnCode CDictionary::ePopulateWordFormDataTables()
{
    if (!m_pDb)
    {
        return H_ERROR_DB;
    }

    ET_ReturnCode rc = H_NO_ERROR;

    uint64_t uiQueryHandle = 0;
    CEString sQuery(sQueryBase);
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id;";
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    std::clock_t totalTime = 0;

    int iPercentDone = 0;
    bool bMoreData = true;
    vector<CLexeme> vecLexemes;
    for (int iRow = 0; bMoreData; ++iRow)
    {
        CLexeme lexeme(this);
        try
        {
            rc = eGetSingleRow(lexeme.stGetPropertiesForWriteAccess(), uiQueryHandle);
            if (H_NO_ERROR == rc)
            {
                lexeme.eGenerateParadigm();
                lexeme.eAssignStemIds();
                vecLexemes.push_back(lexeme);
            }
            else
            {
                if (H_NO_MORE == rc)
                {
                    bMoreData = false;
                }
                else
                {
                    ERROR_LOG(L"Error retrieving lexeme data.");
                    continue;
                }
            }
        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
        }

        if (iRow > 0 && (iRow % 1000 == 0 || !bMoreData))
        {
            clock_t dbProcTime = clock();

            CEString sMsg(L"------------ Saving stem data ------------- \r\n");
            sMsg += CEString::sToString(iRow);
            //CLogger::bWriteLog(wstring(xxxx));
            ERROR_LOG(sMsg)
            std::wcout << wstring(sMsg) << std::endl;

            m_pDb->BeginTransaction();

            for (vector<CLexeme>::iterator itLexeme = vecLexemes.begin(); itLexeme != vecLexemes.end(); ++itLexeme)
            {
                for (unsigned int uiWf = 0; uiWf < (*itLexeme).uiTotalWordForms(); ++uiWf)
                {
                    CWordForm * pWf = NULL;
                    rc = (*itLexeme).eGetWordForm(uiWf, pWf);
                    if (rc != H_NO_ERROR)
                    {
                        continue;
                    }

                    if (0 == pWf->m_llStemId)
                    {
                        CEString sMsg(L"Unable to find stem id for \"");
                        sMsg += pWf->m_sLemma;
                        sMsg += L'"';
                        ERROR_LOG(sMsg);

                        continue;
                    }
                    pWf->bSaveToDb();
                }
            }

            m_pDb->CommitTransaction();

            totalTime += clock() - dbProcTime;
            double dDuration = totalTime / (double)CLOCKS_PER_SEC;
            CEString sDurationMsg(L"Row ");
            sDurationMsg += CEString::sToString(iRow);
            sDurationMsg += L"; ";
            sDurationMsg += CEString::sToString(dDuration);
            sDurationMsg += L" seconds total";
//            CLogger::bWriteLog(wstring(sDurationMsg));
            std::wcout << wstring(sDurationMsg) << std::endl;
            totalTime = 0;

            vecLexemes.clear();

        }

    }       //  for ...

    return H_NO_ERROR;

}       //  ePopulateWordFormDataTables()

ET_ReturnCode CDictionary::ePopulateHashToDescriptorTable(PROGRESS_CALLBACK_CLR pProgress)
{
    if (!m_pDb)
    {
        return H_ERROR_DB;
    }

    ET_ReturnCode rc = H_NO_ERROR;

    __int64 llCount = 0;
    try
    {
        CEString 
            sCountQuery(L"SELECT COUNT() FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id; ");
        uint64_t uiCountQueryHandle = 0;
        rc = eQueryDb(sCountQuery, uiCountQueryHandle);
        if (H_NO_ERROR != rc)
        {
            return rc;
        }

        bool bRet = m_pDb->bGetRow(uiCountQueryHandle);
        if (!bRet)
        {
            return H_ERROR_UNEXPECTED;
        }

        m_pDb->GetData(0, llCount, uiCountQueryHandle);
        if (H_NO_ERROR != rc)
        {
            return rc;
        }
    }
    catch (CException& ex)
    {
        HandleDbException(ex);
        rc = H_ERROR_DB;
    }

    try
    {
        m_pDb->ClearTable(L"lexeme_hash_to_descriptor");
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            m_pDb->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
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

    uint64_t uiQueryHandle = 0;
    CEString sQuery(sQueryBase);
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id;";
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    std::clock_t totalTime = 0;

    m_pDb->BeginTransaction();

    int iPercentDone = 0;
    bool bMoreData = true;
    vector<CLexeme> vecLexemes;
    for (int iRow = 0; bMoreData; ++iRow)
    {
        CLexeme lexeme(this);
        try
        {
            rc = eGetSingleRow(lexeme.stGetPropertiesForWriteAccess(), uiQueryHandle);
            if (H_NO_ERROR == rc)
            {
                uint64_t uiInsertHandle = 0;

                try
                {
                    CEString sHash = lexeme.sHash();
                    const StLexemeProperties& stProperties = lexeme.stGetProperties();
                    if (0 == uiInsertHandle)
                    {
                        m_pDb->uiPrepareForInsert(L"lexeme_hash_to_descriptor", 3, (sqlite3_stmt *&)uiInsertHandle, false);
                    }
                    m_pDb->Bind(1, sHash, uiInsertHandle);
                    m_pDb->Bind(2, stProperties.iDbKey, uiInsertHandle);
                    m_pDb->Bind(3, stProperties.iInflectionId, uiInsertHandle);
                    m_pDb->InsertRow(uiInsertHandle);
                    m_pDb->Finalize(uiInsertHandle);
                }
                catch (CException& exc)
                {
                    CEString sMsg(exc.szGetDescription());
                    CEString sError;
                    try
                    {
                        m_pDb->GetLastError(sError);
                        sMsg += CEString(L", error %d: ");
                        sMsg += sError;
                    }
                    catch (...)
                    {
                        sMsg = L"Apparent DB error ";
                    }

                    sMsg += CEString::sToString(m_pDb->iGetLastError());
                    ERROR_LOG(sMsg);

                    continue;
                }
            }
            else
            {
                if (H_NO_MORE == rc)
                {
                    bMoreData = false;
                }
                else
                {
                    ERROR_LOG(L"Error retrieving lexeme data.");
                    continue;
                }
            }
        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
        }

        if (iRow > 0 && (iRow % 1000 == 0 || !bMoreData))
        {
            int iPercentDone = min(100, (int)(100 * (double)iRow / (double)llCount));
            pProgress(iPercentDone, false);


            clock_t dbProcTime = clock();

            CEString sMsg(L"------------ Saving lexeme hashes ------------- \r\n");
            sMsg += CEString::sToString(iRow);
            //CLogger::bWriteLog(wstring(xxxx));
            ERROR_LOG(sMsg)
                std::wcout << wstring(sMsg) << std::endl;

            totalTime += clock() - dbProcTime;
            double dDuration = totalTime / (double)CLOCKS_PER_SEC;
            CEString sDurationMsg(L"Row ");
            sDurationMsg += CEString::sToString(iRow);
            sDurationMsg += L"; ";
            sDurationMsg += CEString::sToString(dDuration);
            sDurationMsg += L" seconds total";
            //            CLogger::bWriteLog(wstring(sDurationMsg));
            std::wcout << wstring(sDurationMsg) << std::endl;
            totalTime = 0;

        }
    }       //  for ...

    m_pDb->CommitTransaction();

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eCountLexemes(int64_t& iLexemes)
{
    ET_ReturnCode rc = H_NO_ERROR;

    iLexemes = -1;

    if (!m_pDb)
    {
        return H_ERROR_DB;
    }

    CEString sQuery(L"SELECT descriptor.id, COUNT(*) FROM descriptor;");

    try
    {

        uint64_t uiQueryHandle = 0;
        rc = eQueryDb(sQuery, uiQueryHandle);
        if (H_NO_ERROR != rc)
        {
            return rc;
        }

        bool bRet = m_pDb->bGetRow(uiQueryHandle);
        if (!bRet)
        {
            return H_ERROR_UNEXPECTED;
        }

        m_pDb->GetData(1, iLexemes, uiQueryHandle);
        if (H_NO_ERROR != rc)
        {
            return rc;
        }

    }
    catch (CException& ex)
    {
        HandleDbException(ex);
        rc = H_ERROR_DB;
    }

    return rc;

}   //  eCountLexemes()


ET_ReturnCode CDictionary::eGetFirstLexeme(ILexeme *& pLexeme)
{
    m_itCurrentLexeme = m_vecLexemes.begin();
    if (m_vecLexemes.end() == m_itCurrentLexeme)
    {
        return H_FALSE;
    }

    pLexeme = *m_itCurrentLexeme;

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eGetNextLexeme(ILexeme *& pLexeme)
{
    if (m_itCurrentLexeme != m_vecLexemes.end())
    {
        ++m_itCurrentLexeme;
    }

    if (m_vecLexemes.end() == m_itCurrentLexeme)
    {
        return H_NO_MORE;
    }

    pLexeme = *m_itCurrentLexeme;

    return H_NO_ERROR;
}

//
// Helpers
//

ET_ReturnCode CDictionary::eQueryDb(const CEString& sSelect, uint64_t& uiQueryHandle)
{
    ET_ReturnCode rc = H_NO_ERROR;

    if (NULL == m_pDb)
    {
        ASSERT(0);
        ERROR_LOG(L"DB pointer is NULL.");
        return H_ERROR_POINTER;
    }

    try
    {
        uiQueryHandle = m_pDb->uiPrepareForSelect(sSelect);
    }
    catch (CException& ex)
    {
        HandleDbException(ex);
        rc = H_ERROR_DB;
    }

    return rc;
}

ET_ReturnCode CDictionary::eGetSingleRow(StLexemeProperties& stProperties, uint64_t uiQueryHandle)
{
    if (NULL == m_pDb)
    {
        ASSERT(0);
        ERROR_LOG(L"DB pointer is NULL.");
        return H_ERROR_POINTER;
    }

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        if (m_pDb->bGetRow(uiQueryHandle))
        {
            m_pDb->GetData(0, stProperties.sSourceForm, uiQueryHandle);
            int iHeadwordId = -1;
            m_pDb->GetData(1, iHeadwordId, uiQueryHandle);
            m_pDb->GetData(2, stProperties.sHeadwordComment, uiQueryHandle);
            m_pDb->GetData(3, stProperties.iDbKey, uiQueryHandle);
            m_pDb->GetData(4, stProperties.sGraphicStem, uiQueryHandle);
            m_pDb->GetData(5, stProperties.bIsVariant, uiQueryHandle);
            m_pDb->GetData(6, stProperties.sMainSymbol, uiQueryHandle);
            m_pDb->GetData(7, (int&)stProperties.ePartOfSpeech, uiQueryHandle);

            m_pDb->GetData(8, stProperties.bIsPluralOf, uiQueryHandle);
            bool bIntransitive = false;
            m_pDb->GetData(9, bIntransitive, uiQueryHandle);        // "нп" == !bTransitive
            stProperties.bTransitive = !bIntransitive;
            bool bReflexive = false;
            m_pDb->GetData(10, bReflexive, uiQueryHandle);
            if (POS_VERB == stProperties.ePartOfSpeech)
            {
                stProperties.eReflexive = bReflexive ? REFL_YES : REFL_NO;
            }
            else
            {
                stProperties.eReflexive = REFL_UNDEFINED;
            }
            m_pDb->GetData(11, stProperties.sMainSymbolPluralOf, uiQueryHandle);
            m_pDb->GetData(12, stProperties.sAltMainSymbol, uiQueryHandle);
            m_pDb->GetData(13, stProperties.sInflectionType, uiQueryHandle);
            m_pDb->GetData(14, stProperties.sComment, uiQueryHandle);
            m_pDb->GetData(15, stProperties.sAltMainSymbolComment, uiQueryHandle);
            m_pDb->GetData(16, stProperties.sAltInflectionComment, uiQueryHandle);
            m_pDb->GetData(17, stProperties.sVerbStemAlternation, uiQueryHandle);
            m_pDb->GetData(18, stProperties.bPartPastPassZhd, uiQueryHandle);
            m_pDb->GetData(19, stProperties.iSection, uiQueryHandle);
            m_pDb->GetData(20, stProperties.bNoComparative, uiQueryHandle);
            m_pDb->GetData(21, stProperties.bAssumedForms, uiQueryHandle);
            m_pDb->GetData(22, stProperties.bYoAlternation, uiQueryHandle);
            m_pDb->GetData(23, stProperties.bOAlternation, uiQueryHandle);
            m_pDb->GetData(24, stProperties.bSecondGenitive, uiQueryHandle);
            m_pDb->GetData(25, stProperties.bHasAspectPair, uiQueryHandle);
            //            m_pDb->GetData (25, stProperties.iAspectPairType);
            //            m_pDb->GetData (26, stProperties.sAspectPairComment);
            m_pDb->GetData(26, stProperties.sQuestionableForms, uiQueryHandle);
            m_pDb->GetData(27, stProperties.bHasIrregularForms, uiQueryHandle);
            m_pDb->GetData(28, stProperties.bHasDeficiencies, uiQueryHandle);
            m_pDb->GetData(29, stProperties.sRestrictedForms, uiQueryHandle);
            m_pDb->GetData(30, stProperties.sContexts, uiQueryHandle);
            m_pDb->GetData(31, stProperties.sTrailingComment, uiQueryHandle);
            stProperties.iInflectionId = -1;
            m_pDb->GetData(32, stProperties.iInflectionId, uiQueryHandle);
            m_pDb->GetData(33, stProperties.bPrimaryInflectionGroup, uiQueryHandle);
            m_pDb->GetData(34, stProperties.iType, uiQueryHandle);
            m_pDb->GetData(35, (int&)stProperties.eAccentType1, uiQueryHandle);
            m_pDb->GetData(36, (int&)stProperties.eAccentType2, uiQueryHandle);
            m_pDb->GetData(37, stProperties.bShortFormsRestricted, uiQueryHandle);
            m_pDb->GetData(38, stProperties.bPastParticipleRestricted, uiQueryHandle);
            m_pDb->GetData(39, stProperties.bShortFormsIncomplete, uiQueryHandle);
            m_pDb->GetData(40, stProperties.bNoPastParticiple, uiQueryHandle);
            m_pDb->GetData(41, stProperties.bFleetingVowel, uiQueryHandle);
            m_pDb->GetData(42, stProperties.iStemAugment, uiQueryHandle);

            CEString sStressQuery(L"SELECT stress_position, is_primary FROM stress WHERE headword_id = ");
            sStressQuery += CEString::sToString(iHeadwordId);

            uint64_t uiStressHandle = m_pDb->uiPrepareForSelect(sStressQuery);
            while (m_pDb->bGetRow(uiStressHandle))
            {
                int iPos = -1;
                bool bPrimary = false;
                m_pDb->GetData(0, iPos, uiStressHandle);
                m_pDb->GetData(1, bPrimary, uiStressHandle);
                stProperties.sSourceForm.SetVowels(g_szRusVowels);
                int iStressedSyll = stProperties.sSourceForm.uiGetSyllableFromVowelPos(iPos);
                if (bPrimary)
                {
                    stProperties.vecSourceStressPos.push_back(iStressedSyll);
                }
                else
                {
                    stProperties.vecSecondaryStressPos.push_back(iStressedSyll);
                }
            }
            m_pDb->Finalize(uiStressHandle);

            if (stProperties.vecSourceStressPos.empty())
            {
                ASSERT(stProperties.vecSecondaryStressPos.empty());
                CEString sHwCommentQuery(L"SELECT comment FROM headword WHERE id = ");
                sHwCommentQuery += CEString::sToString(iHeadwordId);

                uint64_t uiHwCommentHandle = m_pDb->uiPrepareForSelect(sHwCommentQuery);
                while (m_pDb->bGetRow(uiHwCommentHandle))
                {
                    CEString sHwComment;
                    m_pDb->GetData(0, sHwComment, uiHwCommentHandle);
                    if (L"_без удар._" == sHwComment)
                    {
                        stProperties.bIsUnstressed = true;
                    }
                }
                m_pDb->Finalize(uiHwCommentHandle);

                if (!stProperties.bIsUnstressed && !(POS_PARTICLE == stProperties.ePartOfSpeech))
                {
                    ASSERT(1 == stProperties.sSourceForm.uiGetNumOfSyllables());
                    stProperties.vecSourceStressPos.push_back(0);
                }
            }

            CEString sHomonymsQuery(L"SELECT homonym_number FROM homonyms WHERE headword_id = ");
            sHomonymsQuery += CEString::sToString(iHeadwordId);

            uint64_t uiHomonymsHandle = m_pDb->uiPrepareForSelect(sHomonymsQuery);
            while (m_pDb->bGetRow(uiHomonymsHandle))
            {
                int iHomonym = -1;
                m_pDb->GetData(0, iHomonym, uiHomonymsHandle);
                stProperties.vecHomonyms.push_back(iHomonym);
            }
            m_pDb->Finalize(uiHomonymsHandle);

            CEString sDeviationQuery(L"SELECT deviation_type, is_optional FROM common_deviation WHERE inflection_id = ");
            sDeviationQuery += CEString::sToString(stProperties.iInflectionId);

            uint64_t uiDeviationHandle = m_pDb->uiPrepareForSelect(sDeviationQuery);
            while (m_pDb->bGetRow(uiDeviationHandle))
            {
                int iType = -1;
                bool bOptional = false;
                m_pDb->GetData(0, iType, uiDeviationHandle);
                m_pDb->GetData(1, bOptional, uiDeviationHandle);
                stProperties.mapCommonDeviations[iType] = bOptional;
            }
            m_pDb->Finalize(uiDeviationHandle);

            if (POS_NOUN == stProperties.ePartOfSpeech)
            {
                rc = eGetL2Data(stProperties.iDbKey, stProperties);
                if (H_NO_ERROR != rc)
                {
                    ERROR_LOG(L"hGetL2Data() failed.");
                    return rc;
                }
            }

			if (POS_VERB == stProperties.ePartOfSpeech)
			{
				if (stProperties.bHasAspectPair)
				{
					CEString sAspectPairQuery(L"SELECT type, data, is_variant, comment FROM aspect_pair WHERE descriptor_id = ");
					sAspectPairQuery += CEString::sToString(stProperties.iDbKey);

					uint64_t uiAspectPairHandle = m_pDb->uiPrepareForSelect(sAspectPairQuery);
					while (m_pDb->bGetRow(uiAspectPairHandle))
					{
						int iType = 0;
						bool bIsVariant = false;
						CEString sData;
						m_pDb->GetData(0, iType, uiAspectPairHandle);
						m_pDb->GetData(1, sData, uiAspectPairHandle);
						m_pDb->GetData(2, bIsVariant, uiAspectPairHandle);
						m_pDb->GetData(3, stProperties.sAltAspectPairComment, uiAspectPairHandle);

						if (bIsVariant)
						{
							stProperties.iAltAspectPairType = iType;
							stProperties.sAltAspectPairData = sData;
						}
						else
						{
							stProperties.iAspectPairType = iType;
							stProperties.sAspectPairData = sData;
						}
					}
					m_pDb->Finalize(uiAspectPairHandle);
				}
			}

        }   // if (m_pDb->b_GetRow())
        else
        {
            rc = H_NO_MORE;
        }

    }
    catch (CException& ex)
    {
        HandleDbException(ex);
        rc = H_ERROR_DB;
    }

    return rc;

}    //  eGetSingleRow (CLexeme *& pLexeme)

ET_ReturnCode CDictionary::eGetL2Data(__int64 llLexemeId, StLexemeProperties& stProperties)
{
    if (NULL == m_pDb)
    {
        ASSERT(0);
        ERROR_LOG(L"DB pointer is NULL.");
        return H_ERROR_POINTER;
    }

    try
    {
        CEString sQuery(L"SELECT is_optional, preposition FROM second_locative WHERE descriptor_id = ");
        sQuery += CEString::sToString(llLexemeId);
        sQuery += L";";

        uint64_t uiHandle = m_pDb->uiPrepareForSelect(sQuery);

        stProperties.bSecondLocative = false;

        if (m_pDb->bGetRow(uiHandle))
        {
            stProperties.bSecondLocative = true;
            m_pDb->GetData(0, stProperties.bSecondLocativeOptional, uiHandle);
            m_pDb->GetData(1, stProperties.sLoc2Preposition, uiHandle);
        }

        if (m_pDb->bGetRow(uiHandle))
        {
            ASSERT(0);
            CEString sMsg(L"More than one Loc2 record for ");
            sMsg += stProperties.sSourceForm;
            ERROR_LOG(sMsg);
            //        return E_FAIL;
        }

        m_pDb->Finalize(uiHandle);
    }
    catch (CException& ex)
    {
        HandleDbException(ex);
        return H_ERROR_DB;
    }

    return H_NO_ERROR;

}   // eGetL2Data (...)

void CDictionary::HandleDbException(CException& ex)
{
    CEString sMsg(ex.szGetDescription());
    CEString sError;
    try
    {
        m_pDb->GetLastError(sError);
        sMsg += CEString(L", error description: ");
        sMsg += sError;
    }
    catch (...)
    {
        sMsg = L"Apparent DB error ";
    }

    sMsg += L", error code = ";
    sMsg += CEString::sToString(m_pDb->iGetLastError());
    ERROR_LOG(sMsg);

}
