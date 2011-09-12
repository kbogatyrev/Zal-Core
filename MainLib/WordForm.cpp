#include "StdAfx.h"
#include "WordForm.h"
#include "Lexeme.h"

HRESULT CWordForm::get_Wordform (BSTR * pbstrWordForm)
{
    if (m_sWordForm.bIsEmpty())
    {
        return S_FALSE;
    }
    CComBSTR spBstr (m_sWordForm);
    *pbstrWordForm = spBstr.Detach();

    return S_OK;
}

HRESULT CWordForm::get_Lemma (BSTR * pbsLemma)
{
    if (m_sLemma.bIsEmpty())
    {
        return S_FALSE;
    }
    CComBSTR spBstr (m_sLemma);
    *pbsLemma = spBstr.Detach();

    return S_OK;
}

HRESULT CWordForm::get_LexemeId (LONG * plID)
{
    *plID = (long)m_llLexemeId;
    return S_OK;
    //return E_NOTIMPL;
}

HRESULT CWordForm::get_PartOfSpeech (ET_PartOfSpeech * pePartOfSpeech)
{
    *pePartOfSpeech = (ET_PartOfSpeech)m_ePos;
    return S_OK;
}

HRESULT CWordForm::get_Subparadigm (ET_Subparadigm * peSubparadigm)
{
    *peSubparadigm = (ET_Subparadigm)m_eSubparadigm;
    return S_OK;
}

HRESULT CWordForm::get_Case (ET_Case * peCase)
{
    *peCase = (ET_Case)m_eCase;
    return S_OK;
}

HRESULT CWordForm::get_Number (ET_Number * peNumber)
{
    *peNumber = (ET_Number)m_eNumber;
    return S_OK;
}

HRESULT CWordForm::get_Gender (ET_Gender * peGender)
{
    *peGender = (ET_Gender)m_eGender;
    return S_OK;
}

//HRESULT CWordForm::get_Tense (ET_Tense * pe_tense)
//{
//    *pe_tense = (ET_Tense)eo_Tense;
//    return S_OK;
//}

HRESULT CWordForm::get_Person (ET_Person * pePerson)
{
    *pePerson = (ET_Person)m_ePerson;
    return S_OK;
}

HRESULT CWordForm::get_IsReflexive (ET_Reflexive * peReflexive)
{
    *peReflexive = m_eReflexive;
    return S_OK;
}

HRESULT CWordForm::get_Animacy (ET_Animacy * peAnimacy)
{
    *peAnimacy = (ET_Animacy)m_eAnimacy;
    return S_OK;
}

//HRESULT CWordForm::get_Voice (ET_Voice * pe_voice)
//{
//    *pe_voice = (ET_Voice)eo_Voice;
//    return S_OK;
//}

//HRESULT CWordForm::get_SpecialForm (ET_SpecialForm * pe_specialForm)
//{
//    *pe_specialForm = (ET_SpecialForm)eo_SpecialForm;
//    return S_OK;
//}

HRESULT CWordForm::get_Aspect (ET_Aspect * peAspect)
{
    *peAspect = (ET_Aspect)m_eAspect;
    return S_OK;
}

HRESULT CWordForm::get_Stress (LONG * plStressPos)
{
    map<int, ET_StressType>::iterator itPos = m_mapStress.begin();
    while (itPos != m_mapStress.end() && itPos->second != STRESS_PRIMARY)
    {
        ++itPos;
    }

    if (m_mapStress.end() == itPos)
    {
        /*
        ATLASSERT(0);
        ERROR_LOG (L"Unable to find stress position.");
        return E_FAIL;
        */
        // there are stressless words, e. g. some prepositions
        *plStressPos = -1;
        return S_OK;
    }

//    *pl_stressPos = it_pos->first;
    CEString sWf (m_sWordForm);
    sWf.SetVowels (g_szVowels);
    *plStressPos = sWf.uiGetVowelPos (itPos->first);

    if (m_mapStress.size() > 1)
    {
        return S_MORE;
    }
    else
    {
        return S_OK;
    }

}   //  get_Stress (...)

