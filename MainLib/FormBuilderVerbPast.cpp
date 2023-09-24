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
    spWordForm = make_shared<CWordForm>(m_spInflection);
    if (nullptr == spWordForm)
    {
        assert(0);
        ERROR_LOG (L"CreateInstance() failed ");
        return H_ERROR_POINTER;
    }

//    spWordForm->m_spLexeme = m_spLexeme;
    spWordForm->SetInflection(m_spInflection);
    spWordForm->SetPos(POS_VERB);
    spWordForm->SetSubparadigm(SUBPARADIGM_PAST_TENSE);
    spWordForm->SetAspect(m_spLexeme->eAspect());
    spWordForm->SetReflexivity(m_spLexeme->eIsReflexive());
    spWordForm->SetStem(sStem);
    spWordForm->SetEnding(sEnding);
    spWordForm->SetWordForm(sStem + sEnding);
    spWordForm->SetGender(eGender);
    spWordForm->SetNumber(eNumber);
    spWordForm->SetEndingDataId(llEndingKey);
//    spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();
    spWordForm->SetInflectionId(m_spLexeme->llLexemeId());

//    rc = eAssignSecondaryStress (spWordForm);

    return H_NO_ERROR;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderPast::eAssemble(shared_ptr<CWordForm> spWordForm, 
                                          int iStressPos, 
                                          const CEString& sStem,
                                          CEString& sEnding)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        spWordForm->SetStressPos(iStressPos, STRESS_PRIMARY);
        rc = eHandleYoAlternation (iStressPos, SUBPARADIGM_PAST_TENSE, spWordForm);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }
//        spWordForm->SetStem(sStem);

        spWordForm->SetWordForm(spWordForm->sStem() + sEnding);
//        spWordForm->SetWordForm(sStem + sEnding);
        if (m_spInflection->bHasCommonDeviation(1) && !(NUM_SG == spWordForm->eNumber() && GENDER_F == spWordForm->eGender()))
        {
            rc = eRetractStressToPreverb (spWordForm, m_spInflection->bDeviationOptional(1));
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
        }

        if (m_spInflection->bHasCommonDeviation(5) && 3 == m_spInflection->iType() && 1 == m_spInflection->iStemAugment() &&
            NUM_SG == spWordForm->eNumber() && GENDER_M == spWordForm->eGender())
        {
            if (m_spInflection->bDeviationOptional(5))
            {
                auto spVariant = make_shared<CWordForm>();
//                CloneWordForm (spWordForm, spVariant);
                spVariant->eCloneFrom(spWordForm);
                m_spInflection->AddWordForm(spVariant);    // store both versions
                spWordForm = spVariant;
            }

            spWordForm->SetWordForm(sStem + L"нул");
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
                            auto spWfVariant = make_shared<CWordForm>();
//                            CloneWordForm (spWordForm, spWfVariant);
//                            spWfVariant->m_mapStress.clear();
                            spWfVariant->eCloneFrom(spWordForm);
                            spWfVariant->ClearStress();
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
            auto spClone = make_shared<CWordForm>();
//            CloneWordForm (spWordForm, spClone);
            spClone->eCloneFrom(spWordForm);
            m_spInflection->AddWordForm (spClone);    // store both versions
            spWordForm = spClone;
        }

        if (nullptr == spWordForm)
        {
            assert(0);
            ERROR_LOG(L"Word form ptr is NULL");
            return H_ERROR_POINTER;
        }

        map<int, ET_StressType> mapCorrectedStress;
//        ET_StressType eType = ET_StressType::STRESS_TYPE_UNDEFINED;
        auto& mapStressSylls = spWordForm->mapGetStressPositions();
//        auto rcStress = spWordForm->eGetFirstStressSyll(iPos, eType);
        auto itStressSyll = mapStressSylls.begin();
        for (; itStressSyll != mapStressSylls.end(); ++itStressSyll)
        {
            if (STRESS_SECONDARY == (*itStressSyll).second)
            {
                mapCorrectedStress[(*itStressSyll).first] = STRESS_SECONDARY;
                continue;
            }
            if ((*itStressSyll).first < 1)
            {
                assert(0);
                ERROR_LOG(L"Unexpected stress position in cd-1 verb.");
                return H_ERROR_UNEXPECTED;
            }

            auto iStressPos = -1;
            if (spWordForm->sWordForm().bStartsWith(L"пере"))
            {
                if ((*itStressSyll).first < 2)
                {
                    assert(0);
                    ERROR_LOG(L"Unexpected stress position in cd-1 verb.");
                    return H_ERROR_UNEXPECTED;
                }
                iStressPos = (*itStressSyll).first - 2;
            }
            else
            {
                iStressPos = (*itStressSyll).first - 1;
            }
            mapCorrectedStress[iStressPos] = STRESS_PRIMARY;

            // Remove yo at old stress pos
            if (iStressPos < 0 || iStressPos >(int)spWordForm->sStem().uiLength())
            {
                assert(0);
                ERROR_LOG(L"Unexpected stress position in cd-1 verb.");
                return H_ERROR_UNEXPECTED;
            }

            int iAt = spWordForm->sStem().uiGetVowelPos((*itStressSyll).first);
            if (iAt < 0 || iAt >(int)spWordForm->sStem().uiLength())
            {
                assert(0);
                ERROR_LOG(L"Unexpected stress position in cd-1 verb.");
                return H_ERROR_UNEXPECTED;
            }
            if (L'ё' == spWordForm->sWordForm()[iAt])
            {
                spWordForm->sStem()[iAt] = spWordForm->sWordForm()[iAt] = L'е';
            }

        }
        spWordForm->AssignStress(mapCorrectedStress);
    }
    catch (CException& ex)
    {
        CEString sMsg(L"Exception: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG(sMsg);
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

 }   //  eRetractStressToPreverb (...)
