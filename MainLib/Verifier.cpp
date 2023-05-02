#include <iostream>
#include <fstream>
#include <locale>

#include "WordForm.h"
#include "Lexeme.h"
#include "Verifier.h"

using namespace Hlib;

CVerifier::CVerifier(shared_ptr<CDictionary> spDict) : m_spDictionary(spDict), m_eResult(TEST_RESULT_UNDEFINED)
{
    if (nullptr == m_spDictionary)
    {
        throw CException(H_ERROR_POINTER, L"Dictionary object is not available.");
    }

    m_spDb = m_spDictionary->spGetDb();
    if (nullptr == m_spDb)
    {
        throw CException(H_ERROR_POINTER, L"Database object is not available.");
    }

    m_itCurrentLexeme = m_vecStoredLexemes.end();
}

CVerifier::~CVerifier()
{}

ET_ReturnCode CVerifier::eVerify (const CEString& sLexemeHash)
{
    ET_ReturnCode rc = H_NO_ERROR;

    //
    // Generate word forms for selected lexeme
    // 

    rc = eLoadStoredForms(sLexemeHash);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    m_spDictionary->Clear();
    rc = m_spDictionary->eGetLexemesByHash(sLexemeHash);
    if (rc != H_NO_ERROR)
    {
        ERROR_LOG (L"GetLexemesByHash() failed.");
        return rc;
    }
    
    shared_ptr<CLexemeEnumerator> spLE;
    rc = m_spDictionary->eCreateLexemeEnumerator(spLE);
    if (rc != H_NO_ERROR || !spLE)
    {
        ERROR_LOG(L"Failed to create lexeme enumerator.");
        return rc;
    }

    shared_ptr<CLexeme> spLexeme;
    ET_ReturnCode eLex = spLE->eGetFirstLexeme(spLexeme);
    if (eLex != H_NO_ERROR || !spLexeme)
    {
        ERROR_LOG(L"Failed to obtain lexeme instance.");
        return eLex;
    }

    while (H_NO_ERROR == eLex)
    {
        shared_ptr<CInflectionEnumerator> spIE;
        ET_ReturnCode eInfl = spLexeme->eCreateInflectionEnumerator(spIE);
        if (eInfl != H_NO_ERROR || !spIE)
        {
            ERROR_LOG(L"Failed to create inflection enumerator.");
            return eInfl;
        }
        
        shared_ptr<CInflection> spInflection;
        eInfl = spIE->eGetFirstInflection(spInflection);
        if (eInfl != H_NO_ERROR || !spInflection)
        {
            ERROR_LOG(L"Failed to obtain inflection instance.");
            return eLex;
        }

        while (H_NO_ERROR == eInfl)
        {
            bool bRet = true;
            rc = eCheckParadigm(spInflection, sLexemeHash, bRet);
            if (H_NO_ERROR == rc)
            {
                m_eResult = bRet ? TEST_RESULT_OK : TEST_RESULT_FAIL;
                if (!bRet)
                {
                    return H_NO_ERROR;
                }
            }

            eInfl = spIE->eGetNextInflection(spInflection);
            if (eInfl != H_NO_ERROR && eInfl != H_NO_MORE)
            {
                ERROR_LOG(L"Expected lexeme not found.");
                return H_ERROR_UNEXPECTED;
            }
        }

        eLex = spLE->eGetNextLexeme(spLexeme);
        if (eLex != H_NO_ERROR && eLex != H_NO_MORE)
        {
            ERROR_LOG(L"Error in GetNextLexeme().");
            return eLex;
        }
    }

    return rc;

}   //  Verify (...)

ET_TestResult CVerifier::eResult()
{
    return m_eResult;
}

int CVerifier::iCount()
{
    return (int)m_vecStoredLexemes.size();
}

