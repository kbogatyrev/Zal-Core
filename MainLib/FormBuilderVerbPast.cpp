#include "StdAfx.h"

#include "WordForm.h"
#include "Lexeme.h"

#include "FormBuilderVerbPast.h"

static wchar_t arrEndings[][4] = { L"л", L"ла", L"ло", L"ли" };

ET_ReturnCode CFormBuilderPast::eGetEnding (CEString& sLemma, 
                                            ET_Number eNumber, 
                                            ET_Gender eGender, 
                                            CEString& sEnding)
{
    ET_ReturnCode rc = H_NO_ERROR;

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

    return H_NO_ERROR;

}   //  eGetEnding (...)

ET_ReturnCode CFormBuilderPast::eGetStressPositions (const CEString& sLemma,
                                                     const CEString& sEnding,
                                                     ET_Number eNumber, 
                                                     ET_Gender eGender, 
                                                     vector<int>& vecStressPositions)
{
    ET_ReturnCode rc = H_NO_ERROR;

    ET_AccentType eAccentType = AT_UNDEFINED;
    if (AT_UNDEFINED == m_pLexeme->eAccentType2())
    {
        eAccentType = AT_A;
    }
    else
    {
        eAccentType = m_pLexeme->eAccentType2();
    }

    vector<ET_StressLocation> vecStressType;
    rc = eGetPastTenseStressTypes (eAccentType, eNumber, eGender, vecStressType);
    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    vector<ET_StressLocation>::iterator itStressType = vecStressType.begin();
    for (; itStressType != vecStressType.end(); ++itStressType)
    {
        if (STRESS_LOCATION_STEM == *itStressType)
        {
            rc = eGetStemStressPositions (const_cast<CEString&>(sLemma), vecStressPositions);
                                        // only to set vowels, no changes to the string
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
        }

        if (STRESS_LOCATION_ENDING == *itStressType)
        {
            if (AT_C2 == eAccentType && GENDER_M == eGender && NUM_SG == eNumber)
            {
                if (REFL_NO == m_pLexeme->eIsReflexive())
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Unexpected part of speech value.");
                    return H_ERROR_UNEXPECTED;
                }
                // This is or is becoming obsolete? (GDRL, p. 80-81)
                vecStressPositions.push_back (sLemma.uiNSyllables());
            }
            else
            {
                int iPos = -1;
                rc = eGetEndingStressPosition (const_cast<CEString&>(sLemma), const_cast<CEString&>(sEnding), iPos);
                                                // Need to set vowels, no changes to the string.
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                vecStressPositions.push_back (iPos);
            }
        }
    }

    return H_NO_ERROR;

}   //  eGetStressPositions (...)

