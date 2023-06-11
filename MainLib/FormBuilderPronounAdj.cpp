#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "Endings.h"

#include "WordForm.h"
#include "Lexeme.h"
#include "FormBuilderPronounAdj.h"

using namespace Hlib;

CEString CFormBuilderPronounAdj::sGramHash (ET_PartOfSpeech ePos, 
                                            ET_Case eCase, 
                                            ET_Number eNumber, 
                                            ET_Gender eGender, 
                                            ET_Animacy eAnimacy)
{
    CGramHasher hasher (ePos, 
//                        SUBPARADIGM_PRONOUN_ADJ, 
                        (ePos == POS_PRONOUN_ADJ || ePos == POS_NOUN) ? SUBPARADIGM_PRONOUN_ADJ : SUBPARADIGM_NUM_ADJ,
                        eCase, 
                        eNumber, 
                        eGender,
                        PERSON_UNDEFINED,                             
                        eAnimacy, 
                        ASPECT_UNDEFINED, 
                        REFL_UNDEFINED); 

    CEString sHash = hasher.sGramHash();
    return sHash;
}

ET_ReturnCode CFormBuilderPronounAdj::eGetStressPositions (CEString& sStem,
                                                           const CEString& sEnding,
                                                           ET_StressLocation eStressType,
                                                           vector<int>& vecStressPos)
{
    ET_ReturnCode rc = H_NO_ERROR;

    sStem.SetVowels(CEString::g_szRusVowels);

    int iStressPos = -1;

    if (STRESS_LOCATION_STEM == eStressType)
    {
        rc = eGetStemStressPositions (sStem, m_eSubparadigm, NUM_UNDEFINED, vecStressPos);
                                                                    // Number not needed for this subparadigm
        if (rc != H_NO_ERROR)
        {
            return rc;
        }
    }
    else if (STRESS_LOCATION_ENDING == eStressType)
    {
        if (sEnding.uiNSyllables() < 1)
        {
            iStressPos = sStem.uiNSyllables() - 1;
        }
        else
        {
            if (L"мс-п" == m_spLexeme->sInflectionType() &&
                (L"его" == sEnding || L"ему" == sEnding ||
                 L"ого" == sEnding || L"ому" == sEnding))
            {
                iStressPos = sStem.uiNSyllables() + 1;    // одног<о, твоем<у
            }
            else
            {
                iStressPos = sStem.uiNSyllables();
            }
        }

//        hGetEndingStressPosition (str_Stem, sEnding, i_stressPos);
        vecStressPos.push_back (iStressPos);
    }
    else
    {
        assert(0);
        ERROR_LOG (L"Illegal stress type.");
        return H_ERROR_INVALID_ARG;
    }

    return rc;

}   //  eGetStressPositions (...)

