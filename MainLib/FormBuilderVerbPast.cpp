#include "StdAfx.h"

#include "WordForm.h"
#include "Lexeme.h"
#include "Endings.h"
#include "FormBuilderVerbPast.h"

using namespace Hlib;

ET_ReturnCode CFormBuilderPast::eGetStressPositions (const CEString& sStem,
                                                     const CEString& sEnding,
                                                     ET_Number eNumber, 
                                                     ET_Gender eGender, 
                                                     vector<int>& vecStressPositions)
{
    ASSERT(m_pLexeme);   // we assume base class ctor took care of this

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
            rc = m_pLexeme->eGetStemStressPositions (const_cast<CEString&>(sStem), vecStressPositions);
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
                    ASSERT(0);
                    ERROR_LOG (L"Unexpected part of speech value.");
                    return H_ERROR_UNEXPECTED;
                }
                // This is or is becoming obsolete? (GDRL, p. 80-81)
                vecStressPositions.push_back (sStem.uiNSyllables());
            }
            else
            {
                int iPos = -1;
                rc = eGetEndingStressPosition (const_cast<CEString&>(sStem), const_cast<CEString&>(sEnding), iPos);
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

ET_ReturnCode CFormBuilderPast::eCreateFormTemplate (const CEString& sStem,
                                                     const CEString& sEnding,
                                                     ET_Number eNumber, 
                                                     ET_Gender eGender, 
                                                     __int64 llEndingKey,
                                                     CWordForm *& pWordForm)
{
    pWordForm = new CWordForm;
    if (NULL == pWordForm)
    {
        ASSERT(0);
        ERROR_LOG (L"CreateInstance() failed ");
        return H_ERROR_POINTER;
    }

    pWordForm->m_pLexeme = m_pLexeme;
    pWordForm->m_ePos = m_pLexeme->ePartOfSpeech();
    pWordForm->m_eSubparadigm = SUBPARADIGM_PAST_TENSE;
    pWordForm->m_eAspect = m_pLexeme->eAspect();
    pWordForm->m_eReflexive = m_pLexeme->eIsReflexive();
    pWordForm->m_sStem = sStem;
    pWordForm->m_sWordForm = sStem + sEnding;
    pWordForm->m_eGender = eGender;
    pWordForm->m_eNumber = eNumber;
    pWordForm->m_llEndingDataId = llEndingKey;
    pWordForm->m_llLexemeId = m_pLexeme->llLexemeId();

//    rc = eAssignSecondaryStress (pWordForm);

    return H_NO_ERROR;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderPast::eAssemble (CWordForm * pWordForm, 
                                           int iStressPos, 
                                           CEString& sStem,
                                           CEString& sEnding)
{
    ASSERT(m_pLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        pWordForm->m_mapStress[iStressPos] = STRESS_PRIMARY;
        pWordForm->m_sStem = sStem;
        rc = eHandleYoAlternation (iStressPos, SUBPARADIGM_PAST_TENSE, pWordForm->m_sStem);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        pWordForm->m_sWordForm = pWordForm->m_sStem + sEnding;
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

            pWordForm->m_sWordForm = sStem + L"нул";
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
    ASSERT(m_pLexeme);   // we assume base class ctor took care of this

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
        CEString sStem;
        rc = eBuildPastTenseStem (sStem);
        if (H_NO_ERROR != rc)
        {
            return rc;
        }
        if (sStem.bIsEmpty())
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

        m_pEndings = new CPastTenseEndings(m_pLexeme);
        if (NULL == m_pEndings)
        {
            return rc;
        }

        ET_StemAuslaut eAuslaut = STEM_AUSLAUT_UNDEFINED;

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
                if (NUM_SG == eNumber && GENDER_M == eGender)
                {
                    eAuslaut = sStem.bEndsWithOneOf(L"бвгджзклмнпрстфхцчшщ") ? STEM_AUSLAUT_CONSONANT
                                                                             : STEM_AUSLAUT_NOT_CONSONANT;
                }
                else
                {
                    eAuslaut = STEM_AUSLAUT_UNDEFINED;
                }

                rc = ((CPastTenseEndings *)m_pEndings)->eSelect(eGender, eNumber, eAuslaut);
                int64_t iNumEndings = m_pEndings->iCount();
                if (iNumEndings != 1)
                {
                    ERROR_LOG(L"No ending or too many endings");
                    continue;
                }

                CEString sEnding;
                int64_t llEndingKey = -1;
                rc = m_pEndings->eGetEnding(sEnding, llEndingKey);
                if (rc != H_NO_ERROR)
                {
                    continue;
                }

                vector<int> vecStress;
                rc = eGetStressPositions (sStem, sEnding, eNumber, eGender, vecStress);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                CWordForm * pWordForm = NULL;
                rc = eCreateFormTemplate(sStem, sEnding, eNumber, eGender, llEndingKey, pWordForm);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
                
                if (m_pLexeme->bNoRegularForms(pWordForm->sGramHash()))
                {
                    continue;
                }

                if (1 == vecStress.size() || m_pLexeme->bIsMultistressedCompound())
                {
                    vector<int>::iterator itStressPos = vecStress.begin();
                    for (; (H_NO_ERROR == rc) && (itStressPos != vecStress.end()); ++itStressPos)
                    {
                        rc = eAssemble (pWordForm, *itStressPos, sStem, sEnding);
                    }
                    if (rc != H_NO_ERROR && rc != H_NO_MORE)
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
                        rc = eAssemble (pWordForm, *itStressPos, sStem, sEnding);
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
    ASSERT(m_pLexeme);   // we assume base class ctor took care of this

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

                CWordForm * pStub = NULL;
                rc = eCreateFormTemplate (L"", L"", eNumber, eGender, -1, pStub);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
                if (!pStub)
                {
                    return H_ERROR_POINTER;
                }
                
//                if (m_pLexeme->bNoRegularForms(pStub->iGramHash()))
//                {
//                    continue;
//                }

                map<CWordForm *, bool> mapForm2IsOptional;
                rc = m_pLexeme->eGetIrregularForms(pStub->sGramHash(), mapForm2IsOptional);
                delete pStub;
                if (rc != H_NO_ERROR)
                {
                    continue;
                }

                map<CWordForm *, bool>::iterator it = mapForm2IsOptional.begin();
                for (; it != mapForm2IsOptional.end(); ++it)
                {
                    CWordForm * pSgWf = it->first;
                    if (NULL == pSgWf)
                    {
                        ASSERT(0);
                        CEString sMsg(L"Irregular form ptr invalid.");
                        sMsg += m_pLexeme->sSourceForm();
                        ERROR_LOG(sMsg);
                        return H_ERROR_POINTER;
                    }
                    m_pLexeme->AddWordForm(pSgWf);
                }

            }   //  for (ET_Gender eGender ...

        }   //  for (ET_Number eNumber ...
    }
    catch (CException& ex)
    {
        CEString sMsg(L"Exception: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG(sMsg);
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
                    ASSERT(0);
                    ERROR_LOG (L"Irregular form not found.");
                    return H_ERROR_UNEXPECTED;
                }

                if ((*it_if).second.str_Form.empty())
                {
                    ASSERT(0);
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
                        ASSERT(0);
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
                    ASSERT(0);
                    ERROR_LOG (L"Empty irregular f. sg. past.");
                    return H_ERROR_UNEXPECTED;
                }

                if (st_template.str_Form.length() < 2)
                {
                    ASSERT(0);
                    ERROR_LOG (L"f. sg. past too short.");
                    return H_ERROR_UNEXPECTED;
                }
                if (st_template.str_Form[st_template.str_Form.length()-1] != L'а')
                {
                    ASSERT(0);
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
                    ASSERT(0);
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

        if (NULL == pWordForm)
        {
            ASSERT(0);
            ERROR_LOG(L"Word form ptr is NULL");
            return H_ERROR_POINTER;
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
                ASSERT(0);
                ERROR_LOG (L"Unexpected stress position in cd-1 verb.");
                return H_ERROR_UNEXPECTED;
            }

            if (pWordForm->m_sWordForm.bStartsWith (L"пере"))
            {
                if ((*itStressPos).first < 2)
                {
                    ASSERT(0);
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
