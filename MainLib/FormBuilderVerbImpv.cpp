#include "WordForm.h"
#include "Lexeme.h"
#include "FormBuilderVerbImpv.h"

using namespace Hlib;

ET_ReturnCode CFormBuilderImperative::eGetStem (CEString& sStem)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    try
    {
        CEString sHash;
        if (4 == m_spInflection->iType() || 5 == m_spInflection->iType())
        {
            CGramHasher hasher (POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_SG, GENDER_UNDEFINED, 
                                PERSON_3, ANIM_UNDEFINED, m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());
            sHash = hasher.sGramHash();
        }
        else
        {
            CGramHasher hasher (POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_SG, GENDER_UNDEFINED, 
                                PERSON_1, ANIM_UNDEFINED, m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());
            sHash = hasher.sGramHash();
        }

        auto spTemplate = make_shared<CWordForm>();
        ET_ReturnCode rc = m_spInflection->eWordFormFromHash (sHash, 0, spTemplate);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }
        if (nullptr == spTemplate)
        {
            assert(0);
            ERROR_LOG (L"Failed to obtain form template.");
            return H_ERROR_POINTER;
        }

        sStem = spTemplate->sStem();

        if (11 == m_spInflection->iType())
        {
            assert(sStem.bEndsWith(L"ь"));     // stem must end in -ь
            sStem.sErase (sStem.uiLength()-1);
        }
    }
    catch (CException&)
    {
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eGetStem (...)

//
// Ending type is an integer NOT defined in GD but rather assigned in the endings source file
// in order to simplify DB structure. May need to be refactored.
//
ET_ReturnCode CFormBuilderImperative::eGetEndingType (const CEString& sStem, int& iEndingType)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_AccentType eAccentType1 = m_spInflection->eAccentType1();
//    ET_AccentType eAccentType2 = m_spLexeme->eAccentType2();

    int iType = m_spInflection->iType();

    if (11 == iType)
    {
        iEndingType = 1;    // -ей
        assert(m_spLexeme->s1SgStem().bEndsWith(L"ь"));
    }
    else if (sStem.bEndsWithOneOf (CEString::g_szRusVowels))
    {
        if (4 == iType)
        {
            if (AT_B == eAccentType1 || AT_C == eAccentType1)   // кроить, поить
            {
                iEndingType = 2;
            }
            else if (AT_A == eAccentType1 && sStem.bStartsWith(L"вы"))
            {
                iEndingType = 2;
            }
            else
            {
                iEndingType = 3;    // -й: all other stems ending in a vowel
            }
        }
        else
        {
            iEndingType = 3;
        }
    }
    else
    {
        if (AT_A == eAccentType1)
        {
            if (sStem.bStartsWith (L"вы"))
            {
                iEndingType = 4;    // -и
            }
            else if (sStem.bEndsWith (L"щ"))      // 2 consonants or щ or CьC
            {
                iEndingType = 4;    // -и
            }
            else if (sStem.uiLength() >= 2 &&
                CEString::bIn (sStem[sStem.uiLength()-1], CEString::g_szRusConsonants) &&
                CEString::bIn (sStem[sStem.uiLength()-2], CEString::g_szRusConsonants))
            {
                iEndingType = 4;    //  -и
            }
            else if (sStem.uiLength() >= 3 &&
                     CEString::bIn (sStem[sStem.uiLength()-1], CEString::g_szRusConsonants) &&
                     L'ь' == sStem[sStem.uiLength()-2] &&
                     CEString::bIn (sStem[sStem.uiLength()-3], CEString::g_szRusConsonants))
            {
                iEndingType = 4;    // -и
            }
            else
            {
                iEndingType = 5;    //  -ь
            }
        }
        else if (AT_B == eAccentType1 || AT_C == eAccentType1 || AT_C1 == eAccentType1)
        {
            iEndingType = 4;
        }
        else
        {
            assert(0);
            ERROR_LOG(L"Unable to determine imperative ending type.");
            return H_ERROR_UNEXPECTED;
        }

    }   // stem ends in consonant

    return H_NO_ERROR;

}   //  eGetEndingType (...)