ET_ReturnCode CFormBuilderPronounAdj::eCreateFormTemplate (ET_Gender eGender,
                                                           ET_Number eNumber, 
                                                           ET_Case eCase, 
                                                           ET_Animacy eAnimacy,
                                                           const CEString& sStem,
                                                           const CEString& sEnding,
                                                           int64_t llEndingKey,
                                                           shared_ptr<CWordForm>& spWordForm)
{
    ET_ReturnCode rc = H_NO_ERROR;
    
    spWordForm = make_shared<CWordForm>(m_spInflection);
    if (nullptr == spWordForm)
    {
		assert(0);
        ERROR_LOG (L"Unable to instantiate CWordForm.");
        return H_ERROR_POINTER;
    }

//    spWordForm->m_spLexeme = m_spLexeme;
    spWordForm->SetInflection(m_spInflection);
    spWordForm->SetPos(m_spLexeme->ePartOfSpeech());
    spWordForm->SetSubparadigm(SUBPARADIGM_PRONOUN_ADJ);
    spWordForm->SetStem(sStem);
    spWordForm->SetEnding(sEnding);
    spWordForm->SetEndingDataId(llEndingKey);
    spWordForm->SetGender(eGender);
    spWordForm->SetCase(eCase);
    spWordForm->SetNumber(eNumber);
    spWordForm->SetAnimacy(eAnimacy);
    spWordForm->SetWordForm(sStem + sEnding);
    spWordForm->SetStatus(m_eStatus);
    spWordForm->SetInflectionId(m_spInflection->llInflectionId());

//    rc = eAssignSecondaryStress (pWordForm);

    return rc;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderPronounAdj::eHandleCommonDeviations ([[maybe_unused]]shared_ptr<CWordForm> spWordForm)
{
    return H_NO_ERROR;

}   //  eHandleCommonDeviations (...)

ET_ReturnCode CFormBuilderPronounAdj::eBuild()
{
	assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    m_spEndings = make_shared<CAdjPronounEndings>(m_spLexeme, m_spInflection);
    if (NULL == m_spEndings)
    {
        return H_ERROR_POINTER;
    }

    CHasher gramTmp;
//    gramTmp.Initialize (POS_PRONOUN_ADJ);
//    gramTmp.Initialize(m_spLexeme->ePartOfSpeech());
//    gramTmp.SetParadigm(SUBPARADIGM_PRONOUN_ADJ);
  
    ET_Subparadigm eSubParadigm = SUBPARADIGM_PRONOUN_ADJ;
    if (m_spLexeme->ePartOfSpeech() == POS_NOUN)
    {
        gramTmp.Initialize(m_spLexeme->eMainSymbolToGender(), ET_Animacy::ANIM_UNDEFINED);
//        gramTmp.Initialize(m_spLexeme->eMainSymbolToGender(), m_spLexeme->eMainSymbolToAnimacy());
        eSubParadigm = SUBPARADIGM_PRONOUN_ADJ;
    }
    else
    {
        gramTmp.Initialize(POS_PRONOUN_ADJ);
        eSubParadigm = SUBPARADIGM_PRONOUN_ADJ;
    }
    gramTmp.SetParadigm(eSubParadigm);

    do
    {
        bool bSkipRegular = false;
        if (m_spLexeme->bHasIrregularForms())
        {
            CEString sPronAdj = sGramHash (m_spLexeme->ePartOfSpeech(),
                                           gramTmp.m_eCase, gramTmp.m_eNumber,
                                           gramTmp.m_eGender, gramTmp.m_eAnimacy);

            if (m_spLexeme->bHasMissingForms() && H_TRUE != m_spInflection->eFormExists(sPronAdj))
            {
                continue;
            }

            if (m_spInflection->bHasIrregularForm(sPronAdj))
            {
                map<shared_ptr<CWordForm>, bool> mapPronAdj;
                rc = m_spInflection->eGetIrregularForms(sPronAdj, mapPronAdj);
                if (rc != H_NO_ERROR)
                {
                    CEString sMsg(L"Unable to acquire irregular ");
                    sMsg += sPronAdj;
                    ERROR_LOG(sMsg + L" for " + m_spLexeme->sSourceForm());
                    continue;
                }

                auto it = mapPronAdj.begin();
                for (; it != mapPronAdj.end(); ++it)
                {
/*
                    CWordForm * pWordForm = NULL;
                    rc = eCreateFormTemplate(gramTmp.m_eGender,
                                             gramTmp.m_eNumber,
                                             gramTmp.m_eCase,
                                             gramTmp.m_eAnimacy,
                                             L"",
                                             L"",
                                             -1,
                                             pWordForm);
                    if (rc != H_NO_ERROR)
                    {
                        CEString sMsg(L"Unable to create irregular form template for  ");
                        sMsg += sPronAdj;
                        ERROR_LOG(sMsg + L"; source form = " + m_spLexeme->sSourceForm());
                        continue;
                    }

                    pWordForm->m_sWordForm = (*it).first->m_sWordForm;
                    pWordForm->m_mapStress = (*it).first->m_mapStress;
*/
                    m_spInflection->AddWordForm(it->first);

                    bSkipRegular = !(*it).second;  // i.e. no regular variant TODO: this DOES NOT indicate that no regular variants exist
//                    bSkipRegular = true;
                }
            }
        }       //  if (m_spLexeme->m_bHasIrregularForms)

        if (bSkipRegular)
        {
            continue;
        }

        ET_StressLocation eStressType = STRESS_LOCATION_UNDEFINED;
        switch (m_spInflection->eAccentType1())
        {
            case AT_A:
            {
                eStressType = STRESS_LOCATION_STEM;
                break;
            }
            case AT_B:
            {
                eStressType = STRESS_LOCATION_ENDING;
                break;
            }
            case AT_F:
            {
                if (NUM_SG == gramTmp.m_eNumber)
                {
                    eStressType = STRESS_LOCATION_ENDING;
                }
                else if (CASE_NOM == gramTmp.m_eCase)
                {
                    eStressType = STRESS_LOCATION_STEM;
                }
                else if (CASE_ACC == gramTmp.m_eCase && ANIM_NO == gramTmp.m_eAnimacy)
                {
                    eStressType = STRESS_LOCATION_STEM;
                }
                else
                {
                    eStressType = STRESS_LOCATION_ENDING;
                }
                break;
            }
            default:
            {
//                assert(0);
                CEString sMsg(L"Illegal accent type; lexeme = ");
                ERROR_LOG(sMsg + m_spLexeme->sSourceForm());
//                return H_ERROR_UNEXPECTED;
                continue;
            }

        }   //  switch (m_spLexeme->eAccentType1())

        ET_Animacy eAnimacy = ANIM_UNDEFINED;
        if (CASE_ACC == gramTmp.m_eCase)
        {
            if ((GENDER_M == gramTmp.m_eGender && NUM_SG == gramTmp.m_eNumber) || (NUM_PL == gramTmp.m_eNumber))
            {
                eAnimacy = m_spLexeme->ePartOfSpeech() == POS_PRONOUN_ADJ ? gramTmp.m_eAnimacy : m_spLexeme->eMainSymbolToAnimacy();
            }
        }

        ET_Gender eGender = gramTmp.m_eGender;
        if (m_spLexeme->ePartOfSpeech() == POS_NOUN)
        {
            if (NUM_SG == gramTmp.m_eNumber)
            {
                eGender = m_spLexeme->eMainSymbolToGender();
            }
            else
            {
                eGender = GENDER_UNDEFINED;
            }
        }

//        rc = ((CAdjPronounEndings *)m_spEndings)->eSelect(gramTmp.m_eGender, gramTmp.m_eNumber, gramTmp.m_eCase, gramTmp.m_eAnimacy);
        rc = static_pointer_cast<CAdjPronounEndings>(m_spEndings)->eSelect(eGender, gramTmp.m_eNumber, gramTmp.m_eCase, eAnimacy);
        int64_t iNumEndings = m_spEndings->iCount();
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
                CEString sMsg(L"Unable to acquire ");
                sMsg += gramTmp.sGramHash();
                sMsg += L" ending of ";
                ERROR_LOG(sMsg + m_spLexeme->sSourceForm());
                continue;
            }

            auto sStem(m_sStem);
            if (m_spInflection->bFleetingVowel() )//&&
//                (gramTmp.m_eCase != CASE_NOM || gramTmp.m_eGender != GENDER_M || gramTmp.m_eNumber != NUM_SG) &&
 //               (gramTmp.m_eCase != CASE_ACC || gramTmp.m_eGender != GENDER_M || gramTmp.m_eAnimacy != ANIM_NO))
            {
                rc = eFleetingVowelCheck (gramTmp.m_eNumber, gramTmp.m_eCase,
                                          gramTmp.m_eGender, eStressType,
                                          SUBPARADIGM_PRONOUN_ADJ, sEnding, sStem);
                if (rc != H_NO_ERROR)
                {
                    CEString sMsg(L"Fleeting vowel check failed for ");
                    sMsg += gramTmp.sGramHash();
                    sMsg += L" of ";
                    sMsg += m_spLexeme->sSourceForm();
                    ERROR_LOG(sMsg);
                    continue;
                }
            }

            shared_ptr<CWordForm> spWordForm;
            rc = eCreateFormTemplate(gramTmp.m_eGender, 
                                     gramTmp.m_eNumber, 
                                     gramTmp.m_eCase, 
                                     gramTmp.m_eAnimacy, 
                                     sStem,
                                     sEnding, 
                                     llEndingKey,
                                     spWordForm);
            if (rc != H_NO_ERROR)
            {
                CEString sMsg(L"CreateFormTemplate failed for  ");
                sMsg += gramTmp.sGramHash();
                sMsg += L" of ";
                ERROR_LOG(sMsg + m_spLexeme->sSourceForm());
                continue;
            }

            vector<int> vecStress;
            rc = eGetStressPositions (sStem, sEnding, eStressType, vecStress);
            if (rc != H_NO_ERROR)
            {
                CEString sMsg(L"Unable GetStressPositions() failed for ");
                sMsg += gramTmp.sGramHash();
                sMsg += L" of ";
                ERROR_LOG(sMsg + m_spLexeme->sSourceForm());
                continue;
            }

            if (1 == vecStress.size() || m_spInflection->bIsMultistressedCompound())
            {
                vector<int>::iterator itStressPos = vecStress.begin();
                for (; itStressPos != vecStress.end(); ++itStressPos)
                {
//                    spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                    spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
                }
                m_spInflection->AddWordForm (spWordForm);
            }
            else
            {
                vector<int>::iterator itStressPos = vecStress.begin();
                for (; itStressPos != vecStress.end(); ++itStressPos)
                {
                    if (itStressPos != vecStress.begin())
                    {
                        auto spWfVariant = make_shared<CWordForm>();
//                        CloneWordForm(spWordForm, spWfVariant);
                        spWfVariant->eCloneFrom(spWordForm);
                        spWordForm = spWfVariant;
                    }
                    // spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                    spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
                    m_spInflection->AddWordForm (spWordForm);
                }
            }
        }
    } while (gramTmp.bIncrement());

    return H_NO_ERROR;

}   //  eBuild (...)
