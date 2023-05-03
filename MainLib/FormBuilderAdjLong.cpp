#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "Logging.h"
#include "Endings.h"
#include "FormBuilderAdjLong.h"

using namespace Hlib;

ET_ReturnCode CFormBuilderLongAdj::eGetStressPositions (const CEString& sEnding,
                                                        ET_StressLocation eStressType,
                                                        vector<int>& vecStressPos)
{
    ET_ReturnCode rc = H_NO_ERROR;

    m_sStem.SetVowels (CEString::g_szRusVowels);

    int iStressPos = -1;

    if (STRESS_LOCATION_STEM == eStressType)
    {
        rc = eGetStemStressPositions (m_sStem, m_eSubparadigm, NUM_UNDEFINED, vecStressPos);
                                                                // Number not needed for this subparadigm
    }
    else if (STRESS_LOCATION_ENDING == eStressType)
    {
        if (sEnding.uiNSyllables() < 1)
        {
            iStressPos = m_sStem.uiNSyllables() - 1;
        }
        else
        {
            iStressPos = m_sStem.uiNSyllables();
        }

        vecStressPos.push_back (iStressPos);
    }
    else
    {
        assert (0);
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
                                                        int64_t llEndingKey,
                                                        shared_ptr<CWordForm>& spWordForm)
{
    if (m_sStem.bIsEmpty())
    {
        ERROR_LOG (L"No stem"); 
        return H_ERROR_UNEXPECTED;
    }

    spWordForm = make_shared<CWordForm>();
    if (nullptr == spWordForm)
    {
        assert(0);
        ERROR_LOG (L"Error creating CWordForm object");
        return H_ERROR_POINTER;
    }

    spWordForm->SetPos(m_spLexeme->ePartOfSpeech());
    spWordForm->SetSubparadigm(m_eSubparadigm);
    spWordForm->SetStem(m_sStem);
    spWordForm->SetEnding(sEnding);
    spWordForm->SetEndingDataId(llEndingKey);
    spWordForm->SetGender(eGender);
    spWordForm->SetCase(eCase);
    spWordForm->SetNumber(eNumber);
    spWordForm->SetAnimacy(eAnimacy);
    spWordForm->SetReflexivity(m_spLexeme->eIsReflexive());
    spWordForm->SetWordForm(m_sStem + sEnding);
    spWordForm->SetStatus(m_eStatus);
    spWordForm->SetInflectionId(m_spInflection->llInflectionId());
//        m_llLexemeId = m_spLexeme->llLexemeId();

    return H_NO_ERROR;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderLongAdj::eCheckIrregularForms(const CEString& sHash, bool& bHandled)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    if (!m_spLexeme->bHasIrregularForms())
    {
        return H_FALSE;
    }

    bHandled = false;

    map<shared_ptr<CWordForm>, bool> mapIrreg;
    rc = m_spInflection->eGetIrregularForms(sHash, mapIrreg);
    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    auto it = mapIrreg.begin();
    for (; it != mapIrreg.end(); ++it)
    {
        if ((*it).second)   // optional?
        {
            //            bHandled = false;
            bHandled = true;            // TODO: needs review. This DOES NOT indicate that a regular variant exists
        }
        else
        {
            bHandled = true;
        }

        auto spWordForm = make_shared<CWordForm>();
        spWordForm->Copy(*it->first);
//        pWordForm->m_eCase = eCase;   // ending case may differ from actual case, e.g. A.Sg.
        m_spInflection->AddWordForm(spWordForm);
    }

    return H_NO_ERROR;

}   //  eCheckIrregularForms (...)

ET_ReturnCode CFormBuilderLongAdj::eHandleCommonDeviations (shared_ptr<CWordForm> spWordForm)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    try
    {
        if (m_spInflection->bHasCommonDeviation(4) && SUBPARADIGM_PART_PRES_ACT == m_eSubparadigm)
        {
            if (m_spInflection->bDeviationOptional(4))
            {
                auto spVariant = make_shared<CWordForm>();
//                CloneWordForm (spWordForm, spVariant);
                spVariant->eCloneFrom(spWordForm);
                m_spInflection->AddWordForm (spVariant);    // store both versions
                spWordForm = spVariant;
            }

            map<int, ET_StressType> mapCorrectedStress;

//            int iPos = -1;
            ET_StressType eType = ET_StressType::STRESS_TYPE_UNDEFINED;
//            auto rc = spWordForm->eGetFirstStressPos(iPos, eType);
            auto& mapStressSylls = spWordForm->mapGetStressPositions();
            for (auto itStressSyll = mapStressSylls.begin(); itStressSyll != mapStressSylls.end(); ++itStressSyll)            
            {
                if (itStressSyll->first < 1)
                {
                    assert(0);
                    ERROR_LOG(L"Unexpected stress position in cd-4 participle.");
                    return H_ERROR_UNEXPECTED;
                }
                if (STRESS_SECONDARY == eType)
                {
                    mapCorrectedStress[itStressSyll->first] = STRESS_SECONDARY;
                    continue;
                }
                mapCorrectedStress[itStressSyll->first-1] = STRESS_PRIMARY;
            }

            spWordForm->AssignStress(mapCorrectedStress);

        }   //  if (bHasCommonDeviation (4) ...

        if (m_spInflection->bHasCommonDeviation(6) && 
            SUBPARADIGM_PART_PAST_ACT == m_eSubparadigm &&
            3 == m_spInflection->iType() &&
            1 == m_spInflection->iStemAugment())
        {
            if (m_spInflection->bDeviationOptional(6))
            {
                auto spVariant = make_shared<CWordForm>();
//                CloneWordForm (spWordForm, spVariant);
                spVariant->eCloneFrom(spWordForm);
                m_spInflection->AddWordForm (spVariant);    // store both versions
                spWordForm = spVariant;
            }

            CEString sEnding = spWordForm->sWordForm().sSubstr(spWordForm->sStem().uiLength());
            CEString sStem = spWordForm->sStem();
            int iLength = -1;
            if (sStem.bEndsWith (L"вш"))
            {
                iLength = sStem.uiLength()-2;
            }
            else
            {
                iLength = sStem.uiLength()-1;
            }

            sStem = sStem.sSubstr (0, iLength);

            if (sStem.bEndsWith (L"ну"))
            {
                if (m_spInflection->bDeviationOptional(6))
                {
                    // For optional (6) we store both variants
                    sStem = sStem.sSubstr (0, iLength-2);
                    sStem += sStem.bEndsWithOneOf (CEString::g_szRusConsonants) ? L"ш" : L"вш";
                }
                else
                {
                    // Stem already has -nu-; nothing to do (note that we cannot
                    // predict which form was used for derivation)
//                    return H_NO_MORE;
                    return H_NO_ERROR;
                }
            }
            else
            {
                sStem += L"нувш";
            }
            spWordForm->SetWordForm(sStem + sEnding);

        }   //  if (bHasCommonDeviation (6) ... )

        int iCd = -1;
        if (m_spInflection->bHasCommonDeviation(7))
        {
            iCd = 7;
        }
        else if (m_spInflection->bHasCommonDeviation(8))
        {
            iCd = 8;
        }

        if (iCd > 0 && SUBPARADIGM_PART_PAST_PASS_LONG == m_eSubparadigm)
        {
            CEString sStem(spWordForm->sStem());
            int iNSyll = sStem.uiNSyllables();
            if (iNSyll < 1)        //  < 2??
            {
                assert(0);
                CEString sMsg (L"No syllables in part pass past stem.");
                ERROR_LOG (sMsg);
                return H_ERROR_UNEXPECTED;
            }

            if (m_spInflection->bDeviationOptional(iCd))
            {
                auto  spVariant = make_shared<CWordForm>();
                spVariant->eCloneFrom(spWordForm);
//                CloneWordForm (spWordForm, spVariant);
                m_spInflection->AddWordForm(spVariant);    // store both versions
                spWordForm = spVariant;
            }

            map<int, ET_StressType> mapCorrectedStress;
            int iNewStressPos = -1;
//            ET_StressType eType = ET_StressType::STRESS_TYPE_UNDEFINED;
//            auto rc = spWordForm->eGetFirstStressPos(iNewStressPos, eType);
//            while (H_NO_ERROR == rc)
            auto& mapStressSylls = spWordForm->mapGetStressPositions();
            for (auto itStressSyll = mapStressSylls.begin(); itStressSyll != mapStressSylls.end(); ++itStressSyll)
            {
                if (STRESS_SECONDARY == itStressSyll->second)
                {
                    mapCorrectedStress[itStressSyll->first] = STRESS_SECONDARY;
                    continue;
                }
                if (8 == iCd && (*itStressSyll).first < 1)
                {
                    assert(0);
                    ERROR_LOG(L"Unexpected stress position in cd-7 or cd-8 participle.");
                    return H_ERROR_UNEXPECTED;
                }

                if (7 == iCd)
                {
                    iNewStressPos = (*itStressSyll).first + 1;
                    iNewStressPos = sStem.uiNSyllables() - 1;
                }
                else
                {
                    if (8 == iCd)
                    {
                        iNewStressPos = spWordForm->sStem().uiNSyllables() - 2;
                    }
                    else
                    {
                        assert(0);
                        ERROR_LOG(L"Unexpected cd number.");
                        return H_ERROR_UNEXPECTED;
                    }
                }

                mapCorrectedStress[iNewStressPos] = STRESS_PRIMARY;

                // Remove yo at old stress pos
                int iAt = sStem.uiGetVowelPos((*itStressSyll).first);
                if (L'ё' == spWordForm->sWordForm()[iAt])
                {
                    auto&& sStem = spWordForm->sStem();
                    sStem[iAt] = L'е';
                    spWordForm->SetStem(sStem);
                    auto&& sWordForm = spWordForm->sWordForm();
                    sWordForm[iAt] = L'е';
                    spWordForm->SetWordForm(sWordForm);
                }

                // E --> yo at new stress pos
                iAt = sStem.uiGetVowelPos(iNewStressPos);
                if (L'е' == spWordForm->sWordForm()[iAt])
                {
                    auto&& sStem = spWordForm->sStem();
                    sStem[iAt] = L'ё';
                    spWordForm->SetStem(sStem);
                    auto&& sWordForm = spWordForm->sWordForm();
                    sWordForm[iAt] = L'ё';
                    spWordForm->SetWordForm(sWordForm);
                }
            }

            spWordForm->AssignStress(mapCorrectedStress);
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

    assert(m_spLexeme);   // we assume base class ctor took care of this

    if (m_spLexeme->bNoLongForms())
    {
        return H_NO_MORE;
    }

    try
    {
        m_spEndings = make_shared<CAdjLongEndings>(m_spLexeme, m_spInflection, m_eSubparadigm);
        if (NULL == m_spEndings)
        {
            return H_ERROR_POINTER;
        }

        CGramHasher gram_tmp;
//        gram_tmp.Initialize(POS_ADJ);
        gram_tmp.Initialize(m_spLexeme->ePartOfSpeech());
        ET_Subparadigm eSubParadigm = SUBPARADIGM_LONG_ADJ;
        if (m_spLexeme->ePartOfSpeech() == POS_NOUN)
        {
            gram_tmp.Initialize(m_spLexeme->eMainSymbolToGender(), m_spLexeme->eMainSymbolToAnimacy());
            eSubParadigm = SUBPARADIGM_NOUN;
        }
        else
        {
            gram_tmp.Initialize(POS_ADJ);
            eSubParadigm = SUBPARADIGM_LONG_ADJ;
        }
        gram_tmp.SetParadigm(eSubParadigm);

/*      // TODO why?
        bool bPluralNoun = false;
        if (m_pLexeme->sInflectionType() == L"мн." || m_pLexeme->sInflectionType() == L"мн. неод." ||
            m_pLexeme->sInflectionType() == L"мн. одуш." || m_pLexeme->sInflectionType() == L"мн. от")
        {
            bPluralNoun = true;
        }
*/
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
                else if (0 == m_spInflection->iType())
                {
                    eStressType = STRESS_LOCATION_STEM;
                }
                else
                {
                    CEString sMsg(L"Unknown stress type.");
                    sMsg += L"; lexeme = ";
                    sMsg += m_spLexeme->sSourceForm();
                    ERROR_LOG(sMsg);
                    continue;
                }
            }

            ET_Animacy eAnimacy = ANIM_UNDEFINED;
            if (CASE_ACC == gram_tmp.m_eCase)
            {
                if ((GENDER_M == gram_tmp.m_eGender && NUM_SG == gram_tmp.m_eNumber) || (NUM_PL == gram_tmp.m_eNumber))
                {
                    eAnimacy = m_spLexeme->ePartOfSpeech() == POS_ADJ ? gram_tmp.m_eAnimacy : m_spLexeme->eMainSymbolToAnimacy();
                }
            }

            ET_Gender eGender = GENDER_UNDEFINED;
            if (m_spLexeme->ePartOfSpeech() == POS_ADJ 
                || (m_spLexeme->ePartOfSpeech() == POS_PRONOUN_ADJ && L'п' == m_spLexeme->stGetProperties().sInflectionType)
                || (m_spLexeme->ePartOfSpeech() == POS_NUM_ADJ && L'п' == m_spLexeme->stGetProperties().sInflectionType))
            {
                eGender = gram_tmp.m_eGender;
            }
            else if (m_spLexeme->ePartOfSpeech() == POS_NOUN && NUM_SG == gram_tmp.m_eNumber)
            {
                eGender = m_spLexeme->eMainSymbolToGender();
            }

            if (m_spLexeme->bHasMissingForms() && m_spInflection->eFormExists(gram_tmp.sGramHash()) != H_TRUE)
            {
                continue;
            }

            bool bSkipRegular = false;
            // NB: this gram hash must have POS subparadigm, not the inflection type
            CGramHasher gh(m_eSubparadigm, gram_tmp.m_eNumber, gram_tmp.m_eGender, gram_tmp.m_eAnimacy, gram_tmp.m_eCase);
            ET_ReturnCode eRet = eCheckIrregularForms(gh.sGramHash(), bSkipRegular);
            if (eRet != H_NO_ERROR && eRet != H_FALSE)
            {
                ERROR_LOG(L"Error in eCheckIrregularForms()");
                continue;
            }
            if (bSkipRegular)
            {
                continue;
            }

            rc = static_pointer_cast<CAdjLongEndings>(m_spEndings)->eSelect(SUBPARADIGM_LONG_ADJ, eGender, gram_tmp.m_eNumber, gram_tmp.m_eCase, eAnimacy);
            int iNumEndings = m_spEndings->iCount();
            if (iNumEndings < 1)
            {
                if (m_spInflection->iType() == 0)
                {
                    vector<int> vecStress;
                    rc = eGetStressPositions(L"", eStressType, vecStress);
                    if (rc != H_NO_ERROR)
                    {
                        continue;
                    }

                    auto spWordForm = make_shared<CWordForm>();
                    rc = eCreateFormTemplate(gram_tmp.m_eGender,
                                             gram_tmp.m_eNumber,
                                             gram_tmp.m_eCase,
                                             gram_tmp.m_eAnimacy,
                                             L"",
                                             0,
                                             spWordForm);
                    if (rc != H_NO_ERROR)
                    {
                        return rc;
                    }

                    if (1 == vecStress.size() || m_spInflection->bIsMultistressedCompound())
                    {
                        vector<int>::iterator itStressPos = vecStress.begin();
                        for (; itStressPos != vecStress.end(); ++itStressPos)
                        {
//                            spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                            spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
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
                                spWfVariant->eCloneFrom(spWordForm);
//                                CloneWordForm(spWordForm, spWfVariant);
                                spWordForm = spWfVariant;
                            }
//                            spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                            spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
                            m_spInflection->AddWordForm(spWordForm);
                        }
                    }
                }
                continue;
            }

            for (int iEnding = 0; iEnding < iNumEndings; ++iEnding)
            {
                CEString sEnding;
                int64_t llEndingKey;
                m_spEndings->eGetEnding (iEnding, sEnding, llEndingKey);

                auto spWordForm = make_shared<CWordForm>();
                rc = eCreateFormTemplate (gram_tmp.m_eGender, 
                                          gram_tmp.m_eNumber, 
                                          gram_tmp.m_eCase, 
                                          gram_tmp.m_eAnimacy, 
                                          sEnding, 
                                          llEndingKey,
                                          spWordForm);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                vector<int> vecStress;
                rc = eGetStressPositions (sEnding, eStressType, vecStress);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                if (1 == vecStress.size() || m_spInflection->bIsMultistressedCompound())
                {
                    vector<int>::iterator itStressPos = vecStress.begin();
                    for (; itStressPos != vecStress.end(); ++itStressPos)
                    {
                        spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
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
                            spWfVariant->eCloneFrom(spWordForm);
                            spWfVariant->ClearStress();
                            spWordForm = spWfVariant;
                        }
                        spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
                        m_spInflection->AddWordForm(spWordForm);
                    }
                }

            }   //  for (int iEnding = 0; iEnding < iNumEndings; ++iEnding)
        } while (gram_tmp.bIncrement());
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;  // logging should be always done by callee
    }

    return H_NO_ERROR;

}   //  eBuild (...)