ET_ReturnCode CFormBuilderImperative::eHandleCommonDeviations(const CEString& sStem, int& iEndingType)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    if (!m_spInflection->bHasCommonDeviation(2) && !m_spInflection->bHasCommonDeviation(3))
    {
        return H_FALSE;
    }

    if (m_spInflection->bHasCommonDeviation(2))
    { 
        if (!m_spInflection->bDeviationOptional(2))
        {
            if (!m_spLexeme->sSourceForm().bStartsWith(L"вы"))
            {
                assert(0);
                CEString sMsg(L"Assertion failed for ");
                sMsg += m_spLexeme->sSourceForm();
                sMsg += L": non-optional common dev.2 implies vy- prefix.";
                ERROR_LOG(sMsg);

                return H_ERROR_UNEXPECTED;
            }
        }

        if (CEString::bIn(sStem[sStem.uiLength()-1], CEString::g_szRusVowels))
        {
            iEndingType = 6;    // V + -й, -йте
        }
        else
        {
            iEndingType = 7;    // -ь, -ьте
        }
    }   //   if (bHasCommonDeviation(2))

    if (m_spInflection->bHasCommonDeviation(3))
    {
        if (m_spInflection->bDeviationOptional(3))
        {
            iEndingType = 9;
        }
        else
        {
            iEndingType = 8;
        }
    }

    return H_NO_ERROR;

}   //  eHandleCommonDeviations (...)

ET_ReturnCode CFormBuilderImperative::eGetStressPositions (const CEString& sStem,
                                                           const CEString& sEnding,
                                                           vector<int>& vecStressPositions)
{
    assert(m_spLexeme);   // we assume base clas ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    ET_StressLocation eStress = STRESS_LOCATION_UNDEFINED;
    switch (m_spInflection->eAccentType1())
    {
        case AT_A:
        {
            eStress = STRESS_LOCATION_STEM;
            break;
        }
        case AT_B:
        case AT_C:
        case AT_C1:            // хотеть and derivatives
        {
            eStress = STRESS_LOCATION_ENDING;
            break;
        }
        default:
        {
            assert(0);
            ERROR_LOG (L"Illegal accent type.");
            return H_ERROR_GENERAL;
        }
    }

    int iStressPos = -1;
    if (STRESS_LOCATION_STEM == eStress)
    {
        rc = m_spLexeme->eGetStemStressPositions (sStem, vecStressPositions);
    }
    else if (STRESS_LOCATION_ENDING == eStress)
    {
        rc = eGetEndingStressPosition (sStem, sEnding, iStressPos);
        vecStressPositions.push_back (iStressPos);
    }
    else
    {
        assert(0);
        ERROR_LOG (L"Illegal stress type.");
        return H_ERROR_INVALID_ARG;
    }

    return rc;

}   //  eGetStressPositions (...)