//
//  Helpers
//
ET_ReturnCode CVerifier::eLoadStoredForms(const CEString& sLexemeHash)
{
    m_mmapStoredForms.clear();

    if (nullptr == m_spDictionary)
    {
        throw CException(H_ERROR_POINTER, L"Dictionary object is not available.");
    }

    m_spDb = m_spDictionary->spGetDb();
    if (nullptr == m_spDb)
    {
        throw CException(H_ERROR_POINTER, L"Database object is not available.");
    }

    CEString sQuery (L"SELECT DISTINCT id, gram_hash, wordform FROM test_data");
    sQuery += L" WHERE lexeme_id = \"";
    sQuery += sLexemeHash;
    sQuery += L"\";";

    try
    {
        m_spDb->PrepareForSelect (sQuery);
        while (m_spDb->bGetRow())
        {
            int64_t llFormId = -1;
            CEString sHash;
            CEString sSavedWf;
            m_spDb->GetData (0, llFormId);
            m_spDb->GetData (1, sHash);
            m_spDb->GetData (2, sSavedWf);

            CEString sStressQuery (L"SELECT position, is_primary FROM test_data_stress WHERE test_data_id = ");
            sStressQuery += CEString::sToString(llFormId);
            uint64_t uiStressHandle = m_spDb->uiPrepareForSelect (sStressQuery);
//            map<int, bool> mapSavedStress;
            while (m_spDb->bGetRow (uiStressHandle))
            {
                int iPos = -1;
                bool bPrimary = false;
                m_spDb->GetData (0, iPos, uiStressHandle);
                m_spDb->GetData (1, bPrimary, uiStressHandle);

                auto spSavedWf = make_shared<CWordForm>();
                CHasher hasher;
                ET_ReturnCode rc = hasher.eFromHash(sHash, *spSavedWf);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
                spSavedWf->SetWordForm(sSavedWf);
                spSavedWf->SetStressPos(iPos, bPrimary ? STRESS_PRIMARY : STRESS_SECONDARY);
                m_mmapStoredForms.insert (pair<CEString, shared_ptr<CWordForm>>(sHash, spSavedWf));
            }
            m_spDb->Finalize (uiStressHandle);

        }   // while (...)
        
        m_spDb->Finalize();
    }
    catch (CException& exc)
    {
        ERROR_LOG (exc.szGetDescription());
        return H_ERROR_GENERAL;
    }
    catch (...)
    {
        CEString sMsg;
        try
        {
            CEString sError;
            m_spDb->GetLastError(sError);
            sMsg = L"DB error: ";
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }
    
        CEString sError;
        m_spDb->GetLastError(sMsg);
        sMsg += sError;
        ERROR_LOG (sMsg);
 
        return H_ERROR_GENERAL;
    }

    return H_NO_ERROR;

}   //  eLoadStoredForms (...)

