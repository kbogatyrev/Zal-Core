#include "stdafx.h"
#include "TestData.h"
#include "Verifier.h"

HRESULT CTestData::put_DbPath (BSTR bstrPath)
{
    USES_CONVERSION;

    HRESULT hr = S_OK;

    CEString sPath = OLE2W (bstrPath);
    if (sPath == m_sDbPath)
    {
        return S_OK;
    }
    else
    {
        m_sDbPath = sPath;
    }

    if (m_pDb)
    {
        delete m_pDb;
    }

    m_pDb = new CSqlite (m_sDbPath);
    if (!m_pDb)
    {
        return E_FAIL;
    }

//    hr = LoadStoredLexemes();

    return S_OK;

}   //  put_DbPath (...)

HRESULT CTestData::LoadStoredLexemes() //(long l_lowBound, long l_highBound)
{
    HRESULT hr = S_OK;

    if (!m_pDb)
    {
        ERROR_LOG (L"Database not initialized.");
        return hr;
    }

    //if (l_lowBound < 0)
    //{
    //    l_lowBound = 0;
    //}

    //if (l_highBound < 0)
    //{
    //    l_highBound = 0;
    //}

    CEString sQuery (L"SELECT DISTINCT l.lexeme_hash, l.descriptor_id, h.source FROM test_data AS t");
    sQuery += L" INNER JOIN lexeme_hash_to_descriptor AS l ON t.lexeme_id = l.lexeme_hash";
    sQuery += L" INNER JOIN descriptor AS d on l.descriptor_id = d.id";
    sQuery += L" INNER JOIN headword AS h ON h.id = d.word_id;";

    //if (l_lowBound < l_highBound)
    //{
    //    sQuery += L" WHERE lexeme_id >= ";
    //    sQuery += str_ToString (l_lowBound);
    //    sQuery += L" AND lexeme_id <= ";
    //    sQuery += str_ToString (l_highBound);
    //}

    sQuery += L";";

    hr = hGetStoredLexemeData (sQuery);

    return S_OK;

}   //  LoadStoredLexemes (...)

HRESULT CTestData::hGetStoredLexemeData (const CEString& sSelect)
{
    HRESULT hr = S_OK;

    if (NULL == m_pDb)
    {
        ERROR_LOG (L"DB pointer is NULL.");
        return E_POINTER;
    }

    m_coll.clear();

    try
    {
        m_pDb->PrepareForSelect (sSelect);
        while (m_pDb->bGetRow())
        {
            CComObject<CVerifier> * pV;
            hr = CComObject<CVerifier>::CreateInstance (&pV);
            if (S_OK != hr)
            {
                ERROR_LOG (L"CreateInstance failed on CVerifier.");
                return hr;
            }

            m_pDb->GetData (0, (__int64&)pV->m_ulLexemeHash);
//            m_pDb->GetData (1, pco_v->i_DescriptorId);
            m_pDb->GetData (2, pV->m_sHeadword);

            pV->m_sDbPath = m_sDbPath;;

            CComQIPtr<IVerifier> spqiSf (pV);
            if (!spqiSf)
            {
                ATLASSERT(0);
                ERROR_LOG (L"QI failed on IVerifier");
                return E_NOINTERFACE;
            }

            m_coll.push_back (CComVariant (spqiSf));

        }   //  while (m_pDb->bGetRow())

        m_pDb->Finalize();
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
        
}   //  hGetStoredLexemeData (...)
