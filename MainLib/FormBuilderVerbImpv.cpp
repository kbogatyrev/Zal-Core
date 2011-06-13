#include "StdAfx.h"
#include "FormBuilderVerbImpv.h"

HRESULT CFormBuilderImperative::hGetLemma (CEString& sLemma)
{
    int iHash = -1;
    if (4 == m_pLexeme->m_iType || 5 == m_pLexeme->m_iType)
    {
        CGramHasher hasher (POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_SG, GENDER_UNDEFINED, 
                            PERSON_3, ANIM_UNDEFINED, m_pLexeme->m_eAspect, m_pLexeme->m_eReflexive);
        iHash = hasher.iGramHash();
    }
    else
    {
        CGramHasher hasher (POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_SG, GENDER_UNDEFINED, 
                            PERSON_1, ANIM_UNDEFINED, m_pLexeme->m_eAspect, m_pLexeme->m_eReflexive);
        iHash = hasher.iGramHash();
    }

    CWordForm * pTemplate = NULL;
    hWordFormFromHash (iHash, 0, pTemplate);
    sLemma = pTemplate->m_sLemma;

    if (11 == m_pLexeme->m_iType)
    {
        ATLASSERT (sLemma.bEndsWith (L"ь"));
        sLemma.sErase (sLemma.uiLength()-1);
    }

    return S_OK;

}   //  hGetLemma (...)

HRESULT CFormBuilderImperative::hGetEnding (const CEString& sLemma, CEString& sEnding)
{
    HRESULT hr = S_OK;

    ET_AccentType eAccentType1 = m_pLexeme->m_eAccentType1;
    ET_AccentType eAccentType2 = m_pLexeme->m_eAccentType2;

    int iType = m_pLexeme->m_iType;

    if (11 == iType)
    {
        sEnding = L"ей";
    }
    else if (sLemma.bEndsWithOneOf (g_szVowels))
    {
        sEnding = L"й";
        if (4 == iType)
        {
            if (AT_B == eAccentType1 || AT_C == eAccentType1)   // кроить, поить
            {
                sEnding = L"и";
            }
            if (AT_A == eAccentType1 && sLemma.bStartsWith (L"вы"))
            {
                sEnding = L"и";
            }
        }
    }
    else
    {
        if (AT_B == eAccentType1 || AT_C == eAccentType1 || AT_C1 == eAccentType1)
        {
            sEnding = L"и";
        }
        if (AT_A == eAccentType1)
        {
            if (sLemma.bStartsWith (L"вы"))
            {
                sEnding = L"и";
            }
            else if (sLemma.bEndsWith (L"щ"))      // 2 consonants or щ or CьC
            {
                sEnding = L"и";
            }
            else if (sLemma.uiLength() >= 2 && 
                CEString::bIn (sLemma[sLemma.uiLength()-1], g_szConsonants) && 
                CEString::bIn (sLemma[sLemma.uiLength()-2], g_szConsonants))
            {
                sEnding = L"и";
            }
            else if (sLemma.uiLength() >= 3 && 
                     CEString::bIn (sLemma[sLemma.uiLength()-1], g_szConsonants) &&
                     L'ь' == sLemma[sLemma.uiLength()-2] &&
                     CEString::bIn (sLemma[sLemma.uiLength()-3], g_szConsonants))
            {
                sEnding = L"и";
            }
            else
            {
                sEnding = L"ь";
            }
        }
    }   // else

    return S_OK;

}   //  hGetEnding (...)

