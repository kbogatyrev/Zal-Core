#include "WordForm.h"
#include "Lexeme.h"

#include "Endings.h"
#include "FormBuilderVerbPers.h"

using namespace Hlib;

ET_ReturnCode CFormBuilderPersonal::eGetStressType (ET_Number eNumber, ET_Person ePerson,  ET_StressLocation& eStress)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    switch (m_spInflection->eAccentType1())
    {
        case AT_A:
        {
            eStress = STRESS_LOCATION_STEM;
            break;
        }
        case AT_B:
        {
            eStress = STRESS_LOCATION_ENDING;
            break;
        }
        case AT_C:
        {
            if (NUM_SG == eNumber && PERSON_1 == ePerson)
            {
                eStress = STRESS_LOCATION_ENDING;
            }
            else
            {
                eStress = STRESS_LOCATION_STEM;
            }
            break;
        }
        case AT_C1:                // хотеть and derivatives
        {
            if (NUM_SG == eNumber && (PERSON_2 == ePerson || PERSON_3 == ePerson))
            {
                eStress = STRESS_LOCATION_STEM;
            }
            else
            {
                eStress = STRESS_LOCATION_ENDING;
            }
            break;
        }
        default:
        {
            assert(0);
            ERROR_LOG (L"Incompatible stress type.");
            return H_ERROR_UNEXPECTED;
        }
    }

    return rc;

}   //  eGetStressType (...)

ET_ReturnCode CFormBuilderPersonal::eGetAuslautType (ET_Number eNumber, ET_Person ePerson, ET_StemAuslaut& eAuslaut)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    if (m_spInflection->iType() >= 4 && m_spInflection->iType() <= 6)
    {
        if ((PERSON_1 == ePerson) && (NUM_SG == eNumber))
        {
            if (m_spLexeme->s1SgStem().bEndsWithOneOf (L"шжчщц"))
            {
                eAuslaut = STEM_AUSLAUT_SH;
            }
            else
            {
                eAuslaut = STEM_AUSLAUT_NOT_SH;
            }
        }
        if ((PERSON_3 == ePerson) && (NUM_PL == eNumber))
        {
            if (m_spLexeme->s3SgStem().bEndsWithOneOf (L"шжчщц"))
            {
                eAuslaut = STEM_AUSLAUT_SH;
            }
            else
            {
                eAuslaut = STEM_AUSLAUT_NOT_SH;
            }
        }
    }

    return H_NO_ERROR;

}   //  eGetAuslautType (...)

ET_ReturnCode CFormBuilderPersonal::eGetStem(ET_Number eNumber, ET_Person ePerson, CEString& sStem)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    if (PERSON_1 == ePerson && NUM_SG == eNumber)
    {
        sStem = m_spLexeme->s1SgStem();
    }
    else
    {
        if (4 == m_spInflection->iType() || 5 == m_spInflection->iType())
        {
            sStem = m_spLexeme->s3SgStem();
        }
        else
        {
            if (PERSON_3 == ePerson && NUM_PL == eNumber)
            {
                sStem = m_spLexeme->s1SgStem();
            }
            else
            {
                sStem = m_spLexeme->s3SgStem();
            }
        }
    }

    return rc;

}       //  eGetStem (...)

