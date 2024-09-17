#include "WordForm.h"
#include "Lexeme.h"

#include "Endings.h"
#include "FormBuilderVerbPers.h"

using namespace Hlib;

ET_ReturnCode CFormBuilderPersonal::eGetStressType (ET_Number eNumber, ET_Person ePerson,  ET_StressLocation& eStress)
{
    assert(m_pLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    switch (m_pInflection->eAccentType1())
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
    assert(m_pLexeme);   // we assume base class ctor took care of this

    if (m_pInflection->iType() >= 4 && m_pInflection->iType() <= 6)
    {
        if ((PERSON_1 == ePerson) && (NUM_SG == eNumber))
        {
            if (m_pLexeme->s1SgStem().bEndsWithOneOf (L"шжчщц"))
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
            if (m_pLexeme->s3SgStem().bEndsWithOneOf (L"шжчщц"))
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
    assert(m_pLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    if (PERSON_1 == ePerson && NUM_SG == eNumber)
    {
        sStem = m_pLexeme->s1SgStem();
    }
    else
    {
        if (4 == m_pInflection->iType() || 5 == m_pInflection->iType())
        {
            sStem = m_pLexeme->s3SgStem();
        }
        else
        {
            if (PERSON_3 == ePerson && NUM_PL == eNumber)
            {
                sStem = m_pLexeme->s1SgStem();
            }
            else
            {
                sStem = m_pLexeme->s3SgStem();
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
        rc = m_pLexeme->eGetStemStressPositions (const_cast<CEString&>(sStem), vecStressPositions);
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

    spWordForm = make_shared<CWordForm>(m_pInflection);
    if (nullptr == spWordForm)
    {
        assert(0);
        CEString sMsg;
        ERROR_LOG (L"Unable to instantiate CWordForm.");
        return H_ERROR_POINTER;
    }

    spWordForm->SetInflection(m_pInflection);
    spWordForm->SetPos(POS_VERB);
    spWordForm->SetSubparadigm(SUBPARADIGM_PRESENT_TENSE);
    spWordForm->SetReflexivity(m_pLexeme->eIsReflexive());
    spWordForm->SetNumber(eNumber);
    spWordForm->SetPerson(ePerson);
    spWordForm->SetAspect(m_pLexeme->eAspect());
    spWordForm->SetInflectionId(m_pInflection->llInflectionId());

    return rc;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderPersonal::eBuild()
{
    assert(m_pLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
//        bool bNoRegularStems = false;

        if (m_pLexeme->bHasIrregularForms())
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

        m_spEndings = make_shared<CPersonalEndings>(m_pLexeme, m_pInflection);
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
                                    m_pLexeme->eAspect(),
                                    CASE_UNDEFINED, 
                                    m_pLexeme->eIsReflexive());

//                if (m_pLexeme->bHasMissingForms() || m_pLexeme->bImpersonal() || m_pLexeme->bIterative())
                if (m_pLexeme->bImpersonal() || m_pLexeme->bIterative())
                    {
                    if (m_pInflection->eFormExists(hasher.sGramHash()) != H_TRUE)
                    {
                        continue;
                    }
                }

                if (m_pLexeme->bHasMissingForms())
                {
                    auto sHash = hasher.sGramHash();
                    if (m_pInflection->eFormExists(sHash) != H_TRUE)
                    {
                        m_pInflection->SetHypotheticalForm(sHash);
                    }
                }

                // Irregular forms will be handled separately 
                if (m_pInflection->bNoRegularForms(hasher.sGramHash()))
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
//                if ((4 == m_pLexeme->iType() || 5 == m_pLexeme->iType()) ||
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

                    //if (6 == m_pLexeme->iType() && 1 == m_pLexeme->iStemAugment())
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

                    //if (REFL_YES == m_pLexeme->eIsReflexive())
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

                    spWordForm->SetStem(sStem);
                    spWordForm->SetWordForm(sStem + sEnding);
                    spWordForm->SetEnding(sEnding);
                    spWordForm->SetEndingDataId(llEndingKey);

                    if (1 == vecStress.size() || m_pInflection->bIsMultistressedCompound())
                    {
                        vector<int>::iterator itStressPos = vecStress.begin();
                        for (; itStressPos != vecStress.end(); ++itStressPos)
                        {
                            spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
                        }
                        m_pInflection->AddWordForm (spWordForm);
                    }
                    else
                    {
                        vector<int>::iterator itStressPos = vecStress.begin();
                        for (; itStressPos != vecStress.end(); ++itStressPos)
                        {
                            if (itStressPos != vecStress.begin())
                            {
                                shared_ptr<CWordForm> spWfVariant = make_shared<CWordForm>();
                                spWfVariant->eCloneFrom(spWordForm.get());
                                spWordForm = spWfVariant;
                                spWordForm->ClearStress();
                            }
                            spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
                            m_pInflection->AddWordForm(spWordForm);
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
    assert(m_pLexeme);   // we assume base class ctor took care of this

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
                    m_pLexeme->eAspect(),
                    CASE_UNDEFINED,
                    m_pLexeme->eIsReflexive());

                if (!m_pInflection->bHasIrregularForm(hasher.sGramHash()))
                {
                    continue;
                }

                map<CWordForm*, bool> mapResult;
                rc = m_pInflection->eGetIrregularForms(hasher.sGramHash(), mapResult);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                auto itIf = mapResult.begin();
                for (; itIf != mapResult.end(); ++itIf)
                {
                    if (mapResult.end() != itIf)    // form exists
                    {
                        m_pInflection->AddWordForm(make_shared<CWordForm>(*itIf->first));
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
                    if (PERSON_1 == ePerson && m_pLexeme->s1SgStem().bIsEmpty())
                    {
                        CreateIrregular1SgStem((itIf->first)->sWordForm());
                        (itIf->first)->SetStem(m_pLexeme->s1SgStem());
                    }
                    if (PERSON_3 == ePerson && m_pLexeme->s3SgStem().bIsEmpty())
                    {
                        CreateIrregular3SgStem((itIf->first)->sWordForm());
                        (itIf->first)->SetStem(m_pLexeme->s3SgStem());
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
    assert(m_pLexeme);   // we assume base class ctor took care of this

    // GDRL p. 89

    ET_ReturnCode rc = H_NO_ERROR;

    CGramHasher Sg3Hasher (SUBPARADIGM_PRESENT_TENSE, 
                          NUM_SG, 
                          GENDER_UNDEFINED, 
                          PERSON_3, 
                          ANIM_UNDEFINED,
                          m_pLexeme->eAspect(),
                          CASE_UNDEFINED, 
                          m_pLexeme->eIsReflexive());
    
    vector<CWordForm*> vecSg3Irreg;
    rc = eGetIrregularForms(Sg3Hasher.sGramHash(), vecSg3Irreg);
    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    auto itSg3Irreg = vecSg3Irreg.begin();
    for (; itSg3Irreg != vecSg3Irreg.end(); ++itSg3Irreg)
    {
        CEString sWordForm = move((*itSg3Irreg)->sWordForm());

        if (!sWordForm.bEndsWith (L"т"))
        {
            assert(0);
            ERROR_LOG (L"3 Sg form does not end in \'t\'.");
            return H_ERROR_UNEXPECTED;
        }

        sWordForm = move((*itSg3Irreg)->sWordForm());
        sWordForm.sErase (sWordForm.uiLength() - 1);

        if (NUM_SG == eNumber && PERSON_2 == ePerson)
        {
            sWordForm += L"шь";
            
            auto spWordForm = make_shared<CWordForm>(m_pInflection);
            if (nullptr == spWordForm)
            {
                assert(0);
                ERROR_LOG (L"Unable to instantiate CWordForm.");
                return H_ERROR_POINTER;
            }
            
//            spWordForm->m_pLexeme = m_pLexeme;
            spWordForm->AssignStress((*itSg3Irreg)->mapGetStressPositions());
            spWordForm->SetWordForm(sWordForm);
//            pWordForm->m_llLexemeId;

            try
            {
                m_pInflection->AddWordForm(spWordForm);
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
                                          m_pLexeme->eAspect(),
                                          CASE_UNDEFINED, 
                                          m_pLexeme->eIsReflexive());
                    vector<CWordForm*> vecSg1Irreg;
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

                    sWordForm = move((*itSg1Irreg)->sWordForm());
                    sWordForm += L"т";
                }
                else if (sWordForm.bEndsWith (L"и"))
                {
                    sWordForm.sErase(sWordForm.uiLength()-1);
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

        auto spWordForm = make_shared<CWordForm>(m_pInflection);
        if (nullptr == spWordForm)
        {
            assert(0);
            ERROR_LOG (L"Unable to instantiate CWordForm.");
            return H_ERROR_POINTER;
        }

//        spWordForm->m_pLexeme = m_pLexeme;
        spWordForm->AssignStress((*itSg3Irreg)->mapGetStressPositions());
        spWordForm->SetWordForm(sWordForm);
//        spWordForm->m_llLexemeId = m_pLexeme->llLexemeId();
        spWordForm->SetInflectionId(m_pInflection->llInflectionId());

        try
        {
            m_pInflection->AddWordForm(spWordForm);
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
    StLexemeProperties& stProps = m_pLexeme->stGetPropertiesForWriteAccess();
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
    StLexemeProperties& stProps = m_pLexeme->stGetPropertiesForWriteAccess();
    if (s3SgForm.uiLength() < 3)
    {
        assert(0);
        ERROR_LOG(L"3 Sg form too short.");
        return;
    }

    stProps.s3SgStem = s3SgForm;
    stProps.s3SgStem.sErase(s3SgForm.uiLength()-2);
}
