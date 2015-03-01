#include "StdAfx.h"
#include "IDictionary.h"
#include "WordForm.h"
#include "Lexeme.h"
#include "Verifier.h"

using namespace Hlib;

CVerifier::CVerifier(CDictionary * pDict) : m_pDictionary(pDict), m_pDb(NULL), m_eResult(TEST_RESULT_UNDEFINED)
{
    if (NULL == m_pDictionary)
    {
        throw CException(H_ERROR_POINTER, L"Dictionary object is not available.");
    }

    m_pDb = m_pDictionary->pGetDbHandle();
    if (NULL == m_pDb)
    {
        throw CException(H_ERROR_POINTER, L"Database object is not available.");
    }

    m_itCurrentLexeme = m_vecStoredLexemes.end();
}

CVerifier::~CVerifier()
{}

ET_ReturnCode CVerifier::eVerify (const CEString& sLexemeHash)
{
    ET_ReturnCode hr = H_NO_ERROR;

    //
    // Generate word forms for selected lexeme
    // 

    hr = eLoadStoredForms(sLexemeHash);
    if (H_NO_ERROR != hr)
    {
        return hr;
    }

    hr = m_pDictionary->eGetLexemesByHash(sLexemeHash);
    if (H_NO_ERROR != hr)
    {
        ERROR_LOG (L"GetLexemesByHash() failed.");
        return hr;
    }
    
    ILexeme * pLexeme  = NULL;
    hr = m_pDictionary->eGetFirstLexeme(pLexeme);
    if (hr != H_NO_ERROR)
    {
        ERROR_LOG(L"Expected lexeme not found.");
        return H_ERROR_UNEXPECTED;
    }

    while (H_NO_ERROR == hr)
    {
        bool bRet = false;
        hr = eCheckLexeme(*pLexeme, bRet);
        if (H_NO_ERROR == hr)
        {
            m_eResult = bRet ? TEST_RESULT_OK : TEST_RESULT_FAIL;
            return H_NO_ERROR;
        }

        hr = m_pDictionary->eGetNextLexeme(pLexeme);
        if (hr != H_NO_ERROR)
        {
            ERROR_LOG(L"Expected lexeme not found.");
            return H_ERROR_UNEXPECTED;
        }
    }

    return hr;

}   //  Verify (...)

ET_TestResult CVerifier::eResult()
{
    return m_eResult;
}

int CVerifier::iCount()
{
    return m_vecStoredLexemes.size();
}

//
//  Helpers
//
ET_ReturnCode CVerifier::eLoadStoredForms(const CEString& sLexemeHash)
{
    ET_ReturnCode hr = H_NO_ERROR;

    m_mmapStoredForms.clear();

    if (NULL == m_pDb)
    {
        throw CException(H_ERROR_POINTER, L"Database object is not available.");
    }

    CEString sQuery (L"SELECT DISTINCT id, hash, wordform FROM test_data");
    sQuery += L" WHERE lexeme_id = \"";
    sQuery += sLexemeHash;
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

            CWordForm * pSavedWf = new CWordForm();

            CHasher hasher;
            ET_ReturnCode rc = hasher.eFromHash (iHash, *pSavedWf);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }

            pSavedWf->m_sWordForm = sSavedWf;

            CEString sStressQuery (L"SELECT position, is_primary FROM test_data_stress WHERE lexeme_id = \"");
            sStressQuery += sLexemeHash;
            sStressQuery += L"\" AND hash = ";
            sStressQuery += CEString::sToString(iHash);
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
 
        return H_ERROR_GENERAL;
    }
    catch (...)
    {
        CEString sMsg;
        try
        {
            CEString sError;
            m_pDb->GetLastError (sError);
            sMsg = L"DB error: ";
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }
    
        CEString sError;
        m_pDb->GetLastError(sMsg);
        sMsg += sError;
        ERROR_LOG (sMsg);
 
        return H_ERROR_GENERAL;
    }

    return H_NO_ERROR;

}   //  eLoadStoredForms (...)