ET_ReturnCode CFormBuilderPast::eCreateFormTemplate (const CEString& sLemma,
                                                     const CEString& sEnding,
                                                     ET_Number eNumber, 
                                                     ET_Gender eGender, 
                                                     CWordForm *& pWordForm)
{
    pWordForm = new CWordForm;
    if (NULL == pWordForm)
    {
        ATLASSERT(0);
        ERROR_LOG (L"CreateInstance() failed ");
        return H_ERROR_POINTER;
    }

    pWordForm->m_pLexeme = m_pLexeme;
    pWordForm->m_ePos = m_pLexeme->ePartOfSpeech();
    pWordForm->m_eSubparadigm = SUBPARADIGM_PAST_TENSE;
    pWordForm->m_eAspect = m_pLexeme->eAspect();
    pWordForm->m_eReflexive = m_pLexeme->eIsReflexive();
    pWordForm->m_sLemma = sLemma;
    pWordForm->m_sWordForm = sLemma + sEnding;
    pWordForm->m_eGender = eGender;
    pWordForm->m_eNumber = eNumber;

//    rc = eAssignSecondaryStress (pWordForm);

    return H_NO_ERROR;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderPast::eAssemble (CWordForm * pWordForm, 
                                           int iStressPos, 
                                           CEString& sLemma, 
                                           CEString& sEnding)
{
    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        pWordForm->m_mapStress[iStressPos] = STRESS_PRIMARY;
        pWordForm->m_sLemma = sLemma;
        rc = eHandleYoAlternation (iStressPos, SUBPARADIGM_PAST_TENSE, pWordForm->m_sLemma);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        pWordForm->m_sWordForm = pWordForm->m_sLemma + sEnding;
        if (m_pLexeme->bHasCommonDeviation(1) && !(NUM_SG == pWordForm->m_eNumber && GENDER_F == pWordForm->m_eGender))
        {
            rc = eRetractStressToPreverb (pWordForm, m_pLexeme->bDeviationOptional(1));
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
        }

        if (m_pLexeme->bHasCommonDeviation(5) && 3 == m_pLexeme->iType() && 1 == m_pLexeme->iStemAugment() && 
            NUM_SG == pWordForm->m_eNumber && GENDER_M == pWordForm->m_eGender)
        {
            if (m_pLexeme->bDeviationOptional(5))
            {
                CWordForm * pVariant = NULL;
                CloneWordForm (pWordForm, pVariant);
                m_pLexeme->AddWordForm (pVariant);    // store both versions
                pWordForm = pVariant;
            }

            pWordForm->m_sWordForm = sLemma + L"нул";
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

}   // eAssemble (...)

ET_ReturnCode CFormBuilderPast::eBuild()
{
    ET_ReturnCode rc = H_NO_ERROR;

    bool bIsVariant = false;

    if (bHasIrregularPast())
    {
        rc = eBuildIrregular (bIsVariant);
        if (!bIsVariant)
        {
            return rc;
        }
    }

    try
    {
//        vector<int>& vecSourceStressPos = m_pLexeme->vec_SourceStressPos;   
        CEString sLemma;
        rc = eBuildPastTenseStem (sLemma);
        if (H_NO_ERROR != rc)
        {
            return rc;
        }
        if (sLemma.bIsEmpty())
        {
            return H_ERROR_UNEXPECTED;
        }

        ET_AccentType eAt = AT_UNDEFINED;
        if (AT_UNDEFINED == m_pLexeme->eAccentType2())
        {
            eAt = AT_A;
        }
        else
        {
            eAt = m_pLexeme->eAccentType2();
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

                rc = eGetEnding (sLemma, eNumber, eGender, sEnding);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                if (REFL_YES == m_pLexeme->eIsReflexive())
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
                rc = eGetStressPositions (sLemma, sEnding, eNumber, eGender, vecStress);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                CWordForm * pWordForm = NULL;
                rc = eCreateFormTemplate (sLemma, sEnding, eNumber, eGender, pWordForm);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
                
                if (bNoRegularForms(pWordForm->iGramHash()))
                {
                    continue;
                }

                if (1 == vecStress.size() || m_pLexeme->bIsMultistressedCompound())
                {
                    vector<int>::iterator itStressPos = vecStress.begin();
                    for (; (H_NO_ERROR == rc) && (itStressPos != vecStress.end()); ++itStressPos)
                    {
                        rc = eAssemble (pWordForm, *itStressPos, sLemma, sEnding);
                    }
                    if (rc != H_NO_ERROR)
                    {
                        return rc;
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
                            pWfVariant->m_mapStress.clear();
                            pWordForm = pWfVariant;
                        }
                        rc = eAssemble (pWordForm, *itStressPos, sLemma, sEnding);
                        if (rc != H_NO_ERROR)
                        {
                            return rc;
                        }

                        m_pLexeme->AddWordForm (pWordForm);
                    }
                }
            }   //  for (ET_Gender eGender = ... )
        }   // for ...(ET_Number = ... )
    }
    catch (CException& ex)
    {
        CEString sMsg (L"Exception: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG (sMsg);
        return H_EXCEPTION;
    }

    return rc;

}   //  eBuild()

ET_ReturnCode CFormBuilderPast::eBuildIrregular (bool& bIsVariant)
{
    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
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

                CWordForm * pWordForm = NULL;
                rc = eCreateFormTemplate (L"", L"", eNumber, eGender, pWordForm);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
                if (!pWordForm)
                {
                    return H_ERROR_POINTER;
                }
                
                if (bNoRegularForms(pWordForm->iGramHash()))
                {
                    continue;
                }

            }   //  for (ET_Gender eGender ...

        }   //  for (ET_Number eNumber ...
    }
    catch (CException&)
    {
        return H_EXCEPTION;  // logging should be always done by callee
    }

    return H_NO_ERROR;

}   //  eBuildIrregular (...)