ET_ReturnCode CVerifier::eCheckParadigm (shared_ptr<CInflection>& spInflection, [[maybe_unused]]const CEString& sLexemeHash, bool& bCheckedOut)
{
    //std::locale::global(std::locale("ru_ru.utf16"));
    ET_ReturnCode hr = H_NO_ERROR;

    wofstream log1("test.txt", std::ios::binary);
    const unsigned long MaxCode = 0x10ffff;
    const std::codecvt_mode Mode = (std::codecvt_mode)(std::generate_header | std::little_endian);
    std::locale utf16_locale(log1.getloc(), new std::codecvt_utf16<wchar_t, MaxCode, Mode>);
    log1.imbue(utf16_locale);

    hr = spInflection->eGenerateParadigm();
    if (H_NO_ERROR != hr)
    {
        ERROR_LOG (L"GenerateParadigm() failed.");
        return hr;
    }

    for (auto itHash = m_mmapStoredForms.begin(); 
         itHash != m_mmapStoredForms.end(); 
         itHash = m_mmapStoredForms.upper_bound(itHash->first))
    {
        if (L"AspectPair" == itHash->first)
        {
            continue;
        }

        auto nForms = spInflection->iFormCount(itHash->first);
        if (nForms < 1)
        {
            bCheckedOut = false;
            return H_NO_ERROR;
        }

        auto& spStoredForm = itHash->second;

        bool bFormMatch = false;
        for (auto nGeneratedForm = 0; nGeneratedForm < nForms; ++nGeneratedForm)
        {
            shared_ptr<CWordForm> spGeneratedForm;
            hr = spInflection->eWordFormFromHash(itHash->first, nGeneratedForm, spGeneratedForm);
            if (hr != H_NO_ERROR)
            {
                bCheckedOut = false;
                return hr;
            }

            auto koko = spGeneratedForm->sWordForm() + L" : " + spStoredForm->sWordForm();
            auto kiki = wstring(koko);
            log1 << wstring(koko).c_str() << L"\n" << flush;

            if (spGeneratedForm->sWordForm() == spStoredForm->sWordForm())
            {
                if (spStoredForm->mapGetStressPositions() == spGeneratedForm->mapGetStressPositions())
                {
                    bFormMatch = true;
                    break;
                }
                else
                {
                    CEString sMsg(L"Stress mismatch, lexeme hash = ");
                    sMsg += sLexemeHash;
                    sMsg += L", gram hash = ";
                    sMsg += spGeneratedForm->sGramHash();
                    ERROR_LOG(sMsg);
                }
            }
            else
            {
                CEString sMsg(L"Word form mismatch, lexeme hash = ");
                sMsg +=sLexemeHash;
                sMsg += L", gram hash = ";
                sMsg += spGeneratedForm->sGramHash();
                ERROR_LOG(sMsg);
            }
        }       //  for auto nGeneratedForm = 0...

        if (!bFormMatch)
        {
            bCheckedOut = false;
//            return H_NO_ERROR;
        }

    }       //  for (auto itHash = ...)

    log1.flush();
    log1.close();

    shared_ptr<CWordForm> spGeneratedForm;
    hr = spInflection->eGetFirstWordForm(spGeneratedForm);
    if (hr != H_NO_ERROR)
    {
        ERROR_LOG(L"eGetFirstWordForm() failed.");
        return hr;
    }

    for (bool bIterate = true; bIterate;)
    {
        auto pairStoredForms = m_mmapStoredForms.equal_range(spGeneratedForm->sGramHash());
        if (pairStoredForms.first == pairStoredForms.second)
        {
            CEString sMsg(L"No stored forms for ");
            sMsg += spGeneratedForm->sGramHash();
            ERROR_LOG(sMsg);
            return H_ERROR_UNEXPECTED;
        }

        bool bFormMatch = false;
        for (auto itStoredForm = pairStoredForms.first; itStoredForm != pairStoredForms.second; ++itStoredForm)
        { 
            auto spStoredForm = itStoredForm->second;
            if (spGeneratedForm->sWordForm() == spStoredForm->sWordForm())
            {
                if (spGeneratedForm->mapGetStressPositions() == spStoredForm->mapGetStressPositions())
                {
                    bFormMatch = true;
                    break;
                }
            }
        }       //  for auto nGeneratedForm = 0...

        if (!bFormMatch)
        {
            bCheckedOut = false;
            return H_NO_ERROR;
        }

        hr = spInflection->eGetNextWordForm(spGeneratedForm);
        if (H_NO_MORE == hr)
        {
            bIterate = false;
        }

    }       //  for (bool bIterate = true; bIterate;)

    bCheckedOut = true;

    //
    // Check aspect pair(s) if available
    //
    if (spInflection->spGetLexeme()->bHasAspectPair())
    {
        bCheckedOut = false;

        vector<CEString> vecHashes = { L"AspectPair" };
        if (spInflection->spGetLexeme()->bHasAltAspectPair())
        {
            vecHashes.push_back(L"AltAspectPair");
        }

        for (auto itHash = vecHashes.begin(); itHash != vecHashes.end(); ++itHash)
        {
            auto pairRange = m_mmapStoredForms.equal_range(*itHash);
            if (pairRange.first == pairRange.second)
            {
//                ERROR_LOG(L"No stored aspect pairs loaded.");
                return H_NO_ERROR;
            }

            CEString sGeneratedAspectPair;
            int iGeneratedAspectPairStressPos = -1;
            if (vecHashes.begin() == itHash)
            {
                auto eRet = spInflection->spGetLexeme()->eGetAspectPair(sGeneratedAspectPair, iGeneratedAspectPairStressPos);
                if (eRet != H_NO_ERROR)
                {
                    return eRet;
                }
            }
            else
            {
                auto eRet = spInflection->spGetLexeme()->eGetAltAspectPair(sGeneratedAspectPair, iGeneratedAspectPairStressPos);
                if (eRet != H_NO_ERROR)
                {
                    return eRet;
                }
            }

            auto aspectPair = pairRange.first;
            auto spStoredForm = aspectPair->second;
            if (sGeneratedAspectPair == spStoredForm->sWordForm())
            {
                auto itStress = spStoredForm->mapGetStressPositions().find(iGeneratedAspectPairStressPos);
                if (itStress != spStoredForm->mapGetStressPositions().end() && itStress->second == ET_StressType::STRESS_PRIMARY)
                {
                    bCheckedOut = true;
                }
            }
        }
    }       //  if (Lexeme.bHasAspectPair())

    return H_NO_ERROR;
   
}   //  eCheckLexeme (CLexeme& Lexeme, bool& bCheckedOut)

