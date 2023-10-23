#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING


#include <ctime>
#include <cassert>
#include <algorithm>
#include <iomanip>

#include "Logging.h"

#include "WordForm.h"
#include "Lexeme.h"
#include "Inflection.h"
#include "Analytics.h"
#include "Parser.h"
#include "Verifier.h"
#include "Dictionary.h"

using namespace Hlib;

ET_ReturnCode CLexemeEnumerator::eReset()
{
    if (nullptr == m_spDictionary)
    {
        return H_ERROR_POINTER;
    }

    m_itCurrentLexeme = m_spDictionary->m_vecLexemes.begin();
    return H_NO_ERROR;
}

ET_ReturnCode CLexemeEnumerator::eGetFirstLexeme(shared_ptr<CLexeme>& spLexeme)
{
    if (nullptr == m_spDictionary)
    {
        return H_ERROR_POINTER;
    }

    m_itCurrentLexeme = m_spDictionary->m_vecLexemes.begin();
    if (m_spDictionary->m_vecLexemes.end() == m_itCurrentLexeme)
    {
        return H_FALSE;
    }

    spLexeme = (*m_itCurrentLexeme);

    return H_NO_ERROR;
}

ET_ReturnCode CLexemeEnumerator::eGetNextLexeme(shared_ptr<CLexeme>& spLexeme)
{
    if (nullptr == m_spDictionary)
    {
        return H_ERROR_POINTER;
    }

    if (m_itCurrentLexeme != m_spDictionary->m_vecLexemes.end())
    {
        ++m_itCurrentLexeme;
    }

    if (m_spDictionary->m_vecLexemes.end() == m_itCurrentLexeme)
    {
        return H_NO_MORE;
    }

    spLexeme = (*m_itCurrentLexeme);

    return H_NO_ERROR;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


static CEString sQueryBaseDescriptor
                            (L"SELECT \
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
                               descriptor.trailing_comment ");

static CEString sQueryBaseInflection
                            (L"SELECT \
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDictionary::CDictionary()
{}

CDictionary::~CDictionary()
{
    Clear();
}

ET_ReturnCode CDictionary::eInit()
{
    m_spLexemeEnumerator = make_shared<CLexemeEnumerator>(shared_from_this());
    if (!m_spLexemeEnumerator)
    {
        ERROR_LOG(L"Error retrieving CLexemeEnumerator.");
        return H_ERROR_POINTER;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eSetDbPath(const CEString& sPath)
{
    m_sDbPath = sPath;
    try
    {
        m_spDb = make_shared<CSqlite>(m_sDbPath);
        if (!m_spDb)
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

shared_ptr<CSqlite> CDictionary::spGetDb()
{
    if (nullptr == m_spDb) {
        ERROR_LOG(L"Database not available.");
        return nullptr;
    }

    return m_spDb;
}

ET_ReturnCode CDictionary::eCreateLexemeForEdit(shared_ptr<CLexeme>& spLexeme)
{
    if (nullptr == m_spDb)
    {
        return H_ERROR_POINTER;
    }

    spLexeme = make_shared<CLexeme>(shared_from_this());
    if (nullptr == spLexeme)
    {
        return H_ERROR_POINTER;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eCopyEntryForEdit(const shared_ptr<CInflection> spSource, shared_ptr<CInflection>& spCopy)
{
    if (nullptr == m_spDb)
    {
        return H_ERROR_POINTER;
    }

    spCopy = make_shared<CInflection>(*spSource);
    if (nullptr == spCopy)
    {
        return H_ERROR_POINTER;
    }
    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eGetLexemeById(long long llId, shared_ptr<CLexeme>& spLexeme)
{
    ET_ReturnCode rc = H_NO_ERROR;
    CEString sQuery(sQueryBaseDescriptor);
    sQuery += L"FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
    sQuery += L"WHERE descriptor.id = ";
    sQuery += CEString::sToString(llId);
    sQuery += L";";

    uint64_t uiQueryHandle = 0;
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    spLexeme = make_shared<CLexeme>(shared_from_this());
    if (nullptr == spLexeme)
    {
        return H_ERROR_POINTER;
    }

    while (H_NO_ERROR == rc)
    {
        rc = eReadDescriptorData(spLexeme, uiQueryHandle);
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
    m_spDb->Finalize(uiQueryHandle);

    for (auto spLexeme : m_vecLexemes)
    {
        auto sInflectionQuery = sQueryBaseInflection + 
                                L" FROM inflection WHERE descriptor_id = " + 
                                CEString::sToString(spLexeme->llLexemeId());
        uint64_t uiInflQueryHandle = 0;
        rc = eQueryDb(sInflectionQuery, uiInflQueryHandle);
        if (H_NO_ERROR != rc)
        {
            return rc;
        }
        while (H_NO_ERROR == rc)
        {
            auto bSpryazhSm = false;        // TODO -- are we handling spryazh sm??
            rc = eReadInflectionData(spLexeme, uiInflQueryHandle, bSpryazhSm);
        }
        m_spDb->Finalize(uiInflQueryHandle);

        if (spLexeme->nInflections() < 1)   // Add dummy inflection instance if none was retrieved
        {
            spLexeme->AddInflection(make_shared<CInflection>(spLexeme.get()));
        }
    }

    return H_NO_ERROR;

}       // eGetLexemeById()

ET_ReturnCode CDictionary::eGetSecondPart(long long llId, shared_ptr<CLexeme>& spLexeme)
{
    ET_ReturnCode rc = H_NO_ERROR;
    CEString sQuery(sQueryBaseDescriptor);
    sQuery += L"FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
    sQuery += L"WHERE descriptor.id = ";
    sQuery += CEString::sToString(llId);
    sQuery += L";";

    uint64_t uiQueryHandle = 0;
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    rc = eReadDescriptorData(spLexeme, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        ERROR_LOG(L"Error retrieving lexeme data.");
        return rc;
    }

    // Expect no more lexemes with this ID
    auto spDummy = make_shared<CLexeme>(shared_from_this());
    rc = eReadDescriptorData(spDummy, uiQueryHandle);
    if (H_NO_MORE != rc)
    {
        ERROR_LOG(L"More than one row returned for a single lexeme ID or DB error.");
        return H_ERROR_UNEXPECTED;
    }
    m_spDb->Finalize(uiQueryHandle);

    auto sInflectionQuery = sQueryBaseInflection +
        L" FROM inflection WHERE descriptor_id = " +
        CEString::sToString(spLexeme->llLexemeId());
    uint64_t uiInflQueryHandle = 0;
    rc = eQueryDb(sInflectionQuery, uiInflQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }
    while (H_NO_ERROR == rc)
    {
        auto bSpryazhSm = false;        // TODO -- are we handling spryazh sm??
        rc = eReadInflectionData(spLexeme, uiInflQueryHandle, bSpryazhSm);
    }
    m_spDb->Finalize(uiInflQueryHandle);

    return rc;

}       // eGetSecondPart()

ET_ReturnCode CDictionary::eGetLexemesByHash(const CEString& sMd5)
{
    ET_ReturnCode rc = H_NO_ERROR;
    CEString sQuery(sQueryBaseDescriptor);
    sQuery += L"FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
    sQuery += L"INNER JOIN lexeme_hash_to_descriptor as lhd on lhd.descriptor_id=descriptor.id ";
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
        shared_ptr<CLexeme> pLexeme = make_shared<CLexeme>(shared_from_this());
        if (nullptr == pLexeme)
        {
            return H_ERROR_POINTER;
        }

        rc = eReadDescriptorData(pLexeme, uiQueryHandle);
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

        for (auto spLexeme : m_vecLexemes)
        {
            auto sInflectionQuery = sQueryBaseInflection +
                L" FROM inflection WHERE descriptor_id = " +
                CEString::sToString(spLexeme->llLexemeId());
            uint64_t uiInflQueryHandle = 0;
            rc = eQueryDb(sInflectionQuery, uiInflQueryHandle);
            if (H_NO_ERROR != rc)
            {
                return rc;
            }
            while (H_NO_ERROR == rc)
            {
                auto bSpryazhSm = false;        // TODO -- are we handling spryazh sm??
                rc = eReadInflectionData(spLexeme, uiInflQueryHandle, bSpryazhSm);
            }
            m_spDb->Finalize(uiInflQueryHandle);
        }
    }

    m_spDb->Finalize(uiQueryHandle);

    return (H_NO_MORE == rc) ? H_NO_ERROR : rc;

}   //  eGetLexemesByHash (...)

ET_ReturnCode CDictionary::eGetLexemesByInitialForm(const CEString& sSource)
{
    ET_ReturnCode rc = H_NO_ERROR;
    CEString sQuery(sQueryBaseDescriptor);
    sQuery += L" FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
    sQuery += L"WHERE headword.source = \"";
    sQuery += sSource;
    sQuery += L"\";";

    uint64_t uiQueryHandle = 0;
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    vector<shared_ptr<CLexeme>> vecLexemesFound;

    bool bFound = false;
    while (H_NO_ERROR == rc)
    {
        shared_ptr<CLexeme> spLexeme = make_shared<CLexeme>(shared_from_this());
        if (nullptr == spLexeme)
        {
            return H_ERROR_POINTER;
        }

        rc = eReadDescriptorData(spLexeme, uiQueryHandle);
        if (H_NO_ERROR == rc)
        {
            bFound = true;
            vecLexemesFound.push_back(spLexeme);
        }
        if (H_NO_ERROR != rc && H_NO_MORE != rc)
        {
            Clear();
            ERROR_LOG(L"Error retrieving lexeme data.");
            return rc;
        }
    }

    m_spDb->Finalize(uiQueryHandle);

    CEString sSpryazhSmQuery(sQueryBaseDescriptor);
    sSpryazhSmQuery += L", no_aspect_pair ";
    sSpryazhSmQuery += L"FROM headword INNER JOIN spryazh_sm_headwords ON headword.id = spryazh_sm_headwords.headword_id ";
    sSpryazhSmQuery += L"INNER JOIN descriptor ON spryazh_sm_headwords.ref_descriptor_id = descriptor.id ";
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
        shared_ptr<CLexeme> spLexeme = make_shared<CLexeme>(shared_from_this());
        if (nullptr == spLexeme)
        {
            return H_ERROR_POINTER;
        }

        auto bSpryazhSm = true;
        rc = eReadDescriptorData(spLexeme, uiQueryHandle, bSpryazhSm);
        if (H_NO_ERROR == rc)
        {
            bFound = true;
            vecLexemesFound.push_back(spLexeme);
        }
        if (H_NO_ERROR != rc && H_NO_MORE != rc)
        {
            Clear();
            ERROR_LOG(L"Error retrieving lexeme data.");
            return rc;
        }
    }
    m_spDb->Finalize(uiQueryHandle);

    for (auto spLexeme : vecLexemesFound)
    {
        auto sInflectionQuery = sQueryBaseInflection +
            L" FROM inflection WHERE descriptor_id = " +
            CEString::sToString(spLexeme->llLexemeId());
        uint64_t uiInflectionQueryHandle = 0;
        rc = eQueryDb(sInflectionQuery, uiInflectionQueryHandle);
        if (H_NO_ERROR != rc)
        {
            return rc;
        }
        while (H_NO_ERROR == rc)
        {
            auto bSpryazhSm = false;        // TODO -- are we handling spryazh sm??
            rc = eReadInflectionData(spLexeme, uiInflectionQueryHandle, bSpryazhSm);
        }
        m_spDb->Finalize(uiInflectionQueryHandle);

        if (spLexeme->nInflections() < 1)   // Add dummy inflection instance if none was retrieved
        {
            spLexeme->AddInflection(make_shared<CInflection>(spLexeme.get()));
        }
    }

    for (auto& spLexeme : vecLexemesFound)
    {
        if (spLexeme->stGetProperties().llSecondPartId > 0)
        {
            shared_ptr<CLexeme> spSecondLexeme = make_shared<CLexeme>(shared_from_this());
            rc = eGetSecondPart(spLexeme->stGetProperties().llSecondPartId, spSecondLexeme);
            spLexeme->SetSecondPart(spSecondLexeme);
        }

        if (spLexeme->stGetProperties().bSpryazhSm)
        {
            spLexeme->eHandleSpryazhSmEntry();   // currently, it is always H_NO_ERROR
        }
    }

    if (H_NO_MORE == rc && !bFound)
    {
        return H_FALSE;
    }

    m_vecLexemes.insert(m_vecLexemes.end(), make_move_iterator(vecLexemesFound.begin()),
        make_move_iterator(vecLexemesFound.end()));

    return rc;

}   //  eGetLexemesByInitialForm()

void CDictionary::Clear()
{
    m_vecLexemes.clear();
}

ET_ReturnCode CDictionary::Clear(shared_ptr<CLexeme> spLexeme)
{
    vector<shared_ptr<CLexeme>>::iterator itL = m_vecLexemes.begin();
    for (; itL != m_vecLexemes.end(); ++itL)
    {
        if (*itL == spLexeme)
        {
            m_vecLexemes.erase(itL);
            return H_NO_ERROR;
        }
    }

    return H_ERROR_UNEXPECTED;
}

ET_ReturnCode CDictionary::eCreateLexemeEnumerator(shared_ptr<CLexemeEnumerator>& spLexemeEnumerator)
{
    spLexemeEnumerator = make_shared<CLexemeEnumerator>(shared_from_this());
    if (!m_spLexemeEnumerator)
    {
        ERROR_LOG(L"Error retrieving CLexemeEnumerator.");
        return H_ERROR_POINTER;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eGetParser(shared_ptr<CParser>& spParser)
{
    if (nullptr == m_spDb)
    {
        ERROR_LOG(L"Error retrieving parser instance.");
        return H_ERROR_POINTER;
    }

    if (nullptr == m_spParser)
    {
        m_spParser = make_shared<CParser>(shared_from_this());
    }

    m_spParser->SetDb(m_spDb);

    spParser = m_spParser;

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eGetAnalytics(shared_ptr<CAnalytics>& spAnalytics)
{
    if (nullptr == m_spDb)
    {
        ERROR_LOG(L"Error retrieving IAnalytics interface.");
        return H_ERROR_POINTER;
    }

    if (nullptr == m_spParser)
    {
        m_spParser = make_unique<CParser>(shared_from_this());
        m_spParser->SetDb(m_spDb);
    }

    if (nullptr == m_spAnalytics)
    {
        m_spAnalytics = make_unique<CAnalytics>(m_spDb, m_spParser);
        spAnalytics = m_spAnalytics;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eGetVerifier(shared_ptr<CVerifier>& spVerifier)
{
    if (nullptr == m_spDb)
    {
        ERROR_LOG(L"Error retrieving the verifier: no database.");
        return H_ERROR_POINTER;
    }

    if (nullptr == m_spVerifier)
    {
        m_spVerifier = make_shared<CVerifier>(shared_from_this());
    }

    spVerifier = m_spVerifier;

    return H_NO_ERROR;
}

#ifdef WIN32
ET_ReturnCode CDictionary::eExportTestData(CEString& sPath, PROGRESS_CALLBACK_CLR pProgressCallback)
{
    if (nullptr == m_spDb)
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
        bRet = m_spDb->bExportTables(sPath, vecTables, pProgressCallback);
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
    if (nullptr == m_spDb)
    {
        ERROR_LOG(L"Unable to obtain database handle.");
        return H_ERROR_POINTER;
    }

    CEString sQuery = L"DELETE FROM test_data_stress";
    m_spDb->Delete(sQuery);
    sQuery = L"DELETE FROM test_data";
    m_spDb->Delete(sQuery);

    bool bRet = true;

    vector<CEString> vecTables;
    vecTables.push_back(L"test_data");
    vecTables.push_back(L"test_data_stress");

    try
    {
        bool bAutoincrement = false;
        bRet = m_spDb->bImportTables(sPath, bAutoincrement, pProgressCallback);
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

    pProgressCallback(100, true);

    return H_NO_ERROR;

}       //  eImportTestData()
#endif

//
// Generate all word forms and store them in DB
//
ET_ReturnCode CDictionary::eGenerateAllForms()
{
    if (!m_spDb)
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
    if (!m_spDb)
    {
        return H_ERROR_DB;
    }

    ET_ReturnCode rc = H_NO_ERROR;
    uint64_t uiQueryHandle = 0;
    CEString sQuery(L"SELECT source FROM headword;");
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        std::wcout << "**** exiting." << endl << endl;
        return rc;
    }

    std::wcout << endl << endl << "****      POPULATING STEMS TABLE" << endl << endl;

    m_spDb->BeginTransaction();

    bool bMoreData = true;
    for (int iRow = 0; bMoreData; ++iRow)
    {
        auto bRet = m_spDb->bGetRow(uiQueryHandle);
        if (!bRet)
        {
            bMoreData = false;
            continue;
        }

        CEString sHeadword;
        m_spDb->GetData(0, sHeadword, uiQueryHandle);
        auto rc = eGetLexemesByInitialForm(sHeadword);
        if (rc != H_NO_ERROR && rc != H_NO_MORE && rc != H_FALSE)
        {
            CEString sMsg(L"Unable to retrieve lexeme data for ");
            sMsg += sHeadword;
//            return H_ERROR_UNEXPECTED;
            continue;
        }

        for (auto spLexeme : m_vecLexemes)
        {
            for (auto spInflection : spLexeme->m_vecInflections)
            {
                try
                {
                    rc = spInflection->eGenerateParadigm();
                    if (H_NO_ERROR != rc)
                    {
                        CEString sMsg(L"Error generating paradigm for ");
                        sMsg += spLexeme->sSourceForm();
                        ERROR_LOG(sMsg);
                    }

                    rc = spInflection->eSaveStemsToDb();
                    if (H_NO_ERROR != rc)
                    {
                        CEString sMsg(L"Error saving stems for ");
                        sMsg += spLexeme->sSourceForm();
                        ERROR_LOG(sMsg);
                    }
                }
                catch (CException& ex)
                {
                    ERROR_LOG(ex.szGetDescription());
                    //            return H_EXCEPTION;
                }
            }
        }

        m_vecLexemes.clear();

        if ((H_NO_ERROR == rc) && (iRow > 0) && (iRow % 1000 == 0 || !bMoreData))
        {
            m_spDb->CommitTransaction();

            m_spDb->BeginTransaction();
            CEString sMsg = CEString::sToString(iRow);
            sMsg += L" rows";
            ERROR_LOG(sMsg);

            std::wcout << right << setw(7) << iRow << " rows" << endl;
        }
    }       //  for (int iRow = 0; bMoreData; ++iRow)

    m_spDb->Finalize(uiQueryHandle);

    m_spDb->CommitTransaction();
    std::wcout << "done." << endl << endl;
    return (H_NO_MORE == rc) ? H_NO_ERROR : rc;

}       //  ePopulateStemsTable()

ET_ReturnCode CDictionary::ePopulateWordFormDataTables()
{
    if (!m_spDb)
    {
        std::wcout << endl << "****  No database." << endl;
        return H_ERROR_DB;
    }

    ET_ReturnCode rc = H_NO_ERROR;
    uint64_t uiQueryHandle = 0;
    CEString sQuery(L"SELECT source FROM headword;");

    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        std::wcout << endl << "****  DB query failed" << endl;
        return rc;
    }

    std::wcout << endl << endl << "****      GENERATING WORDFORMS" << endl << endl;

    std::clock_t totalTime = 0;

    std::vector<shared_ptr<CInflection>> vecProcessedInflections;

    bool bMoreData = true;
    for (int iRow = 0; bMoreData; ++iRow)
    {
        auto bRet = m_spDb->bGetRow(uiQueryHandle);
        if (!bRet)
        {
            bMoreData = false;
        }
        else
        {
            CEString sHeadword;
            m_spDb->GetData(0, sHeadword, uiQueryHandle);
            auto rc = eGetLexemesByInitialForm(sHeadword);
            if (rc != H_NO_ERROR && rc != H_NO_MORE)
            {
                CEString sMsg(L"Unable to retrieve lexeme data for ");
                sMsg += sHeadword;
            }

            for (auto spLexeme : m_vecLexemes)
            {
                for (auto spInflection : spLexeme->m_vecInflections)
                {
                    try
                    {
                        rc = spInflection->eGenerateParadigm();
                        if (H_NO_ERROR != rc)
                        {
                            CEString sMsg(L"Error generating paradigm for ");
                            sMsg += spLexeme->sSourceForm();
                            ERROR_LOG(sMsg);
                        }

                        rc = spInflection->eAssignStemIds();
                        if (H_NO_ERROR != rc)
                        {
                            CEString sMsg(L"Error saving stems for ");
                            sMsg += spLexeme->sSourceForm();
                            ERROR_LOG(sMsg);
                        }

                        vecProcessedInflections.push_back(spInflection);
                    }
                    catch (CException& ex)
                    {
                        ERROR_LOG(ex.szGetDescription());
                        //            return H_EXCEPTION;
                    }
                }
            }       //   for (auto spLexeme : m_vecLexemes)
        }
        m_vecLexemes.clear();

        if ((H_NO_ERROR == rc) && (iRow > 0) && (iRow % 1000 == 0 || !bMoreData))
        {
            clock_t dbProcTime = clock();
            m_spDb->BeginTransaction();
            CEString sMsg = CEString::sToString(iRow);
            sMsg += L" rows";
            MESSAGE_LOG(sMsg);

            for (auto spInflection : vecProcessedInflections)
            {
                for (unsigned int uiWf = 0; uiWf < spInflection->uiTotalWordForms(); ++uiWf)
                {
                    shared_ptr<CWordForm> spWf;
                    rc = spInflection->eGetWordForm(uiWf, spWf);
                    if (rc != H_NO_ERROR)
                    {
                        continue;
                    }

                    if (0 == spWf->llStemId())
                    {
                        CEString sMsg(L"Unable to find stem id for \"");
                        sMsg += spWf->sStem();
                        sMsg += L'"';
                        sMsg += L" lexeme = " + spInflection->spLexeme()->sSourceForm();
                        ERROR_LOG(sMsg);

                        continue;
                    }
                    spWf->bSaveToDb();
                }
            }

            m_spDb->CommitTransaction();

            vecProcessedInflections.clear();

            totalTime += clock() - dbProcTime;
            double dDuration = (clock() - dbProcTime) / (double)CLOCKS_PER_SEC;
            CEString sDurationMsg(L"Row ");
            sDurationMsg += CEString::sToString(iRow);
            sDurationMsg += L"; ";
            sDurationMsg += CEString::sToString(dDuration);
            sDurationMsg += L" seconds total";
            ERROR_LOG(sDurationMsg);
//            std::wcout << "Row " << right << setw(6) << iRow << ", " << fixed << setprecision(3) << dDuration << " seconds" << endl;
        }
    }       //   for (int iRow = 0; bMoreData; ++iRow)
    m_spDb->Finalize(uiQueryHandle);

    std::wcout << "done." << endl << endl;

    return (H_NO_MORE == rc) ? H_NO_ERROR : rc;

}       //  ePopulateWordFormDataTables()

ET_ReturnCode CDictionary::ePopulateHashToDescriptorTable(PROGRESS_CALLBACK_CLR pProgressCLR, PROGRESS_CALLBACK_PYTHON pProgressPython)
{
    if (!m_spDb)
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

        bool bRet = m_spDb->bGetRow(uiCountQueryHandle);
        if (!bRet)
        {
            return H_ERROR_UNEXPECTED;
        }

        m_spDb->GetData(0, llCount, uiCountQueryHandle);
        if (H_NO_ERROR != rc)
        {
            return rc;
        }
        m_spDb->Finalize(uiCountQueryHandle);

    }
    catch (CException& ex)
    {
        HandleDbException(ex);
        rc = H_ERROR_DB;
    }

    try
    {
        m_spDb->ClearTable(L"lexeme_hash_to_descriptor");
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            m_spDb->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(m_spDb->iGetLastError());
        ERROR_LOG(sMsg);

        return H_ERROR_DB;
    }

    uint64_t uiQueryHandle = 0;
    CEString sQuery(sQueryBaseDescriptor);
    sQuery += L"FROM headword INNER JOIN descriptor ON descriptor.word_id = headword.id ";
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id;";
    rc = eQueryDb(sQuery, uiQueryHandle);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    struct StLexemeIds
    {
        long long llDescriptorId;
        long long llInflectionId;

        StLexemeIds(long long llDid, long long llIid) :
            llDescriptorId(llDid), llInflectionId(llIid)
        {}
    };

    map<CEString, StLexemeIds> mapHashToIds;

    std::clock_t totalTime = 0;
    bool bMoreData = true;
    for (int iRow = 0; bMoreData; ++iRow)
    {
        clock_t dbProcTime = clock();
        shared_ptr<CLexeme> spLexeme = make_shared<CLexeme>(shared_from_this());
        try
        {
            rc = eReadDescriptorData(spLexeme, uiQueryHandle);
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

            uiQueryHandle = 0;
            for (auto spLexeme : m_vecLexemes)
            {
                auto sInflectionQuery = sQueryBaseInflection +
                    L" FROM inflection WHERE descriptor_id = " +
                    CEString::sToString(spLexeme->llLexemeId());
                uint64_t uiQueryHandle = 0;
                rc = eQueryDb(sQueryBaseInflection, uiQueryHandle);
                if (H_NO_ERROR != rc)
                {
                    return rc;
                }
                while (H_NO_ERROR == rc)
                {
                    auto bSpryazhSm = false;        // TODO -- are we handling spryazh sm??
                    rc = eReadInflectionData(spLexeme, uiQueryHandle, bSpryazhSm);
                }
            }

            shared_ptr<CInflectionEnumerator> spIe;
            auto rc = spLexeme->eCreateInflectionEnumerator(spIe);
            if (rc != H_NO_ERROR)
            {
                ERROR_LOG(L"UNable to retrieve inflection data.");
                continue;
            }

            shared_ptr<CInflection> spInflection;
            rc = spIe->eGetFirstInflection(spInflection);
            while (H_NO_ERROR == rc)
            {
                if (!spInflection)
                {
                    ERROR_LOG(L"Inflection instance is NULL.");
                    continue;
                }

                CEString sHash = spInflection->sHash();
                mapHashToIds.emplace(sHash, StLexemeIds(spLexeme->llLexemeId(), spInflection->llDescriptorId()));

                rc = spIe->eGetNextInflection(spInflection);
            }

            if (!bMoreData || (iRow > 0 && (iRow % 1000 == 0)))
            { 
                m_spDb->BeginTransaction();

                for (auto itLex = mapHashToIds.begin(); itLex != mapHashToIds.end(); ++itLex)
                {
                    uint64_t uiInsertHandle = 0;

                    try
                    {
                        if (0 == uiInsertHandle)
                        {
                            sqlite3_stmt* pStmt = nullptr;
                            m_spDb->uiPrepareForInsert(L"lexeme_hash_to_descriptor", 3, pStmt, false);
                            uiInsertHandle = (uint64_t)pStmt;
                        }
                        m_spDb->Bind(1, (*itLex).first, uiInsertHandle);
                        m_spDb->Bind(2, (uint64_t)(*itLex).second.llDescriptorId, uiInsertHandle);
                        m_spDb->Bind(3, (uint64_t)(*itLex).second.llInflectionId, uiInsertHandle);
                        m_spDb->InsertRow(uiInsertHandle);
                        m_spDb->Finalize(uiInsertHandle);
                    }
                    catch (CException& exc)
                    {
                        CEString sMsg(exc.szGetDescription());
                        CEString sError;
                        try
                        {
                            m_spDb->GetLastError(sError);
                            sMsg += CEString(L", error %d: ");
                            sMsg += sError;
                        }
                        catch (...)
                        {
                            sMsg = L"Apparent DB error ";
                        }

                        sMsg += CEString::sToString(m_spDb->iGetLastError());
                        ERROR_LOG(sMsg);

                        continue;
                    }
                }       //  for (auto itLex = mapHashToProperties.begin(); ...)

                m_spDb->CommitTransaction();

                mapHashToIds.clear();
            
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

//          CEString sMsg(L"------------ Saving lexeme hashes ------------- \r\n");
//          sMsg += CEString::sToString(iRow);
//          CLogger::bWriteLog(wstring(xxxx));
//          ERROR_LOG(sMsg)
//          std::wcout << string(sMsg.stl_sToUtf8()) << std::endl;

//          CEString sDurationMsg(L"Row ");
//          sDurationMsg += CEString::sToString(iRow);
//          sDurationMsg += L"; ";
//          sDurationMsg += CEString::sToString(dDuration);
//          sDurationMsg += L" seconds total"; 
//          CLogger::bWriteLog(wstring(sDurationMsg));
//          std::wcout << string(sDurationMsg.stl_sToUtf8()) << std::endl;
//          totalTime = 0;

        }
    }       //  for ...
    m_spDb->Finalize(uiQueryHandle);

    return H_NO_ERROR;

}       //  ePopulateHashToDescriptorTable()

ET_ReturnCode CDictionary::eCountLexemes(int64_t& iLexemes)
{
    ET_ReturnCode rc = H_NO_ERROR;

    iLexemes = -1;

    if (!m_spDb)
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

        bool bRet = m_spDb->bGetRow(uiQueryHandle);
        if (!bRet)
        {
            return H_ERROR_UNEXPECTED;
        }

        m_spDb->GetData(1, iLexemes, uiQueryHandle);
        if (H_NO_ERROR != rc)
        {
            return rc;
        }
        m_spDb->Finalize(uiQueryHandle);

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

    if (nullptr == m_spDb)
    {
        assert(0);
        ERROR_LOG(L"DB pointer is NULL.");
        return H_ERROR_POINTER;
    }

    try
    {
        uiQueryHandle = m_spDb->uiPrepareForSelect(sSelect);
    }
    catch (CException& ex)
    {
        HandleDbException(ex);
        rc = H_ERROR_DB;
    }

    return rc;
}

ET_ReturnCode CDictionary::eReadDescriptorData(shared_ptr<CLexeme> spLexeme, uint64_t uiQueryHandle, bool bIsSpryazhSm)
{
    if (nullptr == m_spDb)
    {
        assert(0);
        ERROR_LOG(L"DB pointer is NULL.");
        return H_ERROR_POINTER;
    }

    auto& stProperties = spLexeme->stGetPropertiesForWriteAccess();

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        if (m_spDb->bGetRow(uiQueryHandle))
        {
/*
        static CEString sQueryBaseDescriptor(L"SELECT headword.source, headword.id, headword.comment, headword.variant_comment, headword.plural_of, 
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
                                            descriptor.trailing_comment
                                                             46
                                            
                                            , inflection.id, inflection.is_primary, inflection.inflection_type, inflection.accent_type1, 
                                                               46                   47                  48                      49                          50
                                            inflection.accent_type2, inflection.short_form_restrictions, inflection.past_part_restrictions, inflection.no_short_form, 
                                                            51                           52                                   53                             54
                                            no_past_part, fleeting_vowel, stem_augment, is_edited);
                                                            55                      56               
*/

            m_spDb->GetData(0, stProperties.sSourceForm, uiQueryHandle);                         //  0 source
            uint64_t uiHeadwordId = 0;
            m_spDb->GetData(1, uiHeadwordId, uiQueryHandle);
            stProperties.llHeadwordId = uiHeadwordId;                                           //  1 id [headword]
            m_spDb->GetData(2, stProperties.sHeadwordComment, uiQueryHandle);                    //  2 comment
            m_spDb->GetData(3, stProperties.sHeadwordVariantComment, uiQueryHandle);             //  3 variant_comment
            m_spDb->GetData(4, stProperties.sPluralOf, uiQueryHandle);                           //  4 plural_of
            m_spDb->GetData(5, stProperties.sUsage, uiQueryHandle);                              //  5 usage
            m_spDb->GetData(6, stProperties.sHeadwordVariant, uiQueryHandle);                    //  6 variant
            m_spDb->GetData(7, stProperties.sSeeRef, uiQueryHandle);                             //  7 see_ref
            m_spDb->GetData(8, stProperties.sBackRef, uiQueryHandle);                            //  8 back_ref
            uint64_t uiSourceEntryId = 0;
            m_spDb->GetData(9, uiSourceEntryId, uiQueryHandle);                                  //  9 source_entry_id
            stProperties.llSourceEntryId = uiSourceEntryId;
            m_spDb->GetData(10, stProperties.bSpryazhSm, uiQueryHandle);                         // 10 spryazh_sm
            m_spDb->GetData(11, stProperties.bSecondPart, uiQueryHandle);                        // 11 second_part
            uint64_t uiDescriptorId = 0;
            m_spDb->GetData(12, uiDescriptorId, uiQueryHandle);                                  // 12 id [descriptor]
            stProperties.llDescriptorId = uiDescriptorId;
            m_spDb->GetData(13, stProperties.sGraphicStem, uiQueryHandle);                       // 13 graphic_stem
            uint64_t uiSecondPartId = 0;
            m_spDb->GetData(14, uiSecondPartId, uiQueryHandle);                                  // 14 second_part_id
            stProperties.llSecondPartId = uiSecondPartId;
            m_spDb->GetData(15, stProperties.bIsVariant, uiQueryHandle);                         // 15 is_variant
            m_spDb->GetData(16, stProperties.sMainSymbol, uiQueryHandle);                        // 16 main_symbol
            int iPartOfSpeech = 0;
            m_spDb->GetData(17, iPartOfSpeech, uiQueryHandle);                                   // 17 part_of_speech
            stProperties.ePartOfSpeech = (ET_PartOfSpeech)iPartOfSpeech;
            m_spDb->GetData(18, stProperties.bIsPluralOf, uiQueryHandle);                        // 18 is_plural_of
            bool bIntransitive = false;
            m_spDb->GetData(19, bIntransitive, uiQueryHandle);                                   // 19 is_intransitive; "нп" == !bTransitive
            stProperties.bTransitive = !bIntransitive;
            bool bReflexive = false;
            m_spDb->GetData(20, bReflexive, uiQueryHandle);                                      // 20 is_reflexive
            if (POS_VERB == stProperties.ePartOfSpeech)
            {
                stProperties.eReflexive = bReflexive ? REFL_YES : REFL_NO;
            }
            else
            {
                stProperties.eReflexive = REFL_UNDEFINED;
            }
            m_spDb->GetData(21, stProperties.sMainSymbolPluralOf, uiQueryHandle);                // 21 main_symbol_plural_of
            m_spDb->GetData(22, stProperties.sAltMainSymbol, uiQueryHandle);                     // 22 alt_main_symbol
            m_spDb->GetData(23, stProperties.sInflectionType, uiQueryHandle);                    // 23 inflection_type
            m_spDb->GetData(24, stProperties.sComment, uiQueryHandle);                           // 24 comment
            m_spDb->GetData(25, stProperties.sAltMainSymbolComment, uiQueryHandle);              // 25 alt_main_symbol_comment
            m_spDb->GetData(26, stProperties.sAltInflectionComment, uiQueryHandle);              // 26 alt_inflection_comment
            m_spDb->GetData(27, stProperties.sVerbStemAlternation, uiQueryHandle);               // 27 verb_stem_alternation
            m_spDb->GetData(28, stProperties.bPartPastPassZhd, uiQueryHandle);                   // 28 part_past_pass_zhd
            m_spDb->GetData(29, stProperties.iSection, uiQueryHandle);                           // 29 section
            m_spDb->GetData(30, stProperties.bNoComparative, uiQueryHandle);                     // 30 no_comparative
            m_spDb->GetData(31, stProperties.bNoLongForms, uiQueryHandle);                       // 31 no_long_forms
            m_spDb->GetData(32, stProperties.bAssumedForms, uiQueryHandle);                      // 32 assumed_forms
            m_spDb->GetData(33, stProperties.bYoAlternation, uiQueryHandle);                     // 33 yo_alternation
            m_spDb->GetData(34, stProperties.bOAlternation, uiQueryHandle);                      // 34 o_alternation
            m_spDb->GetData(35, stProperties.bSecondGenitive, uiQueryHandle);                    // 35 second_genitive
            m_spDb->GetData(36, stProperties.bIsImpersonal, uiQueryHandle);                      // 36 is_impersonal
            m_spDb->GetData(37, stProperties.bIsIterative, uiQueryHandle);                       // 37 is_iterative
            m_spDb->GetData(38, stProperties.bHasAspectPair, uiQueryHandle);                     // 38 has_aspect_pair
            m_spDb->GetData(39, stProperties.bHasDifficultForms, uiQueryHandle);                 // 39 has_difficult_forms
            m_spDb->GetData(40, stProperties.bHasMissingForms, uiQueryHandle);                   // 40 has_missing_forms
            m_spDb->GetData(41, stProperties.bHasIrregularForms, uiQueryHandle);                 // 41 has_irregular_forms
            m_spDb->GetData(42, stProperties.sIrregularFormsLeadComment, uiQueryHandle);         // 42 irregular_forms_lead_comment
            m_spDb->GetData(43, stProperties.sRestrictedContexts, uiQueryHandle);                // 43 restricted_contexts
            m_spDb->GetData(44, stProperties.sContexts, uiQueryHandle);                          // 44 contexts
            m_spDb->GetData(45, stProperties.sCognate, uiQueryHandle);                           // 45 cognate
            m_spDb->GetData(46, stProperties.sTrailingComment, uiQueryHandle);                   // 46 trailing_comment
            if (bIsSpryazhSm)
            {
                m_spDb->GetData(58, stProperties.bSpryazhSmNoAspectPair, uiQueryHandle);         // 58 no_aspect_pair (optional)
            }

            CEString sStressQuery(L"SELECT stress_position, is_primary FROM stress WHERE is_variant = \"");
            sStressQuery += L"0\" AND headword_id = ";
            sStressQuery += CEString::sToString(stProperties.llHeadwordId);

            uint64_t uiStressHandle = m_spDb->uiPrepareForSelect(sStressQuery);
            while (m_spDb->bGetRow(uiStressHandle))
            {
                int iPos = -1;
                bool bPrimary = false;
                m_spDb->GetData(0, iPos, uiStressHandle);
                m_spDb->GetData(1, bPrimary, uiStressHandle);
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
            m_spDb->Finalize(uiStressHandle);

            if (stProperties.vecSourceStressPos.empty())
            {
                CEString sHwCommentQuery(L"SELECT comment FROM headword WHERE id = ");
                sHwCommentQuery += CEString::sToString(stProperties.llHeadwordId);

                uint64_t uiHwCommentHandle = m_spDb->uiPrepareForSelect(sHwCommentQuery);
                while (m_spDb->bGetRow(uiHwCommentHandle))
                {
                    CEString sHwComment;
                    m_spDb->GetData(0, sHwComment, uiHwCommentHandle);
                    if (L"_без удар._" == sHwComment)
                    {
                        stProperties.bIsUnstressed = true;
                    }
                }
                m_spDb->Finalize(uiHwCommentHandle);

                if (!stProperties.bIsUnstressed && !(POS_PARTICLE == stProperties.ePartOfSpeech) &&
                    1 == stProperties.sSourceForm.uiGetNumOfSyllables())
                {
                    assert(1 == stProperties.sSourceForm.uiGetNumOfSyllables());
                    stProperties.vecSourceStressPos.push_back(0);
                }
            }

            CEString sHomonymsQuery(L"SELECT homonym_number FROM homonyms WHERE headword_id = ");
            sHomonymsQuery += CEString::sToString(stProperties.llHeadwordId);

            uint64_t uiHomonymsHandle = m_spDb->uiPrepareForSelect(sHomonymsQuery);
            while (m_spDb->bGetRow(uiHomonymsHandle))
            {
                int iHomonym = -1;
                m_spDb->GetData(0, iHomonym, uiHomonymsHandle);
                stProperties.vecHomonyms.push_back(iHomonym);
            }
            m_spDb->Finalize(uiHomonymsHandle);

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

                    uint64_t uiAspectPairHandle = m_spDb->uiPrepareForSelect(sAspectPairQuery);
                    while (m_spDb->bGetRow(uiAspectPairHandle))
                    {
                        int iType = 0;
                        bool bIsVariant = false;
                        CEString sData;
                        m_spDb->GetData(0, iType, uiAspectPairHandle);
                        m_spDb->GetData(1, sData, uiAspectPairHandle);
                        m_spDb->GetData(2, bIsVariant, uiAspectPairHandle);
                        m_spDb->GetData(3, stProperties.sAltAspectPairComment, uiAspectPairHandle);

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
                    m_spDb->Finalize(uiAspectPairHandle);
                }
            }
        }   // if (m_spDb->b_GetRow())
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

}    //  eReadDescriptorData()

ET_ReturnCode CDictionary::eReadInflectionData(shared_ptr<CLexeme>spLexeme, uint64_t uiQueryHandle, [[maybe_unused]]bool bIsSpryazhSm)
{
    if (nullptr == m_spDb)
    {
        assert(0);
        ERROR_LOG(L"DB pointer is NULL.");
        return H_ERROR_POINTER;
    }

    ET_ReturnCode rc = H_NO_ERROR;

    /*
        static CEString sQueryBaseInflection(L"SELECT inflection.id, inflection.is_primary, inflection.inflection_type, inflection.accent_type1,
                                                            0                       1                       2                           3
                                               inflection.accent_type2, inflection.short_form_restrictions, inflection.past_part_restrictions,
                                                              4                               5                                    6
                                               inflection.no_short_form, inflection.no_past_part, inflection.fleeting_vowel, inflection.stem_augment ");
                                                               7                           8                      9                         10
    */
    try
    {
        if (m_spDb->bGetRow(uiQueryHandle))
        {
            auto spInflection = make_shared<CInflection>(spLexeme.get());

            auto& stProperties = spInflection->stGetPropertiesForWriteAccess();

            m_spDb->GetData(0, stProperties.llInflectionId, uiQueryHandle);            //  0 source
            m_spDb->GetData(1, stProperties.bPrimaryInflectionGroup, uiQueryHandle);             //  1 is_primary
            m_spDb->GetData(2, stProperties.iType, uiQueryHandle);                               //  2 inflection_type
            int iAccentType1 = 0;
            m_spDb->GetData(3, iAccentType1, uiQueryHandle);                                     //  3 accent_type1
            stProperties.eAccentType1 = (ET_AccentType)iAccentType1;
            int iAccentType2 = 0;
            m_spDb->GetData(4, iAccentType2, uiQueryHandle);                                     //  4 accent_type2
            stProperties.eAccentType2 = (ET_AccentType)iAccentType2;
            m_spDb->GetData(5, stProperties.bShortFormsRestricted, uiQueryHandle);               //  5 short_form_restrictions
            if (spLexeme->sMainSymbol() == L"м")
            {
                stProperties.bShortFormsRestricted = false;     // nominalized adjectives: text parser may copy that from source
            }
            m_spDb->GetData(6, stProperties.bPastParticipleRestricted, uiQueryHandle);           //  6 past_part_restrictions
            m_spDb->GetData(7, stProperties.bShortFormsIncomplete, uiQueryHandle);               //  7 no_short_form
            if (spLexeme->sMainSymbol() == L"м")
            {
                stProperties.bShortFormsIncomplete = false;     // nominalized adjectives: text parser may copy that from source
            }
            m_spDb->GetData(8, stProperties.bNoPassivePastParticiple, uiQueryHandle);            //  8 no_past_part
            m_spDb->GetData(9, stProperties.bFleetingVowel, uiQueryHandle);                      //  9 fleeting_vowel

            m_spDb->GetData(10, stProperties.iStemAugment, uiQueryHandle);                       // 10 stem_augment

            CEString sDeviationQuery(L"SELECT deviation_type, is_optional FROM common_deviation WHERE inflection_id = ");
            sDeviationQuery += CEString::sToString(stProperties.llInflectionId);

            uint64_t uiDeviationHandle = m_spDb->uiPrepareForSelect(sDeviationQuery);
            while (m_spDb->bGetRow(uiDeviationHandle))
            {
                int iType = -1;
                bool bOptional = false;
                m_spDb->GetData(0, iType, uiDeviationHandle);
                m_spDb->GetData(1, bOptional, uiDeviationHandle);
                stProperties.mapCommonDeviations[iType] = bOptional;
            }
            m_spDb->Finalize(uiDeviationHandle);
            spLexeme->AddInflection(spInflection);

        }   // if (m_spDb->bGetRow())
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

}    //  eReadInflectionData()

ET_ReturnCode CDictionary::eGetP2Data(int64_t llLexemeId, StLexemeProperties& stProperties)
{
    if (nullptr == m_spDb)
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

        uint64_t uiHandle = m_spDb->uiPrepareForSelect(sQuery);

        stProperties.bSecondPrepositional = false;

        if (m_spDb->bGetRow(uiHandle))
        {
            stProperties.bSecondPrepositional = true;
            m_spDb->GetData(0, stProperties.bSecondPrepositionalOptional, uiHandle);
            m_spDb->GetData(1, stProperties.sP2Preposition, uiHandle);
        }

        if (m_spDb->bGetRow(uiHandle))
        {
            assert(0);
            CEString sMsg(L"More than one P2 record for ");
            sMsg += stProperties.sSourceForm;
            ERROR_LOG(sMsg);
            //        return E_FAIL;
        }

        m_spDb->Finalize(uiHandle);
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

ET_ReturnCode CDictionary::eDeleteLexeme(CLexeme* spLexeme)
{
    const StLexemeProperties& stProperties = spLexeme->stGetProperties();

    try
    {
        if (nullptr == m_spDb)
        {
            assert(0);
            ERROR_LOG(L"DB pointer is NULL.");
            return H_ERROR_POINTER;
        }

        CEString sWordQuery = L"SELECT word_id FROM descriptor WHERE id = " + CEString::sToString(stProperties.llDescriptorId);
        m_spDb->PrepareForSelect(sWordQuery);
        bool bFound = false;
        int iWordId = -1;
        while (m_spDb->bGetRow())
        {
            if (bFound)
            {
                ERROR_LOG(L"Warning: multiple word ID's.");
                break;
            }
            m_spDb->GetData(0, iWordId);
            if (iWordId >= 0)
            {
                bFound = true;
            }
        }
        m_spDb->Finalize();

        CEString sInflectionQuery = L"SELECT id FROM inflection WHERE descriptor_id = " + CEString::sToString(stProperties.llDescriptorId);
        m_spDb->PrepareForSelect(sInflectionQuery);
        vector<long long> vecInflectionIds;
        while (m_spDb->bGetRow())
        {
            int64_t uiInflectionId = -1;
            m_spDb->GetData(0, uiInflectionId);
            vecInflectionIds.push_back((long long)uiInflectionId);
        }
        m_spDb->Finalize();

        for (auto llInflectionId : vecInflectionIds)
        {
            CEString sDeleteCommonDeviationQuery(L"DELETE FROM common_deviation WHERE inflection_id = " + CEString::sToString(llInflectionId));
            m_spDb->Exec(sDeleteCommonDeviationQuery);
        }

        vector<CEString> vecTables{ L"aspect_pair", L"missing_forms", L"difficult_forms", L"second_genitive", L"second_locative", L"inflection" };

        for (CEString sTable : vecTables)
        {
            CEString sDeleteStmt = L"DELETE FROM " + sTable + L" WHERE descriptor_id = ";
            sDeleteStmt += CEString::sToString(stProperties.llDescriptorId);
            sDeleteStmt += L";";
            m_spDb->Exec(sDeleteStmt);
        }

        CEString sSelectIrregularStmt = L"SELECT id FROM irregular_forms WHERE descriptor_id = " + 
        CEString::sToString(stProperties.llDescriptorId) + L";";
        m_spDb->PrepareForSelect(sSelectIrregularStmt);
        vector<int> vecIrregularFormIds;
        while (m_spDb->bGetRow())
        {
            int iIrregularFormId = -1;
            m_spDb->GetData(0, iIrregularFormId);
            vecIrregularFormIds.push_back(iIrregularFormId);
        }
        m_spDb->Finalize();

        for (auto sId : vecIrregularFormIds)
        {
            CEString sDeleteStmt = L"DELETE FROM irregular_stress WHERE id = ";
            sDeleteStmt += CEString::sToString(sId);
            sDeleteStmt += L";";
            m_spDb->Exec(sDeleteStmt);
        }

        CEString sDeleteDescriptorQuery(L"DELETE FROM descriptor WHERE id = " + CEString::sToString(stProperties.llDescriptorId));
        m_spDb->Exec(sDeleteDescriptorQuery);

        bool bDeleteHeadword = false;
        CEString sExistQuery = L"SELECT 1 FROM descriptor WHERE word_id = " + CEString::sToString(iWordId) + L";";
        m_spDb->PrepareForSelect(sExistQuery);
        if (!m_spDb->bGetRow())
        {
            bDeleteHeadword = true;
        }
        m_spDb->Finalize();

        if (bDeleteHeadword)
        {
            CEString sDeleteStmt = L"DELETE FROM stress WHERE headword_id = ";
            sDeleteStmt += CEString::sToString(iWordId);
            sDeleteStmt += L";";
            m_spDb->Exec(sDeleteStmt);

            sDeleteStmt = L"DELETE FROM homonyms WHERE headword_id = ";
            sDeleteStmt += CEString::sToString(iWordId);
            sDeleteStmt += L";";
            m_spDb->Exec(sDeleteStmt);

            sDeleteStmt = L"DELETE FROM headword WHERE id = ";
            sDeleteStmt += CEString::sToString(iWordId);
            sDeleteStmt += L";";
            m_spDb->Exec(sDeleteStmt);
        }
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}        //  eDeleteLexeme()

ET_ReturnCode CDictionary::eUpdateHeadword(CLexeme* spLexeme)
{
//    ET_ReturnCode eRet = H_NO_ERROR;

    StLexemeProperties& stProperties = spLexeme->stGetPropertiesForWriteAccess();
    if (stProperties.llHeadwordId < 0)
    {
        ERROR_LOG(L"Invalid headword ID");
        return H_ERROR_UNEXPECTED;
    }

    try
    {
        m_spDb->BeginTransaction();

        sqlite3_stmt* pStmt = nullptr;
        vector<CEString> vecColumns = { L"source", L"plural_of", L"comment", L"usage", L"variant",
            L"variant_comment", L"see_ref", L"back_ref", L"source_entry_id", L"spryazh_sm", L"second_part", 
            L"is_edited"};
        auto llUpdateHandle = m_spDb->uiPrepareForUpdate(L"headword", vecColumns, stProperties.llHeadwordId, pStmt);
        m_spDb->Bind(1, stProperties.sSourceForm, llUpdateHandle);
        m_spDb->Bind(2, stProperties.sPluralOf, llUpdateHandle);
        m_spDb->Bind(3, stProperties.sHeadwordComment, llUpdateHandle);
        m_spDb->Bind(4, stProperties.sUsage, llUpdateHandle);
        m_spDb->Bind(5, stProperties.sHeadwordVariant, llUpdateHandle);
        m_spDb->Bind(6, stProperties.sHeadwordVariantComment, llUpdateHandle);
        m_spDb->Bind(7, stProperties.sSeeRef, llUpdateHandle);
        m_spDb->Bind(8, stProperties.sBackRef, llUpdateHandle);
        uint64_t uiSourceEntryId = 0;
        m_spDb->Bind(9, uiSourceEntryId, llUpdateHandle);
        stProperties.llSourceEntryId = uiSourceEntryId;
        m_spDb->Bind(10, stProperties.bSpryazhSm, llUpdateHandle);
        m_spDb->Bind(11, stProperties.bSecondPart, llUpdateHandle);
        m_spDb->Bind(12, true, llUpdateHandle);                   // is_edited

        m_spDb->UpdateRow(llUpdateHandle);
        m_spDb->Finalize(llUpdateHandle);

        m_spDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        m_spDb->RollbackTransaction();
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eUpdateHeadword()

ET_ReturnCode CDictionary::eSaveNewHeadword(CLexeme * spLexeme)
{
//    ET_ReturnCode eRet = H_NO_ERROR;

    StLexemeProperties& stProperties = spLexeme->stGetPropertiesForWriteAccess();
    if (stProperties.llHeadwordId >= 0)
    {
        ERROR_LOG(L"Invalid headword ID");
        return H_ERROR_UNEXPECTED;
    }

    try
    {
        m_spDb->BeginTransaction();

        sqlite3_stmt* pStmt = nullptr;
        m_spDb->uiPrepareForInsert(L"headword", 12, pStmt);
        auto llInsertHandle = (unsigned long long)pStmt;

        m_spDb->Bind(1, stProperties.sSourceForm, llInsertHandle);
        m_spDb->Bind(2, stProperties.sPluralOf, llInsertHandle);
        m_spDb->Bind(3, stProperties.sHeadwordComment, llInsertHandle);
        m_spDb->Bind(4, stProperties.sUsage, llInsertHandle);
        m_spDb->Bind(5, stProperties.sHeadwordVariant, llInsertHandle);
        m_spDb->Bind(6, stProperties.sHeadwordVariantComment, llInsertHandle);
        m_spDb->Bind(7, stProperties.sSeeRef, llInsertHandle);
        m_spDb->Bind(8, stProperties.sBackRef, llInsertHandle);
        uint64_t uiSourceEntryId = 0;
        m_spDb->Bind(9, uiSourceEntryId, llInsertHandle);
        stProperties.llSourceEntryId = uiSourceEntryId;
        m_spDb->Bind(10, stProperties.bSpryazhSm, llInsertHandle);
        m_spDb->Bind(11, stProperties.bSecondPart, llInsertHandle);
        m_spDb->Bind(12, true, llInsertHandle);                   // is_edited

        m_spDb->InsertRow(llInsertHandle);

        stProperties.llHeadwordId = m_spDb->llGetLastKey();

        m_spDb->Finalize(llInsertHandle);

        if (stProperties.vecHomonyms.size() > 0)
        {
            sqlite3_stmt* pStmt = nullptr;
            m_spDb->uiPrepareForInsertOrReplace(L"homonyms", 4, pStmt);
            llInsertHandle = (unsigned long long)pStmt;
            for (auto iHomonym : stProperties.vecHomonyms)
            {
                int64_t iHeadwordId = stProperties.llHeadwordId;
                m_spDb->Bind(1, iHeadwordId, llInsertHandle);
                m_spDb->Bind(2, iHomonym, llInsertHandle);
                m_spDb->Bind(3, false, llInsertHandle);      // is_variant
                m_spDb->Bind(4, true, llInsertHandle);       // is_edited
            }

            m_spDb->InsertRow(llInsertHandle);
            m_spDb->Finalize(llInsertHandle);
        }

        stProperties.sSourceForm.SetVowels(CEString::g_szRusVowels);

        llInsertHandle = 0;
        if (stProperties.vecSourceStressPos.size() > 0)
        {
            pStmt = nullptr;
            m_spDb->uiPrepareForInsert(L"stress", 5, pStmt);
            llInsertHandle = (unsigned long long)pStmt;

            for (auto iStressPos : stProperties.vecSourceStressPos)
            {
                int64_t iHeadwordId = stProperties.llHeadwordId;
                m_spDb->Bind(1, iHeadwordId, llInsertHandle);
                m_spDb->Bind(2, iStressPos, llInsertHandle);
                m_spDb->Bind(3, true, llInsertHandle);       // primary
                m_spDb->Bind(4, false, llInsertHandle);      // not a variant
                m_spDb->Bind(5, true, llInsertHandle);       // is_edited
                m_spDb->InsertRow(llInsertHandle);
            }

            for (auto iStressPos : stProperties.vecSecondaryStressPos)
            {
                int64_t iHeadwordId = stProperties.llHeadwordId;
                m_spDb->Bind(1, iHeadwordId, llInsertHandle);
                m_spDb->Bind(2, iStressPos, llInsertHandle);
                m_spDb->Bind(3, false, llInsertHandle);       // secondary
                m_spDb->Bind(4, false, llInsertHandle);       // not a variant
                m_spDb->Bind(5, true, llInsertHandle);        // is_edited
                m_spDb->InsertRow(llInsertHandle);
            }

            for (auto iStressPos : stProperties.vecSourceVariantStressPos)
            {
                int64_t iHeadwordId = stProperties.llHeadwordId;
                m_spDb->Bind(1, iHeadwordId, llInsertHandle);
                m_spDb->Bind(2, iStressPos, llInsertHandle);
                m_spDb->Bind(3, true, llInsertHandle);       // primary
                m_spDb->Bind(4, true, llInsertHandle);       // variant
                m_spDb->Bind(5, true, llInsertHandle);       // is_edited
                m_spDb->InsertRow(llInsertHandle);
            }

            for (auto iStressPos : stProperties.vecSecondaryVariantStressPos)
            {
                int64_t iHeadwordId = stProperties.llHeadwordId;
                m_spDb->Bind(1, iHeadwordId, llInsertHandle);

                m_spDb->Bind(2, iStressPos, llInsertHandle);
                m_spDb->Bind(3, false, llInsertHandle);       // primary
                m_spDb->Bind(4, true, llInsertHandle);        // variant
                m_spDb->Bind(5, true, llInsertHandle);        // is_edited
                m_spDb->InsertRow(llInsertHandle);
            }

            m_spDb->Finalize(llInsertHandle);
        }
        m_spDb->CommitTransaction();

    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        m_spDb->RollbackTransaction();
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveNewHeadword()

ET_ReturnCode CDictionary::eSaveHeadwordStress(CLexeme* spLexeme)
{
    StLexemeProperties& stProperties = spLexeme->stGetPropertiesForWriteAccess();
    if (stProperties.llHeadwordId < 0)
    {
        ERROR_LOG(L"Invalid headword ID");
        return H_ERROR_UNEXPECTED;
    }

    int64_t iHeadwordId = stProperties.llHeadwordId;

    try
    {
        m_spDb->BeginTransaction();

        CEString sDeleteStressQuery(L"DELETE FROM stress WHERE headword_id = ");
        sDeleteStressQuery += CEString::sToString(iHeadwordId);
        m_spDb->Exec(sDeleteStressQuery);

        if (stProperties.vecSourceStressPos.size() > 0)
        {
            sqlite3_stmt* pStmt = nullptr;
            m_spDb->uiPrepareForInsert(L"stress", 5, pStmt);
            auto llInsertHandle = (long long)pStmt;


            for (auto iStressPos : stProperties.vecSourceStressPos)
            {

                m_spDb->Bind(1, iHeadwordId, llInsertHandle);
                m_spDb->Bind(2, iStressPos, llInsertHandle);
                m_spDb->Bind(3, true, llInsertHandle);       // primary
                m_spDb->Bind(4, false, llInsertHandle);      // not a variant
                m_spDb->Bind(5, true, llInsertHandle);       // is_edited
                m_spDb->InsertRow(llInsertHandle);
            }

            for (auto iStressPos : stProperties.vecSecondaryStressPos)
            {
                m_spDb->Bind(1, iHeadwordId, llInsertHandle);
                m_spDb->Bind(2, iStressPos, llInsertHandle);
                m_spDb->Bind(3, false, llInsertHandle);       // secondary
                m_spDb->Bind(4, false, llInsertHandle);       // not a variant
                m_spDb->Bind(5, true, llInsertHandle);        // is_edited
                m_spDb->InsertRow(llInsertHandle);
            }

            for (auto iStressPos : stProperties.vecSourceVariantStressPos)
            {
                m_spDb->Bind(1, iHeadwordId, llInsertHandle);
                m_spDb->Bind(2, iStressPos, llInsertHandle);
                m_spDb->Bind(3, true, llInsertHandle);       // primary
                m_spDb->Bind(4, true, llInsertHandle);       // variant
                m_spDb->Bind(5, true, llInsertHandle);       // is_edited
                m_spDb->InsertRow(llInsertHandle);
            }

            for (auto iStressPos : stProperties.vecSecondaryVariantStressPos)
            {
                m_spDb->Bind(1, iHeadwordId, llInsertHandle);
                m_spDb->Bind(2, iStressPos, llInsertHandle);
                m_spDb->Bind(3, false, llInsertHandle);       // primary
                m_spDb->Bind(4, true, llInsertHandle);        // variant
                m_spDb->Bind(5, true, llInsertHandle);        // is_edited
                m_spDb->InsertRow(llInsertHandle);
            }

            m_spDb->Finalize(llInsertHandle);
        }

        m_spDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        m_spDb->RollbackTransaction();
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveHeadwordStress()

ET_ReturnCode CDictionary::eSaveHomonyms(CLexeme* spLexeme)
{
//    ET_ReturnCode eRet = H_NO_ERROR;

    StLexemeProperties& stProperties = spLexeme->stGetPropertiesForWriteAccess();
    if (stProperties.llHeadwordId < 0)
    {
        ERROR_LOG(L"Invalid headword ID");
        return H_ERROR_UNEXPECTED;
    }

    int64_t iHeadwordId = stProperties.llHeadwordId;

    try
    {
        m_spDb->BeginTransaction();

        CEString sDeleteStressQuery(L"DELETE FROM stress WHERE headword_id = ");
        sDeleteStressQuery += CEString::sToString(iHeadwordId);
        m_spDb->Exec(sDeleteStressQuery);

        if (stProperties.vecHomonyms.size() > 0)
        {
            sqlite3_stmt* pStmt = nullptr;
            m_spDb->uiPrepareForInsertOrReplace(L"homonyms", 4, pStmt);
            auto llInsertHandle = (long long)pStmt;

            for (auto iHomonym : stProperties.vecHomonyms)
            {
                m_spDb->Bind(1, iHeadwordId, llInsertHandle);
                m_spDb->Bind(2, iHomonym, llInsertHandle);
                m_spDb->Bind(3, false, llInsertHandle);      // is_variant
                m_spDb->Bind(4, true, llInsertHandle);       // is_edited
            }

            m_spDb->InsertRow(llInsertHandle);
            m_spDb->Finalize(llInsertHandle);
        }

        m_spDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        m_spDb->RollbackTransaction();
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveHomonyms()

ET_ReturnCode CDictionary::eSaveAspectPairInfo(CLexeme* spLexeme)
{
//    ET_ReturnCode eRet = H_NO_ERROR;

    try
    {
        m_spDb->BeginTransaction();

        StLexemeProperties& stProperties = spLexeme->stGetPropertiesForWriteAccess();

        CEString sDeleteAspectPairQuery(L"DELETE FROM aspect_pair WHERE descriptor_id = ");
        sDeleteAspectPairQuery += CEString::sToString(stProperties.llDescriptorId);
        m_spDb->Exec(sDeleteAspectPairQuery);

        sqlite3_stmt* pStmt = nullptr;
        m_spDb->uiPrepareForInsert(L"aspect_pair", 6, pStmt);
        auto llInsertHandle = (unsigned long long)pStmt;

        auto iDescriptorId = (int64_t)stProperties.llDescriptorId;
        m_spDb->Bind(1, iDescriptorId, llInsertHandle);
        m_spDb->Bind(2, stProperties.iAspectPairType, llInsertHandle);
        m_spDb->Bind(3, stProperties.sAltAspectPairData, llInsertHandle);
        m_spDb->Bind(4, stProperties.sAltAspectPairComment, llInsertHandle);                          
        m_spDb->Bind(5, false, llInsertHandle);                  // is variant -- currently not supported by UI
        m_spDb->Bind(6, true, llInsertHandle);                   // is_edited

        m_spDb->InsertRow(llInsertHandle);
        m_spDb->Finalize(llInsertHandle);
        m_spDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        m_spDb->RollbackTransaction();
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveAspectPairInfo()

ET_ReturnCode CDictionary::eSaveP2Info(CLexeme* spLexeme)
{
//    ET_ReturnCode eRet = H_NO_ERROR;

    try
    {
        m_spDb->BeginTransaction();

        StLexemeProperties& stProperties = spLexeme->stGetPropertiesForWriteAccess();

        CEString sDeleteAspectPairQuery(L"DELETE FROM second_locative WHERE descriptor_id = ");
        sDeleteAspectPairQuery += CEString::sToString(stProperties.llDescriptorId);
        m_spDb->Exec(sDeleteAspectPairQuery);

        sqlite3_stmt* pStmt = nullptr;
        m_spDb->uiPrepareForInsert(L"second_locative", 4, pStmt);
        auto llInsertHandle = (unsigned long long)pStmt;

        int64_t iDescriptorId = stProperties.llDescriptorId;
        m_spDb->Bind(1, iDescriptorId, llInsertHandle);
        m_spDb->Bind(2, stProperties.bSecondPrepositionalOptional, llInsertHandle);
        m_spDb->Bind(3, stProperties.sP2Preposition, llInsertHandle);
        m_spDb->Bind(4, true, llInsertHandle);                   // is_edited

        m_spDb->InsertRow(llInsertHandle);
        m_spDb->Finalize(llInsertHandle);
        m_spDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        m_spDb->RollbackTransaction();
        return H_EXCEPTION;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CDictionary::eSaveCommonDeviation(CInflection* spInflection)
{
//    ET_ReturnCode eRet = H_NO_ERROR;

    try
    {
        m_spDb->BeginTransaction();

        StInflectionProperties& stProperties = spInflection->stGetPropertiesForWriteAccess();

        if (stProperties.llInflectionId < 0)
        {
            ERROR_LOG(L"Illegal inflection key.");
            return H_ERROR_DB;
        }

        CEString sDeleteCommonDeviationQuery(L"DELETE FROM common_deviation WHERE inflection_id = ");
        sDeleteCommonDeviationQuery += CEString::sToString(stProperties.llInflectionId);
        m_spDb->Exec(sDeleteCommonDeviationQuery);

        for (auto& pairDeviation : stProperties.mapCommonDeviations)
        {
            sqlite3_stmt* pStmt = nullptr;
            m_spDb->uiPrepareForInsert(L"common_deviation", 4, pStmt);
            auto llInsertHandle = (unsigned long long)pStmt;

            int64_t iInflectionId = stProperties.llInflectionId;
            m_spDb->Bind(1, iInflectionId, llInsertHandle);
            m_spDb->Bind(2, pairDeviation.first, llInsertHandle);
            m_spDb->Bind(3, pairDeviation.second, llInsertHandle);
            m_spDb->Bind(4, true, llInsertHandle);                   // is_edited

            m_spDb->InsertRow(llInsertHandle);
            m_spDb->Finalize(llInsertHandle);
        }

        m_spDb->CommitTransaction();

    }
    catch (CException& ex)
    {
        m_spDb->RollbackTransaction();
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveCommonDeviation()

ET_ReturnCode CDictionary::eSaveInflectionInfo(CInflection* spInflection)
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
        m_spDb->BeginTransaction();

        StInflectionProperties& stProperties = spInflection->stGetPropertiesForWriteAccess();

//        if (stProperties.llInflectionId < 0)
//        {
//            ERROR_LOG(L"Warning: no inflection key, assume new lexeme.");
//            return H_ERROR_DB;
//        }
//        else
//        {
//            CEString sDeleteInflectionQuery(L"DELETE FROM inflection WHERE descriptor_id = ");
//            sDeleteInflectionQuery += CEString::sToString(stProperties.llDescriptorId);
//            m_spDb->Exec(sDeleteInflectionQuery);
//        }

        sqlite3_stmt* pStmt = nullptr;
        m_spDb->uiPrepareForInsert(L"inflection", 12, pStmt);
        auto llInsertHandle = (unsigned long long)pStmt;

        int64_t iDescriptorId = spInflection->llDescriptorId();
        m_spDb->Bind(1, iDescriptorId, llInsertHandle);
        m_spDb->Bind(2, stProperties.bPrimaryInflectionGroup, llInsertHandle);
        m_spDb->Bind(3, stProperties.iType, llInsertHandle);
        m_spDb->Bind(4, stProperties.eAccentType1, llInsertHandle);
        m_spDb->Bind(5, stProperties.eAccentType2, llInsertHandle);
        m_spDb->Bind(6, stProperties.bShortFormsRestricted, llInsertHandle);
        m_spDb->Bind(7, stProperties.bPastParticipleRestricted, llInsertHandle);
        m_spDb->Bind(8, stProperties.bShortFormsIncomplete, llInsertHandle);
        m_spDb->Bind(9, stProperties.bNoPassivePastParticiple, llInsertHandle);
        m_spDb->Bind(10, stProperties.bFleetingVowel, llInsertHandle);
        m_spDb->Bind(11, stProperties.iStemAugment, llInsertHandle);
        m_spDb->Bind(12, true, llInsertHandle);                   // is_edited

        m_spDb->InsertRow(llInsertHandle);
        m_spDb->Finalize(llInsertHandle);

        stProperties.llInflectionId = m_spDb->llGetLastKey();

        m_spDb->CommitTransaction();

    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        m_spDb->RollbackTransaction();
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eSaveInflectionInfo()

ET_ReturnCode CDictionary::eUpdateDescriptorInfo(CLexeme* spLexeme)
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

    StLexemeProperties& stProperties = spLexeme->stGetPropertiesForWriteAccess();
    if (stProperties.llDescriptorId < 0)
    {
        ERROR_LOG(L"Unable to update descriptor: illegal descripto ID");
        return H_ERROR_UNEXPECTED;
    }

    try
    {
        m_spDb->BeginTransaction();

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
        auto llUpdateHandle = m_spDb->uiPrepareForUpdate(L"descriptor", vecColumns, stProperties.llDescriptorId, pStmt);

        int64_t iHeadwordId = stProperties.llHeadwordId;
        m_spDb->Bind(1, iHeadwordId, llUpdateHandle);                            //  1 word_id
        m_spDb->Bind(2, stProperties.sGraphicStem, llUpdateHandle);              //  2 graphic_stem
        int64_t iSecondPartId = stProperties.llSecondPartId;
        m_spDb->Bind(3, iSecondPartId, llUpdateHandle);                          //  3 second_part_id
        m_spDb->Bind(4, stProperties.bIsVariant, llUpdateHandle);                //  4 is_variant
        m_spDb->Bind(5, stProperties.sMainSymbol, llUpdateHandle);               //  5 main_symbol
        m_spDb->Bind(6, stProperties.ePartOfSpeech, llUpdateHandle);             //  6 part_of_speech
        m_spDb->Bind(7, stProperties.bIsPluralOf, llUpdateHandle);               //  7 is_plural_of
        m_spDb->Bind(8, !stProperties.bTransitive, llUpdateHandle);              //  8 inflection_type
        bool bIsRefexive = (REFL_YES == stProperties.eReflexive) ? true : false;
        m_spDb->Bind(9, bIsRefexive, llUpdateHandle);                            //  9 is_reflexive
        m_spDb->Bind(10, stProperties.sMainSymbolPluralOf, llUpdateHandle);      // 10 main_symbol_plural_of
        m_spDb->Bind(11, stProperties.sAltMainSymbol, llUpdateHandle);           // 11 alt_main_symbol
        m_spDb->Bind(12, stProperties.sInflectionType, llUpdateHandle);          // 12 inflection_type
        m_spDb->Bind(13, stProperties.sComment, llUpdateHandle);                 // 13 comment
        m_spDb->Bind(14, stProperties.sAltMainSymbolComment, llUpdateHandle);    // 14 alt_main_symbol_comment
        m_spDb->Bind(15, stProperties.sAltInflectionComment, llUpdateHandle);    // 15 alt_inflection_comment
        m_spDb->Bind(16, stProperties.sVerbStemAlternation, llUpdateHandle);     // 16 verb_stem_alternation
        m_spDb->Bind(17, stProperties.bPartPastPassZhd, llUpdateHandle);         // 17 part_past_pass_zhd
        m_spDb->Bind(18, stProperties.iSection, llUpdateHandle);                 // 18 section
        m_spDb->Bind(19, stProperties.bNoComparative, llUpdateHandle);           // 19 no_comparative
        m_spDb->Bind(20, stProperties.bNoLongForms, llUpdateHandle);             // 20 no_long_forms
        m_spDb->Bind(21, stProperties.bAssumedForms, llUpdateHandle);            // 21 assumed_forms
        m_spDb->Bind(22, stProperties.bYoAlternation, llUpdateHandle);           // 22 yo_alternation
        m_spDb->Bind(23, stProperties.bOAlternation, llUpdateHandle);            // 23 o_alternation
        m_spDb->Bind(24, stProperties.bSecondGenitive, llUpdateHandle);          // 24 second_genitive
        m_spDb->Bind(25, stProperties.bIsImpersonal, llUpdateHandle);            // 25 is_impersonal
        m_spDb->Bind(26, stProperties.bIsIterative, llUpdateHandle);             // 26 is_iterative
        m_spDb->Bind(27, stProperties.bHasAspectPair, llUpdateHandle);           // 27 has_aspect_pair
        m_spDb->Bind(28, stProperties.bHasDifficultForms, llUpdateHandle);       // 28 has_difficult_forms
        m_spDb->Bind(29, stProperties.bHasMissingForms, llUpdateHandle);         // 29 has_missing_forms
        m_spDb->Bind(30, stProperties.bHasIrregularForms, llUpdateHandle);       // 30 has_irregular_forms
        m_spDb->Bind(31, stProperties.sIrregularFormsLeadComment, llUpdateHandle);    //  31 irregular_forms_lead_comment
        m_spDb->Bind(32, stProperties.sRestrictedContexts, llUpdateHandle);      // 32 restricted_contexts
        m_spDb->Bind(33, stProperties.sContexts, llUpdateHandle);                // 33 contexts
        m_spDb->Bind(34, stProperties.sCognate, llUpdateHandle);                 // 34 cognate
        m_spDb->Bind(35, stProperties.sTrailingComment, llUpdateHandle);         // 35 trailing_comment
        m_spDb->Bind(36, true, llUpdateHandle);                                  // 36 is_edited

        m_spDb->InsertRow(llUpdateHandle);
        m_spDb->Finalize(llUpdateHandle);

        m_spDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        m_spDb->RollbackTransaction();
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eUpdateDescriptorInfo()

ET_ReturnCode CDictionary::eSaveDescriptorInfo(CLexeme* spLexeme)
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

    StLexemeProperties& stProperties = spLexeme->stGetPropertiesForWriteAccess();

    try
    {
        m_spDb->BeginTransaction();

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
        m_spDb->uiPrepareForInsert(L"descriptor", 36, pStmt, true);
        auto llInsertHandle = (unsigned long long)pStmt;

        int64_t iHeadwordId = stProperties.llHeadwordId;
        m_spDb->Bind(1, iHeadwordId, llInsertHandle);                                //  1 word_id
        m_spDb->Bind(2, stProperties.sGraphicStem, llInsertHandle);                  //  2 graphic_stem
        int64_t iSecondPartId = stProperties.llSecondPartId;
        m_spDb->Bind(3, iSecondPartId, llInsertHandle);                              //  3 second_part_id
        m_spDb->Bind(4, stProperties.bIsVariant, llInsertHandle);                    //  4 is_variant
        m_spDb->Bind(5, stProperties.sMainSymbol, llInsertHandle);                   //  5 main_symbol
        m_spDb->Bind(6, stProperties.ePartOfSpeech, llInsertHandle);                 //  6 part_of_speech
        m_spDb->Bind(7, stProperties.bIsPluralOf, llInsertHandle);                   //  7 is_plural_of
        m_spDb->Bind(8, !stProperties.bTransitive, llInsertHandle);                  //  8 inflection_type
        bool bIsRefexive = (REFL_YES == stProperties.eReflexive) ? true : false;
        m_spDb->Bind(9, bIsRefexive, llInsertHandle);                            //  9 is_reflexive
        m_spDb->Bind(10, stProperties.sMainSymbolPluralOf, llInsertHandle);      // 10 main_symbol_plural_of
        m_spDb->Bind(11, stProperties.sAltMainSymbol, llInsertHandle);           // 11 alt_main_symbol
        m_spDb->Bind(12, stProperties.sInflectionType, llInsertHandle);          // 12 inflection_type
        m_spDb->Bind(13, stProperties.sComment, llInsertHandle);                 // 13 comment
        m_spDb->Bind(14, stProperties.sAltMainSymbolComment, llInsertHandle);    // 14 alt_main_symbol_comment
        m_spDb->Bind(15, stProperties.sAltInflectionComment, llInsertHandle);    // 15 alt_inflection_comment
        m_spDb->Bind(16, stProperties.sVerbStemAlternation, llInsertHandle);     // 16 verb_stem_alternation
        m_spDb->Bind(17, stProperties.bPartPastPassZhd, llInsertHandle);         // 17 part_past_pass_zhd
        m_spDb->Bind(18, stProperties.iSection, llInsertHandle);                 // 18 section
        m_spDb->Bind(19, stProperties.bNoComparative, llInsertHandle);           // 19 no_comparative
        m_spDb->Bind(20, stProperties.bNoLongForms, llInsertHandle);             // 20 no_long_forms
        m_spDb->Bind(21, stProperties.bAssumedForms, llInsertHandle);            // 21 assumed_forms
        m_spDb->Bind(22, stProperties.bYoAlternation, llInsertHandle);           // 22 yo_alternation
        m_spDb->Bind(23, stProperties.bOAlternation, llInsertHandle);            // 23 o_alternation
        m_spDb->Bind(24, stProperties.bSecondGenitive, llInsertHandle);          // 24 second_genitive
        m_spDb->Bind(25, stProperties.bIsImpersonal, llInsertHandle);            // 25 is_impersonal
        m_spDb->Bind(26, stProperties.bIsIterative, llInsertHandle);             // 26 is_iterative
        m_spDb->Bind(27, stProperties.bHasAspectPair, llInsertHandle);           // 27 has_aspect_pair
        m_spDb->Bind(28, stProperties.bHasDifficultForms, llInsertHandle);       // 28 has_difficult_forms
        m_spDb->Bind(29, stProperties.bHasMissingForms, llInsertHandle);         // 29 has_missing_forms
        m_spDb->Bind(30, stProperties.bHasIrregularForms, llInsertHandle);       // 30 has_irregular_forms
        m_spDb->Bind(31, stProperties.sIrregularFormsLeadComment, llInsertHandle);    //  31 irregular_forms_lead_comment
        m_spDb->Bind(32, stProperties.sRestrictedContexts, llInsertHandle);      // 32 restricted_contexts
        m_spDb->Bind(33, stProperties.sContexts, llInsertHandle);                // 33 contexts
        m_spDb->Bind(34, stProperties.sCognate, llInsertHandle);                 // 34 cognate
        m_spDb->Bind(35, stProperties.sTrailingComment, llInsertHandle);         // 35 trailing_comment
        m_spDb->Bind(36, true, llInsertHandle);                                  // 36 is_edited

        m_spDb->InsertRow(llInsertHandle);
        m_spDb->Finalize(llInsertHandle);

        stProperties.llDescriptorId = m_spDb->llGetLastKey();

        m_spDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        m_spDb->RollbackTransaction();
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eSaveDescriptorInfo()

ET_ReturnCode CDictionary::eGetWordIdFromLemexeId(int64_t llDescriptorId, int64_t& llWordId)
{
    ET_ReturnCode eRet = H_FALSE;

    try
    {
        CEString sQuery = L"SELECT word_id FROM descriptor WHERE id = " + CEString::sToString(llDescriptorId);
        m_spDb->PrepareForSelect(sQuery);
        bool bFound = false;
        while (m_spDb->bGetRow())
        {
            if (bFound)
            {
                ERROR_LOG(L"Warning: multiple word ID's.");
                break;
            }
            m_spDb->GetData(0, llWordId);
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

ET_ReturnCode CDictionary::eMarkLexemeAsEdited(shared_ptr<CLexeme> spLexeme)
{
    long long llDescriptorId = spLexeme->llLexemeId();

    if (llDescriptorId < 0)
    {
        ERROR_LOG(L"Illegal descriptor ID.");
        return H_ERROR_UNEXPECTED;
    }

    if (nullptr == m_spDb)
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
        uint64_t uiHandle = m_spDb->uiPrepareForSelect(sQuery);
        if (m_spDb->bGetRow(uiHandle))
        {
            m_spDb->GetData(0, sLexemeHash, uiHandle);
        }

        if (m_spDb->bGetRow(uiHandle))
        {
            assert(0);
            CEString sMsg(L"More than one lexeme hash record for descriptor id ");
            sMsg += CEString::sToString(llDescriptorId);
            ERROR_LOG(sMsg);
        }

        m_spDb->Finalize(uiHandle);
    }
    catch (CException& ex)
    {
        HandleDbException(ex);
        return H_ERROR_DB;
    }

    try
    {
        sqlite3_stmt* pStmt = nullptr;
        m_spDb->uiPrepareForInsert(L"edited_lexemes", 1, pStmt, false);
        auto iInsertHandle = (int64_t)pStmt;
        m_spDb->Bind(1, sLexemeHash, iInsertHandle);
        m_spDb->InsertRow(iInsertHandle);
        m_spDb->Finalize(iInsertHandle);
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            m_spDb->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(m_spDb->iGetLastError());
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
        m_spDb->GetLastError(sError);
        sMsg += CEString(L", error description: ");
        sMsg += sError;
    }
    catch (...)
    {
        sMsg = L"Apparent DB error ";
    }

    sMsg += L", error code = ";
    sMsg += CEString::sToString(m_spDb->iGetLastError());
    ERROR_LOG(sMsg);

}
