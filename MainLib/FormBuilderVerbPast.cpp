#include "StdAfx.h"
#include "FormBuilderVerbPast.h"

static wchar_t arrEndings[][4] = { L"л", L"ла", L"ло", L"ли" };

HRESULT CFormBuilderPast::hGetEnding (CEString& sLemma, 
                                      ET_Number eNumber, 
                                      ET_Gender eGender, 
                                      CEString& sEnding)
{
    HRESULT hr = S_OK;

    if (NUM_SG == eNumber)
    {
        if (GENDER_M == eGender && sLemma.bEndsWithOneOf (g_szConsonants))
        {
            sEnding = L"";
        }
        else
        {
            sEnding = arrEndings[eGender-1];
        }
    }
    else
    {
        sEnding = arrEndings[3];
    }

    return S_OK;

}   //  hGetEnding (...)

HRESULT CFormBuilderPast::hGetStressPositions (const CEString& sLemma,
                                               const CEString& sEnding,
                                               ET_Number eNumber, 
                                               ET_Gender eGender, 
                                               vector<int>& vecStressPositions)
{
    HRESULT hr = S_OK;

    ET_AccentType eAccentType = AT_UNDEFINED;
    if (AT_UNDEFINED == m_pLexeme->m_eAccentType2)
    {
        eAccentType = AT_A;
    }
    else
    {
        eAccentType = m_pLexeme->m_eAccentType2;
    }

    vector<ET_StressLocation> vecStressType;
    hGetPastTenseStressTypes (eAccentType, eNumber, eGender, vecStressType);

    vector<ET_StressLocation>::iterator itStressType = vecStressType.begin();
    for (; itStressType != vecStressType.end(); ++itStressType)
    {
        if (STRESS_LOCATION_STEM == *itStressType)
        {
            hGetStemStressPositions (const_cast<CEString&>(sLemma), vecStressPositions);
                                        // only to set vowels, no changes to the string
        }

        if (STRESS_LOCATION_ENDING == *itStressType)
        {
            if (AT_C2 == eAccentType && GENDER_M == eGender && NUM_SG == eNumber)
            {
                if (REFL_NO == m_pLexeme->m_eReflexive)
                {
                    ATLASSERT(0);
                    CEString sMsg (L"Unexpected part of speech value.");
                    ERROR_LOG (sMsg);
                    throw CException (E_FAIL, sMsg);
                }
                // This is or is becoming obsolete? (GDRL, p. 80-81)
                vecStressPositions.push_back (sLemma.uiNSyllables());
            }
            else
            {
                int iPos = -1;
                hGetEndingStressPosition (const_cast<CEString&>(sLemma), const_cast<CEString&>(sEnding), iPos);
                                                // Need to set vowels, no changes to the string.
                vecStressPositions.push_back (iPos);
            }
        }
    }

    return S_OK;

}   //  h_GetStressPositions (...)

HRESULT CFormBuilderPast::hCreateFormTemplate (const CEString& sLemma,
                                               const CEString& sEnding,
                                               ET_Number eNumber, 
                                               ET_Gender eGender, 
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
    pWordForm->m_eSubparadigm = SUBPARADIGM_PAST_TENSE;
    pWordForm->m_eAspect = m_pLexeme->m_eAspect;
    pWordForm->m_eReflexive = m_pLexeme->m_eReflexive;
    pWordForm->m_sLemma = sLemma;
    pWordForm->m_sWordForm = sLemma + sEnding;
    pWordForm->m_eGender = eGender;
    pWordForm->m_eNumber = eNumber;

    AssignSecondaryStress (pWordForm);

    return S_OK;

}   //  h_CreateFormTemplate (...)

HRESULT CFormBuilderPast::hAssemble (CWordForm * pWordForm, 
                                     int iStressPos, 
                                     CEString& sLemma, 
                                     CEString& sEnding)
{
    pWordForm->m_mapStress[iStressPos] = STRESS_PRIMARY;
    pWordForm->m_sLemma = sLemma;
    hHandleYoAlternation (iStressPos, SUBPARADIGM_PAST_TENSE, pWordForm->m_sLemma);
    pWordForm->m_sWordForm = pWordForm->m_sLemma + sEnding;
    if (bHasCommonDeviation (1) && !(NUM_SG == pWordForm->m_eNumber && GENDER_F == pWordForm->m_eGender))
    {
        hRetractStressToPreverb (pWordForm, bDeviationOptional (1));
    }

    if (bHasCommonDeviation (5) && 3 == m_pLexeme->m_iType && 1 == m_pLexeme->m_iStemAugment && 
        NUM_SG == pWordForm->m_eNumber && GENDER_M == pWordForm->m_eGender)
    {
        if (bDeviationOptional (5))
        {
            CComObject<CWordForm> * pVariant = NULL;
            hCloneWordForm (pWordForm, pVariant);
            m_pLexeme->hAddWordForm (pVariant);    // store both versions
            pWordForm = pVariant;
        }

        pWordForm->m_sWordForm = sLemma + L"нул";
    }

    return S_OK;

}   // hAssemble (...)