ET_ReturnCode CVerifier::eCheckLexeme (ILexeme& Lexeme, bool& bCheckedOut)
{
    ET_ReturnCode hr = H_NO_ERROR;

    hr = Lexeme.eGenerateParadigm();
    if (H_NO_ERROR != hr)
    {
        ERROR_LOG (L"GenerateWordForms() failed.");
        return hr;
    }

    IWordForm * pItf = NULL;
    hr = Lexeme.eGetFirstWordForm(pItf);
    while (H_NO_ERROR == hr)
    {
        CWordForm * pGeneratedForm = dynamic_cast<CWordForm *>(pItf);
        int iHash = pGeneratedForm->iGramHash();

        pair<multimap<int, IWordForm *>::iterator, 
             multimap<int, IWordForm *>::iterator> pairRange = 
                m_mmapStoredForms.equal_range (iHash);
        if (pairRange.first == pairRange.second)
        {
            ERROR_LOG (L"No stored forms loaded.");
            return H_ERROR_UNEXPECTED;
        }

        bCheckedOut = false;

        multimap<int, IWordForm *>::iterator itStoredForm = pairRange.first;
        for (; itStoredForm != pairRange.second; ++itStoredForm)
        {
            //
            // More than one word form may be generated for a single hash; if there is
            // a generated form that matches the saved form, we mark this as a
            // success; if none of the generated forms matched the saved form,
            // we return fail.
            //
            CWordForm * pStoredForm = dynamic_cast<CWordForm *>((*itStoredForm).second);
            if (pGeneratedForm->sWordForm() == pStoredForm->sWordForm())
            {
                if (pStoredForm->m_mapStress == pGeneratedForm->m_mapStress)
                {
                    bCheckedOut = true;
                }
            }
        }       // gram hash loop

        if (!bCheckedOut)
        {
            return H_NO_ERROR;
        }

        hr = Lexeme.eGetNextWordForm(pItf);
        if (hr < 0)
        {
            return hr;
        }

    }   //  generated forms loop

    return H_NO_ERROR;

}   //  eCheckLexeme (CLexeme& Lexeme, bool& bCheckedOut)

bool CVerifier::bWordFormsMatch(IWordForm * pLhs,IWordForm * pRhs)
{
    if (pLhs->iGramHash() != pRhs->iGramHash())
    {
        return false;
    }

    if (pLhs->sWordForm() != pRhs->sWordForm())
    {
        return false;
    }

    if (pLhs->sLemma() != pRhs->sLemma())
    {
        return false;
    }

    int iLeftStressPos, iRightStressPos = 0;
    ET_StressType eLeftStressType, eRightStressType = STRESS_TYPE_UNDEFINED;
    ET_ReturnCode eGotLeftStress = pLhs->eGetFirstStressPos(iLeftStressPos, eLeftStressType);
    while (H_NO_ERROR == eGotLeftStress)
    {
        ET_ReturnCode eGotRightStress = pRhs->eGetFirstStressPos(iRightStressPos, eRightStressType);
        while (H_NO_ERROR == eGotRightStress)
        {
            if (iLeftStressPos == iRightStressPos && eLeftStressType == eRightStressType)
            {
                return true;
            }
            eGotRightStress = pRhs->eGetNextStressPos(iRightStressPos, eRightStressType);
        }

        eGotLeftStress = pLhs->eGetNextStressPos(iLeftStressPos, eLeftStressType);
    }

    return false;

}       //  bWordFormsMatch()