ET_ReturnCode CFormBuilderPersonal::eGetStressPositions (const CEString& sStem,
                                                         const CEString& sEnding,
                                                         ET_StressLocation eStressType,
                                                         vector<int>& vecStressPositions)
{
    ET_ReturnCode rc = H_NO_ERROR;

    if (STRESS_LOCATION_STEM == eStressType)
    {
        rc = m_spLexeme->eGetStemStressPositions (const_cast<CEString&>(sStem), vecStressPositions);
                                    // needed to set vowels
    }
    else if (STRESS_LOCATION_ENDING == eStressType)
    {
        int iStressPos = -1;
        rc = eGetEndingStressPosition (const_cast<CEString&>(sStem), const_cast<CEString&>(sEnding), iStressPos);
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

ET_ReturnCode CFormBuilderPersonal::eCreateFormTemplate (ET_Number eNumber, ET_Person ePerson, shared_ptr<CWordForm>& spWordForm)
{
    ET_ReturnCode rc = H_NO_ERROR;

    spWordForm = make_shared<CWordForm>();
    if (nullptr == spWordForm)
    {
        assert(0);
        CEString sMsg;
        ERROR_LOG (L"Unable to instantiate CWordForm.");
        return H_ERROR_POINTER;
    }

    spWordForm->m_spLexeme = m_spLexeme;
    spWordForm->m_ePos = m_spLexeme->ePartOfSpeech();
    spWordForm->m_eSubparadigm = SUBPARADIGM_PRESENT_TENSE;
    spWordForm->m_eReflexivity = m_spLexeme->eIsReflexive();
    spWordForm->m_eNumber = eNumber;
    spWordForm->m_ePerson = ePerson;
    spWordForm->m_eAspect = m_spLexeme->eAspect();
    spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();

    return rc;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderPersonal::eBuild()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
//        bool bNoRegularStems = false;

        if (m_spLexeme->bHasIrregularForms())
        {
            rc = eHandleIrregularForms();
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
        }

        rc = eBuildVerbStems();
        if (rc != H_NO_ERROR)
        {
            return rc;
        }
//        if (H_NO_MORE == rc)
//        {
//            bNoRegularStems = true;
//        }

        m_spEndings = make_shared<CPersonalEndings>(m_spLexeme, m_spInflection);
        if (NULL == m_spEndings)
        {
            return rc;
        }

        for (ET_Number eNumber = NUM_SG; (H_NO_ERROR == rc && eNumber <= NUM_PL); ++eNumber)
        {
            for (ET_Person ePerson = PERSON_1; (H_NO_ERROR == rc && ePerson <= PERSON_3); ++ePerson)
            {
                CGramHasher hasher (SUBPARADIGM_PRESENT_TENSE, 
                                    eNumber, 
                                    GENDER_UNDEFINED, 
                                    ePerson, 
                                    ANIM_UNDEFINED,                                       
                                    m_spLexeme->eAspect(),
                                    CASE_UNDEFINED, 
                                    m_spLexeme->eIsReflexive());

//                if (m_spLexeme->bHasMissingForms() || m_spLexeme->bImpersonal() || m_spLexeme->bIterative())
                if (m_spLexeme->bImpersonal() || m_spLexeme->bIterative())
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

                // Irregular forms will be handled separately 
                if (m_spInflection->bNoRegularForms(hasher.sGramHash()))
                {
                    continue;
                }
                
                ET_StressLocation eStress = STRESS_LOCATION_UNDEFINED;
                rc = eGetStressType (eNumber, ePerson, eStress);
                if (rc != H_NO_ERROR)
                {
                    continue;
                }
            
                ET_StemAuslaut eAuslaut = STEM_AUSLAUT_UNDEFINED;
                rc = eGetAuslautType (eNumber, ePerson, eAuslaut);
                if (rc != H_NO_ERROR)
                {
                    continue;
                }

//                int iConjugation = INFLECTION_TYPE_UNDEFINED;
//                if ((4 == m_spLexeme->iType() || 5 == m_spLexeme->iType()) ||
//                    (PERSON_1 == ePerson && NUM_SG == eNumber) || 
//                    (PERSON_3 == ePerson && NUM_PL == eNumber))
//                {
//                    iConjugation = iType;
//                }
//                else
//                {
// WE ONLY NEED THIS FOR -E- VERBS!
                    rc = eGetStressType(eNumber, ePerson, eStress);
                    if (rc != H_NO_ERROR)
                    {
                        continue;
                    }
//                }

                static_pointer_cast<CPersonalEndings>(m_spEndings)->eSelect(ePerson, eNumber, eStress, eAuslaut);
                int64_t iNumEndings = m_spEndings->iCount();
                if (iNumEndings < 1)
                {
                    ERROR_LOG (L"No endings");
                    continue;
                }

                for (int iEnding = 0; (rc == H_NO_ERROR && iEnding < iNumEndings); ++iEnding)
                {
                    CEString sEnding;
                    int64_t llEndingKey = -1;
                    rc = m_spEndings->eGetEnding (iEnding, sEnding, llEndingKey);
                    if (rc != H_NO_ERROR)
                    {
                        continue;
                    }

                    //if (6 == m_spLexeme->iType() && 1 == m_spLexeme->iStemAugment())
                    //{
                    //    if ((PERSON_1 == ePerson && NUM_SG == eNumber) ||
                    //        (PERSON_3 == ePerson && NUM_PL == eNumber))
                    //    {
                    //        if (sEnding.bIsEmpty() || L'ю' != sEnding[0])
                    //        {
                    //            assert(0);
                    //            ERROR_LOG (L"Unexpected ending");
                    //            continue;
                    //        }
                    //        sEnding[0] = L'у';
                    //    }
                    //}

                    //if (REFL_YES == m_spLexeme->eIsReflexive())
                    //{
                    //    if (sEnding.bEndsWithOneOf (g_szRusVowels))
                    //    {
                    //        sEnding += L"сь";
                    //    }
                    //    else
                    //    {
                    //        sEnding += L"ся";
                    //    }
                    //}

                    CEString sStem;
                    rc = eGetStem(eNumber, ePerson, sStem);
                    if (rc != H_NO_ERROR)
                    {
                        continue;
                    }
                    rc = eFleetingVowelCheck (sStem);
                    if (rc != H_NO_ERROR)
                    {
                        continue;
                    }

                    vector<int> vecStress;
                    rc = eGetStressPositions (sStem, sEnding, eStress, vecStress);
                    if (rc != H_NO_ERROR)
                    {
                        continue;
                    }

                    shared_ptr<CWordForm> spWordForm;
                    rc = eCreateFormTemplate (eNumber, ePerson, spWordForm);
                    if (rc != H_NO_ERROR || nullptr == spWordForm)
                    {
                        continue;
                    }

                    spWordForm->m_sStem = sStem;
                    spWordForm->m_sWordForm = sStem + sEnding;
                    spWordForm->m_sEnding = sEnding;
                    spWordForm->m_llEndingDataId = llEndingKey;

                    if (1 == vecStress.size() || m_spInflection->bIsMultistressedCompound())
                    {
                        vector<int>::iterator itStressPos = vecStress.begin();
                        for (; itStressPos != vecStress.end(); ++itStressPos)
                        {
                            spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
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
                                shared_ptr<CWordForm> spWfVariant;
                                CloneWordForm (spWordForm, spWfVariant);
                                spWordForm = spWfVariant;
                            }
                            spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                            m_spInflection->AddWordForm(spWordForm);
                        }
                    }

                }   //  for (int iEnding = 0; ... )

            }   //  for (ET_Person ePerson = PERSON_1; ... )

        }   //  for (ET_Number eNumber = NUM_SG; ... )

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
// Needed for abbreviated subparadigm rendering (GDRL 15, 16, 35, 89)
//
ET_ReturnCode CFormBuilderPersonal::eHandleIrregularForms()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {

        for (ET_Number eNumber = NUM_SG; eNumber <= NUM_PL; ++eNumber)
        {
            for (ET_Person ePerson = PERSON_1; ePerson <= PERSON_3; ++ePerson)
            {
                CGramHasher hasher(SUBPARADIGM_PRESENT_TENSE,
                    eNumber,
                    GENDER_UNDEFINED,
                    ePerson,
                    ANIM_UNDEFINED,
                    m_spLexeme->eAspect(),
                    CASE_UNDEFINED,
                    m_spLexeme->eIsReflexive());

                if (!m_spInflection->bHasIrregularForm(hasher.sGramHash()))
                {
                    continue;
                }

                map<shared_ptr<CWordForm>, bool> mapResult;
                rc = m_spInflection->eGetIrregularForms(hasher.sGramHash(), mapResult);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                auto itIf = mapResult.begin();
                for (; itIf != mapResult.end(); ++itIf)
                {
                    if (mapResult.end() != itIf)    // form exists
                    {
                        m_spInflection->AddWordForm(itIf->first);
                    }
                    else                            // form needs to be constructed per GDRL rules
                    {
                        if (NUM_SG == eNumber && (PERSON_1 == ePerson || PERSON_3 == ePerson))  // always given
                        {
                            assert(0);
                            ERROR_LOG(L"Irregular form not in database.");
                            return H_ERROR_UNEXPECTED;
                        }

                        rc = eBuildIrregularForms(eNumber, ePerson);
                        if (rc != H_NO_ERROR)
                        {
                            return rc;
                        }
                    }

                    // Not sure if this is correct...
                    if (PERSON_1 == ePerson && m_spLexeme->s1SgStem().bIsEmpty())
                    {
                        CreateIrregular1SgStem((itIf->first)->sWordForm());
                        (itIf->first)->m_sStem = m_spLexeme->s1SgStem();
                    }
                    if (PERSON_3 == ePerson && m_spLexeme->s3SgStem().bIsEmpty())
                    {
                        CreateIrregular3SgStem((itIf->first)->sWordForm());
                        (itIf->first)->m_sStem = m_spLexeme->s3SgStem();
                    }
                }   //  for (; itIf != mapResult.end(); ++itIf)

            }   //  for (ET_Person ePerson = PERSON_1; ePerson <= PERSON_3; ++ePerson)

        }   //  for (ET_Number eNumber = NUM_SG; eNumber <= NUM_PL; ++eNumber)
    }
    catch (CException& ex)
    {
        CEString sMsg(L"Exception: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG(sMsg);
    }

    return rc;

}   //  eHandleIrregularForms()

ET_ReturnCode CFormBuilderPersonal::eBuildIrregularForms (ET_Number eNumber, ET_Person ePerson)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    // GDRL p. 89

    ET_ReturnCode rc = H_NO_ERROR;

    CGramHasher Sg3Hasher (SUBPARADIGM_PRESENT_TENSE, 
                          NUM_SG, 
                          GENDER_UNDEFINED, 
                          PERSON_3, 
                          ANIM_UNDEFINED,
                          m_spLexeme->eAspect(),
                          CASE_UNDEFINED, 
                          m_spLexeme->eIsReflexive());
    
    vector<shared_ptr<CWordForm>> vecSg3Irreg;
    rc = eGetIrregularForms(Sg3Hasher.sGramHash(), vecSg3Irreg);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    auto itSg3Irreg = vecSg3Irreg.begin();
    for (; itSg3Irreg != vecSg3Irreg.end(); ++itSg3Irreg)
    {
        CEString sWordForm = (*itSg3Irreg)->m_sWordForm;

        if (!sWordForm.bEndsWith (L"т"))
        {
            assert(0);
            ERROR_LOG (L"3 Sg form does not end in \'t\'.");
            return H_ERROR_UNEXPECTED;
        }

        sWordForm = (*itSg3Irreg)->m_sWordForm;
        sWordForm.sErase (sWordForm.uiLength() - 1);

        if (NUM_SG == eNumber && PERSON_2 == ePerson)
        {
            sWordForm += L"шь";
            
            auto spWordForm = make_shared<CWordForm>();
            if (nullptr == spWordForm)
            {
                assert(0);
                ERROR_LOG (L"Unable to instantiate CWordForm.");
                return H_ERROR_POINTER;
            }
            
            spWordForm->m_spLexeme = m_spLexeme;
            spWordForm->m_mapStress = (*itSg3Irreg)->m_mapStress;
            spWordForm->m_sWordForm = sWordForm;
//            pWordForm->m_llLexemeId;

            try
            {
                m_spInflection->AddWordForm(spWordForm);
            }
            catch (CException& ex)
            {
                CEString sMsg(L"Exception: ");
                sMsg += ex.szGetDescription();
                ERROR_LOG(sMsg);
                return H_EXCEPTION;
            }

            return H_NO_ERROR;
        }

        if (NUM_PL != eNumber)
        {
            assert(0);
            ERROR_LOG (L"Unexpected ET_Number value.");
            return H_ERROR_UNEXPECTED;
        }

        //  Plural only from this point:
        switch (ePerson)
        {
            case PERSON_1:
            {
                sWordForm += L"м";
                break;
            }
            case PERSON_2:
            {
                sWordForm += L"те";
                break;
            }
            case PERSON_3:
            {
                if (sWordForm.bEndsWith (L"е") || sWordForm.bEndsWith (L"ё"))
                {
                    CGramHasher Sg1Hasher (SUBPARADIGM_PRESENT_TENSE, 
                                          NUM_SG, 
                                          GENDER_UNDEFINED, 
                                          PERSON_1, 
                                          ANIM_UNDEFINED,
                                          m_spLexeme->eAspect(),
                                          CASE_UNDEFINED, 
                                          m_spLexeme->eIsReflexive());
                    vector<shared_ptr<CWordForm>> vecSg1Irreg;
                    rc = eGetIrregularForms(Sg1Hasher.sGramHash(), vecSg1Irreg);
                    if (H_NO_ERROR != rc)
                    {
                        return rc;
                    }

                    auto itSg1Irreg = vecSg1Irreg.begin();
                    if (vecSg1Irreg.end() == itSg1Irreg)
                    {
                        assert(0);
                        ERROR_LOG (L"Unable to obtain irregular 1 Sg form.");
                        return H_ERROR_UNEXPECTED;
                    }

                    sWordForm = (*itSg1Irreg)->m_sWordForm;
                    sWordForm += L"т";
                }
                else if (sWordForm.bEndsWith (L"и"))
                {
                    sWordForm.sErase (sWordForm.uiLength()-1);
                    if (sWordForm.bEndsWithOneOf (L"шжчщц"))
                    {
                        sWordForm += L"ат";
                    }
                    else
                    {
                        sWordForm += L"ят";
                    }
                }
                else
                {
                    assert(0);
                    ERROR_LOG (L"3 Sg form does not end in \'et\' or \'it\'.");
                    return H_ERROR_UNEXPECTED;
                }
                break;
        
            }   //  PERSON_3            
            default:
            {
                assert(0);
                ERROR_LOG (L"Illegal person value.");
                return H_ERROR_UNEXPECTED;
            }
    
        }       //  switch

        auto spWordForm = make_shared<CWordForm>();
        if (nullptr == spWordForm)
        {
            assert(0);
            ERROR_LOG (L"Unable to instantiate CWordForm.");
            return H_ERROR_POINTER;
        }

        spWordForm->m_spLexeme = m_spLexeme;
        spWordForm->m_mapStress = (*itSg3Irreg)->m_mapStress;
        spWordForm->m_sWordForm = sWordForm;
        spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();

        try
        {
            m_spInflection->AddWordForm(spWordForm);
        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
            return H_EXCEPTION;
        }
    }   //  for (; itSg3Irreg != vecSg3Irreg.end(); ++itSg3Irreg)
    
    return H_NO_ERROR;

}       //  eBuildIrregularForm (...)

void CFormBuilderPersonal::CreateIrregular1SgStem(const CEString& s1SgForm)
{
    StLexemeProperties stProps = m_spLexeme->stGetPropertiesForWriteAccess();
    if (s1SgForm.uiLength() < 2)
    {
        assert(0);
        ERROR_LOG(L"1 Sg form too short.");
        return;
    }

    stProps.s1SgStem = s1SgForm;
    stProps.s1SgStem.sErase(s1SgForm.uiLength()-1);
}

void CFormBuilderPersonal::CreateIrregular3SgStem(const CEString& s3SgForm)
{
    StLexemeProperties stProps = m_spLexeme->stGetPropertiesForWriteAccess();
    if (s3SgForm.uiLength() < 3)
    {
        assert(0);
        ERROR_LOG(L"3 Sg form too short.");
        return;
    }

    stProps.s3SgStem = s3SgForm;
    stProps.s3SgStem.sErase(s3SgForm.uiLength()-2);
}
