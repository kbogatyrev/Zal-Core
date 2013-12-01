#include "StdAfx.h"
#include "Endings.h"
#include "FormBuilderAdjLong.h"

int CFormBuilderLongAdj::iGramHash (ET_PartOfSpeech ePos, ET_Subparadigm eSubparadigm,
                                    ET_Case eCase, ET_Number eNumber, ET_Gender eGender, 
                                    ET_Animacy eAnimacy, ET_Reflexive eRefl)
{
    CGramHasher hasher (ePos, eSubparadigm, eCase, eNumber, eGender, PERSON_UNDEFINED, eAnimacy, 
                        ASPECT_UNDEFINED, eRefl); 

    int iHash = hasher.iGramHash();

    return iHash;
}

int CFormBuilderLongAdj::iGramHashNSgMLong()
{
    CGramHasher hasher (POS_ADJ, SUBPARADIGM_LONG_ADJ, CASE_UNDEFINED, NUM_UNDEFINED, GENDER_UNDEFINED, PERSON_UNDEFINED,                             
                        ANIM_NO, ASPECT_UNDEFINED, REFL_UNDEFINED);

    int iHash = hasher.iGramHash();

    return iHash;
}

ET_ReturnCode CFormBuilderLongAdj::eGetStressPositions (const CEString& sEnding,
                                                        ET_StressLocation eStressType,
                                                        vector<int>& vecStressPos)
{
    ET_ReturnCode rc = H_NO_ERROR;

    m_sLemma.SetVowels (g_szVowels);

    int iStressPos = -1;

    if (STRESS_LOCATION_STEM == eStressType)
    {
        rc = eGetStemStressPositions (m_sLemma, m_eSubparadigm, vecStressPos);
    }
    else if (STRESS_LOCATION_ENDING == eStressType)
    {
        if (sEnding.uiNSyllables() < 1)
        {
            iStressPos = m_sLemma.uiNSyllables() - 1;
        }
        else
        {
            iStressPos = m_sLemma.uiNSyllables();
        }

        vecStressPos.push_back (iStressPos);
    }
    else
    {
        ATLASSERT (0);
        ERROR_LOG (L"Illegal stress type.");
        return H_ERROR_INVALID_ARG;
    }

    return rc;

}   //  eGetStressPositions (...)