ET_ReturnCode CFormBuilderImperative::eCreateFormTemplate (const CEString& sStem,
                                                           CEString& sEnding,
                                                           int64_t llEndingDataId,
                                                           ET_Number eNumber,
                                                           shared_ptr<CWordForm>& m_spWordForm)
{
    ET_ReturnCode rc = H_NO_ERROR;

    m_spWordForm = make_shared<CWordForm>(m_spInflection);
    if (NULL == m_spWordForm)
    {
        assert(0);
        ERROR_LOG (L"Unable to instantiate word form object.");
        return H_ERROR_POINTER;
    }

//    m_spWordForm->m_spLexeme = m_spLexeme;
    m_spWordForm->SetInflection(m_spInflection);
    m_spWordForm->SetPos(POS_VERB);
    m_spWordForm->SetSubparadigm(SUBPARADIGM_IMPERATIVE);
    m_spWordForm->SetAspect(m_spLexeme->eAspect());
    m_spWordForm->SetReflexivity(m_spLexeme->eIsReflexive());
    m_spWordForm->SetStem(sStem);
    m_spWordForm->SetEndingDataId(llEndingDataId);
    m_spWordForm->SetWordForm(sStem + sEnding);

    m_spWordForm->SetNumber(eNumber);
    m_spWordForm->SetPerson(PERSON_2);
    m_spWordForm->SetInflectionId(m_spInflection->llInflectionId());

//    rc = eAssignSecondaryStress (m_spWordForm);

    return rc;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderImperative::eBuild()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    if (bHasIrregularImperative() || bHasIrregularPresent())
    {
        rc = eBuildIrregularForms();
//        if (H_NO_ERROR != rc)
//        {
            return rc;  // what if an irregular form is optional?
//        }
    }

    if (m_spLexeme->bImpersonal() || m_spLexeme->bIterative())
    {
        return H_NO_ERROR;
    }

    try
    {
        CEString sStem;

        if (17 == m_spLexeme->iSection())
        {
            assert(14 == m_spInflection->iType());
            return H_NO_MORE;
        }

        if (13 == m_spInflection->iType())
        {
            sStem = m_spLexeme->sInfStem();
        }
        else if (4 == m_spInflection->iType() || 5 == m_spInflection->iType())
        {
            sStem = m_spLexeme->s3SgStem();
        }
        else
        {
            sStem = m_spLexeme->s1SgStem();
        }

        if (sStem.bIsEmpty())
        {
//            assert(0);
            CEString sMsg (L"Empty stem for ");
            sMsg += m_spLexeme->sSourceForm();
            ERROR_LOG (sMsg);
            return H_ERROR_GENERAL;
        }

        if (11 == m_spInflection->iType())
        {
            assert(sStem.bEndsWith(L"ь"));
            sStem.sErase (sStem.uiLength()-1);
        }
        
        rc = eFleetingVowelCheck (sStem);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        int iEndingType = -1;
        rc = eGetEndingType(sStem, iEndingType);
        if (rc != H_NO_ERROR || iEndingType < 0)
        {
            assert(0);
            CEString sMsg(L"Failed to determine imperative ending type for ");
            sMsg += m_spLexeme->sSourceForm();
            ERROR_LOG(sMsg);
            return rc;
        }

        int iDeviantEndingType = -1;
        rc = eHandleCommonDeviations(sStem, iDeviantEndingType);
        if (rc != H_NO_ERROR && rc != H_FALSE)
        {
            return rc;
        }

        if (rc != H_FALSE && iDeviantEndingType < 0)
        {
            assert(0);
            CEString sMsg(L"Unexpected variant ending type for ");
            sMsg += m_spLexeme->sSourceForm();
            ERROR_LOG(sMsg);
            return H_ERROR_UNEXPECTED;
        }

        m_spEndings = make_shared<CImperativeEndings>(m_spLexeme, m_spInflection);
        if (nullptr == m_spEndings)
        {
            return rc;
        }

        for (ET_Number eNumber = NUM_SG; eNumber < NUM_COUNT; ++eNumber)
        {
            if (m_spLexeme->bHasMissingForms())
            {
                CEString sGramHash(L"Impv_");
                sGramHash += (NUM_SG == eNumber) ? L"Sg" : L"Pl";
                sGramHash += L"_2";
                if (H_TRUE != m_spInflection->eFormExists(sGramHash))
                {
                    continue;
                }
            }

            bool bHasRegularEnding = true;
            bool bDeviationOptional = false;
            for (int iCommonDeviation = 2; iCommonDeviation <= 3; ++iCommonDeviation)
            {
                if (m_spInflection->bHasCommonDeviation(iCommonDeviation))
                {
                    rc = static_pointer_cast<CImperativeEndings>(m_spEndings)->eSelect(eNumber, iDeviantEndingType, bDeviationOptional);
                    if (rc < 0)
                    {
                        continue;
                    }

                    // Add regular ending(s) if deviation is optional
                    if (2 == iCommonDeviation)
                    {
                        if (m_spInflection->bDeviationOptional(2))
                        {
                            bDeviationOptional = true;
                        }
                        else
                        {
                            bHasRegularEnding = false;
                        }
                    }
                    else
                    {
                        bDeviationOptional = false;
                        bHasRegularEnding = false;
                    }
                }
            }
            
            if (bHasRegularEnding)
            {
                rc = static_pointer_cast<CImperativeEndings>(m_spEndings)->eSelect(eNumber, iEndingType, bDeviationOptional);
                if (rc < 0)
                {
                    continue;
                }
            }

            int64_t iNumEndings = m_spEndings->iCount();
            if (rc != H_NO_ERROR || iNumEndings < 1)
            {
                if (m_spInflection->iType() != 0)
                {
                    assert(0);
                    CEString sMsg(L"Failed to get imperative endings for ");
                    sMsg += m_spLexeme->sSourceForm();
                    ERROR_LOG(sMsg);
                }
                continue;
            }

            vector <shared_ptr<CWordForm>> vecWordForms;
            for (int iEnding = 0; (rc == H_NO_ERROR && iEnding < iNumEndings); ++iEnding)
            {
                CEString sEnding;
                int64_t llEndingKey = -1;
                rc = m_spEndings->eGetEnding(iEnding, sEnding, llEndingKey);
                if (rc != H_NO_ERROR)
                {
                    continue;
                }

                vector<int> vecStress;
                rc = eGetStressPositions(sStem, sEnding, vecStress);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

//                vector<int>::iterator itStressPos = vecStress.begin();

                auto m_spWordForm = make_shared<CWordForm>();
                rc = eCreateFormTemplate(sStem, sEnding, llEndingKey, eNumber, m_spWordForm);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                if (1 == vecStress.size() || m_spInflection->bIsMultistressedCompound())
                {
                    vector<int>::iterator itStressPos = vecStress.begin();
                    for (; itStressPos != vecStress.end(); ++itStressPos)
                    {
//                        m_spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;  // primary
                        m_spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
                    }

                    m_spInflection->AddWordForm(m_spWordForm);
                }
                else
                {
                    vector<int>::iterator itStressPos = vecStress.begin();
                    for (; itStressPos != vecStress.end(); ++itStressPos)
                    {
                        if (itStressPos != vecStress.begin())
                        {
                            auto spWfVariant = make_shared<CWordForm>();
//                            CloneWordForm(m_spWordForm, spWfVariant);
                            spWfVariant->eCloneFrom(m_spWordForm);
                            m_spWordForm = spWfVariant;
                        }
//                        m_spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                        m_spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
                        m_spInflection->AddWordForm(m_spWordForm);
                    
                    }   //  for (; itStressPos != vecSourceStressPos.end(); ...)
                }    // else
            }      // for (int iEnding = 0; ...
        }       //  for (ET_Number eNumber = NUM_SG; ...
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

//
// Irregular forms may or may not be in the database
//
ET_ReturnCode CFormBuilderImperative::eBuildIrregularForms()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    CGramHasher sg2ImpvHash (SUBPARADIGM_IMPERATIVE, 
                             NUM_SG, 
                             GENDER_UNDEFINED, 
                             PERSON_2, 
                             ANIM_UNDEFINED, 
                             m_spLexeme->eAspect(),
                             CASE_UNDEFINED, 
                             m_spLexeme->eIsReflexive());

    CGramHasher pl2ImpvHash (sg2ImpvHash);
    pl2ImpvHash.m_eNumber = NUM_PL;

    if (bHasIrregularImperative())
    {
        // Just add the plural form
        map<shared_ptr<CWordForm>, bool> mapSg2Impv;
        rc = m_spInflection->eGetIrregularForms(sg2ImpvHash.sGramHash(), mapSg2Impv);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        auto it = mapSg2Impv.begin();
        for (; it != mapSg2Impv.end(); ++it)
        {
            auto spSgWf = it->first;
            if (nullptr == spSgWf)
            {
                assert(0);
                CEString sMsg (L"Irregular form ptr invalid.");
                sMsg += m_spLexeme->sSourceForm();
                ERROR_LOG (sMsg);
                return H_ERROR_POINTER;
            }

            spSgWf->SetInflectionId(m_spInflection->llInflectionId());
            m_spInflection->AddWordForm(spSgWf);

            auto spPlWf = make_shared<CWordForm>(m_spInflection);
            spPlWf->Copy(*spSgWf);
            spPlWf->SetNumber(NUM_PL);

            auto sWordForm = spPlWf->sWordForm();
            if (REFL_YES == m_spLexeme->eIsReflexive())
            {
                sWordForm.sRemoveCharsFromEnd(2);
            }
            else
            {
                sWordForm += L"те";
            }
            spPlWf->SetWordForm(sWordForm);
            m_spInflection->AddWordForm(spPlWf);
        }
        
        return H_NO_ERROR;

    }   //   if (bHasIrregularImperative())

    //
    // No irregular forms in DB: derive from present tense
    //
    if (!bHasIrregularPresent())
    {
        assert(0);
        CEString sMsg (L"No irregular present.");
        sMsg += m_spLexeme->sSourceForm();
        ERROR_LOG (sMsg);
        return H_ERROR_UNEXPECTED;
    }

    if (m_spLexeme->bHasMissingForms())
    {
        CEString sGramHash(L"Impv_*_*");
        if (H_TRUE != m_spInflection->eFormExists(sGramHash))
        {
            return H_NO_ERROR;
        }
    }

    m_spEndings = make_shared<CImperativeEndings>(m_spLexeme, m_spInflection);
    if (nullptr == m_spEndings)
    {
        return rc;
    }

    //
    // No imperative forms provided -- construct imperative from the present tense using
    // the rules for conjugation tables and abbreviated entries (p.88-89)
    //
    CGramHasher pl3Hash (SUBPARADIGM_PRESENT_TENSE, NUM_PL, GENDER_UNDEFINED, PERSON_3, ANIM_UNDEFINED, 
                         m_spLexeme->eAspect(), CASE_UNDEFINED, m_spLexeme->eIsReflexive());

    map<shared_ptr<CWordForm>, bool> mapPl3;
    rc = m_spInflection->eGetIrregularForms(pl3Hash.sGramHash(), mapPl3);
    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    auto itPl3 = mapPl3.begin();
    for (; itPl3 != mapPl3.end(); ++itPl3)
    {
        for (ET_Number eNumber = ET_Number::NUM_SG; eNumber != ET_Number::NUM_COUNT; ++eNumber)
        {
            CEString s3PlStem((*itPl3).first->sWordForm());
            s3PlStem.SetVowels(CEString::g_szRusVowels);
            if (REFL_NO == m_spLexeme->eIsReflexive())
            {
                if (!s3PlStem.bEndsWith(L"ют") && !s3PlStem.bEndsWith(L"ут") && !s3PlStem.bEndsWith(L"ят") &&
                    !s3PlStem.bEndsWith(L"ат"))
                {
                    assert(0);
                    CEString sMsg(L"Unexpected 3 Pl ending: ");
                    sMsg += s3PlStem;
                    sMsg += L"; lexeme = ";
                    sMsg += m_spLexeme->sSourceForm();
                    ERROR_LOG(sMsg);
                    return H_ERROR_UNEXPECTED;
                }
            }
            else if (REFL_YES == m_spLexeme->eIsReflexive())
            {
                if (!s3PlStem.bEndsWith(L"ются") && !s3PlStem.bEndsWith(L"утся") &&
                    !s3PlStem.bEndsWith(L"ятся") && !s3PlStem.bEndsWith(L"атся"))
                {
                    assert(0);
                    CEString sMsg(L"Unexpected 3 Pl refl. ending: ");
                    sMsg += s3PlStem;
                    ERROR_LOG(sMsg);
                    return H_ERROR_UNEXPECTED;
                }
            }
            else
            {
                assert(0);
                CEString sMsg(L"Unexpected eo_Reflexive value for: ");
                sMsg += s3PlStem;
                sMsg += L"; lexeme = ";
                sMsg += m_spLexeme->sSourceForm();
                ERROR_LOG(sMsg);
                return H_ERROR_UNEXPECTED;
            }

            auto gramHash = (ET_Number::NUM_SG == eNumber) ? sg2ImpvHash : pl2ImpvHash;
            auto spWf = make_shared<CWordForm>(gramHash.sGramHash(), m_spInflection);
            if (nullptr == spWf)
            {
                assert(0);
                CEString sMsg(L"Unable to instantiate CWordForm for ");
                sMsg += m_spLexeme->sSourceForm();
                ERROR_LOG(sMsg);
                return H_ERROR_UNEXPECTED;
            }

            spWf->SetInflectionId(m_spInflection->llInflectionId());

            int iCharsToErase = REFL_YES == m_spLexeme->eIsReflexive() ? 4 : 2;
            s3PlStem.sErase(s3PlStem.uiLength() - iCharsToErase);
            spWf->SetStem(s3PlStem);

            if (s3PlStem.bEndsWithOneOf(CEString::g_szRusConsonants))
            {
                int iEndingType = 4;
                rc = static_pointer_cast<CImperativeEndings>(m_spEndings)->eSelect(eNumber, iEndingType);
                if (rc < 0 || m_spEndings->iCount() < 1)
                {
                    assert(0);
                    CEString sMsg(L"No imperative ending for ");
                    sMsg += m_spLexeme->sSourceForm();
                    ERROR_LOG(sMsg);
                    continue;
                }

                //            spSg->SetWordForm(s3Pl + &&&&);
                spWf->SetStressPos(s3PlStem.uiGetNumOfSyllables(), STRESS_PRIMARY);
            }
            else
            {
                int iEndingType = 3;
                rc = static_pointer_cast<CImperativeEndings>(m_spEndings)->eSelect(eNumber, iEndingType);
                if (rc < 0 || m_spEndings->iCount() < 1)
                {
                    assert(0);
                    CEString sMsg(L"No imperative ending for ");
                    sMsg += m_spLexeme->sSourceForm();
                    ERROR_LOG(sMsg);
                    continue;
                }
                spWf->AssignStress((*itPl3).first->mapGetStressPositions());
            }

            CEString sEnding;
            int64_t llEndingKey = -1;
            rc = m_spEndings->eGetEnding(0, sEnding, llEndingKey);
            if (rc != H_NO_ERROR)
            {
                continue;
            }

            spWf->SetEnding(sEnding);
            spWf->SetEndingDataId(llEndingKey);
            spWf->SetWordForm(spWf->sStem() + sEnding);
            spWf->AssignStress(spWf->mapGetStressPositions());

            m_spInflection->AddWordForm(spWf);
        }
    }

    return H_NO_ERROR;

}   //  eBuildIrregularForms()
