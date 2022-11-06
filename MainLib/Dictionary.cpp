#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <ctime>
#include <cassert>
#include <algorithm>
#include <iomanip>

#include "Logging.h"

#include "WordForm.h"
#include "Lexeme.h"
#include "Analytics.h"
#include "Parser.h"
#include "Verifier.h"
#include "Dictionary.h"

using namespace Hlib;

ET_ReturnCode CLexemeEnumerator::eReset()
{
    if (nullptr == m_pDictionary)
    {
        return H_ERROR_POINTER;
    }

    m_itCurrentLexeme = m_pDictionary->m_vecLexemes.begin();
    return H_NO_ERROR;
}

ET_ReturnCode CLexemeEnumerator::eGetFirstLexeme(ILexeme*& pLexeme)
{
    if (nullptr == m_pDictionary)
    {
        return H_ERROR_POINTER;
    }

    m_itCurrentLexeme = m_pDictionary->m_vecLexemes.begin();
    if (m_pDictionary->m_vecLexemes.end() == m_itCurrentLexeme)
    {
        return H_FALSE;
    }

    pLexeme = (*m_itCurrentLexeme).get();

    return H_NO_ERROR;
}

ET_ReturnCode CLexemeEnumerator::eGetFirstLexeme(CLexeme*& pLexeme)
{
    if (nullptr == m_pDictionary)
    {
        return H_ERROR_POINTER;
    }

    m_itCurrentLexeme = m_pDictionary->m_vecLexemes.begin();
    if (m_pDictionary->m_vecLexemes.end() == m_itCurrentLexeme)
    {
        return H_FALSE;
    }

    pLexeme = (*m_itCurrentLexeme).get();

    return H_NO_ERROR;
}

ET_ReturnCode CLexemeEnumerator::eGetNextLexeme(ILexeme*& pLexeme)
{
    if (nullptr == m_pDictionary)
    {
        return H_ERROR_POINTER;
    }

    if (m_itCurrentLexeme != m_pDictionary->m_vecLexemes.end())
    {
        ++m_itCurrentLexeme;
    }

    if (m_pDictionary->m_vecLexemes.end() == m_itCurrentLexeme)
    {
        return H_NO_MORE;
    }

    pLexeme = (*m_itCurrentLexeme).get();

    return H_NO_ERROR;
}

