#include "StdAfx.h"
#include "FormBuilderBase.h"

void CFormBuilderBase::ReportDbError()
{
    if (!m_pDb)
    {
        ATLASSERT(0);
        ERROR_LOG (L"No DB handle");
        return;
    }

    CEString sMsg;
    try
    {
        CEString sError;
        m_pDb->GetLastError (sError);
        sMsg = L"DB error: ";
        sMsg += sError;
        sMsg += L"; code = ";
        sMsg += CEString::sToString (m_pDb->iGetLastError());
    }
    catch (...)
    {
        sMsg = L"Apparent DB error ";
    }

    sMsg += CEString::sToString (m_pDb->iGetLastError());
    ERROR_LOG (sMsg);
}

void CFormBuilderBase::AssignSecondaryStress (CComObject<CWordForm> * pWf)
{
    try
    {
        CEString& sGrStem = m_pLexeme->m_sGraphicStem;
        vector<int>::iterator itPos = m_pLexeme->m_vecSecondaryStressPos.begin();
        for (; itPos != m_pLexeme->m_vecSecondaryStressPos.end(); ++itPos)
        {
            unsigned int uiStressedSyll = sGrStem.uiGetSyllableFromVowelPos (*itPos);
            pWf->m_mapStress[uiStressedSyll] = STRESS_SECONDARY;
        }
    }
    catch (CException ex)
    {
        ATLASSERT(0);
        CEString sMsg (L"Stressed syllable not found, error: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG (sMsg);
    }
}       //  AssignSecondaryStress (...)

void CFormBuilderBase::AssignSecondaryStress (StIrregularForm& stIf)
{
    CEString& sGrStem = m_pLexeme->m_sGraphicStem;
    vector<int>::iterator itPos = m_pLexeme->m_vecSecondaryStressPos.begin();
    for (; itPos != m_pLexeme->m_vecSecondaryStressPos.end(); ++itPos)
    {
        int iStressedSyll = sGrStem.uiGetSyllableFromVowelPos (*itPos);
        if (iStressedSyll < 0)
        {
            ATLASSERT(0);
            CEString sMsg (L"Stressed syllable not found.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }
        stIf.mapStress[iStressedSyll] = STRESS_SECONDARY;
    }
}

//
// GDRL p. 11: a dash in a word with two main stress marks means a compound form
// with two main stresses; otherwise 
//
bool CFormBuilderBase::bMultiStress (const CEString& sLemma, vector<int>& vecStressPos)
{
    if (vecStressPos.size() < 2)
    {
        return false;
    }

    unsigned int uiDashPos = sLemma.uiFind (L"-");
    if (ecNotFound == uiDashPos || uiDashPos < 1)
    {
        return false;
    }

    // find any two that are separated by dash
    vector<int>::iterator itLeft = vecStressPos.begin();
    unsigned int uiLeftVowelPos = sLemma.uiGetVowelPos (*itLeft);
    if (uiLeftVowelPos < 0)
    {
        ATLASSERT(0);
        CEString sMsg (L"Can't find stressed vowel.");
        ERROR_LOG (sMsg);
        throw CException (E_FAIL, sMsg);
    }
    if (uiLeftVowelPos > uiDashPos)
    {
        return false;
    }

    vector<int>::iterator itRight = itLeft + 1;
    for (; itRight != vecStressPos.end(); ++itRight)
    {
        if (sLemma.uiGetVowelPos (*itRight) > uiDashPos)
        {
            return true;
        }
    }

    return false;

}   //  bMultiStress (...)

HRESULT CFormBuilderBase::hCloneWordForm (const CComObject<CWordForm> * pSource, 
                                          CComObject<CWordForm> *& pClone)
{
    HRESULT hr = S_OK;

    hr = CComObject<CWordForm>::CreateInstance (&pClone);
    if (S_OK != hr)
    {
        ATLASSERT(0);
        ERROR_LOG (L"CreateInstance failed for CWordForm.");
        return hr;
    }

    pClone->m_pLexeme = pSource->m_pLexeme;
    pClone->m_sWordForm = pSource->m_sWordForm;
    pClone->m_sLemma = pSource->m_sLemma;
    pClone->m_llLexemeId = pSource->m_llLexemeId;
    pClone->m_mapStress = pSource->m_mapStress;
    pClone->m_ePos = pSource->m_ePos;
    pClone->m_eCase = pSource->m_eCase;
    pClone->m_eNumber = pSource->m_eNumber;
    pClone->m_eSubparadigm = pSource->m_eSubparadigm;
    pClone->m_eGender = pSource->m_eGender;
//    pClone->m_eTense = pSource->m_eTense;
    pClone->m_ePerson = pSource->m_ePerson;
    pClone->m_eAnimacy = pSource->m_eAnimacy;
    pClone->m_eReflexive = pSource->m_eReflexive;
//    pClone->m_eVoice = pSource->m_eVoice;
    pClone->m_eStatus = pSource->m_eStatus;

    return S_OK;

}  // h_CloneWordForm (...)

HRESULT CFormBuilderBase::hCloneWordForm (const  CWordForm * pSource, CComObject<CWordForm> *& pClone)
{
    HRESULT hr = S_OK;

    hr = CComObject<CWordForm>::CreateInstance (&pClone);
    if (S_OK != hr)
    {
        ERROR_LOG (L"CreateInstance failed for CWordForm.");
        ATLASSERT(0);
        return hr;
    }
    pClone->m_pLexeme = pSource->m_pLexeme;
    pClone->m_sWordForm = pSource->m_sWordForm;
    pClone->m_sLemma = pSource->m_sLemma;
    pClone->m_llLexemeId = pSource->m_llLexemeId;
    pClone->m_mapStress = pSource->m_mapStress;
    pClone->m_ePos = pSource->m_ePos;
    pClone->m_eCase = pSource->m_eCase;
    pClone->m_eNumber = pSource->m_eNumber;
    pClone->m_eSubparadigm = pSource->m_eSubparadigm;
    pClone->m_eGender = pSource->m_eGender;
//    pClone->m_eTense = pSource->m_eTense;
    pClone->m_ePerson = pSource->m_ePerson;
    pClone->m_eAnimacy = pSource->m_eAnimacy;
    pClone->m_eReflexive = pSource->m_eReflexive;
//    pClone->m_eVoice = pSource->m_eVoice;
//    pClone->m_eSpecialForm = pSource->m_eSpecialForm;
//    pClone->m_eAspect = pSource->m_eAspect;
    pClone->m_eStatus = pSource->m_eStatus;

    return S_OK;

}  // h_CloneWordForm (...)

HRESULT CFormBuilderBase::hWordFormFromHash (int iHash, unsigned int uiAt, CWordForm *& pWf)
{
    pair<multimap<int, CComVariant>::iterator, multimap<int, CComVariant>::iterator> pairRange;
    pairRange = m_pLexeme->m_coll.equal_range (iHash);
    if (pairRange.first == pairRange.second)
    {
        ATLASSERT(0);
        CEString sMsg (L"Unable to locate word form in collection.");
        ERROR_LOG (sMsg);
        throw CException (E_FAIL, sMsg);
    }

    if (distance (pairRange.first, pairRange.second) - 1 < (int)uiAt)
    {
        ATLASSERT(0);
        CEString sMsg (L"Bad form number.");
        ERROR_LOG (sMsg);
        throw CException (E_INVALIDARG, sMsg);
    }

    multimap<int, CComVariant>::iterator itWf (pairRange.first);
    advance (itWf, uiAt);
    if ((*itWf).first != iHash)
    {
        ATLASSERT(0);
        CEString sMsg (L"Error extracting map element.");
        ERROR_LOG (sMsg);
        throw CException (E_INVALIDARG, sMsg);
    }

    CComQIPtr<IWordForm> spWf = (*itWf).second.pdispVal;
    if (NULL == spWf)
    {
        ATLASSERT(0);
        CEString sMsg (L"QI for IWordForm failed.");
        ERROR_LOG (sMsg);
        throw CException (E_POINTER, sMsg);
    }

    pWf = dynamic_cast<CWordForm *> (spWf.p);
    if (NULL == pWf)
    {
        ATLASSERT(0);
        CEString sMsg (L"Failed to obtain 3 Pl. word form.");
        ERROR_LOG (sMsg);
        throw CException (E_POINTER, sMsg);
    }

    return S_OK;

}   //  hWordFormFromHash (...)

HRESULT CFormBuilderBase::hHandleIrregularForms (CComObject<CWordForm> * pWordForm, bool& bIsVariant)
{
    HRESULT hr = S_OK;
 
    int iHash = pWordForm->iGramHash();
    StIrregularForm stIf;
    hr = m_pLexeme->hGetFirstIrregularForm (iHash, stIf);
    if (S_OK != hr)
    {
        return hr;
    }

    while (S_OK == hr)
    {
        bIsVariant = stIf.bIsVariant;
        pWordForm->m_sWordForm = stIf.sForm;
        pWordForm->m_mapStress = stIf.mapStress;
        m_pLexeme->hAddWordForm (pWordForm);            
        
        hr = m_pLexeme->hGetNextIrregularForm (iHash, stIf);
        if (S_OK == hr)
        {
            CComObject<CWordForm> * pNext = NULL;
            hr = hCloneWordForm (pWordForm, pNext);
            if (S_OK != hr)
            {
                return hr;
            }
            pWordForm = pNext;
        }
    }       //  while (S_OK == hr)

    return S_OK;

}       //  hHandleIrregularForms (...)

unsigned int CFormBuilderBase::uiNForms (int iHash)
{
    return m_pLexeme->m_coll.count (iHash);
}

bool CFormBuilderBase::bHasCommonDeviation (int iCd)
{
    bool bRet = false;
    map<int, bool>::iterator itCd = m_pLexeme->m_mapCommonDeviations.find (iCd);
    return (itCd != m_pLexeme->m_mapCommonDeviations.end());
}

bool CFormBuilderBase::bDeviationOptional (int iCd)
{
    bool bRet = false;
    map<int, bool>::iterator itCd = m_pLexeme->m_mapCommonDeviations.find (iCd);
    if (itCd != m_pLexeme->m_mapCommonDeviations.end())
    {
        return (*itCd).second;
    }
    else
    {
        ATLASSERT(0);
        CEString sMsg (L"Common deviation expected.");
        ERROR_LOG (sMsg);
        throw CException (E_FAIL, sMsg);
    }
}       //  bDeviationOptional (...)

HRESULT CFormBuilderBase::hRetractStressToPreverb (CWordForm * pWordForm, bool bIsOptional)
{
    HRESULT hr = S_OK;

    if (bIsOptional)
    {
        CComObject<CWordForm> * pVariant = NULL;
        hCloneWordForm (pWordForm, pVariant);
        m_pLexeme->hAddWordForm (pVariant);    // store both versions
        pWordForm = pVariant;
    }

    map<int, ET_StressType> mapCorrectedStress;
    map<int, ET_StressType>::iterator itStressPos = pWordForm->m_mapStress.begin();
    for (; itStressPos != pWordForm->m_mapStress.end(); ++itStressPos)
    {
          if (STRESS_SECONDARY == (*itStressPos).second)
          {
            mapCorrectedStress[(*itStressPos).first] = STRESS_SECONDARY;
            continue;
        }
        if ((*itStressPos).first < 1)
        {
            ATLASSERT(0);
            CEString sMsg (L"Unexpected stress position in cd-1 verb.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }
        if (pWordForm->m_sWordForm.bStartsWith (L"пере"))
        {
            if ((*itStressPos).first < 2)
            {
                ATLASSERT(0);
                CEString sMsg (L"Unexpected stress position in cd-1 verb.");
                ERROR_LOG (sMsg);
                throw CException (E_FAIL, sMsg);
            }
            mapCorrectedStress[(*itStressPos).first-2] = STRESS_PRIMARY;
        }
        else
        {
            mapCorrectedStress[(*itStressPos).first-1] = STRESS_PRIMARY;
        }
    }
    pWordForm->m_mapStress = mapCorrectedStress;

    return S_OK;

}   //  hRetractStressToPreverb (...)
