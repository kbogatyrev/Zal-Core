#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "Endings.h"
#include "WordForm.h"
#include "Lexeme.h"
#include "FormBuilderNouns.h"

using namespace Hlib;

CEString CFormBuilderNouns::sGramHash (ET_Gender eoGender, ET_Animacy eAnimacy, ET_Case eCase, ET_Number eNumber)
{
    CGramHasher hasher (eoGender, eAnimacy, eCase, eNumber);
    CEString sHash = hasher.sGramHash();
    return sHash;
}

ET_ReturnCode CFormBuilderNouns::eHandleStemAugment (CEString& sStem, ET_Number eNumber, ET_Case eCase)
{
    assert(m_pLexeme);   // we assume base class ctor took care of this

    if (m_pInflection->iStemAugment() < 1)
    {
        return H_NO_ERROR;
    }

    if (1 == m_pInflection->iType())
    {
        sStem.sErase (sStem.uiLength()-2, 2);  // римлянин, южанин, армянин
    }
    if (3 == m_pInflection->iType())
    {
        const CEString& sGs = m_pLexeme->sGraphicStem();
        if (NUM_SG == eNumber)
        {
            if ((CASE_NOM == eCase) || 
                (ANIM_NO == m_pLexeme->eInflectionTypeToAnimacy() && CASE_ACC == eCase))
            {
                return H_NO_ERROR;
            }
            else
            {
                sStem.sErase (sStem.uiLength()-2, 1);
                return H_NO_ERROR;
            }
        }
        if (NUM_PL == eNumber)
        {
            if (sGs.bEndsWith (L"онок"))
            {
                sStem.sErase (sStem.uiLength()-4, 4);
                sStem += L"ат";
                return H_NO_ERROR;
            }
            if (sGs.bEndsWith (L"ёнок"))
            {
                sStem.sErase (sStem.uiLength()-4, 4);
                sStem += L"ят";
                return H_NO_ERROR;
            }
            if (sGs.bEndsWith (L"оночек"))
            {
                sStem.sErase (sStem.uiLength()-6, 6);
                if ((CASE_GEN == eCase) ||                        
                    (CASE_ACC == eCase && ANIM_YES == m_pLexeme->eInflectionTypeToAnimacy()))
                                                     // they all should be animate?
                {
                    sStem += L"аток";
                }
                else
                {
                    sStem += L"атк";
                }
                return H_NO_ERROR;
            }
            if (sGs.bEndsWith (L"ёночек"))
            {
                sStem.sErase (sStem.uiLength()-6, 6);
                if ((CASE_GEN == eCase) ||                        
                    (CASE_ACC == eCase && ANIM_YES == m_pLexeme->eInflectionTypeToAnimacy()))
                                                     // they all should be animate?
                {
                    sStem += L"яток";
                }
                else
                {
                    sStem += L"ятк";
                }
                return H_NO_ERROR;
            }
        }
    }       //  if (3 == m_pLexeme->i_Type)
    if (8 == m_pInflection->iType())
    {
        if (NUM_SG == eNumber)
        {
            if ((CASE_NOM == eCase) || 
                (ANIM_NO == m_pLexeme->eInflectionTypeToAnimacy() && CASE_ACC == eCase))
            {
                return H_NO_ERROR;
            }
            else
            {
                sStem += L"ен";
            }
        }
        else
        {
            sStem += L"ен";
        }
    }

    return H_NO_ERROR;

}   //  eHandleStemAugment (...)

