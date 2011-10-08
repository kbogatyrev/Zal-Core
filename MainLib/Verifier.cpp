#include "StdAfx.h"
#include "Dictionary.h"
#include "WordForm.h"
#include "Lexeme.h"
#include "Verifier.h"

HRESULT CVerifier::put_DbPath (BSTR bstrDbPath)
{
    USES_CONVERSION;

    m_sDbPath = OLE2W (bstrDbPath);

    if (m_sDbPath.bIsEmpty())
    {
        ATLASSERT(0);
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT CVerifier::put_Headword (BSTR bstrHeadWord)
{
    USES_CONVERSION;
    m_sHeadword = OLE2W (bstrHeadWord);
    return S_OK;
}

HRESULT CVerifier::get_Headword (BSTR * pbstrVal)
{
    CComBSTR spbstrHeadword (m_sHeadword);
    *pbstrVal = spbstrHeadword.Detach();
    return S_OK;
}

HRESULT CVerifier::get_LexemeId (BSTR * pbstrVal)
{
    CComBSTR spbstrLexemeHash (m_sLexemeHash);
    *pbstrVal = spbstrLexemeHash.Detach();
    return S_OK;
}

HRESULT CVerifier::put_LexemeId (BSTR bstrLexemeId)
{
    USES_CONVERSION;
    m_sLexemeHash = OLE2W (bstrLexemeId);
    return S_OK;
}

HRESULT CVerifier::Verify (BSTR bstrLexemeId, ET_TestResult * peResult)
{
    USES_CONVERSION;

    HRESULT hr = S_OK;

    m_sLexemeHash = OLE2W (bstrLexemeId);

    //
    // Generate word forms for selected lexeme
    // 
    CComObject<CDictionary> * pDictionary;
    hr = CComObject<CDictionary>::CreateInstance (&pDictionary);
    if (S_OK != hr)
    {
        ERROR_LOG (L"CreateInstance failed on CDictionary.");
        return hr;
    }

    pDictionary->put_DbPath (CComBSTR (m_sDbPath));

    hr = hLoadStoredForms();
    if (S_OK != hr)
    {
        return hr;
    }

    hr = pDictionary->GetLexemesByHash (m_sLexemeHash);
    if (S_OK != hr)
    {
        ERROR_LOG (L"GetLexemesByHash() failed.");
        return hr == S_OK ? E_FAIL : hr;
    }

    LONG lCount = 0;
    pDictionary->get_Count (&lCount);
    if (lCount < 1)
    {
        ERROR_LOG (L"No lexemes found.");
        return E_UNEXPECTED;
    }
    
    CComPtr<ILexeme> spLexeme;
    for (int iLexeme = 0; iLexeme < lCount; ++iLexeme)
    {
        CComVariant spVar = pDictionary->m_coll[iLexeme];
        if (VT_UNKNOWN == spVar.vt || VT_DISPATCH == spVar.vt)
        {
            CComQIPtr<ILexeme> spQiLex = spVar.pdispVal;
            spLexeme = spQiLex.Detach();
        }
        else
        {
            ERROR_LOG (L"Unexpected VARIANT data type.");
            return E_UNEXPECTED;
        }

        CLexeme * pLexeme = dynamic_cast<CLexeme *> (spLexeme.p);
        if (NULL == pLexeme)
        {
            ATLASSERT(0);
            CEString sMsg (L"Failed to obtain pointer to CLexeme.");
            ERROR_LOG (sMsg);
            return E_POINTER;
        }

        hr = hCheckLexeme (*pLexeme);
        if (S_OK == hr)
        {
            m_eResult = TEST_RESULT_OK;
            *peResult = m_eResult;
            return S_OK;
        }
        
        m_eResult = TEST_RESULT_FAIL;
        *peResult = m_eResult;

    }   //  for (int iLexeme ...)

    if (SUCCEEDED (hr))
    {
        return S_OK;
    }
    else
    {
        return hr;
    }

}   //  Verify (...)

HRESULT CVerifier::get_Result (ET_TestResult * peoResult)
{
    *peoResult = m_eResult;
    return S_OK;
}

//
//  Helpers
//
HRESULT CVerifier::hLoadStoredForms()
{
    HRESULT hr = S_OK;

    m_mmapStoredForms.clear();

    if (m_pDb)
    {
        delete m_pDb;
    }

    m_pDb = new CSqlite (m_sDbPath);
    if (!m_pDb)
    {
        return E_FAIL;
    }

    CEString sQuery (L"SELECT DISTINCT id, hash, wordform FROM test_data");
    sQuery += L" WHERE lexeme_id = \"";
    sQuery += m_sLexemeHash;
    sQuery += L"\";";

    try
    {
        m_pDb->PrepareForSelect (sQuery);
        while (m_pDb->bGetRow())
        {
            int iFormId = -1;
            int iHash = -1;
            CEString sSavedWf;
            m_pDb->GetData (0, iFormId);
            m_pDb->GetData (1, iHash);
            m_pDb->GetData (2, sSavedWf);

            CComObject<CWordForm> * pSavedWf;
            hr = CComObject<CWordForm>::CreateInstance (&pSavedWf);
            if (S_OK != hr)
            {
                ERROR_LOG (L"CreateInstance failed on CWordForm.");
                return hr;
            }

            CHasher hasher;
            hasher.FromHash (iHash, *pSavedWf);
            pSavedWf->m_sWordForm = sSavedWf;

            CEString sStressQuery 
                (L"SELECT position, is_primary FROM test_data_stress WHERE form_id = ");
            sStressQuery += CEString::sToString (iFormId);
            unsigned int uiStressHandle = m_pDb->uiPrepareForSelect (sStressQuery);
//            map<int, bool> mapSavedStress;
            while (m_pDb->bGetRow (uiStressHandle))
            {
                int iPos = -1;
                bool bPrimary = false;
                m_pDb->GetData (0, iPos, uiStressHandle);
                m_pDb->GetData (1, bPrimary, uiStressHandle);

                pSavedWf->m_mapStress[iPos] = bPrimary ? STRESS_PRIMARY : STRESS_SECONDARY;

                m_mmapStoredForms.insert (pair<int, IWordForm *>(iHash, pSavedWf));
            }
            m_pDb->Finalize (uiStressHandle);

        }   // while (...)
        
        m_pDb->Finalize();
    }
    catch (CException& exc)
    {
        ERROR_LOG (exc.szGetDescription());
 
        return E_FAIL;
    }
    catch (...)
    {
        CEString sMsg;
        try
        {
            CEString sError;
            m_pDb->GetLastError (sError);
            sMsg = L"DB error %d: ";
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }
    
        CString csMsg;
        csMsg.Format (sMsg, m_pDb->iGetLastError());
        ERROR_LOG ((LPCTSTR)csMsg);
 
        return E_FAIL;
    }

    return S_OK;

}   //  hLoadStoredForms (...)

HRESULT CVerifier::hCheckLexeme (CLexeme& Lexeme)
{
    HRESULT hr = S_OK;

    hr = Lexeme.GenerateWordForms();
    if (S_OK != hr)
    {
        ERROR_LOG (L"GenerateWordForms() failed.");
        return hr;
    }

    long lGeneratedForms = 0;
    hr = Lexeme.get_Count (&lGeneratedForms);
    if (S_OK != hr)
    {
        ATLASSERT(0);
        ERROR_LOG (L"get_Count() failed.");
        return hr;
    }

    for (long lAt = 0; lAt < lGeneratedForms; ++lAt)
    {
        CComVariant spVar;
        hr = Lexeme.get_Item (lAt+1, &spVar);
        if (S_OK != hr)
        {
            ATLASSERT(0);
            ERROR_LOG (L"get_Item() failed.");
            continue;
        }

        if (spVar.vt != VT_UNKNOWN && spVar.vt != VT_DISPATCH)
        {
            ATLASSERT(0);
            ERROR_LOG (L"IUnknown or IDispatch ptr expected.");
            continue;
        }

        CComQIPtr<IWordForm> spqiGeneratedWf = spVar.punkVal;
        if (!spqiGeneratedWf)
        {
            ATLASSERT(0);
            ERROR_LOG (L"Unable to extract IWordForm.");
            continue;
        }

        CWordForm * pGeneratedWf = (CWordForm *)spqiGeneratedWf.p;
        int iHash = pGeneratedWf->iGramHash();

        pair<multimap<int, IWordForm *>::iterator, 
             multimap<int, IWordForm *>::iterator> pairRange = 
                m_mmapStoredForms.equal_range (iHash);

        if (pairRange.first == pairRange.second)
        {
            ATLASSERT(0);
            ERROR_LOG (L"No stored forms loaded.");
            return E_FAIL;
        }

        // m_eResult = TEST_RESULT_FAIL;
        bool bResult = false;
        multimap<int, IWordForm *>::iterator itStoredWf = pairRange.first;
        for (; itStoredWf != pairRange.second; ++itStoredWf)
        {
            //
            // More than one word form may be generated for a single hash; if there is
            // a generated form that matches the saved form, we mark this as a
            // success; if none of the generated forms matched the saved form,
            // we return fail.
            //
//pig
            CWordForm * pStoredWf = (CWordForm *)(*itStoredWf).second;
            if (pGeneratedWf->m_sWordForm == pStoredWf->m_sWordForm &&
                pGeneratedWf->m_mapStress == pStoredWf->m_mapStress)
            {
                bResult = true;
                break;
            }
        }       // gram hash loop

        if (!bResult)
        {
            return S_FALSE;
        }

    }   //  generated forms loop

    return S_OK;

}   //  hCheckLexeme (CLexeme& Lexeme)