HRESULT CFormBuilderPast::hBuild()
{
    HRESULT hr = S_OK;

    bool bIsVariant = false;

    if (bHasIrregularPast())
    {
        hr = hBuildIrregular (bIsVariant);
        if (!bIsVariant)
        {
            return hr;
        }
    }

    try
    {
//        vector<int>& vecSourceStressPos = m_pLexeme->vec_SourceStressPos;   
        CEString sLemma;
        hr = hBuildPastTenseStem (sLemma);
		if (S_OK != hr)
		{
			return hr;
		}
		if (sLemma.bIsEmpty())
		{
			return E_UNEXPECTED;
		}

        ET_AccentType eAt = AT_UNDEFINED;
        if (AT_UNDEFINED == m_pLexeme->m_eAccentType2)
        {
            eAt = AT_A;
        }
        else
        {
            eAt = m_pLexeme->m_eAccentType2;
        }

        CEString sEnding;
        for (ET_Number eNumber = NUM_SG; eNumber < NUM_COUNT; ++eNumber)
        {
            for (ET_Gender eGender = GENDER_UNDEFINED; eGender < GENDER_COUNT; ++eGender)
            {
                if (NUM_PL == eNumber && eGender != GENDER_UNDEFINED)
                {
                    continue;
                }
                if (NUM_SG == eNumber && eGender == GENDER_UNDEFINED)
                {
                    continue;
                }

                hGetEnding (sLemma, eNumber, eGender, sEnding);

                if (REFL_YES == m_pLexeme->m_eReflexive)
                {
                    if (sEnding.bIsEmpty())
                    {
                        sEnding += L"ся";
                    }
                    else
                    {
                        if (sEnding.bEndsWithOneOf (g_szVowels))
                        {
                            sEnding += L"сь";
                        }
                        else
                        {
                            sEnding += L"ся";
                        }
                    }
                }

                vector<int> vecStress;
                hGetStressPositions (sLemma, sEnding, eNumber, eGender, vecStress);

                CComObject<CWordForm> * pWordForm = NULL;
                hCreateFormTemplate (sLemma, sEnding, eNumber, eGender, pWordForm);
                
                bool bIsVariant = false;
                hr = hHandleIrregularForms (pWordForm, bIsVariant);
                if (S_OK == hr)
                {
                    if (!bIsVariant)
                    {
                        continue;
                    }
                }
                else
                {
                    if (FAILED (hr))
                    {
                        ATLASSERT (0);
                        CString csMsg;
                        csMsg.Format (L"h_HandleIrregularForms() failed, error %x.", hr);
                        ERROR_LOG ((LPCTSTR)csMsg);
//                            throw CException (hr, (LPCTSTR)csMsg);
                        continue;
                    }
                }

//                int iGramHash = pWordForm->i_GramHash();
                if (1 == vecStress.size() || bMultiStress (sLemma, vecStress))
                {
                    vector<int>::iterator itStressPos = vecStress.begin();
                    for (; itStressPos != vecStress.end(); ++itStressPos)
                    {
                        hAssemble (pWordForm, *itStressPos, sLemma, sEnding);
                    }
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
                            pWfVariant->m_mapStress.clear();
                            pWordForm = pWfVariant;
                        }
                        hAssemble (pWordForm, *itStressPos, sLemma, sEnding);
                        m_pLexeme->hAddWordForm (pWordForm);
                    }
                }
            }   //  for (ET_Gender eGender = ... )
        }   // for ...(ET_Number = ... )
    }
    catch (CException ex)
    {
        return ex.iGetErrorCode();  // logging should be always done by callee
    }

    return hr;

}   //  h_Build()

