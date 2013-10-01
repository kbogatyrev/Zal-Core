#include "StdAfx.h"
#include "Endings.h"
#include "FormBuilderAdjShort.h"

void CFormBuilderShortAdj::GetEndings()
{
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
        
}   //  GetShortFormEndings()

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
        m_pLexeme->WordFormFromHash (hasher.iGramHash(), 0, pNSgMLong);
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

void CFormBuilderShortAdj::CreateFormTemplate (const CEString& sLemma,
                                               const CEString& sEnding,
                                               ET_Gender eGender,
                                               ET_Number eNumber,
                                               CWordForm *& pWordForm)
{
    HRESULT hr = S_OK;

    pWordForm = new CWordForm;
    if (NULL == pWordForm)
    {
        ATLASSERT(0);
        CString csMsg;
        csMsg.Format (L"Unable to create an instance of CWordForm");
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
            CWordForm * pMVariant = NULL;
            CloneWordForm (pWordForm, pMVariant);
            m_pLexeme->AddWordForm (pMVariant);
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
    if (m_pLexeme->bHasCommonDeviation(7))
    {
        iCd = 7;
    }
    else if (m_pLexeme->bHasCommonDeviation(8))  // TODO -- what's that??
    {
        iCd = -1;
    }
    else
    {
        iCd = -1;
    }

    if (iCd > 0 && SUBPARADIGM_PART_PAST_PASS_SHORT == m_eSubparadigm)
    {
        if (m_pLexeme->bDeviationOptional(iCd))   // store both forms
        {
            CWordForm * pMVariant = NULL;
            CloneWordForm (pWordForm, pMVariant);
            m_pLexeme->AddWordForm (pMVariant);
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

HRESULT CFormBuilderShortAdj::hBuild()
{
    HRESULT hr = S_OK;

    try
    {
        if (m_pLexeme->m_ePartOfSpeech != POS_ADJ)
        {
            return S_OK;     // Only adjectives can have short forms
        }

        GetEndings();

        for (ET_Gender eGender = GENDER_UNDEFINED; eGender <= GENDER_N; ++eGender)
        {
            ET_Number eNumber = (GENDER_UNDEFINED == eGender) ? NUM_PL : NUM_SG;           
            CGramHasher hash (SUBPARADIGM_SHORT_ADJ, eNumber, eGender, ANIM_UNDEFINED, CASE_UNDEFINED);
            if (bNoRegularForms(hash.iGramHash()))
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
                    hash.m_eGender = eGender;
                    hash.m_eNumber = eNumber;
                    if (bNoRegularForms(hash.iGramHash()))
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
                            GetStemStressPositions (m_sLemma, SUBPARADIGM_SHORT_ADJ, vecStressPos);
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

                    CWordForm * pWordForm = NULL;
                    CreateFormTemplate (m_sLemma, sEnding, eGender, eNumber, pWordForm);

//                    long iGramHash = pWordForm->i_GramHash();
                    if (1 == vecStressPos.size() || bMultiStress (m_sLemma, vecStressPos))
                    {
                        vector<int>::iterator itStressPos = vecStressPos.begin();
                        for (; itStressPos != vecStressPos.end(); ++itStressPos)
                        {
                            pWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                            HandleYoAlternation (*itStressType, *itStressPos, m_sLemma, sEnding);
                        }
                        pWordForm->m_sWordForm = m_sLemma + sEnding;
                        hHandleDeviations (pWordForm);
                        m_pLexeme->AddWordForm (pWordForm);
                    }
                    else
                    {
                        vector<int>::iterator itStressPos = vecStressPos.begin();
                        for (; itStressPos != vecStressPos.end(); ++itStressPos)
                        {
                            if (itStressPos != vecStressPos.begin())
                            {
                                CWordForm * pWfVariant = NULL;
                                CloneWordForm (pWordForm, pWfVariant);
                                pWordForm = pWfVariant;
                            }
                            pWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                            HandleYoAlternation (*itStressType, *itStressPos, m_sLemma, sEnding);
                            pWordForm->m_sWordForm = m_sLemma + sEnding;
                            hHandleDeviations (pWordForm);
                            m_pLexeme->AddWordForm (pWordForm);
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
