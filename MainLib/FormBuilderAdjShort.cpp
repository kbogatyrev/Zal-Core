#include "StdAfx.h"
#include "Endings.h"
#include "FormBuilderAdjShort.h"

HRESULT CFormBuilderShortAdj::hGetEndings()
{
    HRESULT hr = S_OK;

    CEString sSelect (L"SELECT DISTINCT ending, gender, number, stress FROM endings");
    sSelect += L" WHERE inflection_class = ";
    sSelect += CEString::sToString (ENDING_CLASS_SHORT_ADJECTIVE);
    sSelect += L" AND inflection_type = ";
    sSelect += CEString::sToString (m_iType);
    sSelect += L";";

    try
    {
        m_pDb->PrepareForSelect (sSelect);
        while (m_pDb->bGetRow())
        {
            CEString sEnding;
            StEndingDescriptor stD;
            m_pDb->GetData (0, sEnding);
            m_pDb->GetData (1, (int&)stD.eGender);
            m_pDb->GetData (2, (int&)stD.eNumber);
            m_pDb->GetData (3, (int&)stD.eStress);
            HRESULT hr = m_pEndings->hAddEnding (sEnding, stD);
        }
        m_pDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();    
        throw CException (E_FAIL, L"");
    }
    
    return S_OK;
        
}   //  h_GetShortFormEndings()

