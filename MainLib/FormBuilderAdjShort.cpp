#include "WordForm.h"
#include "Lexeme.h"
#include "Endings.h"
#include "FormBuilderAdjShort.h"

using namespace Hlib;

CFormBuilderShortAdj::CFormBuilderShortAdj (shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection) 
    : CFormBuilderDecl(spLexeme, spInflection, SUBPARADIGM_SHORT_ADJ),
    m_sStem(spLexeme->sGraphicStem()),
    m_iType (m_spInflection->iType()),
    m_iVerbDeviation(0),
    m_eAccentType1 (m_spInflection->eAccentType1()),
    m_eAccentType2 (m_spInflection->eAccentType2()),
    m_bFleetingVowel (m_spInflection->bFleetingVowel())
{
    if (AT_UNDEFINED == m_eAccentType2)
    {
        switch (m_eAccentType1)
        {
            case AT_A:
            {
                m_eAccentType2 = AT_A;
                break;
            }
            case AT_A1:
            {
                m_eAccentType2 = AT_A1;
                break;
            }
            case AT_B:
            {
                m_eAccentType2 = AT_B;
                break;
            }
            default:
            {
//                assert(0);
                CEString sMsg (L"Accent type undefined for short forms; lexeme = ");
                ERROR_LOG(sMsg + m_spLexeme->sSourceForm());
                throw CException (H_CTOR, sMsg);
            }
        }       // switch
    }
}

//
// Use with participles
//
CFormBuilderShortAdj::CFormBuilderShortAdj (shared_ptr<CLexeme> spLexeme, 
                                            shared_ptr<CInflection> spInflection,
                                            bool bYoAlternation, 
                                            const CEString& sStem,
                                            ET_Subparadigm eSubparadigm,
                                            ET_AccentType eAccentType1,
                                            ET_AccentType eAccentType2,
                                            vector<int>&& vecStressPos,
                                            bool bFleetingVowel,
                                            int iDeviation,
                                            ET_Status eStatus) : 
    CFormBuilderDecl(spLexeme, spInflection, eSubparadigm, bFleetingVowel, bYoAlternation), 
    m_sStem(sStem),
    m_iType (1),
    m_iVerbDeviation(iDeviation),
    m_vecStressPos (vecStressPos),
    m_eAccentType1 (eAccentType1),
    m_eAccentType2 (eAccentType2),
    m_bFleetingVowel (bFleetingVowel),
    m_eStatus (eStatus)
{
    if (iDeviation > 0)
    {
        if (iDeviation != 2)
        {
            assert(0);
            CEString sMsg (L"Unexpected common deviation in short participle.");
            ERROR_LOG (sMsg);
            throw CException (H_CTOR, sMsg);
        }
    }
}

ET_ReturnCode CFormBuilderShortAdj::eGetStressTypes (ET_Number eNumber, 
                                                     ET_Gender eGender, 
                                                     vector<ET_StressLocation>& vecStressType)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    if (NUM_PL == eNumber && GENDER_UNDEFINED != eGender)
    {
        assert(0);
        ERROR_LOG (L"Unexpected gender/number values.");
        return H_ERROR_INVALID_ARG;
    }

    if (NUM_SG == eNumber && GENDER_UNDEFINED == eGender)
    {
        assert(0);
        ERROR_LOG (L"Unexpected gender/number values.");
        return H_ERROR_INVALID_ARG;
    }