ET_ReturnCode CFormBuilderLongAdj::eBuildParticiple()
{
    ET_ReturnCode rc = H_NO_ERROR;

    assert(m_spLexeme);   // we assume base class ctor took care of this

    if (-1 == m_iStemStressPos)
    {
        assert(0);
        ERROR_LOG (L"No stress position for a participle.");
        return H_ERROR_GENERAL;
    }

    try
    {
        m_spEndings =  make_shared<CAdjLongEndings>(m_spLexeme, m_spInflection, m_eSubparadigm);
        if (nullptr == m_spEndings)
        {
            return H_ERROR_POINTER;
        }

        CGramHasher gramHash;
        gramHash.Initialize(POS_VERB);
        gramHash.m_eAspect = m_spLexeme->eAspect();
        gramHash.SetParadigm (m_eSubparadigm);
        bool bInitialForm = true;
        do
        {
            static_pointer_cast<CAdjLongEndings>(m_spEndings)->
                eSelect(m_eSubparadigm, gramHash.m_eGender, gramHash.m_eNumber, gramHash.m_eCase, gramHash.m_eAnimacy);
            int iNumEndings = m_spEndings->iCount();
            if (iNumEndings < 1)
            {
                ERROR_LOG (L"No endings");
                continue;
            }

            for (int iEnding = 0; iEnding < iNumEndings; ++iEnding)
            {
                CEString sEnding;
                int64_t llEndingKey = -1;
                rc = m_spEndings->eGetEnding(iEnding, sEnding, llEndingKey);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                if (!m_bIrregular)
                {
                    if (SUBPARADIGM_PART_PAST_ACT == m_eSubparadigm && m_spLexeme->sTrailingComment().bStartsWith(L"но прич. прош. "))
                    {
                        int iChr = m_sStem.uiLength() - 1;
                        while (iChr >= 0 && m_sStem[iChr] != L'е' && m_sStem[iChr] != L'ё')
                        {
                            --iChr;
                        }
                        if (L'ё' == m_sStem[iChr])
                        {
                            m_sStem[iChr] = L'е';
                        }
                        m_iStemStressPos = m_sStem.uiGetSyllableFromVowelPos(iChr);
                    }
                    else
                    {
                        rc = eHandleYoAlternation(STRESS_LOCATION_STEM, m_iStemStressPos, m_sStem, sEnding);
                        if (rc != H_NO_ERROR)
                        {
                            return rc;
                        }
                    }
                }

//                CWordForm * pStressTemplate = NULL;
                auto spWordForm = make_shared<CWordForm>();
                rc = eCreateFormTemplate (gramHash.m_eGender,
                                          gramHash.m_eNumber,
                                          gramHash.m_eCase,
                                          gramHash.m_eAnimacy,
                                          sEnding, 
                                          llEndingKey,
                                          spWordForm);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                spWordForm->SetAspect(gramHash.m_eAspect);
                spWordForm->SetStressPos(m_iStemStressPos, STRESS_PRIMARY);
                if (!m_bIrregular)
                {
                    rc = eHandleCommonDeviations(spWordForm);
                    if (rc != H_NO_ERROR)
                    {
                        return rc;
                    }
                }

                if (m_bIrregular && bInitialForm)
                {
                    spWordForm->SetIrregular(true);
                }

                bInitialForm = false;

                m_spInflection->AddWordForm(spWordForm);

            }   //  for (int iEnding = 0; iEnding < iNumEndings; ++iEnding)

        } while (gramHash.bIncrement());
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;  // logging should be always done by callee
    }

    return H_NO_ERROR;

}   //  eBuildParticiple (...)