HRESULT CFormBuilderImperative::hGetStressPositions (CEString& sLemma, 
                                                     CEString& sEnding,
                                                     vector<int>& vecStressPositions)
{
    HRESULT hr = S_OK;

    ET_StressLocation eStress = STRESS_LOCATION_UNDEFINED;
    switch (m_pLexeme->m_eAccentType1)
    {
        case AT_A:
        {
            eStress = STRESS_LOCATION_STEM;
            break;
        }
        case AT_B:
        case AT_C:
		case AT_C1:			// хотеть and derivatives
        {
            eStress = STRESS_LOCATION_ENDING;
            break;
        }
        default:
        {
            ATLASSERT(0);
            CEString sMsg (L"Illegal accent type.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }
    }

    int iStressPos = -1;
    if (STRESS_LOCATION_STEM == eStress)
    {
        hGetStemStressPositions (sLemma, vecStressPositions);
    }
    else if (STRESS_LOCATION_ENDING == eStress)
    {
        hGetEndingStressPosition (sLemma, sEnding, iStressPos);
        vecStressPositions.push_back (iStressPos);
    }
    else
    {
        ATLASSERT(0);
        CEString sMsg (L"Illegal stress type.");
        ERROR_LOG (sMsg);
        throw CException (E_INVALIDARG, sMsg);
    }

    return hr;

}   //  hGetStressPositions (...)

HRESULT CFormBuilderImperative::hCreateFormTemplate (const CEString& sLemma,
                                                     CEString& sEnding,
                                                     ET_Number eNumber,
                                                     CComObject<CWordForm> *& pWordForm)
{
    HRESULT hr = S_OK;

    hr = CComObject<CWordForm>::CreateInstance (&pWordForm);
    if (S_OK != hr)
    {
        ATLASSERT(0);
        CString csMsg;
        csMsg.Format (L"CreateInstance() failed, error %x.", hr);
        ERROR_LOG ((LPCTSTR)csMsg);
        throw CException (hr, (LPCTSTR)csMsg);
    }

    pWordForm->m_pLexeme = m_pLexeme;
    pWordForm->m_ePos = m_pLexeme->m_ePartOfSpeech;
    pWordForm->m_eSubparadigm = SUBPARADIGM_IMPERATIVE;
    pWordForm->m_eAspect = m_pLexeme->m_eAspect;
    pWordForm->m_eReflexive = m_pLexeme->m_eReflexive;
    pWordForm->m_sLemma = sLemma;
    if (NUM_PL == eNumber)
    {
        sEnding += L"те";
    }
    pWordForm->m_sWordForm = sLemma + sEnding;
    if (REFL_YES == m_pLexeme->m_eReflexive)
    {
        int iAt = pWordForm->m_sWordForm.uiLength() - 1;
        if (CEString::bIn (pWordForm->m_sWordForm[iAt], g_szVowels))
        {
            pWordForm->m_sWordForm += L"сь";
        }
        else
        {
            pWordForm->m_sWordForm += L"ся";
        }
    }

    pWordForm->m_eNumber = eNumber;
    pWordForm->m_ePerson = PERSON_2;

    AssignSecondaryStress (pWordForm);

    return S_OK;

}   //  hCreateFormTemplate (...)

HRESULT CFormBuilderImperative::hHandleCommonDeviations (CWordForm * pWordForm)
{
    HRESULT hr = S_OK;

    if (!bHasCommonDeviation (2) && !bHasCommonDeviation (3))
    {
        return S_FALSE;
    }

    int iEndingOffset = (REFL_NO == m_pLexeme->m_eReflexive)
        ? pWordForm->m_sWordForm.uiLength() - 1
        : pWordForm->m_sWordForm.uiLength() - 3;

    if (NUM_PL == pWordForm->m_eNumber)
    {
        iEndingOffset -= 2;
    }

    if (iEndingOffset < 1)
    {
        ATLASSERT(0);
        CEString sMsg (L"Bad ending offset.");
        ERROR_LOG (sMsg);
        throw CException (E_POINTER, sMsg);
    }

    if (bHasCommonDeviation (2))
    {
        if (bDeviationOptional (2))   // store both forms
        {
            CComObject<CWordForm> * pVariant = NULL;
            hCloneWordForm (pWordForm, pVariant);
            m_pLexeme->hAddWordForm (pVariant);
            pWordForm = pVariant;
        }

        if (CEString::bIn (pWordForm->m_sWordForm[iEndingOffset-1], g_szVowels))
        {
            pWordForm->m_sWordForm[iEndingOffset] = L'й';
        }
        else
        {
            pWordForm->m_sWordForm[iEndingOffset] = L'ь';
        }
    }   //      if (bHasCommonDeviation (2))

    if (bHasCommonDeviation (3))
    {
        if (NUM_SG == pWordForm->m_eNumber)
        {
            if (bDeviationOptional (3))   // store both forms
            {
                pWordForm->m_sWordForm[iEndingOffset] = L'ь';
                CComObject<CWordForm> * pVariant = NULL;
                hCloneWordForm (pWordForm, pVariant);
                m_pLexeme->hAddWordForm (pVariant);
                pWordForm = pVariant;
            }
            pWordForm->m_sWordForm[iEndingOffset] = L'и';
        }
        else
        {
            pWordForm->m_sWordForm[iEndingOffset] = L'ь';
        }
    }

    return S_OK;

}   //  hHandleCommonDeviations (...)

HRESULT CFormBuilderImperative::hCheckForIrregularForms (bool& bIsVariant)
                                                // check if irregular forms were built and
                                                // regular processing must be skipped
{
    HRESULT hr = S_OK;

    if (bHasIrregularImperative() || bHasIrregularPresent())
    {
        hr = hBuildIrregularForms();
        if (S_OK != hr)
        {
            return hr;
        }
    }
    else
    {
        return S_FALSE;
    }
        
    bIsVariant = false;
    for (ET_Number eNumber = NUM_SG; eNumber < NUM_COUNT; ++eNumber)
    {
        CComObject<CWordForm> * pWordForm = NULL;
        hr = CComObject<CWordForm>::CreateInstance (&pWordForm);
        if (S_OK != hr)
        {
            ATLASSERT(0);
            CString csMsg;
            csMsg.Format (L"CreateInstance() failed, error %x.", hr);
            ERROR_LOG ((LPCTSTR)csMsg);
            throw CException (hr, (LPCTSTR)csMsg);
        }

        pWordForm->m_pLexeme = m_pLexeme;
        pWordForm->m_ePos = m_pLexeme->m_ePartOfSpeech;
        pWordForm->m_eSubparadigm = SUBPARADIGM_IMPERATIVE;
        pWordForm->m_eAspect = m_pLexeme->m_eAspect;
        pWordForm->m_eReflexive = m_pLexeme->m_eReflexive;
        pWordForm->m_ePerson = PERSON_2;
        pWordForm->m_eNumber = eNumber;

        AssignSecondaryStress (pWordForm);
        
        hr = hHandleIrregularForms (pWordForm, bIsVariant);
        if (S_OK != hr)
        {
            ATLASSERT (0);
            CString csMsg;
            csMsg.Format (L"h_HandleIrregularForms() failed, error %x.", hr);
            ERROR_LOG ((LPCTSTR)csMsg);
            return hr;
        }
    }       //  for (ET_Number eo_number = NUM_SG; ...

    return hr;

  }     //  hCheckForIrregularForms (...)

HRESULT CFormBuilderImperative::hBuild()
{
    HRESULT hr = S_OK;

    bool bIsVariant = false;
    hr = hCheckForIrregularForms (bIsVariant);
    if (S_OK == hr && !bIsVariant)
    {
        return hr;
    }

    try
    {
        CEString sLemma;

        if (17 == m_pLexeme->m_iSection)
        {
            ATLASSERT (14 == m_pLexeme->m_iType);
            return S_FALSE;
        }

        if (4 == m_pLexeme->m_iType || 5 == m_pLexeme->m_iType)
        {
            sLemma = m_pLexeme->m_s3SgStem;
        }
        else
        {
            sLemma = m_pLexeme->m_s1SgStem;
        }

        if (sLemma.bIsEmpty())
        {
            ATLASSERT(0);
            CEString sMsg (L"Empty lemma for ");
            sMsg += m_pLexeme->m_sInfinitive;
            ERROR_LOG (sMsg);
            return E_FAIL;
        }

        if (11 == m_pLexeme->m_iType)
        {
            ATLASSERT (sLemma.bEndsWith (L"ь"));
            sLemma.sErase (sLemma.uiLength()-1);
        }
        
        hFleetingVowelCheck (sLemma);

        CEString sEnding;
        hGetEnding (sLemma, sEnding);

        vector<int> vecStress;
        hGetStressPositions (sLemma, sEnding, vecStress);

        for (ET_Number eNumber = NUM_SG; eNumber < NUM_COUNT; ++eNumber)
        {
            vector<int>::iterator itStressPos = vecStress.begin();

            CComObject<CWordForm> * pWordForm = NULL;
            hCreateFormTemplate (sLemma, sEnding, eNumber, pWordForm);

            if (1 == vecStress.size() || bMultiStress (sLemma, vecStress))
            {
                vector<int>::iterator itStressPos = vecStress.begin();
                for (; itStressPos != vecStress.end(); ++itStressPos)
                {
                    pWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;  // primary
                }
                hHandleCommonDeviations (pWordForm);
                m_pLexeme->hAddWordForm (pWordForm);
            }
            else
            {
                vector<int>::iterator itStressPos = vecStress.begin();
                for (; itStressPos != vecStress.end(); ++itStressPos)
                {
                    if (itStressPos != vecStress.begin())
                    {
                        CComObject<CWordForm> * pWfVariant = NULL;
                        hCloneWordForm (pWordForm, pWfVariant);
                        pWordForm = pWfVariant;
                    }
                    pWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                    hHandleCommonDeviations (pWordForm);
                    m_pLexeme->hAddWordForm (pWordForm);
                }
            }
        }       //  for (; itStressPos != vecSourceStressPos.end(); ...)
    }
    catch (CException ex)
    {
        return ex.iGetErrorCode();  // logging should be always done by callee
    }

    return hr;

}   //  hBuildImperativeForms()

//
// Irregular forms may or may not be in the database
//
HRESULT CFormBuilderImperative::hBuildIrregularForms()
{
    HRESULT hr = S_OK;

    CGramHasher sgHash (SUBPARADIGM_IMPERATIVE, 
                        NUM_SG, 
                        GENDER_UNDEFINED, 
                        PERSON_2, 
                        ANIM_UNDEFINED, 
						m_pLexeme->m_eAspect,
                        CASE_UNDEFINED, 
                        m_pLexeme->m_eReflexive);
    CGramHasher plHash (sgHash);
    plHash.m_eNumber = NUM_PL;

    if (bHasIrregularImperative())
    {
        StIrregularForm stIfSg;
        hr = m_pLexeme->hGetFirstIrregularForm (sgHash.iGramHash(), stIfSg);
        if (FAILED (hr))
        {
            ATLASSERT(0);
            CEString sMsg (L"Error retrieving irregular imperative singular for ");
            sMsg += m_pLexeme->m_sInfinitive;
            ERROR_LOG (sMsg);
            return hr;
        }

        StIrregularForm stIfPl;
        HRESULT hPlFormRet = m_pLexeme->hGetFirstIrregularForm (plHash.iGramHash(), stIfPl);
        if (FAILED (hPlFormRet))
        {
            ATLASSERT(0);
            CEString sMsg (L"Error retrieving irregular imperative plural for ");
            sMsg += m_pLexeme->m_sInfinitive;
            ERROR_LOG (sMsg);
            return hPlFormRet;
        }

        // Derive from Sg if not in DB
        if (S_OK != hPlFormRet)
        {
            stIfPl = stIfSg;
            stIfPl.sForm += L"те";
            if (REFL_YES == m_pLexeme->m_eReflexive)
            {
                stIfPl.sForm += L"сь";
            }
        }
        
        m_pLexeme->hAddIrregularForm (plHash.iGramHash(), stIfPl);
            
        return S_OK;

    }   //   if (bHasIrregularImperative())

    //
    // No irregular forms in DB: derive from present tense
    //
    if (!bHasIrregularPresent())
    {
        ATLASSERT(0);
        CEString sMsg (L"No irregular present.");
        sMsg += m_pLexeme->m_sInfinitive;
        ERROR_LOG (sMsg);
        return E_UNEXPECTED;
    }

    //
    // No imperative forms provided -- construct imperative from present tense
    //
    StIrregularForm st3Pl;
    hr = hGet3PlIrregular (st3Pl);
    if (S_OK != hr)
    {
        return hr;
    }

    CEString s3Pl (st3Pl.sForm);
    if (REFL_NO == m_pLexeme->m_eReflexive)
    {
        if (!s3Pl.bEndsWith (L"ют") && !s3Pl.bEndsWith (L"ут") && !s3Pl.bEndsWith (L"ят") && 
            !s3Pl.bEndsWith (L"ат"))
        {
            ATLASSERT(0);
            CEString sMsg (L"Unexpected 3 Pl ending: ");
            sMsg += s3Pl;
            ERROR_LOG (sMsg);
            return E_UNEXPECTED;
        }
    }
    else if (REFL_YES == m_pLexeme->m_eReflexive)
    {
        if (!s3Pl.bEndsWith (L"ются") && !s3Pl.bEndsWith (L"утся") && 
            !s3Pl.bEndsWith (L"ятся") && !s3Pl.bEndsWith (L"атся"))
        {
            ATLASSERT(0);
            CEString sMsg (L"Unexpected 3 Pl refl. ending: ");
            sMsg += s3Pl;
            ERROR_LOG (sMsg);
            return E_UNEXPECTED;
        }
    }
    else
    {
        ATLASSERT(0);
        CEString sMsg (L"Unexpected eo_Reflexive value for: ");
        sMsg += m_pLexeme->m_sInfinitive;
        ERROR_LOG (sMsg);
        return E_UNEXPECTED;
    }

    int iCharsToErase = REFL_YES == m_pLexeme->m_eReflexive ? 4 : 2;
    s3Pl.sErase (s3Pl.uiLength()-iCharsToErase);

    StIrregularForm stIfSg, stIfPl;

    if (s3Pl.bEndsWithOneOf (g_szConsonants))
    {
        stIfSg.sForm = s3Pl + L"и";
        stIfSg.mapStress[s3Pl.uiGetNumOfSyllables()] = STRESS_PRIMARY;
    }
    else
    {
        stIfSg.sForm = s3Pl + L"й";
        stIfSg.mapStress = st3Pl.mapStress;
    }

    stIfPl.sForm = stIfSg.sForm + L"те";
    stIfPl.mapStress = stIfSg.mapStress;

    if (REFL_YES == m_pLexeme->m_eReflexive)
    {
        int iAt = stIfSg.sForm.uiLength()-1;
        if (CEString::bIn (stIfSg.sForm[iAt], g_szVowels))
        {
            stIfSg.sForm += L"сь";
        }
        else
        {
            stIfSg.sForm += L"ся";
        }
        stIfPl.sForm += L"сь";
    }

    m_pLexeme->hAddIrregularForm (sgHash.iGramHash(), stIfSg);
    m_pLexeme->hAddIrregularForm (plHash.iGramHash(), stIfPl);

    return S_OK;

}   //  hBuildIrregularForms()