//    if (GENDER_M == eGender)
//    {
//        assert (NUM_SG == eNumber);
//        vecStressType.push_back (STRESS_LOCATION_STEM);
//        return H_NO_ERROR;
//    }

    //
    // Exception: part past passive short ending in stressed -Annyj/-jAnnyj;
    // see GDRL p. 86 footnote 4
    //
    if (SUBPARADIGM_PART_PAST_PASS_SHORT == m_eSubparadigm)
    {
        CGramHasher hasher (POS_VERB, SUBPARADIGM_PART_PAST_PASS_LONG, CASE_NOM, NUM_SG, 
                            GENDER_M, PERSON_UNDEFINED, ANIM_NO, m_spLexeme->eAspect(), 
                            m_spLexeme->eIsReflexive());
        shared_ptr<CWordForm> spNSgMLong;
        rc = m_spInflection->eWordFormFromHash (hasher.sGramHash(), 0, spNSgMLong);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }
        if (NULL == spNSgMLong)
        {
            assert(0);
            ERROR_LOG (L"Failed to obtain N Sg m of the long form.");
            return H_ERROR_POINTER;
        }
        
        CEString sNSgMLong (spNSgMLong->m_sWordForm);
        if (sNSgMLong.bEndsWith (L"анный") || sNSgMLong.bEndsWith (L"янный") ||
            sNSgMLong.bEndsWith (L"енный"))
        {
            map<int, ET_StressType>::iterator itStress = spNSgMLong->m_mapStress.begin();
            for (; itStress != spNSgMLong->m_mapStress.end(); ++itStress)
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
            assert(0);
            ERROR_LOG (L"Undefined accent type.");
            return H_ERROR_GENERAL;
        }
        case AT_A:
        {
            vecStressType.push_back (STRESS_LOCATION_STEM);

            // Type sorvana: GDRL, p. 86
            if (SUBPARADIGM_PART_PAST_PASS_SHORT == m_eSubparadigm && 
                GENDER_F == eGender && 
                AT_C == m_spInflection->eAccentType2())
            {
                vecStressType.push_back (STRESS_LOCATION_ENDING);
            }

            return H_NO_ERROR;
        }
        case AT_A1:
        {
            if (GENDER_M == eGender)
            {
                assert(NUM_SG == eNumber);
                vecStressType.push_back(STRESS_LOCATION_STEM);
                return H_NO_ERROR;
            }
            if (GENDER_F == eGender)
            {
                assert (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return H_NO_ERROR;
            }
            if (GENDER_N == eGender)
            {
                assert (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                return H_NO_ERROR;
            }
            if (NUM_PL == eNumber)
            {
                vecStressType.push_back (STRESS_LOCATION_STEM);
                return H_NO_ERROR;
            }

            assert(0);
            ERROR_LOG (L"Bad arguments.");
            return H_ERROR_INVALID_ARG;
        }
        case AT_B:
        {
            vecStressType.push_back (STRESS_LOCATION_ENDING);
            return H_NO_ERROR;
        }
        case AT_B1:
        {
            if (GENDER_M == eGender)
            {
                assert(NUM_SG == eNumber);
                vecStressType.push_back(STRESS_LOCATION_ENDING);
                return H_NO_ERROR;
            }
            if (GENDER_F == eGender || GENDER_N == eGender)
            {
                assert (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return H_NO_ERROR;
            }
            if (NUM_PL == eNumber)
            {
                assert (GENDER_UNDEFINED == eGender);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return H_NO_ERROR;
            }

            assert(0);
            ERROR_LOG (L"Bad arguments.");
            return H_ERROR_INVALID_ARG;
        }
        case AT_C:
        {
            if (GENDER_M == eGender)
            {
                assert(NUM_SG == eNumber);
                vecStressType.push_back(STRESS_LOCATION_STEM);
                return H_NO_ERROR;
            }
            if (GENDER_F == eGender)
            {
                assert (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return H_NO_ERROR;
            }
            if (GENDER_N == eGender || NUM_PL == eNumber)
            {
                vecStressType.push_back (STRESS_LOCATION_STEM);
                return H_NO_ERROR;
            }

            assert(0);
            ERROR_LOG (L"Bad arguments.");
            return H_ERROR_INVALID_ARG;
        }
        case AT_C1:
        {
            if (GENDER_M == eGender)
            {
                assert(NUM_SG == eNumber);
                vecStressType.push_back(STRESS_LOCATION_STEM);
                return H_NO_ERROR;
            }
            if (GENDER_F == eGender)
            {
                assert (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return H_NO_ERROR;
            }
            if (GENDER_N == eGender)
            {
                assert (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                return H_NO_ERROR;
            }
            if (NUM_PL == eNumber)
            {
                assert (GENDER_UNDEFINED == eGender);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return H_NO_ERROR;
            }

            assert(0);
            ERROR_LOG (L"Bad arguments.");
            return H_ERROR_INVALID_ARG;
        }
        case AT_C2:
        {
            if (GENDER_M == eGender)
            {
                assert(NUM_SG == eNumber);
                vecStressType.push_back(STRESS_LOCATION_STEM);
                return H_NO_ERROR;
            }
            if (GENDER_F == eGender)
            {
                assert (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return H_NO_ERROR;
            }
            if (GENDER_N == eGender)
            {
                assert (NUM_SG == eNumber);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return H_NO_ERROR;
            }
            if (NUM_PL == eNumber)
            {
                assert (GENDER_UNDEFINED == eGender);
                vecStressType.push_back (STRESS_LOCATION_STEM);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
                return H_NO_ERROR;
            }

            assert(0);
            ERROR_LOG (L"Bad arguments.");
            return H_ERROR_INVALID_ARG;
        }
        default:
        {
            assert(0);
            ERROR_LOG (L"Illegal accent type.");
            return H_ERROR_INVALID_ARG;
        }
    }

    return H_ERROR_INVALID_ARG;

}   // eGetStressType()

ET_ReturnCode CFormBuilderShortAdj::eCreateFormTemplate (const CEString& sStem,
                                                         const CEString& sEnding,
                                                         int64_t llEndingKey,
                                                         ET_Gender eGender,
                                                         ET_Number eNumber,
                                                         shared_ptr<CWordForm>& spWordForm)
{
    ET_ReturnCode rc = H_NO_ERROR;

    spWordForm = make_shared<CWordForm>();
    if (nullptr == spWordForm)
    {
        assert(0);
        ERROR_LOG (L"Unable to create an instance of CWordForm");
        return H_ERROR_POINTER;
    }

    spWordForm->m_spLexeme = m_spLexeme;
    spWordForm->m_ePos = m_spLexeme->ePartOfSpeech();
    spWordForm->m_eSubparadigm = m_eSubparadigm;
    spWordForm->m_sStem = sStem;
    spWordForm->m_sEnding = sEnding;
    spWordForm->m_llEndingDataId = llEndingKey;
    spWordForm->m_sWordForm = sStem + sEnding;
    spWordForm->m_eGender = eGender;
    spWordForm->m_eNumber = eNumber;
    spWordForm->m_eStatus = m_eStatus;
    if (POS_VERB == m_spLexeme->ePartOfSpeech())
    {
        spWordForm->m_eAspect = m_spLexeme->eAspect();
        spWordForm->m_eReflexivity = m_spLexeme->eIsReflexive();
    }
    spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();

//    rc = eAssignSecondaryStress (pWordForm);

    return rc;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderShortAdj::eHandleDeviations(shared_ptr<CWordForm> spWordForm)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

//    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        //
        // A. Common deviations defined for adjectives (1-2)
        //
        bool bHasCD = false;
        bool bOptionalCD = false;
        int iCd = -1;
        if (SUBPARADIGM_SHORT_ADJ == m_eSubparadigm)
        {
            for (iCd = 1; iCd <= 2; ++iCd)
            {
                if (m_spInflection->bFindCommonDeviation(iCd, bOptionalCD))
                {
                    bHasCD = true;
                    break;
                }
            }
        }

        //
        // Only adjectives in -nnyj/-nnij or participia
        //
        if (bHasCD || m_iVerbDeviation > 0)
        {
            if (bOptionalCD && SUBPARADIGM_SHORT_ADJ == m_eSubparadigm)   // store both forms
            {
                shared_ptr<CWordForm> spMVariant;
                CloneWordForm(spWordForm, spMVariant);
                m_spInflection->AddWordForm(spMVariant);
                spWordForm = spMVariant;
            }

            if (GENDER_M == spWordForm->m_eGender)
            {
                spWordForm->m_sWordForm.sErase(spWordForm->m_sWordForm.uiLength() - 1);
                if (m_bFleetingVowel)
                {
                    spWordForm->m_sWordForm.sErase(spWordForm->m_sWordForm.uiLength() - 1);
                }
                auto nSyll = spWordForm->m_sWordForm.uiNSyllables();
                if (spWordForm->m_mapStress.find(nSyll) != spWordForm->m_mapStress.end())
                {
                    if (spWordForm->m_mapStress[nSyll] == STRESS_PRIMARY)
                    {
                        spWordForm->m_mapStress.erase(nSyll);
                        spWordForm->m_mapStress[nSyll - 1] = STRESS_PRIMARY;
                    }
                }

            }
            else if (2 == iCd || (2 == m_iVerbDeviation && SUBPARADIGM_PART_PAST_PASS_SHORT == m_eSubparadigm))
            {
                spWordForm->m_sWordForm.sErase(spWordForm->m_sWordForm.uiLength() - 2, 1);
            }
        }

        //
        // B. Common deviations defined for verbs (7-8)
        //
        if (m_spInflection->bHasCommonDeviation(7))
        {
            iCd = 7;
        }
        else if (m_spInflection->bHasCommonDeviation(8))  // TODO -- what's that??
        {
            iCd = -1;
        }
        else
        {
            iCd = -1;
        }

        if (iCd > 0 && SUBPARADIGM_PART_PAST_PASS_SHORT == m_eSubparadigm)
        {
            if (m_spInflection->bDeviationOptional(iCd))   // store both forms
            {
                shared_ptr<CWordForm> spMVariant;
                CloneWordForm (spWordForm, spMVariant);
                m_spInflection->AddWordForm(spMVariant);
                spWordForm = spMVariant;
            }

            map<int, ET_StressType> mapCorrectedStress;
            auto itStressPos = spWordForm->m_mapStress.begin();
            for (; itStressPos != spWordForm->m_mapStress.end(); ++itStressPos)
            {
                if (!(*itStressPos).second)
                {
                    mapCorrectedStress[(*itStressPos).first] = STRESS_SECONDARY;
                    continue;
                }
                if ((*itStressPos).first < 1)
                {
                    assert(0);
                    ERROR_LOG (L"Unexpected stress position in cd-7 or cd-8 participle.");
                    return H_ERROR_UNEXPECTED;
                }
                CEString sWf (spWordForm->m_sWordForm);
                mapCorrectedStress[sWf.uiNSyllables()-1] = STRESS_PRIMARY;
            }
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

}   //  eHandleDeviations (...)

ET_ReturnCode CFormBuilderShortAdj::eBuild()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
//        if (m_pLexeme->ePartOfSpeech() != POS_ADJ)
//        {
//            return H_NO_ERROR;     // Only adjectives can have short forms
//        }

        if (m_spLexeme->sSourceForm().bEndsWith(L"ийся"))
        {
            return H_NO_ERROR;
        }

        m_spEndings = make_shared<CAdjShortEndings>(m_spLexeme, m_spInflection, m_eSubparadigm);
        if (nullptr == m_spEndings)
        {
            return H_ERROR_POINTER;
        }

        for (ET_Gender eGender = GENDER_UNDEFINED; eGender <= GENDER_N; ++eGender)
        {
            if (GENDER_M == eGender)
            {
                if (m_spInflection->bShortFormsIncomplete())
                {
                    continue;
                }
            }

            ET_Number eNumber = (GENDER_UNDEFINED == eGender) ? NUM_PL : NUM_SG;
            CGramHasher hasher (SUBPARADIGM_SHORT_ADJ, eNumber, eGender, ANIM_UNDEFINED, CASE_UNDEFINED);

            if (m_spLexeme->bHasMissingForms() && H_TRUE != m_spInflection->eFormExists(hasher.sGramHash()))
            {
                continue;
            }

            bool bNoRegularForms = false;

            map<shared_ptr<CWordForm>, bool> mapIrreg;
            rc = m_spInflection->eGetIrregularForms(hasher.sGramHash(), mapIrreg);
            if (rc != H_NO_ERROR)
            {
                continue;
            }

            auto it = mapIrreg.begin();
            if (!mapIrreg.empty())
            {
                bNoRegularForms = true;
            }

            for (; it != mapIrreg.end(); ++it)
            {
//                if ((*it).second)   // optional?
//                {
//                    bNoRegularForms = false;          // TODO: this DOES NOT indicate that a regular variant exists
//                }

                auto spWordForm = make_shared<CWordForm>((*it).first);
                m_spInflection->AddWordForm(spWordForm);
            }

            if (bNoRegularForms)
            {
                continue;
            }

            if (m_spInflection->bShortFormsIncomplete()) // masc should be skipped altogether, see above
            {
                m_eStatus = STATUS_QUESTIONABLE;
            }
            if (m_spInflection->bShortFormsRestricted())
            {
                m_eStatus = STATUS_QUESTIONABLE;
            }
            if (m_spLexeme->bAssumedForms())
            {
                m_eStatus = STATUS_ASSUMED;
            }

            vector<ET_StressLocation> vecStressType;
            rc = eGetStressTypes(eNumber, eGender, vecStressType);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }

            vector<ET_StressLocation>::iterator itStressType = vecStressType.begin();
            for (; itStressType != vecStressType.end(); ++itStressType)
            {                
                static_pointer_cast<CAdjShortEndings>(m_spEndings)->eSelect(eGender, eNumber, *itStressType);

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

                    CEString sStem(m_sStem);
                    rc = eFleetingVowelCheck (eNumber, CASE_UNDEFINED, eGender, *itStressType, m_eSubparadigm, sEnding, sStem);
                    if (rc != H_NO_ERROR)
                    {
                        return rc;
                    }

                    vector<int> vecStressPos;
                    if (STRESS_LOCATION_STEM == *itStressType)
                    {
                        if (SUBPARADIGM_SHORT_ADJ == m_eSubparadigm)
                        {
                            rc = eGetStemStressPositions (sStem, SUBPARADIGM_SHORT_ADJ, NUM_UNDEFINED, vecStressPos);
                                                                                        // Number not needed for this subparadigm
                            if (rc != H_NO_ERROR)
                            {
                                return rc;
                            }

                            // Correction for M.Sg. with a fleeting vowel, p. 33
                            if (sEnding.uiNSyllables() < 1)
                            {
                                assert(GENDER_M == eGender);
                                if (m_spInflection->bHasFleetingVowel() && m_spInflection->eAccentType2() != AT_B && m_bFleetingVowelPresent)
                                {
                                    for (auto iAt = 0; iAt < (int)vecStressPos.size(); ++iAt)
                                    {
                                        if ((sStem.uiNSyllables() - 1) == vecStressPos.at(iAt))
                                        {
                                            try {
                                                --vecStressPos.at(iAt);
                                            }
                                            catch (const out_of_range& oor) {
                                                assert(0);
                                                CEString sMsg(L"Stress position item out of range: ");
                                                sMsg += CEString::sFromUtf8(oor.what());
                                                ERROR_LOG(sMsg);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            vecStressPos = m_vecStressPos; // assigned in ctor
                        }
                    }
                    else if (STRESS_LOCATION_ENDING == *itStressType)
                    {
                        int iStressPos = -1;
                        eGetEndingStressPosition(sStem, sEnding, iStressPos);
                        if (rc != H_NO_ERROR)
                        {
                            return rc;
                        }

                        vecStressPos.push_back (iStressPos);
                    }
                    else
                    {
                        assert(0);
                        ERROR_LOG (L"Illegal stress type.");
                        return H_ERROR_GENERAL;
                    }

                    shared_ptr<CWordForm> spWordForm;
                    rc = eCreateFormTemplate(sStem, sEnding, llEndingKey, eGender, eNumber, spWordForm);
                    if (rc != H_NO_ERROR)
                    {
                        return rc;
                    }                   

//                    long iGramHash = pWordForm->i_GramHash();
                    if (1 == vecStressPos.size() || m_spInflection->bIsMultistressedCompound())
                    {
                        vector<int>::iterator itStressPos = vecStressPos.begin();
                        for (; itStressPos != vecStressPos.end(); ++itStressPos)
                        {
                            spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                            rc = eHandleYoAlternation(*itStressType, *itStressPos, sStem, sEnding);
                            if (rc != H_NO_ERROR)
                            {
                                return rc;
                            }
                        }
                        spWordForm->m_sWordForm = sStem + sEnding;
                        rc = eHandleDeviations(spWordForm);
                        if (rc != H_NO_ERROR)
                        {
                            return rc;
                        }
                        
                        m_spInflection->AddWordForm(spWordForm);
                    }
                    else
                    {
                        vector<int>::iterator itStressPos = vecStressPos.begin();
                        for (; itStressPos != vecStressPos.end(); ++itStressPos)
                        {
                            if (itStressPos != vecStressPos.begin())
                            {
                                shared_ptr<CWordForm> spWfVariant;
                                CloneWordForm(spWordForm, spWfVariant);
                                spWordForm = spWfVariant;
                            }
                            spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                            rc = eHandleYoAlternation(*itStressType, *itStressPos, sStem, sEnding);
                            if (rc != H_NO_ERROR)
                            {
                                return rc;
                            }

                            spWordForm->m_sWordForm = sStem + sEnding;
                            rc = eHandleDeviations (spWordForm);
                            if (rc != H_NO_ERROR)
                            {
                                return rc;
                            }

                            m_spInflection->AddWordForm(spWordForm);
                        }
                    }

                }   //  for (int iEnding = 0; iEnding < iNumEndings; ++iEnding)

            }   //  for (; itStress != vec_eoStress.end(); ++itStress)

        }   //  for (ET_Gender eGender = GENDER_UNDEFINED; ...
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;  // logging should be always done by callee
    }

    return H_NO_ERROR;

}   //  eBuild()
