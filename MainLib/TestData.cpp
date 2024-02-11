#include "TestData.h"
#include "Verifier.h"

ET_ReturnCode CTestData::put_DbPath (BSTR bstrPath)
{
    USES_CONVERSION;

    ET_ReturnCode hr = H_NO_ERROR;

    CEString sPath = OLE2W (bstrPath);
    if (sPath == m_sDbPath)
    {
        return H_NO_ERROR;
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

    return H_NO_ERROR;

}   //  put_DbPath (...)

ET_ReturnCode CTestData::LoadStoredLexemes() //(long l_lowBound, long l_highBound)
{
    ET_ReturnCode hr = H_NO_ERROR;

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

    return H_NO_ERROR;

}   //  LoadStoredLexemes (...)

ET_ReturnCode CTestData::DeleteStoredLexeme (BSTR bstrLexemeHash)
{
    USES_CONVERSION;

    ET_ReturnCode hr = H_NO_ERROR;

    if (!m_pDb)
    {
        ERROR_LOG (L"Database not initialized.");
        return hr;
    }

    vector<int> vecFormIds;

    CEString sSelectQuery (L"SELECT id FROM test_data WHERE lexeme_id = \"");
    sSelectQuery += OLE2W (bstrLexemeHash);
    sSelectQuery += L"\";";

    try
    {    
        m_pDb->PrepareForSelect (sSelectQuery);
        while (m_pDb->bGetRow())
        {
            int iId = 0;
            m_pDb->GetData (0, iId);
            vecFormIds.push_back (iId);
        }

        if (vecFormIds.empty())
        {
            ERROR_LOG (L"Lexeme hash not found in the database.");
            return E_FAIL;
        }

        CEString sDelQuery1 (L"DELETE FROM test_data WHERE lexeme_id = \"");
        sDelQuery1 += OLE2W (bstrLexemeHash);
        sDelQuery1 += L"\";";
        m_pDb->Exec (sDelQuery1);

        vector<int>::iterator itFormId = vecFormIds.begin();
        for (; itFormId != vecFormIds.end(); ++ itFormId)
        {
            CEString sDelQuery2 (L"DELETE FROM test_data_stress WHERE form_id = \"");
            sDelQuery2 += CEString::sToString (*itFormId);
            sDelQuery2 += L"\";";
            m_pDb->Exec (sDelQuery2);
        }
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
    
    }   // catch...

    return H_NO_ERROR;

}   //  DeleteStoredLexeme (...)

ET_ReturnCode CTestData::hGetStoredLexemeData (const CEString& sSelect)
{
    ET_ReturnCode hr = H_NO_ERROR;

    if (NULL == m_pDb)
    {
        ERROR_LOG (L"DB pointer is NULL.");
        return H_ERROR_POINTER;
    }

    m_coll.clear();

    try
    {
        m_pDb->PrepareForSelect (sSelect);
        while (m_pDb->bGetRow())
        {
            CComObject<CVerifier> * pV;
            hr = CComObject<CVerifier>::CreateInstance (&pV);
            if (H_NO_ERROR != hr)
            {
                ERROR_LOG (L"CreateInstance failed on CVerifier.");
                return hr;
            }

            m_pDb->GetData (0, pV->m_sLexemeHash);
//            m_pDb->GetData (1, pco_v->i_DescriptorId);
            m_pDb->GetData (2, pV->m_sHeadword);

            pV->m_sDbPath = m_sDbPath;;

            CComQIPtr<IVerifier> spqiSf (pV);
            if (!spqiSf)
            {
                assert(0);
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

    return H_NO_ERROR;
        
}   //  hGetStoredLexemeData (...)