HRESULT CFormBuilderShortAdj::hGetStressTypes (ET_Number eNumber, 
                                               ET_Gender eGender, 
                                               vector<ET_StressLocation>& vecStressType)
{
    HRESULT hr = S_OK;

    if (NUM_PL == eNumber && GENDER_UNDEFINED != eGender)
    {
        ATLASSERT(0);
        CEString sMsg (L"Unexpected gender/number values.");
        ERROR_LOG (sMsg);
        throw CException (E_INVALIDARG, sMsg);
    }

    if (NUM_SG == eNumber && GENDER_UNDEFINED == eGender)
    {
        ATLASSERT(0);
        CEString sMsg (L"Unexpected gender/number values.");
        ERROR_LOG (sMsg);
        throw CException (E_INVALIDARG, sMsg);
    }

    if (GENDER_M == eGender)
    {
        ATLASSERT (NUM_SG == eNumber);
        vecStressType.push_back (STRESS_LOCATION_STEM);
        return S_OK;
    }

    //
    // Exception: part past passive short ending in stressed -Annyj/-jAnnyj;
    // see GDRL p. 86 footnote 4
    //
    if (SUBPARADIGM_PART_PAST_PASS_SHORT == m_eSubparadigm)
    {
        CGramHasher hasher (POS_VERB, SUBPARADIGM_PART_PAST_PASS_LONG, CASE_NOM, NUM_SG, 
                            GENDER_M, PERSON_UNDEFINED, ANIM_NO, m_pLexeme->m_eAspect, 
                            m_pLexeme->m_eReflexive);
        CWordForm * pNSgMLong = NULL;
        hWordFormFromHash (hasher.iGramHash(), 0, pNSgMLong);
        if (NULL == pNSgMLong)
        {
            ATLASSERT(0);
            CEString sMsg (L"Failed to obtain N Sg m of the long form.");
            ERROR_LOG (sMsg);
            throw CException (E_POINTER, sMsg);
        }
        CEString sNSgMLong (pNSgMLong->m_sWordForm);
        if (sNSgMLong.bEndsWith (L"анный") || sNSgMLong.bEndsWith (L"янный") ||
            sNSgMLong.bEndsWith (L"енный"))
        {
            map<int, ET_StressType>::iterator itStress = pNSgMLong->m_mapStress.begin();
            for (; itStress != pNSgMLong->m_mapStress.end(); ++itStress)
            {
                if (sNSgMLong.uiNSyllables()-2 == (*itStress).first 
                    && STRESS_PRIMARY == (*itStress).second)
                {
// 1. -at'/-jat' [7] -- 1a, p. 83
// 2. monosyll verbs -- same as past: zvannyj (113) and dannyj 117
    // &&&& TODO
                }
            }
        }
    }

    ET_AccentType eAt = AT_UNDEFINED;
    if (AT_UNDEFINED == m_eAccentType2)
    {
        eAt = m_eAccentType1;
    }
    else
    {
        eAt = m_eAccentType2;
    }

    switch (eAt)
    {
        case AT_UNDEFINED:
        {
            ATLASSERT(0);
            CEString sMsg (L"Undefined accent type.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }
        case AT_A:
        {
            vecStressType.push_back (STRESS_LOCATION_STEM);

            // Type sorvana: GDRL, p. 86
            if (SUBPARADIGM_PART_PAST_PASS_SHORT == m_eSubparadigm && 
                GENDER_F == eGender && 
                AT_C == m_pLexeme->m_eAccentType2)
            {
                vecStressType.push_back (STRESS_LOCATION_ENDING);
            }

            return S_OK;
        }
        case AT_A1:
        {
            if (GENDER_F == eGender)
            {
                ATLASSERT (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            if (GENDER_N == eGender)
            {
                ATLASSERT (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                return S_OK;
            }
            if (NUM_PL == eNumber)
            {
                vecStressType.push_back (STRESS_LOCATION_STEM);
                return S_OK;
            }

            ATLASSERT(0);
            CEString sMsg (L"Bad arguments.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }
        case AT_B:
        {
            vecStressType.push_back (STRESS_LOCATION_ENDING);
            return S_OK;
        }
        case AT_B1:
        {
            if (GENDER_F == eGender || GENDER_N == eGender)
            {
                ATLASSERT (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            if (NUM_PL == eNumber)
            {
                ATLASSERT (GENDER_UNDEFINED == eGender);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }

            ATLASSERT(0);
            CEString sMsg (L"Bad arguments.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }
        case AT_C:
        {
            if (GENDER_F == eGender)
            {
                ATLASSERT (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            if (GENDER_N == eGender || NUM_PL == eNumber)
            {
                vecStressType.push_back (STRESS_LOCATION_STEM);
                return S_OK;
            }

            ATLASSERT(0);
            CEString sMsg (L"Bad arguments.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }
        case AT_C1:
        {
            if (GENDER_F == eGender)
            {
                ATLASSERT (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            if (GENDER_N == eGender)
            {
                ATLASSERT (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                return S_OK;
            }
            if (NUM_PL == eNumber)
            {
                ATLASSERT (GENDER_UNDEFINED == eGender);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }

            ATLASSERT(0);
            CEString sMsg (L"Bad arguments.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }
        case AT_C2:
        {
            if (GENDER_F == eGender)
            {
                ATLASSERT (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            if (GENDER_N == eGender)
            {
                ATLASSERT (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            if (NUM_PL == eNumber)
            {
                ATLASSERT (GENDER_UNDEFINED == eGender);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }

            ATLASSERT(0);
            CEString sMsg (L"Bad arguments.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }
        default:
        {
            ATLASSERT(0);
            CEString sMsg (L"Illegal accent type.");
            ERROR_LOG (sMsg);
            throw CException (E_INVALIDARG, sMsg);
        }
    }

    throw CException (E_INVALIDARG, L"");

}   // h_GetStressType()

HRESULT CFormBuilderShortAdj::hCreateFormTemplate (const CEString& sLemma,
                                                   const CEString& sEnding,
                                                   ET_Gender eGender,
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
    pWordForm->m_eSubparadigm = m_eSubparadigm;
    pWordForm->m_sLemma = sLemma;
    pWordForm->m_sWordForm = sLemma + sEnding;
    pWordForm->m_eGender = eGender;
    pWordForm->m_eNumber = eNumber;
    pWordForm->m_eStatus = m_eStatus;
    if (POS_VERB == m_pLexeme->m_ePartOfSpeech)
    {
        pWordForm->m_eAspect = m_pLexeme->m_eAspect;
        pWordForm->m_eReflexive = m_pLexeme->m_eReflexive;
    }

    AssignSecondaryStress (pWordForm);

    return S_OK;

}   //  hCreateFormTemplate (...)

HRESULT CFormBuilderShortAdj::hHandleDeviations (CWordForm * pWordForm)
{
    HRESULT hr = S_OK;

    //
    // A. Common deviations defined for adjectives (1-2)
    //
    bool bOptionalCD = false;
    int iCd = 1;
    for (; iCd <= 2; ++iCd)
    {
        map<int, bool>::iterator itCd = m_mapCommonDeviations.find (iCd);
        if (m_mapCommonDeviations.end() != itCd)
        {
            bOptionalCD = (*itCd).second;
            break;
        }
    }

    //
    // Only adjectives in -nnyj/-nnij or participia
    //
    if (1 == iCd || 2 == iCd)
    {
        if (1 == iCd && GENDER_M != pWordForm->m_eGender)
        {
            return S_FALSE;
        }

        if (bOptionalCD)   // store both forms
        {
            CComObject<CWordForm> * pMVariant = NULL;
            hCloneWordForm (pWordForm, pMVariant);
            m_pLexeme->hAddWordForm (pMVariant);
            pWordForm = pMVariant;
        }

        if (GENDER_M == pWordForm->m_eGender)
        {
            pWordForm->m_sWordForm = pWordForm->m_sWordForm.sErase (pWordForm->m_sWordForm.uiLength()-1);
            if (m_bFleetingVowel)
            {
                pWordForm->m_sWordForm = pWordForm->m_sWordForm.sErase (pWordForm->m_sWordForm.uiLength()-1);
            }
        }
        else
        {
            pWordForm->m_sWordForm = pWordForm->m_sWordForm.sErase (pWordForm->m_sWordForm.uiLength()-2, 1);
        }
    }   //  if (1 == iCd || 2 == iCd)

    //
    // Common deviations devined for verbs (7-8)
    //
    if (bHasCommonDeviation (7))
    {
        iCd = 7;
    }
    else if (bHasCommonDeviation (8))  // TODO -- what's that??
    {
        iCd = -1;
    }
    else
    {
        iCd = -1;
    }

    if (iCd > 0 && SUBPARADIGM_PART_PAST_PASS_SHORT == m_eSubparadigm)
    {
        if (bDeviationOptional (iCd))   // store both forms
        {
            CComObject<CWordForm> * pMVariant = NULL;
            hCloneWordForm (pWordForm, pMVariant);
            m_pLexeme->hAddWordForm (pMVariant);
            pWordForm = pMVariant;
        }

        map<int, ET_StressType> mapCorrectedStress;
        map<int, ET_StressType>::iterator itStressPos = pWordForm->m_mapStress.begin();
        for (; itStressPos != pWordForm->m_mapStress.end(); ++itStressPos)
        {
            if (!(*itStressPos).second)
            {
                mapCorrectedStress[(*itStressPos).first] = STRESS_SECONDARY;
                continue;
            }
            if ((*itStressPos).first < 1)
            {
                ATLASSERT(0);
                CEString sMsg (L"Unexpected stress position in cd-7 or cd-8 participle.");
                ERROR_LOG (sMsg);
                throw CException (E_FAIL, sMsg);
            }
            CEString sWf (pWordForm->m_sWordForm);
            mapCorrectedStress[sWf.uiNSyllables()-1] = STRESS_PRIMARY;
        }
    }

    return S_OK;

}   //  hHandleDeviations (...)

HRESULT CFormBuilderShortAdj::hCheckIrregularForms (ET_Gender eGender, ET_Number eNumber, bool& bHandled)
{
    HRESULT hr = S_OK;

    bHandled = false;

    CGramHasher hash (SUBPARADIGM_SHORT_ADJ, eNumber, eGender, ANIM_UNDEFINED, CASE_UNDEFINED);
//    multimap<int, StIrregularForm>& mmapIf = m_pLexeme->mmap_IrregularForms;
    StIrregularForm stIf;
    hr = m_pLexeme->hGetFirstIrregularForm (hash.iGramHash(), stIf);
    while (S_OK == hr)
    {
        if (stIf.bIsVariant)
        {
            bHandled = false;
        }
        else
        {
            bHandled = true;
        }

        CComObject<CWordForm> * pWordForm = NULL;
        hCreateFormTemplate (L"", L"", eGender, eNumber, pWordForm);
        pWordForm->m_sWordForm = stIf.sForm;
        pWordForm->m_mapStress = stIf.mapStress;
        m_pLexeme->hAddWordForm (pWordForm);
        hr = m_pLexeme->hGetNextIrregularForm (hash.iGramHash(), stIf);
    }

    return hr;

}   //  hCheckIrregularForms (...)

HRESULT CFormBuilderShortAdj::hBuild()
{
    HRESULT hr = S_OK;

    try
    {
        if (m_pLexeme->m_ePartOfSpeech != POS_ADJ)
        {
            return S_OK;     // Only adjectives can have short forms
        }

        hGetEndings();

        for (ET_Gender eGender = GENDER_UNDEFINED; eGender <= GENDER_N; ++eGender)
        {
            ET_Number eNumber = (GENDER_UNDEFINED == eGender) ? NUM_PL : NUM_SG;

            bool bSkipRegular = false;
            hCheckIrregularForms (eGender, eNumber, bSkipRegular);
            if (bSkipRegular)
            {
                continue;
            }

            vector<ET_StressLocation> vecStressType;
            hGetStressTypes (eNumber, eGender, vecStressType);

            vector<ET_StressLocation>::iterator itStressType = vecStressType.begin();
            for (; itStressType != vecStressType.end(); ++itStressType)
            {
                if (m_pLexeme->m_bHasIrregularForms)
                {
                    bool bSkipRegular = false;
                    hCheckIrregularForms (eGender, eNumber, bSkipRegular);
                    if (bSkipRegular)
                    {
                        continue;
                    }
                }
                
                StEndingDescriptor stD (eGender, eNumber, *itStressType);
                int iNumEndings = m_pEndings->iCount (stD);
                if (iNumEndings < 1)
                {
                    ERROR_LOG (L"No endings");
                    continue;
                }

                for (int iEnding = 0; iEnding < iNumEndings; ++iEnding)
                {
                    CEString sEnding;
                    m_pEndings->hGetEnding (stD, iEnding, sEnding);
                    hFleetingVowelCheck (eNumber,
                                         CASE_UNDEFINED, 
                                         eGender, 
                                         *itStressType, 
                                         m_eSubparadigm,
                                         sEnding, 
                                         m_sLemma);

                    vector<int> vecStressPos;
                    if (STRESS_LOCATION_STEM == *itStressType)
                    {
                        if (SUBPARADIGM_SHORT_ADJ == m_eSubparadigm)
                        {
                            hGetStemStressPositions (m_sLemma, SUBPARADIGM_SHORT_ADJ, vecStressPos);
                        }
                        else
                        {
                            vecStressPos = m_vecStressPos; // assigned in ctor
                        }
                    }
                    else if (STRESS_LOCATION_ENDING == *itStressType)
                    {
                        int iStressPos = -1;
                        hGetEndingStressPosition (m_sLemma, sEnding, iStressPos);
                        vecStressPos.push_back (iStressPos);
                    }
                    else
                    {
                        ATLASSERT(0);
                        CEString sMsg (L"Illegal stress type.");
                        ERROR_LOG (L"Illegal stress type.");
                        return E_FAIL;
                    }

                    CComObject<CWordForm> * pWordForm = NULL;
                    hCreateFormTemplate (m_sLemma, sEnding, eGender, eNumber, pWordForm);

//                    long iGramHash = pWordForm->i_GramHash();
                    if (1 == vecStressPos.size() || bMultiStress (m_sLemma, vecStressPos))
                    {
                        vector<int>::iterator itStressPos = vecStressPos.begin();
                        for (; itStressPos != vecStressPos.end(); ++itStressPos)
                        {
                            pWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                            hHandleYoAlternation (*itStressType, *itStressPos, m_sLemma, sEnding);
                        }
                        pWordForm->m_sWordForm = m_sLemma + sEnding;
                        hHandleDeviations (pWordForm);
                        m_pLexeme->hAddWordForm (pWordForm);
                    }
                    else
                    {
                        vector<int>::iterator itStressPos = vecStressPos.begin();
                        for (; itStressPos != vecStressPos.end(); ++itStressPos)
                        {
                            if (itStressPos != vecStressPos.begin())
                            {
                                CComObject<CWordForm> * pWfVariant = NULL;
                                hCloneWordForm (pWordForm, pWfVariant);
                                pWordForm = pWfVariant;
                            }
                            pWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                            hHandleYoAlternation (*itStressType, *itStressPos, m_sLemma, sEnding);
                            pWordForm->m_sWordForm = m_sLemma + sEnding;
                            hHandleDeviations (pWordForm);
                            m_pLexeme->hAddWordForm (pWordForm);
                        }
                    }

                }   //  for (int iEnding = 0; iEnding < iNumEndings; ++iEnding)

            }   //  for (; itStress != vec_eoStress.end(); ++itStress)

        }   //  for (ET_Gender eGender = GENDER_UNDEFINED; ...
    }
    catch (CException ex)
    {
        return ex.iGetErrorCode();  // logging should be always done by callee
    }

    return S_OK;

}   //  hBuild()