bool CVerifier::bWordFormsMatch(shared_ptr<CWordForm> spLhs, shared_ptr<CWordForm> spRhs)
{
    if (spLhs->sGramHash() != spRhs->sGramHash())
    {
        return false;
    }

    if (spLhs->sWordForm() != spRhs->sWordForm())
    {
        return false;
    }

    if (spLhs->sStem() != spRhs->sStem())
    {
        return false;
    }

    int iLeftStressPos, iRightStressPos = 0;
    ET_StressType eLeftStressType, eRightStressType = STRESS_TYPE_UNDEFINED;
    ET_ReturnCode eGotLeftStress = spLhs->eGetFirstStressPos(iLeftStressPos, eLeftStressType);
    while (H_NO_ERROR == eGotLeftStress)
    {
        ET_ReturnCode eGotRightStress = spRhs->eGetFirstStressPos(iRightStressPos, eRightStressType);
        while (H_NO_ERROR == eGotRightStress)
        {
            if (iLeftStressPos == iRightStressPos && eLeftStressType == eRightStressType)
            {
                return true;
            }
            eGotRightStress = spRhs->eGetNextStressPos(iRightStressPos, eRightStressType);
        }

        eGotLeftStress = spLhs->eGetNextStressPos(iLeftStressPos, eLeftStressType);
    }

    return false;

}       //  bWordFormsMatch()

ET_ReturnCode CVerifier::eLoadStoredLexemes()
{
    if (nullptr == m_spDictionary)
    {
        throw CException(H_ERROR_POINTER, L"Dictionary object is not available.");
    }

    m_spDb = m_spDictionary->spGetDb();
    if (nullptr == m_spDb)
    {
        throw CException(H_ERROR_POINTER, L"Database object is not available.");
    }

    CEString sQuery(L"SELECT DISTINCT l.lexeme_hash, l.descriptor_id, h.source FROM test_data AS t");
    sQuery += L" INNER JOIN lexeme_hash_to_descriptor AS l ON t.lexeme_id = l.lexeme_hash";
    sQuery += L" INNER JOIN descriptor AS d on l.descriptor_id = d.id";
    sQuery += L" INNER JOIN headword AS h ON h.id = d.word_id;";

    eGetStoredLexemeData(sQuery);

    return H_NO_ERROR;

}   //  LoadStoredLexemes (...)