HRESULT CFormBuilderPast::hBuildIrregular (bool& bIsVariant)
{
    HRESULT hr = S_OK;

    for (ET_Number eNumber = NUM_SG; eNumber < NUM_COUNT; ++eNumber)
    {
        for (ET_Gender eGender = GENDER_UNDEFINED; eGender < GENDER_COUNT; ++eGender)
        {
            if (NUM_PL == eNumber && eGender != GENDER_UNDEFINED)
            {
                continue;
            }
            if (NUM_SG == eNumber && eGender == GENDER_UNDEFINED)
            {
                continue;
            }

            CComObject<CWordForm> * pWordForm = NULL;
            hr = hCreateFormTemplate (L"", L"", eNumber, eGender, pWordForm);
            if (!pWordForm || S_OK != hr)
            {
                return S_OK == hr ? E_POINTER : hr;
            }
            
            hr = hHandleIrregularForms (pWordForm, bIsVariant);
            if (S_OK != hr)
            {
                if (FAILED (hr))
                {
                    ATLASSERT (0);
                    CString csMsg;
                    csMsg.Format (L"h_HandleIrregularForms() failed, error %x.", hr);
                    ERROR_LOG ((LPCTSTR)csMsg);
                    continue;
                }
            }

        }   //  for (ET_Gender eGender ...

    }   //  for (ET_Number eNumber ...

    return S_OK;

}   //  h_BuildIrregular (...)

//
// The older version is no longer needed since Tim's code already does that.
//
/*
//
// Needed when for abbreviated subparadigm rendering (GDRL 15, 16, 35, 89)
//
HRESULT CFormBuilderPast::h_BuildIrregular()
{
    HRESULT hr = S_OK;

    for (ET_Number eNumber = NUM_SG; eNumber < NUM_COUNT; ++eNumber)
    {
        for (ET_Gender eGender = GENDER_UNDEFINED; eGender < GENDER_COUNT; ++eGender)
        {
            if (NUM_PL == eNumber && eGender != GENDER_UNDEFINED)
            {
                continue;
            }
            if (NUM_SG == eNumber && eGender == GENDER_UNDEFINED)
            {
                continue;
            }

            CGramHasher co_hash (SUBPARADIGM_PAST_TENSE, 
                                   eNumber, 
                                   eGender, 
                                   PERSON_UNDEFINED, 
                                   ANIM_UNDEFINED,
                                   m_pLexeme->eo_Aspect,
                                   CASE_UNDEFINED, 
                                   m_pLexeme->m_eReflexive);

            multimap<int, StIrregularForm>& mmap_if = m_pLexeme->mmap_IrregularForms;
            multimap<int, StIrregularForm>::iterator it_if = mmap_if.find (co_hash.i_GramHash());
            
            if (GENDER_M == eGender || GENDER_F == eGender)
            {
                // These two forms must be in the db id the verb has 
                // irregular past tense subparadigm; GDRL p. 89
                if (m_pLexeme->mmap_IrregularForms.end() == it_if)
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Irregular form not found.");
                    return E_UNEXPECTED;
                }

                if ((*it_if).second.str_Form.empty())
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Empty irregular form.");
                    return E_UNEXPECTED;
                }

                m_pLexeme->h_AddIrregularForm (co_hash.i_GramHash(), (*it_if).second);

                continue;
            }

            if (GENDER_N == eGender || NUM_PL == eNumber)
            {
                if (m_pLexeme->mmap_IrregularForms.end() != it_if)
                {
                    if ((*it_if).second.str_Form.empty())
                    {
                        ATLASSERT(0);
                        ERROR_LOG (L"Empty irregular form.");
                        return E_UNEXPECTED;
                    }
                    
                    m_pLexeme->h_AddIrregularForm (co_hash.i_GramHash(), (*it_if).second);

                    continue;
                }

                // Construct per GDRL instructions if not found in the db
                StIrregularForm st_template;
                hr = h_GetPastFIrregular (st_template);
                if (S_OK != hr)
                {
                    return hr;
                }

                if (st_template.str_Form.empty())
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Empty irregular f. sg. past.");
                    return E_UNEXPECTED;
                }

                if (st_template.str_Form.length() < 2)
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"f. sg. past too short.");
                    return E_UNEXPECTED;
                }
                if (st_template.str_Form[st_template.str_Form.length()-1] != L'а')
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Unexpected ending in irregular f. sg. past.");
                    return E_UNEXPECTED;
                }

                StIrregularForm st_if (st_template);
                st_if.str_Form = st_template.str_Form.substr (0, st_if.str_Form.length()-1);
                if (GENDER_N == eGender && NUM_SG == eNumber)
                {
                    st_if.str_Form += L'о';
                }
                else if (NUM_PL == eNumber && GENDER_UNDEFINED == eGender)
                {
                    st_if.str_Form += L'и';
                }
                else
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Unexpected number or gender value.");
                    return E_UNEXPECTED;
                }

                m_pLexeme->h_AddIrregularForm (co_hash.i_GramHash(), st_if);

            }       //  if (GENDER_N == eGender || NUM_PL == eNumber)

        }   //  for (ET_Gender eGender ...
    }   //  for (ET_Number eNumber ...

    return S_OK;

}   //  h_BuildIrregular()
*/