ET_ReturnCode CFormBuilderLongAdj::eCreateFormTemplate (ET_Gender eGender,
                                                        ET_Number eNumber, 
                                                        ET_Case eCase, 
                                                        ET_Animacy eAnimacy,
                                                        const CEString& sEnding,
                                                        CWordForm *& pWordForm)
{
    if (m_sLemma.bIsEmpty())
    {
        ERROR_LOG (L"No stem"); 
        return H_ERROR_UNEXPECTED;
    }

    pWordForm = new CWordForm;
    if (NULL == pWordForm)
    {
        ATLASSERT(0);
        ERROR_LOG (L"Error creating CWordForm object");
        return H_ERROR_POINTER;
    }

    pWordForm->m_pLexeme = m_pLexeme;
    pWordForm->m_ePos = m_pLexeme->ePartOfSpeech();
    pWordForm->m_eSubparadigm = m_eSubparadigm;
    pWordForm->m_sLemma = m_sLemma;
    pWordForm->m_eGender = eGender;
    pWordForm->m_eCase = eCase;
    pWordForm->m_eNumber = eNumber;
    pWordForm->m_eAnimacy = eAnimacy;
    pWordForm->m_eReflexive = m_pLexeme->eIsReflexive();
    pWordForm->m_sWordForm = m_sLemma + sEnding;
    if (REFL_YES == m_pLexeme->eIsReflexive())
    {
        pWordForm->m_sWordForm += L"ся";
    }
    pWordForm->m_eStatus = m_eStatus;

    return H_NO_ERROR;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderLongAdj::eHandleCommonDeviations (CWordForm * pWordForm)
{
    ET_ReturnCode rc = H_NO_ERROR;

    ATLASSERT(m_pLexeme);   // we assume base clas ctor took care of this

    try
    {
        if (m_pLexeme->bHasCommonDeviation(4) && SUBPARADIGM_PART_PRES_ACT == m_eSubparadigm)
        {
            if (m_pLexeme->bDeviationOptional(4))
            {
                CWordForm * pVariant = NULL;
                CloneWordForm (pWordForm, pVariant);
                m_pLexeme->AddWordForm (pVariant);    // store both versions
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
                    ERROR_LOG (L"Unexpected stress position in cd-4 participle.");
                    return H_ERROR_UNEXPECTED;
                }
                mapCorrectedStress[(*itStressPos).first-1] = STRESS_PRIMARY;
            }
            pWordForm->m_mapStress = mapCorrectedStress;        

        }   //  if (bHasCommonDeviation (4) ...

        if (m_pLexeme->bHasCommonDeviation(6) && 
            SUBPARADIGM_PART_PAST_ACT == m_eSubparadigm &&
            3 == m_pLexeme->iType() &&
            1 == m_pLexeme->iStemAugment())
        {
            if (m_pLexeme->bDeviationOptional(6))
            {
                CWordForm * pVariant = NULL;
                CloneWordForm (pWordForm, pVariant);
                m_pLexeme->AddWordForm (pVariant);    // store both versions
                pWordForm = pVariant;
            }

            CEString sEnding = pWordForm->m_sWordForm.sSubstr (pWordForm->m_sLemma.uiLength());
            CEString sLemma = pWordForm->m_sLemma;
            int iLength = -1;
            if (sLemma.bEndsWith (L"вш"))
            {
                iLength = sLemma.uiLength()-2;
            }
            else
            {
                iLength = sLemma.uiLength()-1;
            }

            sLemma = sLemma.sSubstr (0, iLength);

            if (sLemma.bEndsWith (L"ну"))
            {
                if (m_pLexeme->bDeviationOptional(6))
                {
                    // For optional (6) we store both variants
                    sLemma = sLemma.sSubstr (0, iLength-2);
                    sLemma += sLemma.bEndsWithOneOf (g_szConsonants) ? L"ш" : L"вш";
                }
                else
                {
                    // Stem already has -nu-; nothing to do (note that we cannot
                    // predict which form was used for derivation)
                    return H_NO_MORE;
                }
            }
            else
            {
                sLemma += L"нувш";
            }
            pWordForm->m_sWordForm = sLemma + sEnding;

        }   //  if (bHasCommonDeviation (6) ... )

        int iCd = -1;
        if (m_pLexeme->bHasCommonDeviation(7))
        {
            iCd = 7;
        }
        else if (m_pLexeme->bHasCommonDeviation(8))
        {
            iCd = 8;
        }

        if (iCd > 0 && SUBPARADIGM_PART_PAST_PASS_LONG == m_eSubparadigm)
        {
            CEString sLemma (pWordForm->m_sLemma);
            int iNSyll = sLemma.uiNSyllables();
            if (iNSyll < 1)        //  < 2??
            {
                ATLASSERT(0);
                CEString sMsg (L"No syllables in part pass past stem.");
                ERROR_LOG (sMsg);
                return H_ERROR_UNEXPECTED;
            }

            if (m_pLexeme->bDeviationOptional(iCd))
            {
                CWordForm * pVariant = NULL;
                CloneWordForm (pWordForm, pVariant);
                m_pLexeme->AddWordForm (pVariant);    // store both versions
                pWordForm = pVariant;
            }

            map<int, ET_StressType> mapCorrectedStress;
            map<int, ET_StressType>::iterator itStressPos = pWordForm->m_mapStress.begin();
            int iNewStressPos = -1;
            for (; itStressPos != pWordForm->m_mapStress.end(); ++itStressPos)
            {
                if (STRESS_SECONDARY == (*itStressPos).second)
                {
                    mapCorrectedStress[(*itStressPos).first] = STRESS_SECONDARY;
                    continue;
                }
                if (8 == iCd && (*itStressPos).first < 1)
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Unexpected stress position in cd-7 or cd-8 participle.");
                    return H_ERROR_UNEXPECTED;
                }

                if (7 == iCd)
                {
                    iNewStressPos = (*itStressPos).first + 1;
                }
                else
                {
                    if (8 == iCd)
                    {
                        iNewStressPos = pWordForm->m_sLemma.uiNSyllables() - 2;
                    }
                    else
                    {
                        ATLASSERT(0);
                        ERROR_LOG (L"Unexpected cd number.");
                        return H_ERROR_UNEXPECTED;
                    }
                }

                mapCorrectedStress[iNewStressPos] = STRESS_PRIMARY;

                // Remove yo at old stress pos
                int iAt = sLemma.uiGetVowelPos ((*itStressPos).first);
                if (L'ё' == pWordForm->m_sWordForm[iAt])
                {
                    pWordForm->m_sLemma[iAt] = pWordForm->m_sWordForm[iAt] = L'е';
                }

                // E --> yo at new stress pos
                iAt = sLemma.uiGetVowelPos (iNewStressPos);
                if (L'е' == pWordForm->m_sWordForm[iAt])
                {
                    pWordForm->m_sLemma[iAt] = pWordForm->m_sWordForm[iAt] = L'ё';
                }
            }
            pWordForm->m_mapStress = mapCorrectedStress;
        }
    }
    catch (CException& ex)
    {
        CEString sMsg (L"Exception: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG (sMsg);
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eHandleCommonDeviations (...)

ET_ReturnCode CFormBuilderLongAdj::eBuild()
{
    ET_ReturnCode rc = H_NO_ERROR;

    ATLASSERT(m_pLexeme);   // we assume base clas ctor took care of this

    if (m_pLexeme->bNoLongForms())
    {
        return H_NO_MORE;
    }

    try
    {
        m_pEndings = new CAdjLongEndings(m_pLexeme, m_eSubparadigm);
        if (NULL == m_pEndings)
        {
            return H_ERROR_POINTER;
        }

        rc = m_pEndings->eLoad();
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        CHasher gram_tmp;
        gram_tmp.Initialize(POS_ADJ);
        gram_tmp.SetParadigm(SUBPARADIGM_LONG_ADJ);
        do
        {
            ET_StressLocation eStressType = STRESS_LOCATION_UNDEFINED;
            if (AT_A == m_eAccentType|| AT_A1 == m_eAccentType)
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else
            {
                if (AT_B == m_eAccentType)
                {
                    eStressType = STRESS_LOCATION_ENDING;
                }
                else
                {
                    ERROR_LOG (L"Unknown stress type.");
                    continue;
                }
            }

            StEndingDescriptor stD (gram_tmp.m_eGender,
                                    gram_tmp.m_eNumber,
                                    gram_tmp.m_eCase,
                                    gram_tmp.m_eAnimacy,
                                    eStressType);
            int iNumEndings = m_pEndings->iCount (stD);
            if (iNumEndings < 1)
            {
                ERROR_LOG (L"No endings");
                continue;
            }

            for (int iEnding = 0; iEnding < iNumEndings; ++iEnding)
            {
                CEString sEnding;
                m_pEndings->eGetEnding (stD, iEnding, sEnding);

                if (L"мс" == m_pLexeme->sInflectionType() && m_pLexeme->bHasFleetingVowel())
                {
                    rc = eFleetingVowelCheck (gram_tmp.m_eNumber, gram_tmp.m_eCase,
                                              gram_tmp.m_eGender, eStressType,
                                              SUBPARADIGM_LONG_ADJ, sEnding, m_sLemma);
                    if (rc != H_NO_ERROR)
                    {
                        return rc;
                    }
                }

                CWordForm * pWordForm = NULL;
                rc = eCreateFormTemplate (gram_tmp.m_eGender, 
                                          gram_tmp.m_eNumber, 
                                          gram_tmp.m_eCase, 
                                          gram_tmp.m_eAnimacy, 
                                          sEnding, 
                                          pWordForm);
//                            pWordForm->str_WordForm = str_Lemma + sEnding;
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                vector<int> vecStress;
                eGetStressPositions (sEnding, eStressType, vecStress);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                if (1 == vecStress.size() || m_pLexeme->bIsMultistressedCompound())
                {
                    vector<int>::iterator itStressPos = vecStress.begin();
                    for (; itStressPos != vecStress.end(); ++itStressPos)
                    {
                        pWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                    }
                    m_pLexeme->AddWordForm (pWordForm);
                }
                else
                {
                    vector<int>::iterator itStressPos = vecStress.begin();
                    for (; itStressPos != vecStress.end(); ++itStressPos)
                    {
                        if (itStressPos != vecStress.begin())
                        {
                            CWordForm * pWfVariant = NULL;
                            CloneWordForm (pWordForm, pWfVariant);
                            pWordForm = pWfVariant;
                        }
                        pWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                        m_pLexeme->AddWordForm (pWordForm);
                    }
                }

            }   //  for (int iEnding = 0; iEnding < iNumEndings; ++iEnding)
        } while (gram_tmp.bIncrement());
    }
    catch (CException ex)
    {
        return H_EXCEPTION;  // logging should be always done by callee
    }

    return H_NO_ERROR;

}   //  eBuild (...)

ET_ReturnCode CFormBuilderLongAdj::eBuildParticiple()
{
    ET_ReturnCode rc = H_NO_ERROR;

    ATLASSERT(m_pLexeme);   // we assume base clas ctor took care of this

    if (-1 == m_iStemStressPos)
    {
        ATLASSERT(0);
        ERROR_LOG (L"No stress position for a participle.");
        return H_ERROR_GENERAL;
    }

    try
    {
        m_pEndings = new CAdjLongEndings(m_pLexeme, m_eSubparadigm);
        if (NULL == m_pEndings)
        {
            return H_ERROR_POINTER;
        }

        rc = m_pEndings->eLoad();
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        CHasher gram_tmp;
        gram_tmp.Initialize(POS_VERB);
        gram_tmp.m_eAspect = m_pLexeme->eAspect();
        gram_tmp.SetParadigm (m_eSubparadigm);
        do
        {
            StEndingDescriptor stD (gram_tmp.m_eGender,
                                    gram_tmp.m_eNumber,
                                    gram_tmp.m_eCase,
                                    gram_tmp.m_eAnimacy,
                                    STRESS_LOCATION_STEM);
            int iNumEndings = m_pEndings->iCount (stD);
            if (iNumEndings < 1)
            {
                ERROR_LOG (L"No endings");
                continue;
            }

            for (int iEnding = 0; iEnding < iNumEndings; ++iEnding)
            {
                CEString sEnding;
                rc = m_pEndings->eGetEnding (stD, iEnding, sEnding);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                if (!m_bIrregular)
                {
                    rc = eHandleYoAlternation (STRESS_LOCATION_STEM, m_iStemStressPos, m_sLemma, sEnding);
                    if (rc != H_NO_ERROR)
                    {
                        return rc;
                    }
                }

//                CWordForm * pStressTemplate = NULL;
                CWordForm * pWordForm = NULL;
                rc = eCreateFormTemplate (gram_tmp.m_eGender, 
                                          gram_tmp.m_eNumber, 
                                          gram_tmp.m_eCase, 
                                          gram_tmp.m_eAnimacy, 
                                          sEnding, 
                                          pWordForm);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                pWordForm->m_eAspect = gram_tmp.m_eAspect;
                pWordForm->m_mapStress[m_iStemStressPos] = STRESS_PRIMARY;
                if (!m_bIrregular)
                {
                    rc = eHandleCommonDeviations (pWordForm);
                    if (rc != H_NO_ERROR)
                    {
                        return rc;
                    }
                }
                m_pLexeme->AddWordForm (pWordForm);

            }   //  for (int iEnding = 0; iEnding < iNumEndings; ++iEnding)

        } while (gram_tmp.bIncrement());
    }
    catch (CException ex)
    {
        return H_EXCEPTION;  // logging should be always done by callee
    }

    return H_NO_ERROR;

}   //  eBuildParticiple (...)