ET_ReturnCode CVerifier::eDeleteStoredLexeme(const CEString& sLexemeHash)
{
    if (nullptr == m_spDictionary)
    {
        throw CException(H_ERROR_POINTER, L"Dictionary object is not available.");
    }

    m_spDb = m_spDictionary->spGetDb();
    if (nullptr == m_spDb)
    {
        throw CException(H_ERROR_POINTER, L"Database object is not available.");
    }

    vector<int> vecFormIds;

    CEString sSelectQuery(L"SELECT id FROM test_data WHERE lexeme_id = \"");
    sSelectQuery += sLexemeHash;
    sSelectQuery += L"\";";

    try
    {
        m_spDb->PrepareForSelect(sSelectQuery);
        while (m_spDb->bGetRow())
        {
            int iId = 0;
            m_spDb->GetData(0, iId);
            vecFormIds.push_back(iId);
        }

        if (vecFormIds.empty())
        {
            ERROR_LOG(L"Lexeme hash not found in the database.");
            return H_ERROR_GENERAL;
        }

        CEString sDelQuery1(L"DELETE FROM test_data WHERE lexeme_id = \"");
        sDelQuery1 += sLexemeHash;
        sDelQuery1 += L"\";";
        m_spDb->Exec(sDelQuery1);

        vector<int>::iterator itFormId = vecFormIds.begin();
        for (; itFormId != vecFormIds.end(); ++itFormId)
        {
            CEString sDelQuery2(L"DELETE FROM test_data_stress WHERE lexeme_id = \"");
            sDelQuery2 += sLexemeHash;
            sDelQuery2 += L"\";";
            m_spDb->Exec(sDelQuery2);
        }
    }
    catch (...)
    {
        CEString sMsg;
        try
        {
            CEString sError;
            m_spDb->GetLastError(sError);
            sMsg = L"DB error %d: ";
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg = CEString::sToString(m_spDb->iGetLastError());
        ERROR_LOG(sMsg);

        return H_ERROR_GENERAL;

    }   // catch...

    return H_NO_ERROR;

}   //  DeleteStoredLexeme (...)

ET_ReturnCode CVerifier::eGetFirstLexemeData(CEString& sHash, CEString& sHeadword)
{
    if (m_vecStoredLexemes.empty())
    {
        return H_NO_MORE;
    }

    m_itCurrentLexeme = m_vecStoredLexemes.begin();
    sHash = (*m_itCurrentLexeme).m_sLexemeHash;
    sHeadword = (*m_itCurrentLexeme).m_sHeadword;

    return H_NO_ERROR;
}

ET_ReturnCode CVerifier::eGetNextLexemeData(CEString& sHash, CEString& sHeadword)
{
    if (m_vecStoredLexemes.end() == m_itCurrentLexeme)
    {
        return H_NO_MORE;
    }

    if (m_vecStoredLexemes.end() == ++m_itCurrentLexeme)
    {
        return H_NO_MORE;
    }

    sHash = (*m_itCurrentLexeme).m_sLexemeHash;
    sHeadword = (*m_itCurrentLexeme).m_sHeadword;

    return H_NO_ERROR;
}

ET_ReturnCode CVerifier::eGetFirstWordForm(shared_ptr<CWordForm>& spWordForm)
{
    m_itCurrentForm = m_mmapStoredForms.begin();
    if (m_mmapStoredForms.end() == m_itCurrentForm)
    {
        return H_FALSE;
    }

    spWordForm = (*m_itCurrentForm).second;

    return H_NO_ERROR;
}

ET_ReturnCode CVerifier::eGetNextWordForm(shared_ptr<CWordForm>& spWordForm)
{
    if (m_itCurrentForm != m_mmapStoredForms.end())
    {
        ++m_itCurrentForm;
    }

    if (m_mmapStoredForms.end() == m_itCurrentForm)
    {
        return H_NO_MORE;
    }

    spWordForm = (*m_itCurrentForm).second;

    return H_NO_ERROR;
}

ET_ReturnCode CVerifier::eGetStoredLexemeData(const CEString& sSelect)
{
    if (nullptr == m_spDictionary)
    {
        throw CException(H_ERROR_POINTER, L"Dictionary object is not available.");
    }

    m_spDb = m_spDictionary->spGetDb();
    if (nullptr == m_spDb)
    {
        throw CException(H_ERROR_POINTER, L"Database object is not available.");
    }

    m_vecStoredLexemes.clear();

    try
    {
        m_spDb->PrepareForSelect(sSelect);
        while (m_spDb->bGetRow())
        {
            CEString sLexemeHash;
            m_spDb->GetData(0, sLexemeHash);
            //            m_pDb->GetData (1, pco_v->i_DescriptorId);
            CEString sHeadword;
            m_spDb->GetData(2, sHeadword);

            m_vecStoredLexemes.push_back(StStoredLexeme(sLexemeHash, sHeadword));

        }   //  while (m_pDb->bGetRow())

        m_spDb->Finalize();
    }
    catch (...)
    {
        CEString sMsg = CEString::sToString (m_spDb->iGetLastError());
        ERROR_LOG((wchar_t *)sMsg);

        return H_ERROR_GENERAL;
    }

    return H_NO_ERROR;

}   //  hGetStoredLexemeData (...)
