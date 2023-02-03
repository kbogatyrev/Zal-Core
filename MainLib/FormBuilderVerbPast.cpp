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
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    ET_AccentType eAccentType = AT_UNDEFINED;
    if (AT_UNDEFINED == m_spInflection->eAccentType2())
    {
        eAccentType = AT_A;
    }
    else
    {
        eAccentType = m_spInflection->eAccentType2();
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
            rc = m_spLexeme->eGetStemStressPositions (const_cast<CEString&>(sStem), vecStressPositions);
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
                if (REFL_NO == m_spLexeme->eIsReflexive())
                {
                    assert(0);
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
                                                     int64_t llEndingKey,
                                                     shared_ptr<CWordForm>& spWordForm)
{
    spWordForm = make_shared<CWordForm>();
    if (NULL == spWordForm)
    {
        assert(0);
        ERROR_LOG (L"CreateInstance() failed ");
        return H_ERROR_POINTER;
    }

    spWordForm->m_spLexeme = m_spLexeme;
    spWordForm->m_ePos = m_spLexeme->ePartOfSpeech();
    spWordForm->m_eSubparadigm = SUBPARADIGM_PAST_TENSE;
    spWordForm->m_eAspect = m_spLexeme->eAspect();
    spWordForm->m_eReflexivity = m_spLexeme->eIsReflexive();
    spWordForm->m_sStem = sStem;
    spWordForm->m_sWordForm = sStem + sEnding;
    spWordForm->m_eGender = eGender;
    spWordForm->m_eNumber = eNumber;
    spWordForm->m_llEndingDataId = llEndingKey;
    spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();

//    rc = eAssignSecondaryStress (spWordForm);

    return H_NO_ERROR;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderPast::eAssemble(shared_ptr<CWordForm> spWordForm, 
                                          int iStressPos, 
                                          CEString& sStem,
                                          CEString& sEnding)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        spWordForm->m_mapStress[iStressPos] = STRESS_PRIMARY;
        spWordForm->m_sStem = sStem;
        rc = eHandleYoAlternation (iStressPos, SUBPARADIGM_PAST_TENSE, spWordForm->m_sStem);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        spWordForm->m_sWordForm = spWordForm->m_sStem + sEnding;
        if (m_spInflection->bHasCommonDeviation(1) && !(NUM_SG == spWordForm->m_eNumber && GENDER_F == spWordForm->m_eGender))
        {
            rc = eRetractStressToPreverb (spWordForm, m_spInflection->bDeviationOptional(1));
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
        }

        if (m_spInflection->bHasCommonDeviation(5) && 3 == m_spInflection->iType() && 1 == m_spInflection->iStemAugment() &&
            NUM_SG == spWordForm->m_eNumber && GENDER_M == spWordForm->m_eGender)
        {
            if (m_spInflection->bDeviationOptional(5))
            {
                shared_ptr<CWordForm> spVariant;
                CloneWordForm (spWordForm, spVariant);
                m_spInflection->AddWordForm(spVariant);    // store both versions
                spWordForm = spVariant;
            }

            spWordForm->m_sWordForm = sStem + L"нул";
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
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    bool bIsVariant = false;

//    if (bHasIrregularPast()) -- NB this won't work with individual form editing.
//    {
        rc = eBuildIrregular (bIsVariant);
        if (!bIsVariant)
        {
//            return rc;
        }
//    }

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

//        ET_AccentType eAt = AT_UNDEFINED;
//        if (AT_UNDEFINED == m_spLexeme->eAccentType2())
//        {
//            eAt = AT_A;
//        }
//        else
//        {
//            eAt = m_spLexeme->eAccentType2();
//        }

        m_spEndings = make_shared<CPastTenseEndings>(m_spLexeme, m_spInflection);
        if (nullptr == m_spEndings)
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

                CGramHasher hasher(SUBPARADIGM_PAST_TENSE,
                    eNumber,
                    eGender,
                    PERSON_UNDEFINED,
                    ANIM_UNDEFINED,
                    m_spLexeme->eAspect(),
                    CASE_UNDEFINED,
                    m_spLexeme->eIsReflexive());

//                if (m_spLexeme->bHasMissingForms() || m_spLexeme->bImpersonal())
                if (m_spLexeme->bImpersonal())
                    {
                    if (m_spInflection->eFormExists(hasher.sGramHash()) != H_TRUE)
                    {
                        continue;
                    }
                }

                if (m_spLexeme->bHasMissingForms())
                {
                    auto sHash = hasher.sGramHash();
                    if (m_spInflection->eFormExists(sHash) != H_TRUE)
                    {
                        m_spInflection->SetHypotheticalForm(sHash);
                    }
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

                rc = static_pointer_cast<CPastTenseEndings>(m_spEndings)->eSelect(eGender, eNumber, eAuslaut);
                int64_t iNumEndings = m_spEndings->iCount();
                if (iNumEndings != 1)
                {
                    ERROR_LOG(L"No ending or too many endings");
                    continue;
                }

                CEString sEnding;
                int64_t llEndingKey = -1;
                rc = m_spEndings->eGetEnding(sEnding, llEndingKey);
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

                shared_ptr<CWordForm> spWordForm;
                rc = eCreateFormTemplate(sStem, sEnding, eNumber, eGender, llEndingKey, spWordForm);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
                
                if (m_spInflection->bNoRegularForms(spWordForm->sGramHash()))
                {
                    continue;
                }

                if (1 == vecStress.size() || m_spInflection->bIsMultistressedCompound())
                {
                    vector<int>::iterator itStressPos = vecStress.begin();
                    for (; (H_NO_ERROR == rc) && (itStressPos != vecStress.end()); ++itStressPos)
                    {
                        rc = eAssemble(spWordForm, *itStressPos, sStem, sEnding);
                    }
                    if (rc != H_NO_ERROR && rc != H_NO_MORE)
                    {
                        return rc;
                    }

                    m_spInflection->AddWordForm(spWordForm);
                }
                else
                {
                    vector<int>::iterator itStressPos = vecStress.begin();
                    for (; itStressPos != vecStress.end(); ++itStressPos)
                    {
                        if (itStressPos != vecStress.begin())
                        {
                            shared_ptr<CWordForm> spWfVariant;
                            CloneWordForm (spWordForm, spWfVariant);
                            spWfVariant->m_mapStress.clear();
                            spWordForm = spWfVariant;
                        }
                        rc = eAssemble(spWordForm, *itStressPos, sStem, sEnding);
                        if (rc != H_NO_ERROR)
                        {
                            return rc;
                        }

                        m_spInflection->AddWordForm(spWordForm);
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

ET_ReturnCode CFormBuilderPast::eBuildIrregular ([[maybe_unused]]bool& bIsVariant)
{
    // TODO: do we need to handle variants?
    assert(m_spLexeme);   // we assume base class ctor took care of this

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

                shared_ptr<CWordForm> spStub;
                rc = eCreateFormTemplate(L"", L"", eNumber, eGender, -1, spStub);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
                if (!spStub)
                {
                    return H_ERROR_POINTER;
                }
                
//                if (m_spLexeme->bNoRegularForms(pStub->iGramHash()))
//                {
//                    continue;
//                }

                map<shared_ptr<CWordForm>, bool> mapForm2IsOptional;
                rc = m_spInflection->eGetIrregularForms(spStub->sGramHash(), mapForm2IsOptional);
                if (rc != H_NO_ERROR)
                {
                    continue;
                }

                auto it = mapForm2IsOptional.begin();
                for (; it != mapForm2IsOptional.end(); ++it)
                {
                    shared_ptr<CWordForm> spSgWf = it->first;
                    if (nullptr == spSgWf)
                    {
                        assert(0);
                        CEString sMsg(L"Irregular form ptr invalid.");
                        sMsg += m_spLexeme->sSourceForm();
                        ERROR_LOG(sMsg);
                        return H_ERROR_POINTER;
                    }
                    m_spInflection->AddWordForm(spSgWf);
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

ET_ReturnCode CFormBuilderPast::eRetractStressToPreverb (shared_ptr<CWordForm> spWordForm, bool bIsOptional)
{
    try
    {
        if (bIsOptional)    // store 1st variant if retraction is optional
        {
            shared_ptr<CWordForm> spClone;
            CloneWordForm (spWordForm, spClone);
            m_spInflection->AddWordForm (spClone);    // store both versions
            spWordForm = spClone;
        }

        if (NULL == spWordForm)
        {
            assert(0);
            ERROR_LOG(L"Word form ptr is NULL");
            return H_ERROR_POINTER;
        }

        map<int, ET_StressType> mapCorrectedStress;
        map<int, ET_StressType>::iterator itStressPos = spWordForm->m_mapStress.begin();
        for (; itStressPos != spWordForm->m_mapStress.end(); ++itStressPos)
        {
              if (STRESS_SECONDARY == (*itStressPos).second)
              {
                mapCorrectedStress[(*itStressPos).first] = STRESS_SECONDARY;
                continue;
            }
            if ((*itStressPos).first < 1)
            {
                assert(0);
                ERROR_LOG (L"Unexpected stress position in cd-1 verb.");
                return H_ERROR_UNEXPECTED;
            }

            auto iStressPos = -1;
            if (spWordForm->m_sWordForm.bStartsWith (L"пере"))
            {
                if ((*itStressPos).first < 2)
                {
                    assert(0);
                    ERROR_LOG (L"Unexpected stress position in cd-1 verb.");
                    return H_ERROR_UNEXPECTED;
                }
                iStressPos = (*itStressPos).first - 2;
            }
            else
            {
                iStressPos = (*itStressPos).first - 1;
            }
            mapCorrectedStress[iStressPos] = STRESS_PRIMARY;

            // Remove yo at old stress pos
            if (iStressPos < 0 || iStressPos > (int)spWordForm->m_sStem.uiLength())
            {
                assert(0);
                ERROR_LOG(L"Unexpected stress position in cd-1 verb.");
                return H_ERROR_UNEXPECTED;
            }

            int iAt = spWordForm->sStem().uiGetVowelPos((*itStressPos).first);
            if (iAt < 0 || iAt > (int)spWordForm->m_sStem.uiLength())
            {
                assert(0);
                ERROR_LOG(L"Unexpected stress position in cd-1 verb.");
                return H_ERROR_UNEXPECTED;
            }
            if (L'ё' == spWordForm->m_sWordForm[iAt])
            {
                spWordForm->m_sStem[iAt] = spWordForm->m_sWordForm[iAt] = L'е';
            }

        }
        spWordForm->m_mapStress = mapCorrectedStress;
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