ET_ReturnCode CFormBuilderNouns::eGetStressType (ET_Number eNumber, ET_Case eCase, ET_StressLocation& eStressType)
{
    assert(m_pLexeme);   // we assume base class ctor took care of this

    if (NUM_UNDEFINED == eNumber)
    {
        assert(0);
        ERROR_LOG (L"Undefined number.");
        return H_ERROR_INVALID_ARG;
    }

    if (CASE_LOC == eCase)
    {
        eStressType = STRESS_LOCATION_ENDING;
        return H_NO_ERROR;
    }

    eStressType = STRESS_LOCATION_STEM;

    if (CASE_PART == eCase)
    {
        eCase = CASE_DAT;
    }

    switch (m_pInflection->eAccentType1())
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
        case AT_B1:
        {
            if (NUM_SG == eNumber && CASE_INST == eCase)
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else
            {
                eStressType = STRESS_LOCATION_ENDING;
            }
            break;
        }
        case AT_C:
        {
            if (NUM_SG == eNumber)
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else
            {
                eStressType = STRESS_LOCATION_ENDING;
            }
            break;
        }
        case AT_D:
        {
            if (NUM_SG == eNumber)
            {
                eStressType = STRESS_LOCATION_ENDING;
            }
            else
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            break;
        }
        case AT_D1:
        {
            if (GENDER_F != m_pLexeme->eInflectionTypeToGender())
            {
                assert(0);
                CEString sMsg (L"Non-feminine noun with D1 stress.");
                ERROR_LOG (sMsg);
                throw CException (H_ERROR_GENERAL, sMsg);
            }
            if (NUM_SG == eNumber && CASE_ACC == eCase) // assume fem -a stems only?
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else if (NUM_SG == eNumber)
            {
                eStressType = STRESS_LOCATION_ENDING;
            }
            else
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            break;
        }
        case AT_E:
        {
            if (NUM_SG == eNumber)
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else if (CASE_NOM == eCase)
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else if (CASE_ACC == eCase && ANIM_NO == m_pLexeme->eInflectionTypeToAnimacy())
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else
            {
                eStressType = STRESS_LOCATION_ENDING;
            }
            break;
        }

        case AT_F:
        {
            if (NUM_SG == eNumber)
            {
                eStressType = STRESS_LOCATION_ENDING;
            }
            else if (CASE_NOM == eCase)
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else if (CASE_ACC == eCase && ANIM_NO == m_pLexeme->eInflectionTypeToAnimacy())
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else
            {
                eStressType = STRESS_LOCATION_ENDING;
            }
            break;
        }
        case AT_F1:
        {
            if (GENDER_F != m_pLexeme->eInflectionTypeToGender())
            {
                assert(0);
                CEString sMsg (L"Non-feminine noun with F1 stress.");
                ERROR_LOG (sMsg);
                throw CException (H_ERROR_GENERAL, sMsg);
            }
            if (NUM_PL == eNumber && CASE_NOM == eCase)
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else if (NUM_SG == eNumber && CASE_ACC == eCase) // fem only?
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else
            {
                eStressType = STRESS_LOCATION_ENDING;
            }
            break;
        }
        case AT_F2:
        {
            if (GENDER_F != m_pLexeme->eInflectionTypeToGender() || 8 != m_pInflection->iType())
            {
                assert(0);   // assume f -i stems only?
                CEString sMsg (L"Non-feminine/type 8 noun with F2 stress.");
                ERROR_LOG (sMsg);
                throw CException (H_EXCEPTION, sMsg);
            }
            if (ANIM_YES == m_pLexeme->eInflectionTypeToAnimacy())
            {
                assert(0);                   // inanimate only?
                CEString sMsg (L"Animate noun with F2 stress.");
                ERROR_LOG (sMsg);
                throw CException (H_EXCEPTION, sMsg);
            }
            if (NUM_PL == eNumber && (CASE_NOM == eCase || CASE_ACC == eCase))
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else if (NUM_SG == eNumber && CASE_INST == eCase)
            {
                eStressType = STRESS_LOCATION_STEM;
            }
            else
            {
                eStressType = STRESS_LOCATION_ENDING;
            }
            break;
        }
        case AT_UNDEFINED:
        {
            if (m_pInflection->iType() != 0)
            {
//                assert(0);
                CEString sMsg(L"Illegal accent type; lexeme = ");
                ERROR_LOG(sMsg + m_pLexeme->sSourceForm());
                return H_ERROR_GENERAL;
            }
            break;
        }
        default:
        {
            assert(0);
            CEString sMsg (L"Illegal accent type.");
            ERROR_LOG (L"Illegal accent type.");
            return H_ERROR_GENERAL;
        }
    }

    return H_NO_ERROR;

}   // eGetStressType()