HRESULT CWordForm::GetStressPos (LONG lAt, LONG * plStressPosition, BOOL * pbIsPrimary)
{
    map<int, ET_StressType>::iterator itStressPos = m_mapStress.find (lAt);
    if (m_mapStress.end() != itStressPos)
    {
        CEString sWf (m_sWordForm);
//        xstr_wf.v_SetVowels (g_sVowels);
        *plStressPosition = sWf.uiGetVowelPos (itStressPos->first);
        *pbIsPrimary = (STRESS_PRIMARY == itStressPos->second) ? TRUE : FALSE;

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

HRESULT CWordForm::get_StressCount (LONG * plVal)
{
    *plVal = m_mapStress.size();
    return S_OK;
}

HRESULT CWordForm::get_IsPrimaryStress (LONG lPos, BOOL * pbType)
{
    CEString sWf (m_sWordForm);
//    sWf.v_SetVowels (g_sVowels);
    unsigned int uiSyll = sWf.uiGetSyllableFromVowelPos (lPos);

    map<int, ET_StressType>::iterator it_ = m_mapStress.find (uiSyll);
    if (m_mapStress.end() == it_)
    {
        ERROR_LOG (L"Unable to find stress position.");
        return E_INVALIDARG;
    }
    else
    {
        *pbType =  (STRESS_PRIMARY == it_->second) ? TRUE : FALSE;
        return S_OK;
    }
}

HRESULT CWordForm::get_Status (ET_Status * peStatus)
{
    *peStatus = (ET_Status)m_eStatus;
    return S_OK;
}

HRESULT CWordForm::get_Hash (LONG * plHash)
{
    HRESULT hr = S_OK;

    *plHash = iGramHash();

    return S_OK;    
}

HRESULT CWordForm::SaveTestData()
{
    HRESULT hr = S_OK;

    CSqlite& db = *m_pLexeme->m_pDb;

    int iHash = iGramHash();
    try
    {
        db.PrepareForInsert (L"test_data", 3);
//        db.v_Bind (1, pco_Lexeme->i_DbKey);  // descriptor id
//        db.v_Bind (1, (__int64)pco_Lexeme->ui_Hash());    // lexeme hash
        db.Bind (1, m_pLexeme->sHash());    // lexeme hash
        db.Bind (2, iHash);
        db.Bind (3, m_sWordForm);
        db.InsertRow();
        db.Finalize();
        
        __int64 llWordFormId = db.llGetLastKey();

        map<int, ET_StressType>::iterator itStress = m_mapStress.begin();
        for (; itStress != m_mapStress.end(); ++itStress)
        {
            db.PrepareForInsert (L"test_data_stress", 3);
            db.Bind (1, llWordFormId);
            db.Bind (2, (*itStress).first);
            ET_StressType eType = (*itStress).second ? STRESS_PRIMARY : STRESS_SECONDARY;
            db.Bind (3, eType);
            db.InsertRow();
            db.Finalize();
        }
    }
    catch (CException& exc)
    {
        db.RollbackTransaction();
        CEString sMsg (exc.szGetDescription());
        CEString sError;
        try
        {
            db.GetLastError (sError);
            sMsg += L", error: ";
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }
    
        sMsg += CEString::sToString (db.iGetLastError());
        ERROR_LOG (sMsg);

        return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, 0xF000 + db.iGetLastError());
    }

    return S_OK;

}   //  SaveTestData()

/////////////////////////////////////////////////////////////////

int CWordForm::iGramHash()
{
    CHasher h_ (*this);
    return h_.iGramHash();
}

HRESULT CWordForm::hDecodeHash (int iHash)
{
    try
    {
        CHasher hasher (*this);
        HRESULT hr = hasher.hDecodeHash (iHash);
        this->m_eAnimacy = hasher.m_eAnimacy;
        this->m_eAspect = hasher.m_eAspect;
        this->m_eCase = hasher.m_eCase;
        this->m_eGender = hasher.m_eGender;
        this->m_eNumber = hasher.m_eNumber;
        this->m_ePerson = hasher.m_ePerson;
        this->m_ePos = hasher.m_ePos;
        this->m_eReflexive = hasher.m_eReflexive;
        this->m_eSubparadigm = hasher.m_eSubparadigm;
        
        return hr;
    }
    catch (CException ex)
    {
        return ex.iGetErrorCode();  // logging should be always done by callee
    }

}   //  hDecodeHash (...)