ET_ReturnCode CVerifier::eLoadStoredLexemes()
{
    ET_ReturnCode hr = H_NO_ERROR;

    if (!m_pDb)
    {
        ERROR_LOG(L"Database not initialized.");
        return hr;
    }

    CEString sQuery(L"SELECT DISTINCT l.lexeme_hash, l.descriptor_id, h.source FROM test_data AS t");
    sQuery += L" INNER JOIN lexeme_hash_to_descriptor AS l ON t.lexeme_id = l.lexeme_hash";
    sQuery += L" INNER JOIN descriptor AS d on l.descriptor_id = d.id";
    sQuery += L" INNER JOIN headword AS h ON h.id = d.word_id;";

    hr = eGetStoredLexemeData(sQuery);

    return H_NO_ERROR;

}   //  LoadStoredLexemes (...)

ET_ReturnCode CVerifier::eDeleteStoredLexeme(const CEString& sLexemeHash)
{
    ET_ReturnCode hr = H_NO_ERROR;

    if (!m_pDb)
    {
        ERROR_LOG(L"Database not initialized.");
        return hr;
    }

    vector<int> vecFormIds;

    CEString sSelectQuery(L"SELECT id FROM test_data WHERE lexeme_id = \"");
    sSelectQuery += sLexemeHash;
    sSelectQuery += L"\";";

    try
    {
        m_pDb->PrepareForSelect(sSelectQuery);
        while (m_pDb->bGetRow())
        {
            int iId = 0;
            m_pDb->GetData(0, iId);
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
        m_pDb->Exec(sDelQuery1);

        vector<int>::iterator itFormId = vecFormIds.begin();
        for (; itFormId != vecFormIds.end(); ++itFormId)
        {
            CEString sDelQuery2(L"DELETE FROM test_data_stress WHERE form_id = \"");
            sDelQuery2 += CEString::sToString(*itFormId);
            sDelQuery2 += L"\";";
            m_pDb->Exec(sDelQuery2);
        }
    }
    catch (...)
    {
        CEString sMsg;
        try
        {
            CEString sError;
            m_pDb->GetLastError(sError);
            sMsg = L"DB error %d: ";
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg = CEString::sToString(m_pDb->iGetLastError());
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

ET_ReturnCode CVerifier::eGetFirstWordForm(IWordForm *& pWordForm)
{
    m_itCurrentForm = m_mmapStoredForms.begin();
    if (m_mmapStoredForms.end() == m_itCurrentForm)
    {
        return H_FALSE;
    }

    pWordForm = (*m_itCurrentForm).second;

    return H_NO_ERROR;
}

ET_ReturnCode CVerifier::eGetNextWordForm(IWordForm *& pWordForm)
{
    if (m_itCurrentForm != m_mmapStoredForms.end())
    {
        ++m_itCurrentForm;
    }

    if (m_mmapStoredForms.end() == m_itCurrentForm)
    {
        return H_NO_MORE;
    }

    pWordForm = (*m_itCurrentForm).second;

    return H_NO_ERROR;
}

ET_ReturnCode CVerifier::eGetStoredLexemeData(const CEString& sSelect)
{
    ET_ReturnCode hr = H_NO_ERROR;

    if (NULL == m_pDb)
    {
        ERROR_LOG(L"DB pointer is NULL.");
        return H_ERROR_POINTER;
    }

    m_vecStoredLexemes.clear();

    try
    {
        m_pDb->PrepareForSelect(sSelect);
        while (m_pDb->bGetRow())
        {
            CEString sLexemeHash;
            m_pDb->GetData(0, sLexemeHash);
            //            m_pDb->GetData (1, pco_v->i_DescriptorId);
            CEString sHeadword;
            m_pDb->GetData(2, sHeadword);

            m_vecStoredLexemes.push_back(StStoredLexeme(sLexemeHash, sHeadword));

        }   //  while (m_pDb->bGetRow())

        m_pDb->Finalize();
    }
    catch (...)
    {
        CEString sMsg = CEString::sToString (m_pDb->iGetLastError());
        ERROR_LOG((LPCTSTR)sMsg);

        return H_ERROR_GENERAL;
    }

    return H_NO_ERROR;

}   //  hGetStoredLexemeData (...)