ET_ReturnCode CFormBuilderNouns::eHandleAccEnding (ET_Number eNumber, ET_Case& eCase, ET_Case& eAltCase)
{
    assert(m_pLexeme);   // we assume base class ctor took care of this

    eAltCase = ET_Case::CASE_UNDEFINED;

    if (NUM_SG == eNumber)
    {
        if (GENDER_M == m_pLexeme->eInflectionTypeToGender())
        {
            ANIM_YES == m_pLexeme->eInflectionTypeToAnimacy()
                ? eCase = CASE_GEN
                : eCase = CASE_NOM;

            if ((L"мо" == m_pLexeme->sMainSymbol() && L"м" == m_pLexeme->sAltMainSymbol()) ||
                (L"м" == m_pLexeme->sMainSymbol() && L"мо" == m_pLexeme->sAltMainSymbol()))
            { 
                bool bAltMainSymbol(true);
                ANIM_YES == m_pLexeme->eInflectionTypeToAnimacy(bAltMainSymbol)
                    ? eAltCase = CASE_GEN
                    : eAltCase = CASE_NOM;
            }

        }
        if (GENDER_N == m_pLexeme->eInflectionTypeToGender())
        {
            auto stProperties = m_pLexeme->stGetProperties();
            if (L"со" == stProperties.sInflectionType)
            {
                eCase = CASE_GEN;
            }
            else
            {
                eCase = CASE_NOM;
            }
        }
    }
    if (NUM_PL == eNumber)
    {
        ANIM_YES == m_pLexeme->eInflectionTypeToAnimacy()
            ? eCase = CASE_GEN
            : eCase = CASE_NOM;

        if ((L"мо" == m_pLexeme->sMainSymbol() && L"м" == m_pLexeme->sAltMainSymbol()) ||
            (L"м" == m_pLexeme->sMainSymbol() && L"мо" == m_pLexeme->sAltMainSymbol()) ||
            (L"жо" == m_pLexeme->sMainSymbol() && L"ж" == m_pLexeme->sAltMainSymbol()) ||
            (L"ж" == m_pLexeme->sMainSymbol() && L"жо" == m_pLexeme->sAltMainSymbol()) ||
            (L"со" == m_pLexeme->sMainSymbol() && L"с" == m_pLexeme->sAltMainSymbol()) ||
            (L"с" == m_pLexeme->sMainSymbol() && L"со" == m_pLexeme->sAltMainSymbol()))
        {
            bool bAltMainSymbol(true);
            ANIM_YES == m_pLexeme->eInflectionTypeToAnimacy(bAltMainSymbol)
                ? eAltCase = CASE_GEN
                : eAltCase = CASE_NOM;
        }
    }

    return H_NO_ERROR;

}   //  eHandleAccEnding

ET_ReturnCode CFormBuilderNouns::eGetStressPositions (const CEString& sStem,
                                                      const CEString& sEnding,
                                                      ET_Number eNumber,
                                                      ET_StressLocation eStressType,
                                                      vector<int>& vecStressPos)
{
    assert(m_pLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    int iStressPos = -1;
    if (STRESS_LOCATION_STEM == eStressType)
    {
        rc = eGetStemStressPositions (sStem, SUBPARADIGM_NOUN, eNumber, vecStressPos);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }
    }
    else if (STRESS_LOCATION_ENDING == eStressType)
    {
        ET_ReturnCode rc = eGetEndingStressPosition (sStem, sEnding, iStressPos);
        if (rc != H_NO_ERROR)
        {
            assert(0);
            ERROR_LOG (L"Bad stress type.");
            return H_ERROR_GENERAL;
        }
        vecStressPos.push_back (iStressPos);
    }
    else
    {
        assert(0);
        ERROR_LOG (L"Bad stress type.");
        return H_ERROR_GENERAL;
    }

    if (vecStressPos.empty())
    {
        vecStressPos.push_back (-1);
    }

    return H_NO_ERROR;

}   //  eGetStressPositions (...)