ET_ReturnCode CLexemeEnumerator::eGetNextLexeme(CLexeme*& pLexeme)
{
    if (nullptr == m_pDictionary)
    {
        return H_ERROR_POINTER;
    }

    if (m_itCurrentLexeme != m_pDictionary->m_vecLexemes.end())
    {
        ++m_itCurrentLexeme;
    }

    if (m_pDictionary->m_vecLexemes.end() == m_itCurrentLexeme)
    {
        return H_NO_MORE;
    }

    pLexeme = (*m_itCurrentLexeme).get();

    return H_NO_ERROR;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static CEString sQueryBase(L"SELECT \
                            headword.source, \
                            headword.id, \
                            headword.comment, \
                            headword.variant_comment, \
                            headword.plural_of, \
                            headword.usage, \
                            headword.variant, \
                            headword.see_ref, \
                            headword.back_ref, \
                            headword.source_entry_id, \
                            headword.spryazh_sm, \
                            headword.second_part, \
                            descriptor.id, \
                            descriptor.graphic_stem, \
                            descriptor.second_part_id, \
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
                            descriptor.no_long_forms, \
                            descriptor.assumed_forms, \
                            descriptor.yo_alternation, \
                            descriptor.o_alternation, \
                            descriptor.second_genitive, \
                            descriptor.is_impersonal, \
                            descriptor.is_iterative, \
                            descriptor.has_aspect_pair, \
                            descriptor.has_difficult_forms, \
                            descriptor.has_missing_forms, \
                            descriptor.has_irregular_forms, \
                            descriptor.irregular_forms_lead_comment, \
                            descriptor.restricted_contexts, \
                            descriptor.contexts, \
                            descriptor.cognate, \
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
                            inflection.stem_augment ");

CDictionary::CDictionary() : m_pDb(nullptr), m_pParser(NULL), m_pAnalytics(NULL), m_pVerifier(NULL)
{}

CDictionary::~CDictionary()
{
    Clear();
}

ET_ReturnCode CDictionary::eSetDbPath(const CEString& sPath)
{
    m_sDbPath = sPath;
    try
    {
//        delete m_pDb;   // needed for ref counting

        m_pDb = make_shared<CSqlite>(m_sDbPath);
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

shared_ptr<CSqlite> CDictionary::pGetDb()               //  non-virtual
{
    if (nullptr == m_pDb) {
        ERROR_LOG(L"Database not available.");
        return nullptr;
    }

    return m_pDb;
}

ET_ReturnCode CDictionary::eCreateLexemeForEdit(ILexeme *& pLexeme)
{
    if (nullptr == m_pDb)
    {
        pLexeme = NULL;
        return H_ERROR_POINTER;
    }

    pLexeme = new CLexeme(this);
    if (NULL == pLexeme)
    {
        return H_ERROR_POINTER;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eCopyLexemeForEdit(const ILexeme * pSource, ILexeme *& pCopy)
{
    if (NULL == m_pDb)
    {
        pSource = NULL;
        return H_ERROR_POINTER;
    }

    const CLexeme * pLexeme = dynamic_cast<const CLexeme *>(pSource);
    pCopy = new CLexeme(*pLexeme);
    if (NULL == pCopy)
    {
        return H_ERROR_POINTER;
    }

    pCopy->SetDictionary(this);

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eGetLexemeById(long long llId, shared_ptr<CLexeme>& spLexeme)       // non-virtual overload
{
    ET_ReturnCode rc = H_NO_ERROR;
    CEString sQuery(sQueryBase);
    sQuery += L"FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id ";
    sQuery += L"WHERE descriptor.id = ";
    sQuery += CEString::sToString(llId);
    sQuery += L";";

    uint64_t uiQueryHandle = 0;
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    spLexeme = make_shared<CLexeme>(this);
    if (nullptr == spLexeme)
    {
        return H_ERROR_POINTER;
    }

    while (H_NO_ERROR == rc)
    {
        rc = eGetSingleRow(spLexeme->stGetPropertiesForWriteAccess(), uiQueryHandle);
        if (H_NO_ERROR == rc)
        {
            m_vecLexemes.push_back(spLexeme);
        }
        if (H_NO_ERROR != rc && H_NO_MORE != rc)
        {
            Clear();
            spLexeme = nullptr;
            ERROR_LOG(L"Error retrieving lexeme data.");
            return rc;
        }
    }
    m_pDb->Finalize(uiQueryHandle);

    return H_NO_ERROR;
}


ET_ReturnCode CDictionary::eGetLexemeById(long long llId, ILexeme *& pILexeme)
{
    shared_ptr<CLexeme> spLexeme;
    auto eRet = eGetLexemeById(llId, spLexeme);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    pILexeme = spLexeme.get();
    return H_NO_ERROR;

}   //  eGetLexemeById (...)

ET_ReturnCode CDictionary::eGetSecondPart(long long llId, shared_ptr<CLexeme>& pLexeme)
{
    ET_ReturnCode rc = H_NO_ERROR;
    CEString sQuery(sQueryBase);
    sQuery += L"FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id ";
    sQuery += L"WHERE descriptor.id = ";
    sQuery += CEString::sToString(llId);
    sQuery += L";";

    uint64_t uiQueryHandle = 0;
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    StLexemeProperties& stProperties = pLexeme->stGetPropertiesForWriteAccess();
    rc = eGetSingleRow(stProperties, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        pLexeme = nullptr;
        ERROR_LOG(L"Error retrieving lexeme data.");
        return rc;
    }

    rc = eGetSingleRow(stProperties, uiQueryHandle);
    if (H_NO_MORE != rc)
    {
        pLexeme = nullptr;
        ERROR_LOG(L"More than one row returned for a single lexeme ID or DB error.");
        return H_ERROR_UNEXPECTED;
    }

    return rc;
}

ET_ReturnCode CDictionary::eGetLexemesByHash(const CEString& sMd5)
{
    ET_ReturnCode rc = H_NO_ERROR;
    CEString sQuery(sQueryBase);
    sQuery += L"FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
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
        shared_ptr<CLexeme> pLexeme = make_shared<CLexeme>(this);
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
            pLexeme = nullptr;
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
    sQuery += L"FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
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
        shared_ptr<CLexeme> pLexeme = make_shared<CLexeme>(this);
        if (nullptr == pLexeme)
        {
            Clear();
            return H_ERROR_POINTER;
        }

        rc = eGetSingleRow(pLexeme->stGetPropertiesForWriteAccess(), uiQueryHandle);
        if (H_NO_ERROR != rc && H_NO_MORE != rc)
        {
            Clear();
            pLexeme = nullptr;
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
    sQuery += L"FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
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

    bool bFound = false;
    while (H_NO_ERROR == rc)
    {
        shared_ptr<CLexeme> pLexeme = make_shared<CLexeme>(this);
        if (NULL == pLexeme)
        {
            return H_ERROR_POINTER;
        }

        rc = eGetSingleRow(pLexeme->stGetPropertiesForWriteAccess(), uiQueryHandle);
        if (H_NO_ERROR == rc)
        {
            bFound = true;
            m_vecLexemes.push_back(pLexeme);
        }
        if (H_NO_ERROR != rc && H_NO_MORE != rc)
        {
            Clear();
            pLexeme = nullptr;
            ERROR_LOG(L"Error retrieving lexeme data.");
            return rc;
        }
    }

    m_pDb->Finalize(uiQueryHandle);

    CEString sSpryazhSmQuery(sQueryBase);
    sSpryazhSmQuery += L", no_aspect_pair ";
    sSpryazhSmQuery += L"FROM headword INNER JOIN spryazh_sm_headwords ON headword.id = spryazh_sm_headwords.headword_id ";
    sSpryazhSmQuery += L"INNER JOIN descriptor ON spryazh_sm_headwords.ref_descriptor_id = descriptor.id ";
    sSpryazhSmQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id ";
    sSpryazhSmQuery += L"WHERE headword.source = \"";
    sSpryazhSmQuery += sSource;
    sSpryazhSmQuery += L"\"";

    uiQueryHandle = 0;
    rc = eQueryDb(sSpryazhSmQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    while (H_NO_ERROR == rc)
    {
        shared_ptr<CLexeme> pLexeme = make_shared<CLexeme>(this);
        if (nullptr == pLexeme)
        {
            return H_ERROR_POINTER;
        }

        auto bSpryazhSm = true;
        rc = eGetSingleRow(pLexeme->stGetPropertiesForWriteAccess(), uiQueryHandle, bSpryazhSm);
        if (H_NO_ERROR == rc)
        {
            bFound = true;
            m_vecLexemes.push_back(pLexeme);
        }
        if (H_NO_ERROR != rc && H_NO_MORE != rc)
        {
            Clear();
            pLexeme = nullptr;
            ERROR_LOG(L"Error retrieving lexeme data.");
            return rc;
        }
    }
    m_pDb->Finalize(uiQueryHandle);

    for (auto& pLexeme : m_vecLexemes)
    {
        if (pLexeme->stGetProperties().llSecondPartId > 0)
        {
            shared_ptr<CLexeme> spSecondLexeme = make_shared<CLexeme>(this);
            rc = eGetSecondPart(pLexeme->stGetProperties().llSecondPartId, spSecondLexeme);
            pLexeme->SetSecondPart(spSecondLexeme);
        }
    }

    if (H_NO_MORE == rc && !bFound)
    {
        return H_FALSE;
    }

    return rc;

}   //  eGetLexemesByInitialForm()

void CDictionary::Clear()
{
    m_vecLexemes.clear();
}

ET_ReturnCode CDictionary::Clear(ILexeme * pLexeme)
{
    vector<shared_ptr<CLexeme>>::iterator itL = m_vecLexemes.begin();
    for (; itL != m_vecLexemes.end(); ++itL)
    {
        if ((*itL).get() == pLexeme)
        {
            m_vecLexemes.erase(itL);
            return H_NO_ERROR;
        }
    }

    return H_ERROR_UNEXPECTED;
}

ET_ReturnCode CDictionary::eCreateLexemeEnumerator(ILexemeEnumerator*& le)
{
    le = new CLexemeEnumerator(this);
    if (!le)
    {
        ERROR_LOG(L"Error retrieving ILexemeEnumerator.");
        return H_ERROR_POINTER;
    }

    return H_NO_ERROR;
}

void CDictionary::DeleteLexemeEnumerator(ILexemeEnumerator* pLe)
{
    delete pLe;
}

ET_ReturnCode CDictionary::eGetParser(IParser *& pParser)
{
    if (NULL == m_pDb)
    {
        m_pParser = NULL;
        ERROR_LOG(L"Error retrieving IParser interface.");
        return H_ERROR_POINTER;
    }

    if (NULL == m_pParser)
    {
        m_pParser = make_shared<CParser>(this);
    }

    m_pParser->SetDb(m_pDb);

    pParser = m_pParser.get();

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eGetAnalytics(IAnalytics*& pAnalytics)
{
    if (NULL == m_pDb)
    {
        m_pAnalytics = NULL;
        ERROR_LOG(L"Error retrieving IAnalytics interface.");
        return H_ERROR_POINTER;
    }

    if (NULL == m_pParser)
    {
        m_pParser = make_unique<CParser>(this);
        m_pParser->SetDb(m_pDb);
    }

    if (NULL == m_pAnalytics)
    {
        m_pAnalytics = make_unique<CAnalytics>(m_pDb, m_pParser);
        pAnalytics = m_pAnalytics.get();
    }

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eGetVerifier(IVerifier*& pVerifier)
{
    if (NULL == m_pDb)
    {
        m_pVerifier = NULL;
        ERROR_LOG(L"Error retrieving IVerifier interface.");
        return H_ERROR_POINTER;
    }

    if (NULL == m_pVerifier)
    {
        m_pVerifier = make_shared<CVerifier>(this);
    }

    pVerifier = m_pVerifier.get();

    return H_NO_ERROR;
}

#ifdef WIN32
ET_ReturnCode CDictionary::eExportTestData(CEString& sPath, PROGRESS_CALLBACK_CLR pProgressCallback)
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

ET_ReturnCode CDictionary::eImportTestData(CEString& sPath, PROGRESS_CALLBACK_CLR pProgressCallback)
{
    if (NULL == m_pDb)
    {
        ERROR_LOG(L"Unable to obtain database handle.");
        return H_ERROR_POINTER;
    }

    CEString sQuery = L"DELETE FROM test_data_stress";
    m_pDb->Delete(sQuery);
    sQuery = L"DELETE FROM test_data";
    m_pDb->Delete(sQuery);

    bool bRet = true;

    vector<CEString> vecTables;
    vecTables.push_back(L"test_data");
    vecTables.push_back(L"test_data_stress");

    try
    {
        bool bAutoincrement = false;
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

//    ET_ReturnCode rc = ePopulateHashToDescriptorTable(pProgressCallback);

    pProgressCallback(100, true);

    return H_NO_ERROR;

}       //  eImportTestData()
#endif

CSqlite * CDictionary::pGetDbHandle()
{
    if (m_pDb)
    {
        return m_pDb.get();
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
//    m_pDb->BeginTransaction();

    vector<shared_ptr<CLexeme>>::iterator itLexeme = m_vecLexemes.begin();
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
            ET_ReturnCode eRet = (*itLexeme)->eSaveStemsToDb();
            if (H_NO_ERROR != eRet)
            {
                ERROR_LOG(L"Error saving stems to database.");
            }

            (*itLexeme)->eAssignStemIds();

            eRet = (*itLexeme)->eSaveWordFormsToDb();
            if (H_NO_ERROR != eRet)
            {
                ERROR_LOG(L"Error saving word forms to database.");
            }
        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
            //            return H_EXCEPTION;
        }
    }       //  for (int iRow = 0; iRow < iLexemes; ++iRow)

    return (H_NO_MORE == rc) ? H_NO_ERROR : rc;

}    //  eGenerateFormsForSelectedLexemes()

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
//    CLogger::pGetInstance()->Flush();
    rc = ePopulateWordFormDataTables();
//    CLogger::pGetInstance()->Flush();

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
    sQuery += L"FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id;";
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    //int iRows = 0;

    cout << endl << endl << "****      POPULATING STEMS TABLE" << endl << endl;

    m_pDb->BeginTransaction();

//    int iPercentDone = 0;
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
                CEString sMsg(L"Error generating paradigm for ");
                sMsg += lexeme.sSourceForm();
                ERROR_LOG(sMsg);
            }

            rc = lexeme.eSaveStemsToDb();
            if (H_NO_ERROR != rc)
            {
                CEString sMsg(L"Error saving stems for ");
                sMsg += lexeme.sSourceForm();
                ERROR_LOG(sMsg);
            }
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

            cout << right << setw(7) << iRow << " rows" << endl;
        }
    }       //  for (...)

    m_pDb->CommitTransaction();

    CEString sSpryazhSmQuery(sQueryBase);
    sSpryazhSmQuery += L"FROM headword INNER JOIN spryazh_sm_headwords ON headword.id = spryazh_sm_headwords.headword_id ";
    sSpryazhSmQuery += L"INNER JOIN descriptor ON spryazh_sm_headwords.ref_descriptor_id = descriptor.id ";
    sSpryazhSmQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id ";
    uint64_t uiSpryazhSmQueryHandle = 0;
    rc = eQueryDb(sSpryazhSmQuery, uiSpryazhSmQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    //iRows = 0;

    std::cout << endl << endl << "Processing spryazh. sm. verbs... ";

    m_pDb->BeginTransaction();

    bMoreData = true;
    for (int iRow = 0; bMoreData; ++iRow)
    {
        CLexeme lexeme(this);
        try
        {
            auto bSpryazhSm = true;
            rc = eGetSingleRow(lexeme.stGetPropertiesForWriteAccess(), uiSpryazhSmQueryHandle, bSpryazhSm);
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
                CEString sMsg(L"Error generating paradigm for ");
                sMsg += lexeme.sSourceForm();
                ERROR_LOG(sMsg);
            }

            rc = lexeme.eSaveStemsToDb();
            if (H_NO_ERROR != rc)
            {
                CEString sMsg(L"Error saving stems for ");
                sMsg += lexeme.sSourceForm();
                ERROR_LOG(sMsg);
            }
        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
            //            return H_EXCEPTION;
        }

    }       //  for (...)

    m_pDb->CommitTransaction();

    std::cout << "done." << endl << endl;

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
    sQuery += L"FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id;";
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    cout << endl << endl << "****      GENERATING WORDFORMS" << endl << endl;

    std::clock_t totalTime = 0;

//    int iPercentDone = 0;
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
                        sMsg += pWf->m_sStem;
                        sMsg += L'"';
                        sMsg += L" lexeme = " + lexeme.sSourceForm();
                        ERROR_LOG(sMsg);

                        continue;
                    }
                    pWf->bSaveToDb();
                }
            }

            m_pDb->CommitTransaction();

            totalTime += clock() - dbProcTime;
            double dDuration = (clock() - dbProcTime) / (double)CLOCKS_PER_SEC;
            CEString sDurationMsg(L"Row ");
            sDurationMsg += CEString::sToString(iRow);
            sDurationMsg += L"; ";
            sDurationMsg += CEString::sToString(dDuration);
            sDurationMsg += L" seconds total";
            ERROR_LOG(sDurationMsg);
            cout << "Row " << right << setw(6) << iRow << ", " << fixed << setprecision(3) << dDuration << " seconds" << endl;

            vecLexemes.clear();

        }       //  if (iRow > 0 && (iRow % 1000 == 0 || !bMoreData))

    }       //  for ...

    std::cout << endl << endl << "Processing spryazh. sm. verbs... ";

    m_pDb->BeginTransaction();

    CEString sSpryazhSmQuery(sQueryBase);
    sSpryazhSmQuery += L"FROM headword INNER JOIN spryazh_sm_headwords ON headword.id = spryazh_sm_headwords.headword_id ";
    sSpryazhSmQuery += L"INNER JOIN descriptor ON spryazh_sm_headwords.ref_descriptor_id = descriptor.id ";
    sSpryazhSmQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id ";
    uint64_t uiSpryazhSmQueryHandle = 0;
    rc = eQueryDb(sSpryazhSmQuery, uiSpryazhSmQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    bMoreData = true;

    for (int iRow = 0; bMoreData; ++iRow)
    {
        CLexeme lexeme(this);
        try
        {
            auto bSpryazhSm = true;
            rc = eGetSingleRow(lexeme.stGetPropertiesForWriteAccess(), uiSpryazhSmQueryHandle, bSpryazhSm);
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
    }

    for (vector<CLexeme>::iterator itLexeme = vecLexemes.begin(); itLexeme != vecLexemes.end(); ++itLexeme)
    {
        for (unsigned int uiWf = 0; uiWf < (*itLexeme).uiTotalWordForms(); ++uiWf)
        {
            CWordForm* pWf = NULL;
            rc = (*itLexeme).eGetWordForm(uiWf, pWf);

            ERROR_LOG(pWf->sWordForm());

            if (rc != H_NO_ERROR)
            {
                continue;
            }

//            if (!pWf->bIrregular() && 0 == pWf->m_llStemId)
//            {
//                CEString sMsg(L"Unable to find stem id for \"");
//                sMsg += pWf->m_sStem;
//                sMsg += L'"';
//                sMsg += L" lexeme = " + itLexeme->sSourceForm();
//                ERROR_LOG(sMsg);
//                continue;
//            }

            pWf->bSaveToDb();

            if (pWf->bIrregular())
            {
                pWf->bSaveIrregularForm();
            }
        }
    }

    m_pDb->CommitTransaction();

    std::cout << endl << "Done." << endl << endl;

    return H_NO_ERROR;

}       //  ePopulateWordFormDataTables()

ET_ReturnCode CDictionary::ePopulateHashToDescriptorTable(PROGRESS_CALLBACK_CLR pProgressCLR, PROGRESS_CALLBACK_PYTHON pProgressPython)
{
    if (!m_pDb)
    {
        return H_ERROR_DB;
    }

    ET_ReturnCode rc = H_NO_ERROR;

    int64_t llCount = 0;
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
    sQuery += L"FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id;";
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    map<CEString, StLexemeProperties> mapHashToProperties;

    std::clock_t totalTime = 0;
//    int iPercentDone = 0;
    bool bMoreData = true;
    for (int iRow = 0; bMoreData; ++iRow)
    {
        clock_t dbProcTime = clock();

        CLexeme lexeme(this);
        try
        {
            rc = eGetSingleRow(lexeme.stGetPropertiesForWriteAccess(), uiQueryHandle);
            if (rc != H_NO_ERROR)
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

            CEString sHash = lexeme.sHash();
            mapHashToProperties[sHash] = lexeme.stGetProperties();

            if (!bMoreData || (iRow > 0 && (iRow % 1000 == 0)))
            { 
                m_pDb->BeginTransaction();

                for (auto itLex = mapHashToProperties.begin(); itLex != mapHashToProperties.end(); ++itLex)
                {
                    uint64_t uiInsertHandle = 0;

                    try
                    {
                        if (0 == uiInsertHandle)
                        {
                            sqlite3_stmt* pStmt = nullptr;
                            m_pDb->uiPrepareForInsert(L"lexeme_hash_to_descriptor", 3, pStmt, false);
                            uiInsertHandle = (uint64_t)pStmt;
                        }
                        m_pDb->Bind(1, (*itLex).first, uiInsertHandle);
                        m_pDb->Bind(2, (uint64_t)(*itLex).second.llDescriptorId, uiInsertHandle);
                        m_pDb->Bind(3, (uint64_t)(*itLex).second.llInflectionId, uiInsertHandle);
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
                }       //  for (auto itLex = mapHashToProperties.begin(); ...)

                m_pDb->CommitTransaction();

                mapHashToProperties.clear();
            
            }       //  if (!bMoreData || (iRow > 0 && (iRow % 100 == 0)))
        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
        }

        if (iRow > 0 && (iRow % 1000 == 0 || !bMoreData))
        {
            int iPercentDone = min(100, (int)(100 * (double)iRow / (double)llCount));

            totalTime += clock() - dbProcTime;
            dbProcTime = clock();
            double dDuration = totalTime / (double)CLOCKS_PER_SEC;
            if (pProgressPython)
            {
                pProgressPython(iPercentDone, false, iRow, dDuration);
            }

            if (pProgressCLR)
            {
                ERROR_LOG(L"CLR interface currentlynot supported.");
            }

//            CEString sMsg(L"------------ Saving lexeme hashes ------------- \r\n");
//            sMsg += CEString::sToString(iRow);
            //CLogger::bWriteLog(wstring(xxxx));
//            ERROR_LOG(sMsg)
//                std::cout << string(sMsg.stl_sToUtf8()) << std::endl;

//            CEString sDurationMsg(L"Row ");
//            sDurationMsg += CEString::sToString(iRow);
//            sDurationMsg += L"; ";
//            sDurationMsg += CEString::sToString(dDuration);
//            sDurationMsg += L" seconds total";
            //            CLogger::bWriteLog(wstring(sDurationMsg));
//            std::cout << string(sDurationMsg.stl_sToUtf8()) << std::endl;
//            totalTime = 0;

        }
    }       //  for ...

    return H_NO_ERROR;

}       //  ePopulateHashToDescriptorTable()

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

int CDictionary::nLexemesFound()
{
    return (int)m_vecLexemes.size();
}

//
// Helpers
//
ET_ReturnCode CDictionary::eQueryDb(const CEString& sSelect, uint64_t& uiQueryHandle)
{
    ET_ReturnCode rc = H_NO_ERROR;

    if (NULL == m_pDb)
    {
        assert(0);
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

ET_ReturnCode CDictionary::eGetSingleRow(StLexemeProperties& stProperties, uint64_t uiQueryHandle, bool bIsSpryazhSm)
{
    if (NULL == m_pDb)
    {
        assert(0);
        ERROR_LOG(L"DB pointer is NULL.");
        return H_ERROR_POINTER;
    }

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        if (m_pDb->bGetRow(uiQueryHandle))
        {
/*
        static CEString sQueryBase(L"SELECT headword.source, headword.id, headword.comment, headword.variant_comment, headword.plural_of, 
                                                        0             1               2                     3                      4
                                            headword.usage, headword.variant, headword.see_ref, headword.back_ref, headword.source_entry_id, 
                                                       5                6                 7                  8                    9          
                                            headword.spryazh_sm, headword.second_part, descriptor.id, descriptor.graphic_stem, descriptor.second_part_id, 
                                                       10               11                       12                      13                      14
                                            descriptor.is_variant, descriptor.main_symbol, descriptor.part_of_speech, descriptor.is_plural_of, descriptor.is_intransitive, 
                                                           15                        16                        17                         18                    19
                                            descriptor.is_reflexive, descriptor.main_symbol_plural_of, descriptor.alt_main_symbol, descriptor.inflection_type,
                                                             20                        21                                 22                         23
                                            descriptor.comment, descriptor.alt_main_symbol_comment, descriptor.alt_inflection_comment, 
                                                          24                          25                                26
                                            descriptor.verb_stem_alternation, descriptor.part_past_pass_zhd, descriptor.section, descriptor.no_comparative, 
                                                                 27                             28                         29                      30
                                            descriptor.no_long_forms, descriptor.assumed_forms, descriptor.yo_alternation, descriptor.o_alternation, 
                                                             31                         32                        33                         34  
                                            descriptor.second_genitive, descriptor.is_impersonal, descriptor.is_iterative, descriptor.has_aspect_pair, 
                                                             35                          36                        37                      38
                                            descriptor.has_difficult_forms, descriptor.has_missing_forms, descriptor.has_irregular_forms, 
                                                               39                            40                             41
                                            descriptor.irregular_forms_lead_comment, descriptor.restricted_contexts, descriptor.contexts, descriptor.cognate, 
                                                                      42                                 43                        44                   45
                                            descriptor.trailing_comment, inflection.id, inflection.is_primary, inflection.inflection_type, inflection.accent_type1, 
                                                               46                   47                  48                      49                          50
                                            inflection.accent_type2, inflection.short_form_restrictions, inflection.past_part_restrictions, inflection.no_short_form, 
                                                            51                           52                                   53                             54
                                            no_past_part, fleeting_vowel, stem_augment, is_edited);
                                                            55                      56               
*/

            m_pDb->GetData(0, stProperties.sSourceForm, uiQueryHandle);                         //  0 source
            uint64_t uiHeadwordId = 0;
            m_pDb->GetData(1, uiHeadwordId, uiQueryHandle);
            stProperties.llHeadwordId = uiHeadwordId;                                           //  1 id [headword]
            m_pDb->GetData(2, stProperties.sHeadwordComment, uiQueryHandle);                    //  2 comment
            m_pDb->GetData(3, stProperties.sHeadwordVariantComment, uiQueryHandle);             //  3 variant_comment
            m_pDb->GetData(4, stProperties.sPluralOf, uiQueryHandle);                           //  4 plural_of
            m_pDb->GetData(5, stProperties.sUsage, uiQueryHandle);                              //  5 usage
            m_pDb->GetData(6, stProperties.sHeadwordVariant, uiQueryHandle);                    //  6 variant
            m_pDb->GetData(7, stProperties.sSeeRef, uiQueryHandle);                             //  7 see_ref
            m_pDb->GetData(8, stProperties.sBackRef, uiQueryHandle);                            //  8 back_ref
            uint64_t uiSourceEntryId = 0;
            m_pDb->GetData(9, uiSourceEntryId, uiQueryHandle);                                  //  9 source_entry_id
            stProperties.llSourceEntryId = uiSourceEntryId;
            m_pDb->GetData(10, stProperties.bSpryazhSm, uiQueryHandle);                         // 10 spryazh_sm
            m_pDb->GetData(11, stProperties.bSecondPart, uiQueryHandle);                        // 11 second_part
            uint64_t uiDescriptorId = 0;
            m_pDb->GetData(12, uiDescriptorId, uiQueryHandle);                                  // 12 id [descriptor]
            stProperties.llDescriptorId = uiDescriptorId;
            m_pDb->GetData(13, stProperties.sGraphicStem, uiQueryHandle);                       // 13 graphic_stem
            uint64_t uiSecondPartId = 0;
            m_pDb->GetData(14, uiSecondPartId, uiQueryHandle);                                  // 14 second_part_id
            stProperties.llSecondPartId = uiSecondPartId;
            m_pDb->GetData(15, stProperties.bIsVariant, uiQueryHandle);                         // 15 is_variant
            m_pDb->GetData(16, stProperties.sMainSymbol, uiQueryHandle);                        // 16 main_symbol
            int iPartOfSpeech = 0;
            m_pDb->GetData(17, iPartOfSpeech, uiQueryHandle);                                   // 17 part_of_speech
            stProperties.ePartOfSpeech = (ET_PartOfSpeech)iPartOfSpeech;
            m_pDb->GetData(18, stProperties.bIsPluralOf, uiQueryHandle);                        // 18 is_plural_of
            bool bIntransitive = false;
            m_pDb->GetData(19, bIntransitive, uiQueryHandle);                                   // 19 is_intransitive; "нп" == !bTransitive
            stProperties.bTransitive = !bIntransitive;
            bool bReflexive = false;
            m_pDb->GetData(20, bReflexive, uiQueryHandle);                                      // 20 is_reflexive
            if (POS_VERB == stProperties.ePartOfSpeech)
            {
                stProperties.eReflexive = bReflexive ? REFL_YES : REFL_NO;
            }
            else
            {
                stProperties.eReflexive = REFL_UNDEFINED;
            }
            m_pDb->GetData(21, stProperties.sMainSymbolPluralOf, uiQueryHandle);                // 21 main_symbol_plural_of
            m_pDb->GetData(22, stProperties.sAltMainSymbol, uiQueryHandle);                     // 22 alt_main_symbol
            m_pDb->GetData(23, stProperties.sInflectionType, uiQueryHandle);                    // 23 inflection_type
            m_pDb->GetData(24, stProperties.sComment, uiQueryHandle);                           // 24 comment
            m_pDb->GetData(25, stProperties.sAltMainSymbolComment, uiQueryHandle);              // 25 alt_main_symbol_comment
            m_pDb->GetData(26, stProperties.sAltInflectionComment, uiQueryHandle);              // 26 alt_inflection_comment
            m_pDb->GetData(27, stProperties.sVerbStemAlternation, uiQueryHandle);               // 27 verb_stem_alternation
            m_pDb->GetData(28, stProperties.bPartPastPassZhd, uiQueryHandle);                   // 28 part_past_pass_zhd
            m_pDb->GetData(29, stProperties.iSection, uiQueryHandle);                           // 29 section
            m_pDb->GetData(30, stProperties.bNoComparative, uiQueryHandle);                     // 30 no_comparative
            m_pDb->GetData(31, stProperties.bNoLongForms, uiQueryHandle);                       // 31 no_long_forms
            m_pDb->GetData(32, stProperties.bAssumedForms, uiQueryHandle);                      // 32 assumed_forms
            m_pDb->GetData(33, stProperties.bYoAlternation, uiQueryHandle);                     // 33 yo_alternation
            m_pDb->GetData(34, stProperties.bOAlternation, uiQueryHandle);                      // 34 o_alternation
            m_pDb->GetData(35, stProperties.bSecondGenitive, uiQueryHandle);                    // 35 second_genitive
            m_pDb->GetData(36, stProperties.bIsImpersonal, uiQueryHandle);                      // 36 is_impersonal
            m_pDb->GetData(37, stProperties.bIsIterative, uiQueryHandle);                       // 37 is_iterative
            m_pDb->GetData(38, stProperties.bHasAspectPair, uiQueryHandle);                     // 38 has_aspect_pair
            m_pDb->GetData(39, stProperties.bHasDifficultForms, uiQueryHandle);                 // 39 has_difficult_forms
            m_pDb->GetData(40, stProperties.bHasMissingForms, uiQueryHandle);                   // 40 has_missing_forms
            m_pDb->GetData(41, stProperties.bHasIrregularForms, uiQueryHandle);                 // 41 has_irregular_forms
            m_pDb->GetData(42, stProperties.sIrregularFormsLeadComment, uiQueryHandle);         // 42 irregular_forms_lead_comment
            m_pDb->GetData(43, stProperties.sRestrictedContexts, uiQueryHandle);                // 43 restricted_contexts
            m_pDb->GetData(44, stProperties.sContexts, uiQueryHandle);                          // 44 contexts
            m_pDb->GetData(45, stProperties.sCognate, uiQueryHandle);                           // 45 cognate
            m_pDb->GetData(46, stProperties.sTrailingComment, uiQueryHandle);                   // 46 trailing_comment
            uint64_t uiInflectionId = -1;
            m_pDb->GetData(47, uiInflectionId, uiQueryHandle);                                  // 47 inflection.id
            stProperties.llInflectionId = uiInflectionId;
            m_pDb->GetData(48, stProperties.bPrimaryInflectionGroup, uiQueryHandle);            // 48 is_primary
            m_pDb->GetData(49, stProperties.iType, uiQueryHandle);                              // 49 inflection_type
            int iAccentType1 = 0;
            m_pDb->GetData(50, iAccentType1, uiQueryHandle);                                    // 50 accent_type1
            stProperties.eAccentType1 = (ET_AccentType)iAccentType1;
            int iAccentType2 = 0;
            m_pDb->GetData(51, iAccentType2, uiQueryHandle);                                    // 51 accent_type2
            stProperties.eAccentType2 = (ET_AccentType)iAccentType2;
            m_pDb->GetData(52, stProperties.bShortFormsRestricted, uiQueryHandle);              // 52 short_form_restrictions
            if (L"м" == stProperties.sMainSymbol)
            {
                stProperties.bShortFormsRestricted = false;     // nominalized adjectives: text parser may copy that from source
            }
            m_pDb->GetData(53, stProperties.bPastParticipleRestricted, uiQueryHandle);          // 53 past_part_restrictions
            m_pDb->GetData(54, stProperties.bShortFormsIncomplete, uiQueryHandle);              // 54 no_short_form
            if (L"м" == stProperties.sMainSymbol)
            {
                stProperties.bShortFormsIncomplete = false;     // nominalized adjectives: text parser may copy that from source
            }
            m_pDb->GetData(55, stProperties.bNoPassivePastParticiple, uiQueryHandle);           // 55 no_past_part
            m_pDb->GetData(56, stProperties.bFleetingVowel, uiQueryHandle);                     // 56 fleeting_vowel

            m_pDb->GetData(57, stProperties.iStemAugment, uiQueryHandle);                       // 57 stem_augment

            if (bIsSpryazhSm)
            {
                m_pDb->GetData(58, stProperties.bSpryazhSmNoAspectPair, uiQueryHandle);         // 58 no_aspect_pair (optional)
            }

            CEString sStressQuery(L"SELECT stress_position, is_primary FROM stress WHERE is_variant = \"");
            sStressQuery += L"0\" AND headword_id = ";
            sStressQuery += CEString::sToString(stProperties.llHeadwordId);

            uint64_t uiStressHandle = m_pDb->uiPrepareForSelect(sStressQuery);
            while (m_pDb->bGetRow(uiStressHandle))
            {
                int iPos = -1;
                bool bPrimary = false;
                m_pDb->GetData(0, iPos, uiStressHandle);
                m_pDb->GetData(1, bPrimary, uiStressHandle);
                stProperties.sSourceForm.SetVowels(CEString::g_szRusVowels);
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
                CEString sHwCommentQuery(L"SELECT comment FROM headword WHERE id = ");
                sHwCommentQuery += CEString::sToString(stProperties.llHeadwordId);

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

                if (!stProperties.bIsUnstressed && !(POS_PARTICLE == stProperties.ePartOfSpeech) &&
                    1 == stProperties.sSourceForm.uiGetNumOfSyllables())
                {
                    assert(1 == stProperties.sSourceForm.uiGetNumOfSyllables());
                    stProperties.vecSourceStressPos.push_back(0);
                }
            }

            CEString sHomonymsQuery(L"SELECT homonym_number FROM homonyms WHERE headword_id = ");
            sHomonymsQuery += CEString::sToString(stProperties.llHeadwordId);

            uint64_t uiHomonymsHandle = m_pDb->uiPrepareForSelect(sHomonymsQuery);
            while (m_pDb->bGetRow(uiHomonymsHandle))
            {
                int iHomonym = -1;
                m_pDb->GetData(0, iHomonym, uiHomonymsHandle);
                stProperties.vecHomonyms.push_back(iHomonym);
            }
            m_pDb->Finalize(uiHomonymsHandle);

            CEString sDeviationQuery(L"SELECT deviation_type, is_optional FROM common_deviation WHERE inflection_id = ");
            sDeviationQuery += CEString::sToString(stProperties.llInflectionId);

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
                rc = eGetP2Data(stProperties.llDescriptorId, stProperties);
                if (H_NO_ERROR != rc)
                {
                    ERROR_LOG(L"hGetP2Data() failed.");
                    return rc;
                }
            }

            if (POS_VERB == stProperties.ePartOfSpeech)
            {
                if (stProperties.bHasAspectPair)
                {
                    CEString sAspectPairQuery(L"SELECT type, data, is_variant, comment FROM aspect_pair WHERE descriptor_id = ");
                    sAspectPairQuery += CEString::sToString(stProperties.llDescriptorId);

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
        rc = H_EXCEPTION;
    }

    return rc;

}    //  eGetSingleRow()

ET_ReturnCode CDictionary::eGetP2Data(int64_t llLexemeId, StLexemeProperties& stProperties)
{
    if (NULL == m_pDb)
    {
        assert(0);
        ERROR_LOG(L"DB pointer is NULL.");
        return H_ERROR_POINTER;
    }

    try
    {
        CEString sQuery(L"SELECT is_optional, preposition FROM second_locative WHERE descriptor_id = ");
        sQuery += CEString::sToString(llLexemeId);
        sQuery += L";";

        uint64_t uiHandle = m_pDb->uiPrepareForSelect(sQuery);

        stProperties.bSecondPrepositional = false;

        if (m_pDb->bGetRow(uiHandle))
        {
            stProperties.bSecondPrepositional = true;
            m_pDb->GetData(0, stProperties.bSecondPrepositionalOptional, uiHandle);
            m_pDb->GetData(1, stProperties.sP2Preposition, uiHandle);
        }

        if (m_pDb->bGetRow(uiHandle))
        {
            assert(0);
            CEString sMsg(L"More than one P2 record for ");
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

}   // eGetP2Data (...)

//static CEString sHeadwordInsertQuery(L"INSERT source, plural_of, comment, usage, variant, variant_comment, see_ref, back_ref ");
//static CEString sHomonyms(L"INSERT headword_id, homonym_number, is_variant");
//static CEString sStressInsertQuery(L"INSERT headword_id, stress_position, is_primary, is_variant");
/*
static CEString sDescriptorInsertQuery(L"INSERT word_id, graphic_stem, is_variant, main_symbol, part_of_speech, is_plural_of, \
                                        is_intransitive, is_reflexive, main_symbol_plural_of, alt_main_symbol, inflection_type, \
                                        comment, alt_main_symbol_comment, alt_inflection_comment, verb_stem_alternation, \
                                        part_past_pass_zhd, section, no_comparative, assumed_forms, yo_alternation, o_alternation, \
                                        second_genitive, is_impersonal, is_iterative, has_aspect_pair, has_difficulties, \
                                        difficult_and_missing_forms, has_irregular_forms, irregular_forms_lead_comment, \
                                        has_deficiencies, restricted_contexts, contexts, cognate, trailing_comment ");
*/
/*                                        
static CEString sInflectionInsertQuery(L"INSERT descriptor_id, is_primary, inflection_type, accent_type1, accent_type2, short_from_restrictions, \
                                        past_part_restrictions, no_short_forms, no_past_part, fleeting_vowel, stem_augment, inflected_parts ");
                                        */
ET_ReturnCode CDictionary::eDeleteLexeme(ILexeme * pLexeme)
{
    const StLexemeProperties& stProperties = pLexeme->stGetProperties();

    try
    {
        if (NULL == m_pDb)
        {
            assert(0);
            ERROR_LOG(L"DB pointer is NULL.");
            return H_ERROR_POINTER;
        }

        CEString sWordQuery = L"SELECT word_id FROM descriptor WHERE id = " + CEString::sToString(stProperties.llDescriptorId);
        m_pDb->PrepareForSelect(sWordQuery);
        bool bFound = false;
        int iWordId = -1;
        while (m_pDb->bGetRow())
        {
            if (bFound)
            {
                ERROR_LOG(L"Warning: multiple word ID's.");
                break;
            }
            m_pDb->GetData(0, iWordId);
            if (iWordId >= 0)
            {
                bFound = true;
            }
        }
        m_pDb->Finalize();

        CEString sInflectionQuery = L"SELECT id FROM inflection WHERE descriptor_id = " + CEString::sToString(stProperties.llDescriptorId);
        m_pDb->PrepareForSelect(sInflectionQuery);
        vector<long long> vecInflectionIds;
        while (m_pDb->bGetRow())
        {
            int64_t uiInflectionId = -1;
            m_pDb->GetData(0, uiInflectionId);
            vecInflectionIds.push_back((long long)uiInflectionId);
        }
        m_pDb->Finalize();

        for (auto llInflectionId : vecInflectionIds)
        {
            CEString sDeleteCommonDeviationQuery(L"DELETE FROM common_deviation WHERE inflection_id = " + CEString::sToString(llInflectionId));
            m_pDb->Exec(sDeleteCommonDeviationQuery);
        }

        vector<CEString> vecTables{ L"aspect_pair", L"missing_forms", L"difficult_forms", L"second_genitive", L"second_locative", L"inflection" };

        for (CEString sTable : vecTables)
        {
            CEString sDeleteStmt = L"DELETE FROM " + sTable + L" WHERE descriptor_id = ";
            sDeleteStmt += CEString::sToString(stProperties.llDescriptorId);
            sDeleteStmt += L";";
            m_pDb->Exec(sDeleteStmt);
        }

        CEString sSelectIrregularStmt = L"SELECT id FROM irregular_forms WHERE descriptor_id = " + 
        CEString::sToString(stProperties.llDescriptorId) + L";";
        m_pDb->PrepareForSelect(sSelectIrregularStmt);
        vector<int> vecIrregularFormIds;
        while (m_pDb->bGetRow())
        {
            int iIrregularFormId = -1;
            m_pDb->GetData(0, iIrregularFormId);
            vecIrregularFormIds.push_back(iIrregularFormId);
        }
        m_pDb->Finalize();

        for (auto sId : vecIrregularFormIds)
        {
            CEString sDeleteStmt = L"DELETE FROM irregular_stress WHERE id = ";
            sDeleteStmt += CEString::sToString(sId);
            sDeleteStmt += L";";
            m_pDb->Exec(sDeleteStmt);
        }

        CEString sDeleteDescriptorQuery(L"DELETE FROM descriptor WHERE id = " + CEString::sToString(stProperties.llDescriptorId));
        m_pDb->Exec(sDeleteDescriptorQuery);

        bool bDeleteHeadword = false;
        CEString sExistQuery = L"SELECT 1 FROM descriptor WHERE word_id = " + CEString::sToString(iWordId) + L";";
        m_pDb->PrepareForSelect(sExistQuery);
        if (!m_pDb->bGetRow())
        {
            bDeleteHeadword = true;
        }
        m_pDb->Finalize();

        if (bDeleteHeadword)
        {
            CEString sDeleteStmt = L"DELETE FROM stress WHERE headword_id = ";
            sDeleteStmt += CEString::sToString(iWordId);
            sDeleteStmt += L";";
            m_pDb->Exec(sDeleteStmt);

            sDeleteStmt = L"DELETE FROM homonyms WHERE headword_id = ";
            sDeleteStmt += CEString::sToString(iWordId);
            sDeleteStmt += L";";
            m_pDb->Exec(sDeleteStmt);

            sDeleteStmt = L"DELETE FROM headword WHERE id = ";
            sDeleteStmt += CEString::sToString(iWordId);
            sDeleteStmt += L";";
            m_pDb->Exec(sDeleteStmt);
        }
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}        //  eDeleteLexeme()

ET_ReturnCode CDictionary::eUpdateHeadword(ILexeme* pLexeme)
{
//    ET_ReturnCode eRet = H_NO_ERROR;

    StLexemeProperties& stProperties = pLexeme->stGetPropertiesForWriteAccess();
    if (stProperties.llHeadwordId < 0)
    {
        ERROR_LOG(L"Invalid headword ID");
        return H_ERROR_UNEXPECTED;
    }

    try
    {
        m_pDb->BeginTransaction();

        sqlite3_stmt* pStmt = nullptr;
        vector<CEString> vecColumns = { L"source", L"plural_of", L"comment", L"usage", L"variant",
            L"variant_comment", L"see_ref", L"back_ref", L"source_entry_id", L"spryazh_sm", L"second_part", 
            L"is_edited"};
        auto llUpdateHandle = m_pDb->uiPrepareForUpdate(L"headword", vecColumns, stProperties.llHeadwordId, pStmt);
        m_pDb->Bind(1, stProperties.sSourceForm, llUpdateHandle);
        m_pDb->Bind(2, stProperties.sPluralOf, llUpdateHandle);
        m_pDb->Bind(3, stProperties.sHeadwordComment, llUpdateHandle);
        m_pDb->Bind(4, stProperties.sUsage, llUpdateHandle);
        m_pDb->Bind(5, stProperties.sHeadwordVariant, llUpdateHandle);
        m_pDb->Bind(6, stProperties.sHeadwordVariantComment, llUpdateHandle);
        m_pDb->Bind(7, stProperties.sSeeRef, llUpdateHandle);
        m_pDb->Bind(8, stProperties.sBackRef, llUpdateHandle);
        uint64_t uiSourceEntryId = 0;
        m_pDb->Bind(9, uiSourceEntryId, llUpdateHandle);
        stProperties.llSourceEntryId = uiSourceEntryId;
        m_pDb->Bind(10, stProperties.bSpryazhSm, llUpdateHandle);
        m_pDb->Bind(11, stProperties.bSecondPart, llUpdateHandle);
        m_pDb->Bind(12, true, llUpdateHandle);                   // is_edited

        m_pDb->UpdateRow(llUpdateHandle);
        m_pDb->Finalize(llUpdateHandle);

        m_pDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        m_pDb->RollbackTransaction();

        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eUpdateHeadword()

ET_ReturnCode CDictionary::eSaveNewHeadword(ILexeme* pLexeme)
{
//    ET_ReturnCode eRet = H_NO_ERROR;

    StLexemeProperties& stProperties = pLexeme->stGetPropertiesForWriteAccess();
    if (stProperties.llHeadwordId >= 0)
    {
        ERROR_LOG(L"Invalid headword ID");
        return H_ERROR_UNEXPECTED;
    }

    try
    {
        m_pDb->BeginTransaction();

        sqlite3_stmt* pStmt = nullptr;
        m_pDb->uiPrepareForInsert(L"headword", 12, pStmt);
        auto llInsertHandle = (unsigned long long)pStmt;

        m_pDb->Bind(1, stProperties.sSourceForm, llInsertHandle);
        m_pDb->Bind(2, stProperties.sPluralOf, llInsertHandle);
        m_pDb->Bind(3, stProperties.sHeadwordComment, llInsertHandle);
        m_pDb->Bind(4, stProperties.sUsage, llInsertHandle);
        m_pDb->Bind(5, stProperties.sHeadwordVariant, llInsertHandle);
        m_pDb->Bind(6, stProperties.sHeadwordVariantComment, llInsertHandle);
        m_pDb->Bind(7, stProperties.sSeeRef, llInsertHandle);
        m_pDb->Bind(8, stProperties.sBackRef, llInsertHandle);
        uint64_t uiSourceEntryId = 0;
        m_pDb->Bind(9, uiSourceEntryId, llInsertHandle);
        stProperties.llSourceEntryId = uiSourceEntryId;
        m_pDb->Bind(10, stProperties.bSpryazhSm, llInsertHandle);
        m_pDb->Bind(11, stProperties.bSecondPart, llInsertHandle);
        m_pDb->Bind(12, true, llInsertHandle);                   // is_edited

        m_pDb->InsertRow(llInsertHandle);

        stProperties.llHeadwordId = m_pDb->llGetLastKey();

        m_pDb->Finalize(llInsertHandle);

        if (stProperties.vecHomonyms.size() > 0)
        {
            sqlite3_stmt* pStmt = nullptr;
            m_pDb->uiPrepareForInsertOrReplace(L"homonyms", 4, pStmt);
            llInsertHandle = (unsigned long long)pStmt;
            for (auto iHomonym : stProperties.vecHomonyms)
            {
                int64_t iHeadwordId = stProperties.llHeadwordId;
                m_pDb->Bind(1, iHeadwordId, llInsertHandle);
                m_pDb->Bind(2, iHomonym, llInsertHandle);
                m_pDb->Bind(3, false, llInsertHandle);      // is_variant
                m_pDb->Bind(4, true, llInsertHandle);       // is_edited
            }

            m_pDb->InsertRow(llInsertHandle);
            m_pDb->Finalize(llInsertHandle);
        }

        stProperties.sSourceForm.SetVowels(CEString::g_szRusVowels);

        llInsertHandle = 0;
        if (stProperties.vecSourceStressPos.size() > 0)
        {
            pStmt = nullptr;
            m_pDb->uiPrepareForInsert(L"stress", 5, pStmt);
            llInsertHandle = (unsigned long long)pStmt;

            for (auto iStressPos : stProperties.vecSourceStressPos)
            {
                int64_t iHeadwordId = stProperties.llHeadwordId;
                m_pDb->Bind(1, iHeadwordId, llInsertHandle);
                m_pDb->Bind(2, iStressPos, llInsertHandle);
                m_pDb->Bind(3, true, llInsertHandle);       // primary
                m_pDb->Bind(4, false, llInsertHandle);      // not a variant
                m_pDb->Bind(5, true, llInsertHandle);       // is_edited
                m_pDb->InsertRow(llInsertHandle);
            }

            for (auto iStressPos : stProperties.vecSecondaryStressPos)
            {
                int64_t iHeadwordId = stProperties.llHeadwordId;
                m_pDb->Bind(1, iHeadwordId, llInsertHandle);
                m_pDb->Bind(2, iStressPos, llInsertHandle);
                m_pDb->Bind(3, false, llInsertHandle);       // secondary
                m_pDb->Bind(4, false, llInsertHandle);       // not a variant
                m_pDb->Bind(5, true, llInsertHandle);        // is_edited
                m_pDb->InsertRow(llInsertHandle);
            }

            for (auto iStressPos : stProperties.vecSourceVariantStressPos)
            {
                int64_t iHeadwordId = stProperties.llHeadwordId;
                m_pDb->Bind(1, iHeadwordId, llInsertHandle);
                m_pDb->Bind(2, iStressPos, llInsertHandle);
                m_pDb->Bind(3, true, llInsertHandle);       // primary
                m_pDb->Bind(4, true, llInsertHandle);       // variant
                m_pDb->Bind(5, true, llInsertHandle);       // is_edited
                m_pDb->InsertRow(llInsertHandle);
            }

            for (auto iStressPos : stProperties.vecSecondaryVariantStressPos)
            {
                int64_t iHeadwordId = stProperties.llHeadwordId;
                m_pDb->Bind(1, iHeadwordId, llInsertHandle);

                m_pDb->Bind(2, iStressPos, llInsertHandle);
                m_pDb->Bind(3, false, llInsertHandle);       // primary
                m_pDb->Bind(4, true, llInsertHandle);        // variant
                m_pDb->Bind(5, true, llInsertHandle);        // is_edited
                m_pDb->InsertRow(llInsertHandle);
            }

            m_pDb->Finalize(llInsertHandle);
        }
/*
        CEString sDescriptorUpdateQuery(L"UPDATE descriptor SET word_id = ");
        sDescriptorUpdateQuery += CEString::sToString(stProperties.llHeadwordId);
        sDescriptorUpdateQuery += L" WHERE id = ";
        sDescriptorUpdateQuery += CEString::sToString(stProperties.llDescriptorId);
        m_pDb->Exec(sDescriptorUpdateQuery);
*/
        m_pDb->CommitTransaction();

    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        m_pDb->RollbackTransaction();
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveNewHeadword()

ET_ReturnCode CDictionary::eSaveHeadwordStress(ILexeme* pLexeme)
{
    StLexemeProperties& stProperties = pLexeme->stGetPropertiesForWriteAccess();
    if (stProperties.llHeadwordId < 0)
    {
        ERROR_LOG(L"Invalid headword ID");
        return H_ERROR_UNEXPECTED;
    }

    int64_t iHeadwordId = stProperties.llHeadwordId;

    try
    {
        m_pDb->BeginTransaction();

        CEString sDeleteStressQuery(L"DELETE FROM stress WHERE headword_id = ");
        sDeleteStressQuery += CEString::sToString(iHeadwordId);
        m_pDb->Exec(sDeleteStressQuery);

        if (stProperties.vecSourceStressPos.size() > 0)
        {
            sqlite3_stmt* pStmt = nullptr;
            m_pDb->uiPrepareForInsert(L"stress", 5, pStmt);
            auto llInsertHandle = (long long)pStmt;


            for (auto iStressPos : stProperties.vecSourceStressPos)
            {

                m_pDb->Bind(1, iHeadwordId, llInsertHandle);
                m_pDb->Bind(2, iStressPos, llInsertHandle);
                m_pDb->Bind(3, true, llInsertHandle);       // primary
                m_pDb->Bind(4, false, llInsertHandle);      // not a variant
                m_pDb->Bind(5, true, llInsertHandle);       // is_edited
                m_pDb->InsertRow(llInsertHandle);
            }

            for (auto iStressPos : stProperties.vecSecondaryStressPos)
            {
                m_pDb->Bind(1, iHeadwordId, llInsertHandle);
                m_pDb->Bind(2, iStressPos, llInsertHandle);
                m_pDb->Bind(3, false, llInsertHandle);       // secondary
                m_pDb->Bind(4, false, llInsertHandle);       // not a variant
                m_pDb->Bind(5, true, llInsertHandle);        // is_edited
                m_pDb->InsertRow(llInsertHandle);
            }

            for (auto iStressPos : stProperties.vecSourceVariantStressPos)
            {
                m_pDb->Bind(1, iHeadwordId, llInsertHandle);
                m_pDb->Bind(2, iStressPos, llInsertHandle);
                m_pDb->Bind(3, true, llInsertHandle);       // primary
                m_pDb->Bind(4, true, llInsertHandle);       // variant
                m_pDb->Bind(5, true, llInsertHandle);       // is_edited
                m_pDb->InsertRow(llInsertHandle);
            }

            for (auto iStressPos : stProperties.vecSecondaryVariantStressPos)
            {
                m_pDb->Bind(1, iHeadwordId, llInsertHandle);
                m_pDb->Bind(2, iStressPos, llInsertHandle);
                m_pDb->Bind(3, false, llInsertHandle);       // primary
                m_pDb->Bind(4, true, llInsertHandle);        // variant
                m_pDb->Bind(5, true, llInsertHandle);        // is_edited
                m_pDb->InsertRow(llInsertHandle);
            }

            m_pDb->Finalize(llInsertHandle);
        }

        m_pDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        m_pDb->RollbackTransaction();

        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveHeadwordStress()

ET_ReturnCode CDictionary::eSaveHomonyms(ILexeme* pLexeme)
{
//    ET_ReturnCode eRet = H_NO_ERROR;

    StLexemeProperties& stProperties = pLexeme->stGetPropertiesForWriteAccess();
    if (stProperties.llHeadwordId < 0)
    {
        ERROR_LOG(L"Invalid headword ID");
        return H_ERROR_UNEXPECTED;
    }

    int64_t iHeadwordId = stProperties.llHeadwordId;

    try
    {
        m_pDb->BeginTransaction();

        CEString sDeleteStressQuery(L"DELETE FROM stress WHERE headword_id = ");
        sDeleteStressQuery += CEString::sToString(iHeadwordId);
        m_pDb->Exec(sDeleteStressQuery);

        if (stProperties.vecHomonyms.size() > 0)
        {
            sqlite3_stmt* pStmt = nullptr;
            m_pDb->uiPrepareForInsertOrReplace(L"homonyms", 4, pStmt);
            auto llInsertHandle = (long long)pStmt;

            for (auto iHomonym : stProperties.vecHomonyms)
            {
                m_pDb->Bind(1, iHeadwordId, llInsertHandle);
                m_pDb->Bind(2, iHomonym, llInsertHandle);
                m_pDb->Bind(3, false, llInsertHandle);      // is_variant
                m_pDb->Bind(4, true, llInsertHandle);       // is_edited
            }

            m_pDb->InsertRow(llInsertHandle);
            m_pDb->Finalize(llInsertHandle);
        }

        m_pDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        m_pDb->RollbackTransaction();
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveHomonyms()

ET_ReturnCode CDictionary::eSaveAspectPairInfo(ILexeme * pLexeme)
{
//    ET_ReturnCode eRet = H_NO_ERROR;

    try
    {
        m_pDb->BeginTransaction();

        StLexemeProperties& stProperties = pLexeme->stGetPropertiesForWriteAccess();

        CEString sDeleteAspectPairQuery(L"DELETE FROM aspect_pair WHERE descriptor_id = ");
        sDeleteAspectPairQuery += CEString::sToString(stProperties.llDescriptorId);
        m_pDb->Exec(sDeleteAspectPairQuery);

        sqlite3_stmt* pStmt = nullptr;
        m_pDb->uiPrepareForInsert(L"aspect_pair", 6, pStmt);
        auto llInsertHandle = (unsigned long long)pStmt;

        auto iDescriptorId = (int64_t)stProperties.llDescriptorId;
        m_pDb->Bind(1, iDescriptorId, llInsertHandle);
        m_pDb->Bind(2, stProperties.iAspectPairType, llInsertHandle);
        m_pDb->Bind(3, stProperties.sAltAspectPairData, llInsertHandle);
        m_pDb->Bind(4, stProperties.sAltAspectPairComment, llInsertHandle);                          
        m_pDb->Bind(5, false, llInsertHandle);                  // is variant -- currently not supported by UI
        m_pDb->Bind(6, true, llInsertHandle);                   // is_edited

        m_pDb->InsertRow(llInsertHandle);
        m_pDb->Finalize(llInsertHandle);
        m_pDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveAspectPairInfo()

ET_ReturnCode CDictionary::eSaveP2Info(ILexeme * pLexeme)
{
//    ET_ReturnCode eRet = H_NO_ERROR;

    try
    {
        m_pDb->BeginTransaction();

        StLexemeProperties& stProperties = pLexeme->stGetPropertiesForWriteAccess();

        CEString sDeleteAspectPairQuery(L"DELETE FROM second_locative WHERE descriptor_id = ");
        sDeleteAspectPairQuery += CEString::sToString(stProperties.llDescriptorId);
        m_pDb->Exec(sDeleteAspectPairQuery);

        sqlite3_stmt* pStmt = nullptr;
        m_pDb->uiPrepareForInsert(L"second_locative", 4, pStmt);
        auto llInsertHandle = (unsigned long long)pStmt;

        int64_t iDescriptorId = stProperties.llDescriptorId;
        m_pDb->Bind(1, iDescriptorId, llInsertHandle);
        m_pDb->Bind(2, stProperties.bSecondPrepositionalOptional, llInsertHandle);
        m_pDb->Bind(3, stProperties.sP2Preposition, llInsertHandle);
        m_pDb->Bind(4, true, llInsertHandle);                   // is_edited

        m_pDb->InsertRow(llInsertHandle);
        m_pDb->Finalize(llInsertHandle);
        m_pDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eSaveCommonDeviation(ILexeme * pLexeme)
{
//    ET_ReturnCode eRet = H_NO_ERROR;

    try
    {
        m_pDb->BeginTransaction();

        StLexemeProperties& stProperties = pLexeme->stGetPropertiesForWriteAccess();

        if (stProperties.llInflectionId < 0)
        {
            ERROR_LOG(L"Illegal inflection key.");
            return H_ERROR_DB;
        }

        CEString sDeleteCommonDeviationQuery(L"DELETE FROM common_deviation WHERE inflection_id = ");
        sDeleteCommonDeviationQuery += CEString::sToString(stProperties.llInflectionId);
        m_pDb->Exec(sDeleteCommonDeviationQuery);

        for (auto& pairDeviation : stProperties.mapCommonDeviations)
        {
            sqlite3_stmt* pStmt = nullptr;
            m_pDb->uiPrepareForInsert(L"common_deviation", 4, pStmt);
            auto llInsertHandle = (unsigned long long)pStmt;

            int64_t iInflectionId = stProperties.llInflectionId;
            m_pDb->Bind(1, iInflectionId, llInsertHandle);
            m_pDb->Bind(2, pairDeviation.first, llInsertHandle);
            m_pDb->Bind(3, pairDeviation.second, llInsertHandle);
            m_pDb->Bind(4, true, llInsertHandle);                   // is_edited

            m_pDb->InsertRow(llInsertHandle);
            m_pDb->Finalize(llInsertHandle);
        }

        m_pDb->CommitTransaction();

    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveCommonDeviation()

ET_ReturnCode CDictionary::eSaveInflectionInfo(ILexeme * pLexeme)
{
//    ET_ReturnCode eRet = H_NO_ERROR;
//                                                                 1                  2                              3                        4                      5
//    CREATE TABLE inflection(id INTEGER PRIMARY KEY ASC, descriptor_id INTEGER, is_primary BOOLEAN DEFAULT(0), inflection_type INTEGER, accent_type1 INTEGER, accent_type2 INTEGER, 
//           6                                              7                                     8                                 9                                   10
//    short_form_restrictions BOOLEAN DEAFULT(0), past_part_restrictions BOOLEAN DEAFULT(0), no_short_form BOOLEAN DEAFULT(0), no_past_part BOOLEAN DEAFULT(0), fleeting_vowel BOOLEAN DEFAULT(0), 
//         11                              12      
//    stem_augment BOOLEAN DEFAULT(0), is_edited BOOLEAN DEFAULT(0));

    try
    {
        m_pDb->BeginTransaction();

        StLexemeProperties& stProperties = pLexeme->stGetPropertiesForWriteAccess();

        if (stProperties.llInflectionId < 0)
        {
            ERROR_LOG(L"Warning: no inflection key, assume new lexeme.");
//            return H_ERROR_DB;
        }
        else
        {
            CEString sDeleteInflectionQuery(L"DELETE FROM inflection WHERE descriptor_id = ");
            sDeleteInflectionQuery += CEString::sToString(stProperties.llDescriptorId);
            m_pDb->Exec(sDeleteInflectionQuery);
        }

        sqlite3_stmt* pStmt = nullptr;
        m_pDb->uiPrepareForInsert(L"inflection", 12, pStmt);
        auto llInsertHandle = (unsigned long long)pStmt;

        int64_t iDescriptorId = stProperties.llDescriptorId;
        m_pDb->Bind(1, iDescriptorId, llInsertHandle);
        m_pDb->Bind(2, stProperties.bPrimaryInflectionGroup, llInsertHandle);
        m_pDb->Bind(3, stProperties.iType, llInsertHandle);
        m_pDb->Bind(4, stProperties.eAccentType1, llInsertHandle);
        m_pDb->Bind(5, stProperties.eAccentType2, llInsertHandle);
        m_pDb->Bind(6, stProperties.bShortFormsRestricted, llInsertHandle);
        m_pDb->Bind(7, stProperties.bPastParticipleRestricted, llInsertHandle);
        m_pDb->Bind(8, stProperties.bShortFormsIncomplete, llInsertHandle);
        m_pDb->Bind(9, stProperties.bNoPassivePastParticiple, llInsertHandle);
        m_pDb->Bind(10, stProperties.bFleetingVowel, llInsertHandle);
        m_pDb->Bind(11, stProperties.iStemAugment, llInsertHandle);
        m_pDb->Bind(12, true, llInsertHandle);                   // is_edited

        m_pDb->InsertRow(llInsertHandle);
        m_pDb->Finalize(llInsertHandle);

        stProperties.llInflectionId = m_pDb->llGetLastKey();

        m_pDb->CommitTransaction();

    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eSaveInflectionInfo()

ET_ReturnCode CDictionary::eUpdateDescriptorInfo(ILexeme * pLexeme)
{
//    CREATE TABLE descriptor(id INTEGER PRIMARY KEY ASC, word_id INTEGER, graphic_stem TEXT, second_part_id INTEGER, 
//                            0                              1                  2                   3                      
//    is_variant BOOLEAN DEFAULT(0), main_symbol TEXT, part_of_speech INTEGER, is_plural_of BOOLEAN DEFAULT(0), 
//         4                                5                 6                     7
//    is_intransitive BOOLEAN DEFAULT(0), is_reflexive BOOLEAN DEFAULT(0), main_symbol_plural_of TEXT, 
//           8                                 9                                  10
//    alt_main_symbol TEXT, inflection_type TEXT, comment TEXT, alt_main_symbol_comment TEXT, alt_inflection_comment TEXT, 
//            11                   12                13                   14                           15
//    verb_stem_alternation TEXT, part_past_pass_zhd BOOLEAN DEFAULT(0), section INTEGER, no_comparative BOOLEAN DEFAULT(0), 
//             16                         17                                18                 19
//    no_long_forms BOOLEAN DEFAULT(0), assumed_forms BOOLEAN DEFAULT(0), yo_alternation BOOLEAN DEFAULT(0), o_alternation BOOLEAN DEFAULT(0), 
//            20                               21                              22                                    23
//    second_genitive BOOLEAN DEFAULT(0), is_impersonal BOOLEAN DEFAULT(0), is_iterative BOOLEAN DEFAULT(0), has_aspect_pair BOOLEAN DEFAULT(0), 
//            24                                 25                              26                                    27
//    has_difficult_forms BOOLEAN DEFAULT(0), has_missing_forms BOOLEAN DEFAULT(0), has_irregular_forms BOOLEAN DEFAULT(0), 
//            28                                 29                                        30
//    irregular_forms_lead_comment TEXT, restricted_contexts TEXT, contexts TEXT, cognate TEXT, trailing_comment TEXT, 
//            31                                 32                    33            34             35
//    is_edited BOOLEAN DEFAULT(0), FOREIGN KEY(word_id) REFERENCES headword(id));
//        36

    StLexemeProperties& stProperties = pLexeme->stGetPropertiesForWriteAccess();
    if (stProperties.llDescriptorId < 0)
    {
        ERROR_LOG(L"Unable to update descriptor: illegal descripto ID");
        return H_ERROR_UNEXPECTED;
    }

    try
    {
        m_pDb->BeginTransaction();

        vector<CEString> vecColumns = { L"word_id", L"graphic_stem", L"second_part_id", L"is_variant", L"main_symbol",
//                                           1             2                3                4               5          
            L"part_of_speech", L"is_plural_of", L"is_intransitive", L"is_reflexive", L"main_symbol_plural_of", L"alt_main_symbol",
//                  6                 7                8                   9                   10                      11
            L"inflection_type", L"comment", L"alt_main_symbol_comment", L"alt_inflection_comment", L"verb_stem_alternation", L"part_past_pass_zhd",
//                  12                13               14                        15                   16                              17           
            L"section", L"no_comparative", L"no_long_forms", L"assumed_forms", L"yo_alternation", L"o_alternation", L"second_genitive",
//                  18          19                 20                21                 22               23                24
            L"is_impersonal", L"is_iterative", L"has_aspect_pair", L"has_difficult_forms", L"has_missing_forms", L"has_irregular_forms",
//                  25              26                  27                     28                    29                  30
            L"irregular_forms_lead_comment", L"restricted_contexts", L"contexts", L"cognate", L"trailing_comment", L"is_edited" };
//                  31                                32                  33           34          35                   36   

        sqlite3_stmt* pStmt = nullptr;
        auto llUpdateHandle = m_pDb->uiPrepareForUpdate(L"descriptor", vecColumns, stProperties.llDescriptorId, pStmt);

        int64_t iHeadwordId = stProperties.llHeadwordId;
        m_pDb->Bind(1, iHeadwordId, llUpdateHandle);                            //  1 word_id
        m_pDb->Bind(2, stProperties.sGraphicStem, llUpdateHandle);              //  2 graphic_stem
        int64_t iSecondPartId = stProperties.llSecondPartId;
        m_pDb->Bind(3, iSecondPartId, llUpdateHandle);                          //  3 second_part_id
        m_pDb->Bind(4, stProperties.bIsVariant, llUpdateHandle);                //  4 is_variant
        m_pDb->Bind(5, stProperties.sMainSymbol, llUpdateHandle);               //  5 main_symbol
        m_pDb->Bind(6, stProperties.ePartOfSpeech, llUpdateHandle);             //  6 part_of_speech
        m_pDb->Bind(7, stProperties.bIsPluralOf, llUpdateHandle);               //  7 is_plural_of
        m_pDb->Bind(8, !stProperties.bTransitive, llUpdateHandle);              //  8 inflection_type
        bool bIsRefexive = (REFL_YES == stProperties.eReflexive) ? true : false;
        m_pDb->Bind(9, bIsRefexive, llUpdateHandle);                            //  9 is_reflexive
        m_pDb->Bind(10, stProperties.sMainSymbolPluralOf, llUpdateHandle);      // 10 main_symbol_plural_of
        m_pDb->Bind(11, stProperties.sAltMainSymbol, llUpdateHandle);           // 11 alt_main_symbol
        m_pDb->Bind(12, stProperties.sInflectionType, llUpdateHandle);          // 12 inflection_type
        m_pDb->Bind(13, stProperties.sComment, llUpdateHandle);                 // 13 comment
        m_pDb->Bind(14, stProperties.sAltMainSymbolComment, llUpdateHandle);    // 14 alt_main_symbol_comment
        m_pDb->Bind(15, stProperties.sAltInflectionComment, llUpdateHandle);    // 15 alt_inflection_comment
        m_pDb->Bind(16, stProperties.sVerbStemAlternation, llUpdateHandle);     // 16 verb_stem_alternation
        m_pDb->Bind(17, stProperties.bPartPastPassZhd, llUpdateHandle);         // 17 part_past_pass_zhd
        m_pDb->Bind(18, stProperties.iSection, llUpdateHandle);                 // 18 section
        m_pDb->Bind(19, stProperties.bNoComparative, llUpdateHandle);           // 19 no_comparative
        m_pDb->Bind(20, stProperties.bNoLongForms, llUpdateHandle);             // 20 no_long_forms
        m_pDb->Bind(21, stProperties.bAssumedForms, llUpdateHandle);            // 21 assumed_forms
        m_pDb->Bind(22, stProperties.bYoAlternation, llUpdateHandle);           // 22 yo_alternation
        m_pDb->Bind(23, stProperties.bOAlternation, llUpdateHandle);            // 23 o_alternation
        m_pDb->Bind(24, stProperties.bSecondGenitive, llUpdateHandle);          // 24 second_genitive
        m_pDb->Bind(25, stProperties.bIsImpersonal, llUpdateHandle);            // 25 is_impersonal
        m_pDb->Bind(26, stProperties.bIsIterative, llUpdateHandle);             // 26 is_iterative
        m_pDb->Bind(27, stProperties.bHasAspectPair, llUpdateHandle);           // 27 has_aspect_pair
        m_pDb->Bind(28, stProperties.bHasDifficultForms, llUpdateHandle);       // 28 has_difficult_forms
        m_pDb->Bind(29, stProperties.bHasMissingForms, llUpdateHandle);         // 29 has_missing_forms
        m_pDb->Bind(30, stProperties.bHasIrregularForms, llUpdateHandle);       // 30 has_irregular_forms
        m_pDb->Bind(31, stProperties.sIrregularFormsLeadComment, llUpdateHandle);    //  31 irregular_forms_lead_comment
        m_pDb->Bind(32, stProperties.sRestrictedContexts, llUpdateHandle);      // 32 restricted_contexts
        m_pDb->Bind(33, stProperties.sContexts, llUpdateHandle);                // 33 contexts
        m_pDb->Bind(34, stProperties.sCognate, llUpdateHandle);                 // 34 cognate
        m_pDb->Bind(35, stProperties.sTrailingComment, llUpdateHandle);         // 35 trailing_comment
        m_pDb->Bind(36, true, llUpdateHandle);                                  // 36 is_edited

        m_pDb->InsertRow(llUpdateHandle);
        m_pDb->Finalize(llUpdateHandle);

        m_pDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eUpdateDescriptorInfo()

ET_ReturnCode CDictionary::eSaveDescriptorInfo(ILexeme* pLexeme)
{
    //    CREATE TABLE descriptor(id INTEGER PRIMARY KEY ASC, word_id INTEGER, graphic_stem TEXT, second_part_id INTEGER, 
    //                            0                              1                  2                   3                      
    //    is_variant BOOLEAN DEFAULT(0), main_symbol TEXT, part_of_speech INTEGER, is_plural_of BOOLEAN DEFAULT(0), 
    //         4                                5                 6                     7
    //    is_intransitive BOOLEAN DEFAULT(0), is_reflexive BOOLEAN DEFAULT(0), main_symbol_plural_of TEXT, 
    //           8                                 9                                  10
    //    alt_main_symbol TEXT, inflection_type TEXT, comment TEXT, alt_main_symbol_comment TEXT, alt_inflection_comment TEXT, 
    //            11                   12                13                   14                           15
    //    verb_stem_alternation TEXT, part_past_pass_zhd BOOLEAN DEFAULT(0), section INTEGER, no_comparative BOOLEAN DEFAULT(0), 
    //             16                         17                                18                 19
    //    no_long_forms BOOLEAN DEFAULT(0), assumed_forms BOOLEAN DEFAULT(0), yo_alternation BOOLEAN DEFAULT(0), o_alternation BOOLEAN DEFAULT(0), 
    //            20                               21                              22                                    23
    //    second_genitive BOOLEAN DEFAULT(0), is_impersonal BOOLEAN DEFAULT(0), is_iterative BOOLEAN DEFAULT(0), has_aspect_pair BOOLEAN DEFAULT(0), 
    //            24                                 25                              26                                    27
    //    has_difficult_forms BOOLEAN DEFAULT(0), has_missing_forms BOOLEAN DEFAULT(0), has_irregular_forms BOOLEAN DEFAULT(0), 
    //            28                                 29                                        30
    //    irregular_forms_lead_comment TEXT, restricted_contexts TEXT, contexts TEXT, cognate TEXT, trailing_comment TEXT, 
    //            31                                 32                    33            34             35
    //    is_edited BOOLEAN DEFAULT(0), FOREIGN KEY(word_id) REFERENCES headword(id));
    //        36

    StLexemeProperties& stProperties = pLexeme->stGetPropertiesForWriteAccess();

    try
    {
        m_pDb->BeginTransaction();

        vector<CEString> vecColumns = { L"word_id", L"graphic_stem", L"second_part_id", L"is_variant", L"main_symbol",
            //                                           1             2                3                4               5          
                        L"part_of_speech", L"is_plural_of", L"is_intransitive", L"is_reflexive", L"main_symbol_plural_of", L"alt_main_symbol",
            //                  6                 7                8                   9                   10                      11
                        L"inflection_type", L"comment", L"alt_main_symbol_comment", L"alt_inflection_comment", L"verb_stem_alternation", L"part_past_pass_zhd",
            //                  12                13               14                        15                   16                              17           
                        L"section", L"no_comparative", L"no_long_forms", L"assumed_forms", L"yo_alternation", L"o_alternation", L"second_genitive",
            //                  18          19                 20                21                 22               23                24
                        L"is_impersonal", L"is_iterative", L"has_aspect_pair", L"has_difficult_forms", L"has_missing_forms", L"has_irregular_forms",
            //                  25              26                  27                     28                    29                  30
                        L"irregular_forms_lead_comment", L"restricted_contexts", L"contexts", L"cognate", L"trailing_comment", L"is_edited" };
            //                  31                                32                  33           34          35                   36   

        sqlite3_stmt* pStmt = nullptr;
        m_pDb->uiPrepareForInsert(L"descriptor", 36, pStmt, true);
        auto llInsertHandle = (unsigned long long)pStmt;

        int64_t iHeadwordId = stProperties.llHeadwordId;
        m_pDb->Bind(1, iHeadwordId, llInsertHandle);                                //  1 word_id
        m_pDb->Bind(2, stProperties.sGraphicStem, llInsertHandle);                  //  2 graphic_stem
        int64_t iSecondPartId = stProperties.llSecondPartId;
        m_pDb->Bind(3, iSecondPartId, llInsertHandle);                              //  3 second_part_id
        m_pDb->Bind(4, stProperties.bIsVariant, llInsertHandle);                    //  4 is_variant
        m_pDb->Bind(5, stProperties.sMainSymbol, llInsertHandle);                   //  5 main_symbol
        m_pDb->Bind(6, stProperties.ePartOfSpeech, llInsertHandle);                 //  6 part_of_speech
        m_pDb->Bind(7, stProperties.bIsPluralOf, llInsertHandle);                   //  7 is_plural_of
        m_pDb->Bind(8, !stProperties.bTransitive, llInsertHandle);                  //  8 inflection_type
        bool bIsRefexive = (REFL_YES == stProperties.eReflexive) ? true : false;
        m_pDb->Bind(9, bIsRefexive, llInsertHandle);                            //  9 is_reflexive
        m_pDb->Bind(10, stProperties.sMainSymbolPluralOf, llInsertHandle);      // 10 main_symbol_plural_of
        m_pDb->Bind(11, stProperties.sAltMainSymbol, llInsertHandle);           // 11 alt_main_symbol
        m_pDb->Bind(12, stProperties.sInflectionType, llInsertHandle);          // 12 inflection_type
        m_pDb->Bind(13, stProperties.sComment, llInsertHandle);                 // 13 comment
        m_pDb->Bind(14, stProperties.sAltMainSymbolComment, llInsertHandle);    // 14 alt_main_symbol_comment
        m_pDb->Bind(15, stProperties.sAltInflectionComment, llInsertHandle);    // 15 alt_inflection_comment
        m_pDb->Bind(16, stProperties.sVerbStemAlternation, llInsertHandle);     // 16 verb_stem_alternation
        m_pDb->Bind(17, stProperties.bPartPastPassZhd, llInsertHandle);         // 17 part_past_pass_zhd
        m_pDb->Bind(18, stProperties.iSection, llInsertHandle);                 // 18 section
        m_pDb->Bind(19, stProperties.bNoComparative, llInsertHandle);           // 19 no_comparative
        m_pDb->Bind(20, stProperties.bNoLongForms, llInsertHandle);             // 20 no_long_forms
        m_pDb->Bind(21, stProperties.bAssumedForms, llInsertHandle);            // 21 assumed_forms
        m_pDb->Bind(22, stProperties.bYoAlternation, llInsertHandle);           // 22 yo_alternation
        m_pDb->Bind(23, stProperties.bOAlternation, llInsertHandle);            // 23 o_alternation
        m_pDb->Bind(24, stProperties.bSecondGenitive, llInsertHandle);          // 24 second_genitive
        m_pDb->Bind(25, stProperties.bIsImpersonal, llInsertHandle);            // 25 is_impersonal
        m_pDb->Bind(26, stProperties.bIsIterative, llInsertHandle);             // 26 is_iterative
        m_pDb->Bind(27, stProperties.bHasAspectPair, llInsertHandle);           // 27 has_aspect_pair
        m_pDb->Bind(28, stProperties.bHasDifficultForms, llInsertHandle);       // 28 has_difficult_forms
        m_pDb->Bind(29, stProperties.bHasMissingForms, llInsertHandle);         // 29 has_missing_forms
        m_pDb->Bind(30, stProperties.bHasIrregularForms, llInsertHandle);       // 30 has_irregular_forms
        m_pDb->Bind(31, stProperties.sIrregularFormsLeadComment, llInsertHandle);    //  31 irregular_forms_lead_comment
        m_pDb->Bind(32, stProperties.sRestrictedContexts, llInsertHandle);      // 32 restricted_contexts
        m_pDb->Bind(33, stProperties.sContexts, llInsertHandle);                // 33 contexts
        m_pDb->Bind(34, stProperties.sCognate, llInsertHandle);                 // 34 cognate
        m_pDb->Bind(35, stProperties.sTrailingComment, llInsertHandle);         // 35 trailing_comment
        m_pDb->Bind(36, true, llInsertHandle);                                  // 36 is_edited

        m_pDb->InsertRow(llInsertHandle);
        m_pDb->Finalize(llInsertHandle);

        stProperties.llDescriptorId = m_pDb->llGetLastKey();

        m_pDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveDescriptorInfo()

/*
ET_ReturnCode CDictionary::eSaveMissingFormHashes(ILexeme* pLexeme)
{
//  CREATE TABLE nonexistent_forms (id INTEGER PRIMARY KEY ASC, descriptor_id INTEGER, gram_hash TEXT, is_edited BOOLEAN DEFAULT (0), FOREIGN KEY (descriptor_id) REFERENCES descriptor (id));

    ET_ReturnCode eRet = H_NO_ERROR;

    vector <CEString> vecMissingFormHashes;

    try
    {
        CEString sHash;
        eRet = pLexeme->eGetFirstMissingFormHash(sHash);
        if (H_FALSE == eRet)
        {
            return H_NO_ERROR;  // No missing forms: that's most common case
        }

        if (eRet != H_NO_ERROR || sHash.bIsEmpty())
        {
            ERROR_LOG(L"Error getting gramm hash for the first missing form.");
            return eRet;
        }

        vecMissingFormHashes.push_back(sHash);

        while (H_NO_ERROR == eRet)
        {
            eRet = pLexeme->eGetFirstMissingFormHash(sHash);
            if (H_FALSE == eRet)
            {
                continue;
            }

            if (eRet != H_NO_ERROR || sHash.bIsEmpty())
            {
                ERROR_LOG(L"Error getting gramm hash for the first missing form.");
                return eRet;
            }

            vecMissingFormHashes.push_back(sHash);
        }

        if (NULL == m_pDb)
        {
            ERROR_LOG(L"Bad DB handle.");
            return H_ERROR_UNEXPECTED;
        }

        m_pDb->BeginTransaction();

        CEString sDeleteQuery(L"DELETE FROM nonexistent_forms WHERE descriptor_id = ");
        sDeleteQuery += CEString::sToString(pLexeme->llLexemeId());
        m_pDb->Exec(sDeleteQuery);

        for (auto sHash : vecMissingFormHashes)
        {
            m_pDb->PrepareForInsert(L"nonexistent_forms", 3);

            m_pDb->Bind(1, pLexeme->llLexemeId());
            m_pDb->Bind(2, sHash);
            m_pDb->Bind(3, true);       // is_edited

            m_pDb->InsertRow();
            m_pDb->Finalize();
        }

        m_pDb->CommitTransaction();

    }
    catch (CException & ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveMissingFormHashes()
*/

ET_ReturnCode CDictionary::eGetWordIdFromLemexeId(int64_t llDescriptorId, int64_t& llWordId)
{
    ET_ReturnCode eRet = H_FALSE;

    try
    {
        CEString sQuery = L"SELECT word_id FROM descriptor WHERE id = " + CEString::sToString(llDescriptorId);
        m_pDb->PrepareForSelect(sQuery);
        bool bFound = false;
        while (m_pDb->bGetRow())
        {
            if (bFound)
            {
                ERROR_LOG(L"Warning: multiple word ID's.");
                break;
            }
            m_pDb->GetData(0, llWordId);
            if (llWordId >= 0)
            {
                eRet = H_NO_ERROR;
                bFound = true;
            }
        }
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return eRet;
}

ET_ReturnCode CDictionary::eMarkLexemeAsEdited(ILexeme * pLexeme)
{
    long long llDescriptorId = pLexeme->llLexemeId();

    if (llDescriptorId < 0)
    {
        ERROR_LOG(L"Illegal descriptor ID.");
        return H_ERROR_UNEXPECTED;
    }

    if (NULL == m_pDb)
    {
        assert(0);
        ERROR_LOG(L"DB pointer is NULL.");
        return H_ERROR_POINTER;
    }

    CEString sQuery(L"SELECT lexeme_hash FROM lexeme_hash_to_descriptor WHERE descriptor_id = ");
    sQuery += CEString::sToString(llDescriptorId);

    CEString sLexemeHash;
    try
    {
        uint64_t uiHandle = m_pDb->uiPrepareForSelect(sQuery);
        if (m_pDb->bGetRow(uiHandle))
        {
            m_pDb->GetData(0, sLexemeHash, uiHandle);
        }

        if (m_pDb->bGetRow(uiHandle))
        {
            assert(0);
            CEString sMsg(L"More than one lexeme hash record for descriptor id ");
            sMsg += CEString::sToString(llDescriptorId);
            ERROR_LOG(sMsg);
        }

        m_pDb->Finalize(uiHandle);
    }
    catch (CException& ex)
    {
        HandleDbException(ex);
        return H_ERROR_DB;
    }

    try
    {
        sqlite3_stmt* pStmt = nullptr;
        m_pDb->uiPrepareForInsert(L"edited_lexemes", 1, pStmt, false);
        auto iInsertHandle = (int64_t)pStmt;
        m_pDb->Bind(1, sLexemeHash, iInsertHandle);
        m_pDb->InsertRow(iInsertHandle);
        m_pDb->Finalize(iInsertHandle);
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
    }

    return H_NO_ERROR;

}       //  eMarkLexemeAsEdited()

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