//
// The older version is no longer needed since Tim's code already does that.
//
/*
//
// Needed when for abbreviated subparadigm rendering (GDRL 15, 16, 35, 89)
//
ET_ReturnCode CFormBuilderPast::h_BuildIrregular()
{
    ET_ReturnCode hr = H_NO_ERROR;

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
                                   m_pLexeme->eIsReflexive());

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
                    return H_ERROR_UNEXPECTED;
                }

                if ((*it_if).second.str_Form.empty())
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Empty irregular form.");
                    return H_ERROR_UNEXPECTED;
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
                        return H_ERROR_UNEXPECTED;
                    }
                    
                    m_pLexeme->h_AddIrregularForm (co_hash.i_GramHash(), (*it_if).second);

                    continue;
                }

                // Construct per GDRL instructions if not found in the db
                StIrregularForm st_template;
                hr = h_GetPastFIrregular (st_template);
                if (H_NO_ERROR != hr)
                {
                    return hr;
                }

                if (st_template.str_Form.empty())
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Empty irregular f. sg. past.");
                    return H_ERROR_UNEXPECTED;
                }

                if (st_template.str_Form.length() < 2)
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"f. sg. past too short.");
                    return H_ERROR_UNEXPECTED;
                }
                if (st_template.str_Form[st_template.str_Form.length()-1] != L'а')
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Unexpected ending in irregular f. sg. past.");
                    return H_ERROR_UNEXPECTED;
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
                    return H_ERROR_UNEXPECTED;
                }

                m_pLexeme->h_AddIrregularForm (co_hash.i_GramHash(), st_if);

            }       //  if (GENDER_N == eGender || NUM_PL == eNumber)

        }   //  for (ET_Gender eGender ...
    }   //  for (ET_Number eNumber ...

    return H_NO_ERROR;

}   //  h_BuildIrregular()
*/

ET_ReturnCode CFormBuilderPast::eRetractStressToPreverb (CWordForm * pWordForm, bool bIsOptional)
{
    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        if (bIsOptional)    // store 1st variant if retraction is optional
        {
            CWordForm * pClone = NULL;
            CloneWordForm (pWordForm, pClone);
            m_pLexeme->AddWordForm (pClone);    // store both versions
            pWordForm = pClone;
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
                ERROR_LOG (L"Unexpected stress position in cd-1 verb.");
                return H_ERROR_UNEXPECTED;
            }

            if (pWordForm->m_sWordForm.bStartsWith (L"пере"))
            {
                if ((*itStressPos).first < 2)
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Unexpected stress position in cd-1 verb.");
                    return H_ERROR_UNEXPECTED;
                }
                mapCorrectedStress[(*itStressPos).first-2] = STRESS_PRIMARY;
            }
            else
            {
                mapCorrectedStress[(*itStressPos).first-1] = STRESS_PRIMARY;
            }
        }
        pWordForm->m_mapStress = mapCorrectedStress;
    }
    catch (CException& ex)
    {
        CEString sMsg (L"Exception: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG (sMsg);
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

 }   //  eRetractStressToPreverb (...)