ET_ReturnCode CFormBuilderNouns::eCreateFormTemplate (ET_Number eNumber, 
                                                      ET_Case eCase, 
                                                      const CEString& sStem,
                                                      shared_ptr<CWordForm>& spWordForm)
{
    spWordForm = make_shared<CWordForm>(m_pInflection);
    if (nullptr == spWordForm)
    {
        assert(0);
        ERROR_LOG (L"Unable to instantiate CWordForm.");
        return H_ERROR_POINTER;
    }

//    spWordForm-> m_pLexeme = m_pLexeme;
    spWordForm->SetInflection(m_pInflection);
    spWordForm->SetInflectionId(m_pInflection->llInflectionId());
    spWordForm->SetPos(POS_NOUN);
    spWordForm->SetSubparadigm(SUBPARADIGM_NOUN);
    spWordForm->SetCase(eCase);
    spWordForm->SetStem(sStem);
    spWordForm->SetNumber(eNumber);
    spWordForm->SetGender(m_pInflection->spLexeme()->eInflectionTypeToGender());
    spWordForm->SetAnimacy(m_pInflection->spLexeme()->eInflectionTypeToAnimacy());
    if (NUM_PL == eNumber && m_pLexeme->bAssumedForms())
    {
        spWordForm->SetStatus(STATUS_ASSUMED);
    }
//    spWordForm->m_llLexemeId = m_pLexeme->llLexemeId();
    //rc = eAssignSecondaryStress (pWordForm);
    //if (rc != H_NO_ERROR)
    //{
    //    return rc;
    //}

    return H_NO_ERROR;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderNouns::eCheckIrregularForms (ET_Gender eoGender, 
                                                       ET_Animacy eAnimacy, 
                                                       ET_Case eCase, 
                                                       ET_Case eEndingsCase,
                                                       ET_Number eNumber, 
                                                       bool& bHandled)
{
    assert(m_pLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    if (!m_pLexeme->bHasIrregularForms())
    {
        return H_FALSE;
    }

//    if (m_pLexeme->iInflectedParts() == 2 && m_pLexeme->bIsSecondPart())
//    {
//        return H_NO_ERROR;     // both parts are kept together, no need to repeat
//    }

    bHandled = false;

    CEString sHash = sGramHash (eoGender, eAnimacy, eEndingsCase, eNumber);

    map<CWordForm*, bool> mapIrreg;
    rc = m_pInflection->eGetIrregularForms(sHash, mapIrreg);
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

        shared_ptr<CWordForm> spWordForm = make_shared<CWordForm>();
        spWordForm->Copy(*it->first);
        spWordForm->SetCase(eCase);   // ending case may differ from actual case, e.g. A.Sg.
        m_pInflection->AddWordForm(spWordForm);
    }

    return H_NO_ERROR;

}   //  eCheckIrregularForms (...)

ET_ReturnCode CFormBuilderNouns::eBuild()
{
    assert(m_pLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    m_spEndings = make_shared<CNounEndings>(m_pLexeme, m_pInflection);
    if (nullptr == m_spEndings)
    {
        return H_ERROR_POINTER;
    }

    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    ET_Animacy eAnimacy = m_pLexeme->eInflectionTypeToAnimacy();
    ET_Gender eoGender = m_pLexeme->eInflectionTypeToGender();

    CHasher gramIterator;
    gramIterator.Initialize(eoGender, eAnimacy);
    do
    {
        if ((L"мн." == m_pLexeme->sMainSymbol() || m_pLexeme->bIsPluralOf()) && gramIterator.m_eNumber == NUM_SG)
        {
            continue;
        }

        if (m_pLexeme->bIsPluralOf() && gramIterator.m_eNumber == NUM_SG)
        {
            continue;
        }

        if (NUM_PL == gramIterator.m_eNumber && 
            (CASE_PART == gramIterator.m_eCase || CASE_LOC == gramIterator.m_eCase))
        {
            continue;
        }

        if (CASE_PART == gramIterator.m_eCase && !m_pLexeme->bSecondGenitive())
        {
            continue;
        }

        if (CASE_LOC == gramIterator.m_eCase && !m_pLexeme->bSecondPrepositional())
        {
            continue;
        }

        if (m_pLexeme->bHasMissingForms() && H_TRUE != m_pInflection->eFormExists(gramIterator.sGramHash()))
        {
            continue;
        }

        // Handle acc ending
        ET_Case eEndingCase = gramIterator.m_eCase;
        ET_Case eAltEndingCase = ET_Case::CASE_UNDEFINED;
        if (CASE_ACC == gramIterator.m_eCase)
        {
            rc = eHandleAccEnding (gramIterator.m_eNumber, eEndingCase, eAltEndingCase);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
        }
        if (CASE_PART == gramIterator.m_eCase || CASE_LOC == gramIterator.m_eCase)
        {
            eEndingCase = CASE_DAT;
        }

        auto sFormHash = sGramHash(gramIterator.m_eGender, gramIterator.m_eAnimacy, gramIterator.m_eCase, gramIterator.m_eNumber);        
        auto sEndingHash = sGramHash(gramIterator.m_eGender, gramIterator.m_eAnimacy, eEndingCase, gramIterator.m_eNumber);
        if (sEndingHash != sFormHash)
        {
            if (m_pLexeme->bHasMissingForms() && H_TRUE != m_pInflection->eFormExists(sEndingHash))
            {
                m_pInflection->eSetFormExists(sFormHash, false);
                continue;
            }

            if (m_pLexeme->bHasDifficultForms() && H_TRUE == m_pInflection->eIsFormDifficult(sEndingHash))
            {
                m_pInflection->eSetFormDifficult(sFormHash, true);
                //            continue;
            }
        }

        CEString sStem(m_pLexeme->sGraphicStem());
        if (m_pLexeme->bHasIrregularForms())
        {
            bool bSkipRegular = false;
            rc = eCheckIrregularForms (gramIterator.m_eGender, 
                                       gramIterator.m_eAnimacy,
                                       gramIterator.m_eCase,
                                       eEndingCase,
                                       gramIterator.m_eNumber,
                                       bSkipRegular);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
            if (bSkipRegular)
            {
                // Workaround for lack of "исх. форма иррег." mark in current source
                if (GENDER_M == gramIterator.m_eGender && NUM_SG == gramIterator.m_eNumber && CASE_NOM == gramIterator.m_eCase)
                {
                    m_bIrregularSourceForm = true;
                }
                continue;
            }
        }

        if (m_pInflection->iStemAugment() > 0)
        {
            rc = eHandleStemAugment(sStem, gramIterator.m_eNumber, gramIterator.m_eCase);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
        }

        ET_StressLocation eStress = STRESS_LOCATION_UNDEFINED;
        if (CASE_LOC == gramIterator.m_eCase)
        {
            eStress = STRESS_LOCATION_ENDING;
        }
        else
        {
            rc = eGetStressType (gramIterator.m_eNumber, eEndingCase, eStress);
            if (rc != H_NO_ERROR)
            {
                //return rc;
                continue;       // there is a legit case when a lexeme has no regular forms
            }
        }

        bool bLoop = true;
        while (bLoop)
        {
            static_pointer_cast<CNounEndings>(m_spEndings)->eSelect(gramIterator.m_eNumber, eEndingCase, eStress);
            int64_t iNumEndings = m_spEndings->iCount();
            if (iNumEndings < 1)
            {
                if (m_pInflection->iType() == 0)
                {
                    vector<int> vecStress;
                    rc = eGetStressPositions(sStem, L"", gramIterator.m_eNumber, eStress, vecStress);
                    if (rc != H_NO_ERROR)
                    {
                        bLoop = false;
                        continue;
                    }

                    shared_ptr<CWordForm> spWordForm;
                    rc = eCreateFormTemplate(gramIterator.m_eNumber, gramIterator.m_eCase, sStem, spWordForm);
                    if (rc != H_NO_ERROR)
                    {
                        bLoop = false;
                        continue;
                    }

                    eStress = ET_StressLocation::STRESS_LOCATION_STEM;
                    int64_t llEndingKey = -1;
//                    sStem = m_pLexeme->sSourceForm();
                    CEString sEnding;
                    CreateWordForm(vecStress, eStress, sStem, sEnding, llEndingKey, spWordForm);
                }
                else
                {
                    //                assert(0);
                    //                ERROR_LOG(L"No endings");
                }
                bLoop = false;
                continue;
            }

            CEString sSavedStem(sStem);      // stem can change, e.g. because of a fleetimg vowel
            for (int iEnding = 0; iEnding < iNumEndings; ++iEnding, sStem = sSavedStem)
            {
                // Get ending and modify as necessary
                CEString sEnding;
                int64_t llEndingKey = -1;
                rc = m_spEndings->eGetEnding(iEnding, sEnding, llEndingKey);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                if (8 == m_pInflection->iType() && GENDER_N != m_pLexeme->eInflectionTypeToGender())
                {
                    if (sStem.bEndsWithOneOf(L"шжчщц"))
                    {
                        if (sEnding.bStartsWith(L"я"))
                        {
                            bLoop = false;
                            continue;
                        }
                    }
                    else
                    {
                        if (sEnding.bStartsWith(L"а"))
                        {
                            bLoop = false;
                            continue;
                        }
                    }
                }

//                bool bHasFleetingVowel = false;
                rc = eFleetingVowelCheck(gramIterator.m_eNumber,
                    eEndingCase,
                    gramIterator.m_eGender,
                    eStress,
                    SUBPARADIGM_NOUN,
                    sEnding,
                    sStem);
                if (rc != H_NO_ERROR)
                {
                    bLoop = false;
                    continue;
                }

                vector<int> vecStress;
                rc = eGetStressPositions(sStem, sEnding, gramIterator.m_eNumber, eStress, vecStress);
                if (rc != H_NO_ERROR)
                {
                    bLoop = false;
                    continue;
                }

                shared_ptr<CWordForm> spWordForm;
                rc = eCreateFormTemplate(gramIterator.m_eNumber, gramIterator.m_eCase, sStem, spWordForm);
                if (rc != H_NO_ERROR)
                {
                    bLoop = false;
                    continue;
                }

                if (m_pInflection->iType() == 0)
                {
                    eStress = ET_StressLocation::STRESS_LOCATION_STEM;
                    llEndingKey = 0;
                    sStem = m_pLexeme->sSourceForm();
                    sEnding = L"";
                }

                CreateWordForm(vecStress, eStress, sStem, sEnding, llEndingKey, spWordForm);

            }   //  for (int iEnding = 0; ... )

            if (ET_Case::CASE_ACC == gramIterator.m_eCase)
            {
                if (ET_Case::CASE_UNDEFINED == eAltEndingCase)
                {
                    bLoop = false;
                }
                else
                {
                    eEndingCase = eAltEndingCase;
                    eAltEndingCase = ET_Case::CASE_UNDEFINED;
                }
            }
            else
            {
                bLoop = false;
            }

        }   //  while (bLoop)
    
    } while (gramIterator.bIncrement());

    return H_NO_ERROR;

}    //  eBuild()

void CFormBuilderNouns::CreateWordForm(vector<int>& vecStressPositions, 
                                       ET_StressLocation eStressType, 
                        [[maybe_unused]]const CEString& sStem, 
                                       const CEString& sEnding, 
                                       long long llEndingKey, 
                                       shared_ptr<CWordForm> spWordForm)
{
    ET_ReturnCode rc = H_NO_ERROR;

    auto itStressPos = vecStressPositions.begin();

    for (; itStressPos != vecStressPositions.end(); ++itStressPos)
    {
        if (itStressPos != vecStressPositions.begin() && m_pInflection->bIsMultistressedCompound())
        {
            auto spwfVariant = make_shared<CWordForm>();
//            CloneWordForm(spWordForm, spwfVariant);
            spwfVariant->eCloneFrom(spWordForm.get());
            spwfVariant->ClearStress();
            spWordForm = spwfVariant;
        }

        spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
//        spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;  // primary

        if (m_pInflection->iType() == 0)
        {
//            pWordForm->m_sWordForm = m_pLexeme->sSourceForm();
            spWordForm->SetWordForm(m_pLexeme->sGraphicStem());
        }
        else
        {
            auto&& sStem = spWordForm->sStem();
            rc = eHandleYoAlternation(eStressType, *itStressPos, sStem, sEnding);
            if (rc != H_NO_ERROR)
            {
                continue;
            }
            spWordForm->SetWordForm(sStem + sEnding);
        }

        spWordForm->SetEnding(sEnding);
        spWordForm->SetEndingDataId(llEndingKey);

        m_pInflection->AddWordForm(spWordForm);
    }
}       //  CreateWordForm()
