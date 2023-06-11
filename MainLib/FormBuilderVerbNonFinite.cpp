#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "Endings.h"

using namespace Hlib;

#include "WordForm.h"
#include "Lexeme.h"
#include "Inflection.h"

#include "FormBuilderAdjLong.h"
#include "FormBuilderAdjShort.h"
#include "FormBuilderVerbNonFinite.h"

ET_ReturnCode CFormBuilderNonFinite::eBuild()
{
    ET_ReturnCode rc = H_NO_ERROR;

    rc = eBuildInfinitive();
    if (rc != H_NO_ERROR)
    {
//        return rc;
    }

    rc = eBuildPresentActiveParticiple();
    if (rc != H_NO_ERROR)
    {
//        return rc;
    }

    rc = eBuildPresentAdverbial();
    if (rc != H_NO_ERROR)
    {
//        return rc;
    }

    rc = eBuildPastActiveParticiple();
    if (rc != H_NO_ERROR)
    {
//        return rc;
    }

    rc = eBuildPastAdverbial();
    if (rc != H_NO_ERROR)
    {
//        return rc;
    }

    rc = eBuildPresentPassiveParticiple();
    if (rc != H_NO_ERROR)
    {
//        return rc;
    }

    rc = eBuildPastPassiveParticiple();
    
    return rc;
}

ET_ReturnCode CFormBuilderNonFinite::eBuildInfinitive()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    bool bNoRegularForms = false;

    //
    // Irregular infinitive?
    //
    map<shared_ptr<CWordForm>, bool> mapIrreg;
    rc = m_spInflection->eGetIrregularForms(L"Inf", mapIrreg);
    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    auto it = mapIrreg.begin();
    for (; it != mapIrreg.end(); ++it)
    {
        bNoRegularForms = true;
        auto spWordForm = it->first;
        if (it->second)
        {
            bNoRegularForms = false;
        }
        try
        {
            m_spInflection->AddWordForm(spWordForm);
        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
            return H_EXCEPTION;
        }
    }

    if (bNoRegularForms)
    {
        return mapIrreg.empty() ? H_ERROR_UNEXPECTED : H_NO_ERROR;
    }

    try
    {
        m_spEndings = make_shared<CInfinitiveEndings>(m_spLexeme, m_spInflection);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        bool bHasRegularEnding { true };
        static_pointer_cast<CInfinitiveEndings>(m_spEndings)->eSelect(m_spInflection->iType());
        int64_t iNumEndings = m_spEndings->iCount();
        if (iNumEndings < 1)
        {
            CEString sMsg(L"No ending or too many endings; lexeme = ");
            sMsg += m_spLexeme->sSourceForm();
            bHasRegularEnding = false;
            //            return H_ERROR_UNEXPECTED;
        }

        auto iForms = bHasRegularEnding ? iNumEndings : 1;
        vector <shared_ptr<CWordForm>> vecWordForms;
        for (int iEnding = 0; (rc == H_NO_ERROR && iEnding < iForms); ++iEnding)
        {
            int64_t llEndingKey = -1;
            if (bHasRegularEnding)
            {
                CEString sEnding;
                rc = m_spEndings->eGetEnding(iEnding, sEnding, llEndingKey);
                if (rc != H_NO_ERROR)
                {
                    continue;
                }
            }
            auto spWordForm = make_shared<CWordForm>(m_spInflection);
            if (nullptr == spWordForm)
            {
                assert(0);
                ERROR_LOG(L"Unable to instantiate CWordForm.");
                return H_ERROR_POINTER;
            }
            spWordForm->SetInflection(m_spInflection);
            spWordForm->SetPos(POS_VERB);
            spWordForm->SetSubparadigm(SUBPARADIGM_INFINITIVE);
            spWordForm->SetAspect(m_spLexeme->eAspect());
            spWordForm->SetReflexivity(m_spLexeme->eIsReflexive());
            spWordForm->SetStem(m_spLexeme->sInfStem());
            spWordForm->SetWordForm(m_spLexeme->sSourceForm());
            spWordForm->SetEndingDataId(llEndingKey);
//            spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();
            spWordForm->SetInflectionId(m_spInflection->llInflectionId());

            vector<int> vecStress;

            const StLexemeProperties& stLexemeProperties = m_spLexeme->stGetProperties();
            if (stLexemeProperties.vecSourceStressPos.size() != 1)
            {
                CEString sMsg(L"Multiple primary stress positions in infnitive; lexeme = ");
                sMsg += m_spLexeme->sSourceForm();
                ERROR_LOG(sMsg);
            }

//            rc = m_spLexeme->eGetStemStressPositions(m_spLexeme->sInfStem(), vecStress);
//            if (rc != H_NO_ERROR)
//            {
//                return rc;
//            }

            vecStress = stLexemeProperties.vecSourceStressPos;
            if (1 == vecStress.size() || m_spInflection->bIsMultistressedCompound())
            {
                vector<int>::iterator itStressedSyll = vecStress.begin();
                for (; itStressedSyll != vecStress.end(); ++itStressedSyll)
                {
                    spWordForm->SetStressPos(*itStressedSyll, STRESS_PRIMARY);
                }
                m_spInflection->AddWordForm(spWordForm);
            }
            else
            {
                vector<int>::iterator itStressedSyll = vecStress.begin();
                for (; itStressedSyll != vecStress.end(); ++itStressedSyll)
                {
                    if (itStressedSyll != vecStress.begin())
                    {
                        auto spWfVariant = make_shared<CWordForm>();
//                        CloneWordForm(spWordForm, spWfVariant);
                        spWfVariant->eCloneFrom(spWordForm);
                        spWordForm = spWfVariant;
                    }
//                    spWordForm->m_mapStress[*itStressedSyll] = STRESS_PRIMARY;
                    spWordForm->SetStressPos(*itStressedSyll, STRESS_PRIMARY);
                    m_spInflection->AddWordForm(spWordForm);
                }
            }
        }       //  for (int iEnding = 0;...)
    }
    catch (CException& ex)
    {
        CEString sMsg (L"Exception: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG (sMsg);
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eBuildInfinitive()

ET_ReturnCode CFormBuilderNonFinite::eBuildPresentActiveParticiple()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    //
    // No participle?
    //
    if (m_spLexeme->bHasMissingForms())
    {
        CGramHasher hasher (POS_VERB, SUBPARADIGM_PART_PRES_ACT, CASE_NOM, NUM_SG, GENDER_M, 
                            PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());
        if (m_spLexeme->bHasMissingForms() && m_spInflection->eFormExists(hasher.sGramHash()) != H_TRUE)  // if N. Sg. m doesn't exist all other forms don't exist
        {
            return H_NO_MORE;
        }
    }

    //
    // Irregular participle
    //
    rc = eBuildIrregParticipialFormsLong (SUBPARADIGM_PART_PRES_ACT);
    if (H_NO_ERROR == rc)
    {
        return rc;
    }

    if (L"св" == m_spLexeme->sMainSymbol())
    {
        return H_NO_ERROR;
    }

    //
    // Irregular present tense
    //
    try
    {
        if (bHasIrregularPresent())
        {
            rc = eDeriveIrregPresActiveParticiple();
            return rc;
        }
    }
    catch (CException&)
    {
        return H_EXCEPTION;
    }

    //
    // Regular present tense
    //
//    int iType = m_spInflection->iType();
    CEString sStem;

    CGramHasher hasher (POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_PL, GENDER_UNDEFINED, 
                        PERSON_3, ANIM_UNDEFINED, m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());

    auto nFormCount = m_spInflection->iFormCount(hasher.sGramHash());
    if (nFormCount < 1)
    {
//        assert(0);
        CEString sMsg(L"Failed to obtain praes. 3 pl. form; lexeme = ");
        sMsg += m_spLexeme->sSourceForm();
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

    for (auto n3PlWf = 0; n3PlWf < nFormCount; ++n3PlWf)
    {
        auto sp3PlWf = make_shared<CWordForm>();
        rc = m_spInflection->eWordFormFromHash(hasher.sGramHash(), n3PlWf, sp3PlWf);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }
        if (nullptr == sp3PlWf)
        {
            assert(0);
            ERROR_LOG(L"Failed to obtain praes. 3 pl. form.");
            return H_ERROR_POINTER;
        }

        sStem = sp3PlWf->sWordForm();
        int iErase = (REFL_YES == m_spLexeme->eIsReflexive()) ? 3 : 1;
        if (iErase >= (int)sStem.uiLength())
        {
            assert(0);
            ERROR_LOG(L"Illegal stem length.");
            return H_ERROR_UNEXPECTED;
        }
        sStem.sErase(sStem.uiLength() - iErase);
        sStem += L'щ';

        ET_Subparadigm eSp(SUBPARADIGM_PART_PRES_ACT);
        vector<int> vecStressPos;
        rc = eGetParticipleStressPos(eSp, vecStressPos);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        vector<int>::iterator itStress = vecStressPos.begin();
        for (; itStress != vecStressPos.end(); ++itStress)
        {
            CFormBuilderLongAdj builder(m_spLexeme, m_spInflection, sStem, AT_A, eSp, *itStress);
            rc = builder.eBuildParticiple();
        }
    }

    return rc;

}   //  eBuildPresentActiveParticiple()

ET_ReturnCode CFormBuilderNonFinite::eBuildPresentAdverbial()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    if (L"св" == m_spLexeme->sMainSymbol())
    {
        return H_NO_ERROR;
    }

    ET_ReturnCode rc = H_NO_ERROR;

    CGramHasher advPresHasher(POS_VERB, SUBPARADIGM_ADVERBIAL_PRESENT, CASE_UNDEFINED, NUM_UNDEFINED,
                              GENDER_UNDEFINED, PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(), 
                              m_spLexeme->eIsReflexive());

    if (m_spLexeme->bHasMissingForms())
    {
        CEString sHash = advPresHasher.sGramHash();
        if (m_spLexeme->bHasMissingForms() && m_spInflection->eFormExists(sHash) != H_TRUE)
        {
            return H_FALSE;
        }
    }

    //
    // Irregular adverbial?
    //
    map<shared_ptr<CWordForm>, bool> mapIrreg;
    rc = m_spInflection->eGetIrregularForms(advPresHasher.sGramHash(), mapIrreg);
    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    auto it = mapIrreg.begin();
    for (; it != mapIrreg.end(); ++it)
    {
        auto spWordForm = it->first;

        try
        {
            m_spInflection->AddWordForm(spWordForm);
        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
            return H_EXCEPTION;
        }
    }
    
    if (m_spInflection->bNoRegularForms(advPresHasher.sGramHash()))
    {
        return mapIrreg.empty() ? H_ERROR_UNEXPECTED : H_NO_ERROR;
    }

    int iType = m_spInflection->iType();
    if (3 == iType || 8 == iType || 9 == iType || 11 == iType || 
        (14 == iType && m_spLexeme->stGetProperties().iSection != 17) || 15 == iType)
    {
        return H_NO_ERROR;
    }

    //
    // Irregular 3 Pl pres.? 
    //
    try
    {
        CGramHasher pl3Hash (SUBPARADIGM_PRESENT_TENSE, NUM_PL, GENDER_UNDEFINED, PERSON_3, ANIM_UNDEFINED,
                             m_spLexeme->eAspect(), CASE_UNDEFINED, m_spLexeme->eIsReflexive());
        if (m_spInflection->bHasIrregularForm(pl3Hash.sGramHash()))
        {
            rc = eDeriveIrregPresAdverbial();
//            if (rc != H_NO_ERROR)
//            {
                return rc;
//            }
        }
    }
    catch (CException& ex)
    {
        CEString sMsg (L"Exception: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG (sMsg);
        return H_EXCEPTION;
    }

    //
    // Regular present tense
    //
    rc = ePresAdvGeneral();

    return rc;

}   //  eBuildPresentAdverbial()

ET_ReturnCode CFormBuilderNonFinite::ePresAdvGeneral(ET_Subparadigm eSubparadigm) // may be used as a past adv hence the argument
{
/*
    Деепричастие настоящего времени
    Имеется только у глаголов несовершенного вида, причем только в типах 1, 2, 4—7, 10, 12, 13, 16. 
    Образуется от основы 3 мн. наст, добавлением - я (после шипящих - а). Ударение как в 1 ед.наст. 
    Исключение: в типе 13 деепричастие образуется от основы инфинитива (с ударением инфинитива) 
    (дава́ть — дава́я).
*/

    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    auto spInfWf = make_shared<CWordForm>();  // needed for type 13 verbs only;

    CEString sStem;
    map<int, ET_StressType> mapStress;
    vector<CEString> vecEndings;

    try
    {
        //
        // Stem
        //
        {
            CEString sGramHash;
            if (m_spInflection->iType() == 13)
            {
                CGramHasher infHasher(POS_VERB, SUBPARADIGM_INFINITIVE, CASE_UNDEFINED, NUM_UNDEFINED,
                    GENDER_UNDEFINED, PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(),
                    m_spLexeme->eIsReflexive());
                sGramHash = infHasher.sGramHash();

                rc = m_spInflection->eWordFormFromHash(infHasher.sGramHash(), 0, spInfWf);
                if (rc != H_NO_ERROR || nullptr == spInfWf)
                {
//                    assert(0);
                    CEString sMsg(L"Failed to obtain word form: ");
                    sMsg += sGramHash + L" ";
                    sMsg += m_spLexeme->sInfinitive();
                    ERROR_LOG(sMsg);
                    return rc;
                }
            }
            else
            {
                CGramHasher hasher3Pl(POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_PL,
                    GENDER_UNDEFINED, PERSON_3, ANIM_UNDEFINED, m_spLexeme->eAspect(),
                    m_spLexeme->eIsReflexive());
                sGramHash = hasher3Pl.sGramHash();
            }

            auto nForms = m_spInflection->iFormCount(sGramHash);
            if (nForms < 1)
            {
//                assert(0);
                CEString sMsg(L"No regular forms: ");
                sMsg += m_spLexeme->sSourceForm();
                ERROR_LOG(sMsg);
                return H_ERROR_UNEXPECTED;
            }

            if (nForms > 1)
            {
//                assert(0);
                CEString sMsg(L"Warning: multiple forms for  ");
                sMsg += sGramHash + L" ";
                sMsg += m_spLexeme->sSourceForm();
                sMsg += L". Variants will be ignored.";
                ERROR_LOG(sMsg);
            }

            // Just get the 1st available wordfrom
            auto spSourceWf = make_shared<CWordForm>();
            rc = m_spInflection->eWordFormFromHash(sGramHash, 0, spSourceWf);
            if (rc != H_NO_ERROR || nullptr == spSourceWf)
            {
                assert(0);
                ERROR_LOG(L"Failed to obtain word form from hash.");
                return rc;
            }

            if (spSourceWf->sStem().bIsEmpty())
            {
                if (m_spInflection->bHasIrregularForm(sGramHash))
                {
                    int iCharsToRemove = (ET_Reflexivity::REFL_YES == m_spLexeme->eIsReflexive()) ? 4 : 2;
                    auto sStem = spSourceWf->sWordForm();
                    spSourceWf->SetStem(sStem.sRemoveCharsFromEnd(iCharsToRemove));
                }
                else
                {
                    assert(0);
                    CEString sMsg(L"No stem in regular form. ");
                    sMsg += sGramHash + L" ";
                    sMsg += m_spLexeme->sInfinitive();
                    sMsg += sGramHash + L".";
                    ERROR_LOG(sMsg);
                    return H_ERROR_UNEXPECTED;
                }
            }

            sStem = spSourceWf->sStem();
        }

        //
        // Stress
        //
        {
            if (m_spInflection->iType() == 13)
            {
                if (nullptr == spInfWf)
                {
//                    assert(0);
                    CEString sMsg(L"No word form ");
                    sMsg += m_spLexeme->sSourceForm();
                    sMsg += L".";
                    ERROR_LOG(sMsg);
                    return H_ERROR_UNEXPECTED;
                }

                mapStress = spInfWf->mapGetStressPositions();
            }
            else
            {
                CGramHasher hasher1Sg(POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_SG,
                    GENDER_UNDEFINED, PERSON_1, ANIM_UNDEFINED, m_spLexeme->eAspect(),
                    m_spLexeme->eIsReflexive());

                CEString sGramHash1Sg = hasher1Sg.sGramHash();

                auto sp1SgWf = make_shared<CWordForm>();
                rc = m_spInflection->eWordFormFromHash(hasher1Sg.sGramHash(), 0, sp1SgWf);
                if (rc != H_NO_ERROR || nullptr == sp1SgWf)
                {
                    assert(0);
                    CEString sMsg(L"Failed to obtain word form: ");
                    sMsg += hasher1Sg.sGramHash() + L" ";
                    sMsg += m_spLexeme->sSourceForm();
                    ERROR_LOG(sMsg);
                    return rc;
                }
                mapStress = sp1SgWf->mapGetStressPositions();
            }
        }

        m_spEndings = make_shared<CAdverbialEndings>(m_spLexeme, m_spInflection);
        bool bHusher = sStem.bEndsWithOneOf(CEString::g_szRusHushers) ? true : false;
        bool bVStem = false;        // ignored for pres
        bool bIsVariant = false;
        static_pointer_cast<CAdverbialEndings>(m_spEndings)->eSelect(ET_Subparadigm::SUBPARADIGM_ADVERBIAL_PRESENT, bHusher, bVStem, bIsVariant);

        int64_t iNumEndings = m_spEndings->iCount();
        if (iNumEndings < 1)
        {
            CEString sMsg(L"No pres. adverbial endings: ");
            sMsg += m_spLexeme->sSourceForm();
            ERROR_LOG(sMsg);
            return H_ERROR_UNEXPECTED;
        }

        for (int iEnding = 0; (rc == H_NO_ERROR && iEnding < iNumEndings); ++iEnding)
        {
            CEString sEnding;
            int64_t llEndingKey = -1;
            rc = m_spEndings->eGetEnding(iEnding, sEnding, llEndingKey);
            if (rc != H_NO_ERROR)
            {
                continue;
            }

            auto spWordForm = make_shared<CWordForm>(m_spInflection);
            if (nullptr == spWordForm)
            {
                assert(0);
                ERROR_LOG(L"Unable to instantiate CWordForm.");
                return H_ERROR_POINTER;
            }

//            spWordForm->m_spLexeme = m_spLexeme;
            spWordForm->SetPos(POS_VERB);
            spWordForm->SetSubparadigm(eSubparadigm);   // usually SUBPARADIGM_ADVERBIAL_PRESENT 
                                                        // but can be SUBPARADIGM_ADVERBIAL_PAST with
                                                        // std deviation "9", see p. 83
            spWordForm->SetAspect(m_spLexeme->eAspect());
            spWordForm->SetReflexivity(m_spLexeme->eIsReflexive());
//            spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();
            spWordForm->SetInflectionId(m_spInflection->llInflectionId());
            spWordForm->SetEndingDataId(llEndingKey);
            spWordForm->SetStem(sStem);
            spWordForm->AssignStress(mapStress);
            spWordForm->SetEnding(sEnding);
            spWordForm->SetWordForm(sStem + sEnding);
        
            m_spInflection->AddWordForm(spWordForm);
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

}   //  ePresAdvGeneral()

ET_ReturnCode CFormBuilderNonFinite::eBuildPastAdverbial()
{
/*
    Деепричастие прошедшего времени.
    Имеется у глаголов обоих видов. Однако достаточно употребительны только деепричастия от глаголов совершенного вида; 
    деепричастия от глаголов несовершенного вида в современном языке употребляются очень редко (обычно они заменяются
    деепричастиями наст. времени).

    Образуется от причастия прош. времени заменой -ший(ся) на -ши(сь). 
    Деепричастия на -вши имеют параллельную форму на -в, например, ви́девши и ви́дев (однако соответствующая возвратная форма 
    оканчивается только на -вшись, например, ви́девшись). 
    Особый случай: у невозвратных глаголов совершенного вида типа 9 и подтипа 3° наряду с регулярной формой деепричастия возможна 
    форма соответственно на -ерев и на -нув (-нувши), получаемая заменой конечного -ть инфинитива на -в (с сохранением ударения 
    инфинитива), например, растёрши и растере́в от растере́ть, поги́бши и поги́бнув (поги́бнувши)от поги́бнуть. 
    Другой особый случай образуют глаголы с пометой ⑨; см. стр. 83.    
 
 */

    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    if (m_spLexeme->bHasMissingForms())
    {
        CGramHasher advPastHasher(POS_VERB, SUBPARADIGM_ADVERBIAL_PAST, CASE_UNDEFINED, NUM_UNDEFINED,
            GENDER_UNDEFINED, PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(),
            m_spLexeme->eIsReflexive());

        CEString sHash = advPastHasher.sGramHash();
        if (m_spLexeme->bHasMissingForms() && m_spInflection->eFormExists(sHash) != H_TRUE)
        {
            return H_FALSE;
        }
    }

    //
    // Irregular past adverbial
    //
    try
    {
        CGramHasher pastAdv (POS_VERB, SUBPARADIGM_ADVERBIAL_PAST, CASE_UNDEFINED,
                             NUM_UNDEFINED, GENDER_UNDEFINED, PERSON_UNDEFINED, ANIM_UNDEFINED,
                             m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());

        map<shared_ptr<CWordForm>, bool> mapIrreg;
        rc = m_spInflection->eGetIrregularForms(pastAdv.sGramHash(), mapIrreg);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        auto it = mapIrreg.begin();
        for (; it != mapIrreg.end(); ++it)
        {
            auto spWordForm = it->first;

            try
            {
                m_spInflection->AddWordForm(spWordForm);
            }
            catch (CException& ex)
            {
                ERROR_LOG(ex.szGetDescription());
                return H_EXCEPTION;
            }
        }

        if (m_spInflection->bNoRegularForms(pastAdv.sGramHash()))
        {
            return mapIrreg.empty() ? H_ERROR_UNEXPECTED : H_NO_ERROR;
        }
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

/*
    if (bHasIrregularPast())
    {
        return hr; // has been done already -- combined with past part act?
    }
*/
 
    if (m_spInflection->bHasCommonDeviation(9))
    {
        if (L"св" != m_spLexeme->sMainSymbol())
        {
            assert(0);
            ERROR_LOG(L"Non-perfective verb with CD-9");
            return H_ERROR_UNEXPECTED;
        }

        rc = ePresAdvGeneral(SUBPARADIGM_ADVERBIAL_PAST); // GDRL p. 83: use present form
        if (rc != H_NO_ERROR)
        {
            return rc;
        }
    }

    try
    {
        CGramHasher partPastAct(POS_VERB, SUBPARADIGM_PART_PAST_ACT, CASE_NOM, NUM_SG, GENDER_M,
            PERSON_UNDEFINED, ANIM_NO, m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());

        CEString sGramHash = partPastAct.sGramHash();
        auto nForms = m_spInflection->iFormCount(sGramHash);
        if (nForms < 1)
        {
            assert(0);
            CEString sMsg(L"No regular forms: ");
            sMsg += m_spLexeme->sSourceForm();
            ERROR_LOG(sMsg);
            return H_ERROR_UNEXPECTED;
        }

        if (nForms > 1)
        {
//            assert(0);
            CEString sMsg(L"Warning: multiple forms for  ");
            sMsg += sGramHash + L" ";
            sMsg += m_spLexeme->sSourceForm();
            sMsg += L". Variants will be ignored.";
            ERROR_LOG(sMsg);
        }

        // Just get the 1st available wordfrom
        auto spPastPart = make_shared<CWordForm>();
        rc = m_spInflection->eWordFormFromHash(sGramHash, 0, spPastPart);
        if (rc != H_NO_ERROR || nullptr == spPastPart)
        {
//            assert(0);
            CEString sMsg(L"Failed to obtain word form from hash; lexeme = ");
            sMsg += m_spLexeme->sSourceForm();
            ERROR_LOG(sMsg);
            return rc;
        }

        CEString sStem = spPastPart->sStem();
        sStem.sRemoveCharsFromEnd(1);

        if (m_spInflection->bHasCommonDeviation(5))
        {
            if (sStem.bEndsWith(L"нув"))
            {
                sStem.sRemoveCharsFromEnd(3);
            }
            else if (sStem.bEndsWith(L"в"))
            {
                sStem.sRemoveCharsFromEnd(1);
            }
        }

        m_spEndings = make_shared<CAdverbialEndings>(m_spLexeme, m_spInflection);
        bool bIsReflexive = (m_spLexeme->eIsReflexive() == REFL_YES) ? true : false;
        bool bHusher = false;       // not needed for past adv.
        bool bVStem = false;
        bool bIsVariant = false;
        bool bNuPreserving = false;

        if (m_spInflection->bHasCommonDeviation(6) && !m_spInflection->bDeviationOptional(6))
        {
            bNuPreserving = true;
        }

        bool bRepeat;
        do {
            bRepeat = false;

            if (!bNuPreserving)
            {
                static_pointer_cast<CAdverbialEndings>(m_spEndings)->eSelect(ET_Subparadigm::SUBPARADIGM_ADVERBIAL_PAST, bHusher, bVStem, bIsVariant);
            }
            else
            { 
                static_pointer_cast<CAdverbialEndings>(m_spEndings)->eSelectPastAdvAugmentedEndings(SUBPARADIGM_ADVERBIAL_PAST, bIsVariant);
            }

            int64_t iNumEndings = m_spEndings->iCount();
            if (iNumEndings < 1)
            {
                CEString sMsg(L"No past adverbial endings: ");
                sMsg += m_spLexeme->sSourceForm();
                ERROR_LOG(sMsg);
                return H_ERROR_UNEXPECTED;
            }

            if (!bIsReflexive)
            {
                bVStem = sStem.bEndsWith(L"в") ? true : false;
                if (bVStem)
                {
                    bIsVariant = true;
                    static_pointer_cast<CAdverbialEndings>(m_spEndings)->eSelect(ET_Subparadigm::SUBPARADIGM_ADVERBIAL_PAST, bHusher, bVStem, bIsVariant);
                }
            }

            for (int iEnding = 0; (rc == H_NO_ERROR && iEnding < m_spEndings->iCount()); ++iEnding)
            {
                CEString sEnding;
                int64_t llEndingKey = -1;
                rc = m_spEndings->eGetEnding(iEnding, sEnding, llEndingKey);
                if (rc != H_NO_ERROR)
                {
                    continue;
                }

                auto spWordForm = make_shared<CWordForm>(m_spInflection);
                if (nullptr == spWordForm)
                {
                    assert(0);
                    ERROR_LOG(L"Unable to instantiate CWordForm");
                    return H_ERROR_POINTER;
                }

//                spWordForm->m_spLexeme = m_spLexeme;
                spWordForm->SetPos(POS_VERB);
                spWordForm->SetSubparadigm(SUBPARADIGM_ADVERBIAL_PAST);
                spWordForm->SetStem(sStem);
                if (bNuPreserving)
                {
                    if (!spWordForm->sStem().bEndsWith(L"ну"))       // for variants, we may have stems with and without nu
                    {
                        spWordForm->SetStem(sStem + L"ну");
                    }
                }
                spWordForm->SetAspect(m_spLexeme->eAspect());
                spWordForm->SetReflexivity(m_spLexeme->eIsReflexive());
//                spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();
                spWordForm->SetInflectionId(m_spInflection->llInflectionId());
                spWordForm->SetEndingDataId(llEndingKey);
                spWordForm->SetWordForm(spWordForm->sStem() + sEnding);
                spWordForm->AssignStress(spPastPart->mapGetStressPositions());

                if (L"нсв" == m_spLexeme->sMainSymbol())
                {
                    spWordForm->SetStatus(STATUS_RARE);
                }

                if (L"св" == m_spLexeme->sMainSymbol() && m_spInflection->bHasCommonDeviation(9))
                {
                    spWordForm->SetStatus(STATUS_OBSOLETE);
                }

                m_spInflection->AddWordForm(spWordForm);

            }       //  for (int iEnding = 0; ...

            if (!bNuPreserving && m_spInflection->bHasCommonDeviation(6) && m_spInflection->bDeviationOptional(6))
            {
                bNuPreserving = true;
//                bIsVariant = true;
                bRepeat = true;
            }
        
        } while (bRepeat);

        // GDRL p. 85
        if (L"св" == m_spLexeme->sMainSymbol() && REFL_NO == m_spLexeme->eIsReflexive() &&
            ((3 == m_spInflection->iType() && m_spInflection->iStemAugment() == 1) ||                  // погибнуть
              9 == m_spInflection->iType()))                                                           // растереть
        {
            CGramHasher inf(POS_VERB, SUBPARADIGM_INFINITIVE, CASE_UNDEFINED, NUM_UNDEFINED, GENDER_UNDEFINED,
                PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(), REFL_NO);

            auto spInfinitive = make_shared<CWordForm>();
            auto nInfForms = m_spInflection->iFormCount(inf.sGramHash());
            //            assert(m_spInflection->iFormCount(inf.sGramHash()) == 1);

            if (nInfForms < 1)
            {
                //                assert(0);
                CEString sMsg(L"No infinitive; lexeme =  ");
                sMsg += m_spLexeme->sSourceForm();
                ERROR_LOG(sMsg);
                return H_ERROR_UNEXPECTED;
            }

            if (nForms > 1)
            {
                //                assert(0);
                CEString sMsg(L"Warning: multiple forms for  ");
                sMsg += sGramHash + L" ";
                sMsg += m_spLexeme->sSourceForm();
                sMsg += L". Variants will be ignored.";
                ERROR_LOG(sMsg);
            }

            rc = m_spInflection->eWordFormFromHash(inf.sGramHash(), 0, spInfinitive);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }

            if (nullptr == spInfinitive)
            {
                assert(0);
                ERROR_LOG(L"Failed to obtain infinitive.");
                return H_ERROR_POINTER;
            }

            auto spWordForm = make_shared<CWordForm>(m_spInflection);
            if (nullptr == spWordForm)
            {
                assert(0);
                ERROR_LOG(L"Unable to instantiate CWordForm");
                return H_ERROR_POINTER;
            }

//            spWordForm->m_spLexeme = m_spLexeme;
            spWordForm->SetPos(POS_VERB);
            spWordForm->SetSubparadigm(SUBPARADIGM_ADVERBIAL_PAST);
            spWordForm->SetAspect(m_spLexeme->eAspect());
            spWordForm->SetReflexivity(m_spLexeme->eIsReflexive());
//            spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();
            spWordForm->SetInflectionId(m_spInflection->llInflectionId());

            spWordForm->SetStem(m_spLexeme->sInfStem());

            if (nullptr == m_spEndings)
            {
                assert(0);
                ERROR_LOG(L"What happened to the the endings object?");
                return H_ERROR_POINTER;
            }

//            bool bIsReflexive = m_spLexeme->eIsReflexive() ? true : false;
//            bool bHusher = false;
//            bool bVStem = true;
            bool bIsVariant = false;
            static_pointer_cast<CAdverbialEndings>(m_spEndings)->eSelectPastAdvAugmentedEndings(SUBPARADIGM_ADVERBIAL_PAST, bIsVariant);

            int64_t iNumEndings = m_spEndings->iCount();
            if (iNumEndings < 1)
            {
                CEString sMsg(L"No past adverbial endings: ");
                sMsg += m_spLexeme->sInfinitive();
                ERROR_LOG(sMsg);
                return H_ERROR_UNEXPECTED;
            }

            for (int iEnding = 0; iEnding < m_spEndings->iCount(); ++ iEnding)
            {
                CEString sEnding;
                int64_t llEndingKey = -1;
                rc = m_spEndings->eGetEnding(iEnding, sEnding, llEndingKey);
                if (rc != H_NO_ERROR)
                {
                    return H_ERROR_UNEXPECTED;
                }

                if (iEnding > 0)
                {
                    auto spVariant = make_shared<CWordForm>();
//                    CloneWordForm(spWordForm, spVariant);
                    spVariant->eCloneFrom(spWordForm);
                    spWordForm = spVariant;
                }
                spWordForm->SetEndingDataId(llEndingKey);
                spWordForm->SetWordForm(spWordForm->sStem() + sEnding);
                spWordForm->AssignStress(spInfinitive->mapGetStressPositions());
                m_spInflection->AddWordForm(spWordForm);
            }
        }
    }
    catch (CException& ex)
    {
        CEString sMsg(L"Exception: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG(sMsg);
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eBuildPastAdverbial()

ET_ReturnCode CFormBuilderNonFinite::eBuildPresentPassiveParticiple()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    m_spLexeme->SetHasPresPassParticiple(false);

    //
    // Irregular participle
    //
    rc = eBuildIrregParticipialFormsLong (SUBPARADIGM_PART_PRES_PASS_LONG);

    if (L"св" == m_spLexeme->sMainSymbol() || !m_spLexeme->bTransitive() || REFL_YES == m_spLexeme->eIsReflexive())
    {
        return H_NO_ERROR;
    }

    //
    // Irregular present tense
    //
    try
    {
        if (bHasIrregularPresent())
        {
            rc = eDeriveIrregPresPassiveParticiple();
            return rc;
        }
    }
    catch (CException&)
    {
        return H_EXCEPTION;
    }

    //
    // Regular present tense
    //
    CGramHasher formHasher(POS_VERB, SUBPARADIGM_PART_PRES_PASS_LONG, CASE_NOM, NUM_SG, GENDER_M,
        PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());

    if (m_spLexeme->bHasMissingForms() && m_spInflection->eFormExists(formHasher.sGramHash()) != H_TRUE)  // if N. Sg. m doesn't exist all other forms don't exist
    {
        return H_NO_MORE;
    }

    CGramHasher sourceHasher (POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_PL, GENDER_UNDEFINED, 
                                  PERSON_1, ANIM_UNDEFINED, m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());

    if (m_spLexeme->bHasMissingForms() && m_spInflection->eFormExists(sourceHasher.sGramHash()) != H_TRUE)
    {
        return H_NO_MORE;
    }

    auto sp1Pl = make_shared<CWordForm>();

    auto n1PlForms = m_spInflection->iFormCount(sourceHasher.sGramHash());
    if (n1PlForms < 1)
    {
//        assert(0);
        CEString sMsg(L"Unable to acquire 1 Pl form; lexeme = ");
        sMsg += m_spLexeme->sSourceForm();
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

    for (auto nSourceForm = 0; nSourceForm < n1PlForms; ++nSourceForm)
    {
        rc = m_spInflection->eWordFormFromHash(sourceHasher.sGramHash(), nSourceForm, sp1Pl);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }
        if (nullptr == sp1Pl)
        {
            assert(0);
            ERROR_LOG(L"Failed to obtain infinitive.");
            return H_ERROR_POINTER;
        }

        //        CEString& s1Pl = p1Pl->m_sWordForm;

        rc = eBuildPresPassPartFromSourceForm(sp1Pl);
    }

    return H_NO_ERROR;

}   //  eBuildPresentPassPart()

ET_ReturnCode CFormBuilderNonFinite::eBuildPastActiveParticiple()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    //
    // No participle?
    //
    if (m_spLexeme->bHasMissingForms())
    {
        CGramHasher hasher(POS_VERB, SUBPARADIGM_PART_PAST_ACT, CASE_NOM, NUM_SG, GENDER_M,
            PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());
        if (m_spLexeme->bHasMissingForms() && m_spInflection->eFormExists(hasher.sGramHash()) != H_TRUE)  // if N. Sg. m doesn't exist all other forms don't exist
        {
            return H_NO_MORE;
        }
    }

    //
    // Irregular participle
    //
    rc = eBuildIrregParticipialFormsLong (SUBPARADIGM_PART_PAST_ACT);
    if (H_NO_ERROR == rc)
    {
        return rc;
    }

    //
    // Irregular past tense
    //
    try
    {
        if (bHasIrregularPast())
        {
            rc = eDeriveIrregPastActPartAndAdverbial();
            return rc;
        }
    }
    catch (CException&)
    {
        return H_EXCEPTION;
    }

    //
    // Regular participle
    //
    int iType = m_spInflection->iType();

    auto spWordForm = make_shared<CWordForm>(m_spInflection);
    if (nullptr == spWordForm)
    {
        assert(0);
        ERROR_LOG (L"Unable to instantiate CWordForm");
        return H_ERROR_POINTER;
    }

//    spWordForm->m_spLexeme = m_spLexeme;
//    spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();
    spWordForm->SetInflectionId(m_spInflection->llInflectionId());
    spWordForm->SetAspect(m_spLexeme->eAspect());

    //
    // Type 7 ending in -сти with -т- or -д- alternation mark:
    // 1 Sg stem present tense + -ший
    //
    if (7 == iType && 
        ((m_spLexeme->sSourceForm().bEndsWith(L"сти") || (m_spLexeme->sSourceForm().bEndsWith(L"стись"))) &&
        (L"т" == m_spLexeme->sVerbStemAlternation() || L"д" == m_spLexeme->sVerbStemAlternation())))
    {
        auto sStem = m_spLexeme->s1SgStem();
        spWordForm->SetStem(sStem + L"ш");
    }
    else
    {
        // All other types: derived from Past Sg. m 
        CGramHasher hasher(POS_VERB, SUBPARADIGM_PAST_TENSE, CASE_UNDEFINED, NUM_SG, GENDER_M,
            PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(),
            m_spLexeme->eIsReflexive());
        auto spPastM = make_shared<CWordForm>();
        auto nPastM = m_spInflection->iFormCount(hasher.sGramHash());
        if (nPastM < 1)
        {
            assert(0);
            ERROR_LOG(L"Failed to obtain past tense m. form.");
            return H_ERROR_UNEXPECTED;
        }

        for (auto nSourceForm = 0; nSourceForm < nPastM; ++nSourceForm)
        {
            rc = m_spInflection->eWordFormFromHash(hasher.sGramHash(), nSourceForm, spPastM);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
            if (nullptr == spPastM)
            {
                assert(0);
                ERROR_LOG(L"Failed to obtain past tense m. form.");
                return H_ERROR_POINTER;
            }

            CEString sWf(spPastM->sWordForm());
            if (REFL_YES == m_spLexeme->eIsReflexive())
            {
                if (sWf.uiLength() < 3)
                {
                    assert(0);
                    ERROR_LOG(L"Illegal stem length.");
                    return H_ERROR_GENERAL;
                }

                sWf.sErase(sWf.uiLength() - 2);
            }

            //
            // If past Sg. m ends in -л, replace it with -вший, otherwise add -ший
            //
            if (sWf.bEndsWith(L"л"))
            {
                auto sStem = sWf.sErase(sWf.uiLength() - 1);
                spWordForm->SetStem(sStem + L"вш");
            }
            else
            {
                spWordForm->SetStem(sWf + L"ш");
            }
        }       //  for (auto nSourceForm = 0; nSourceForm < nPastM; ++nSourceForm)

    }

    ET_Subparadigm eSp(SUBPARADIGM_PART_PAST_ACT);
    vector<int> vecStressPos;
    rc = eGetParticipleStressPos(eSp, vecStressPos);
    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    vector<int>::iterator itStress = vecStressPos.begin();
    for (; itStress != vecStressPos.end(); ++itStress)
    {
        CFormBuilderLongAdj builder(m_spLexeme, m_spInflection, spWordForm->sStem(), AT_A, eSp, *itStress);
        rc = builder.eBuildParticiple();
    }

    return rc;

}   //  h_BuildPastActiveParticiple()

ET_ReturnCode CFormBuilderNonFinite::eBuildPastPassiveParticiple()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    CGramHasher formHasher(POS_VERB, SUBPARADIGM_PART_PAST_PASS_LONG, CASE_NOM, NUM_SG, GENDER_M,
        PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(),
        m_spLexeme->eIsReflexive());

    if (m_spLexeme->bHasMissingForms() && m_spInflection->eFormExists(formHasher.sGramHash()) != H_TRUE)  // if N. Sg. m doesn't exist all other forms don't exist
    {
        return H_NO_MORE;
    }

    //
    // Irregular participle
    //
    rc = eBuildIrregParticipialFormsLong (SUBPARADIGM_PART_PAST_PASS_LONG);
    if (H_NO_ERROR == rc)
    {
        if (bHasIrregPartPastPassShort())
        {
            bool bIsVariant = false;
            rc = eBuildIrregPartPastPassShort (bIsVariant);
            if (!bIsVariant || rc != H_NO_ERROR)
            {
                return rc;
            }
        }
        else
        {
            rc = eDeriveIrregPartPastPassShort();
            return rc;
        }
    }

    ET_Status eStatus = STATUS_COMMON;

    if (!m_spLexeme->bTransitive() || REFL_YES == m_spLexeme->eIsReflexive())
    {
        return H_NO_ERROR;
    }

    if (m_spInflection->bNoPassivePastParticiple())
    {
        return H_NO_ERROR;
    }

    if (L"нсв" == m_spLexeme->sMainSymbol())
    {
        if (m_spLexeme->bHasAspectPair())
        {
            return H_NO_ERROR;
        }
        else
        {
            eStatus = STATUS_RARE;
        }
    }

    if (m_spInflection->bPastParticipleRestricted())
    {
        eStatus = STATUS_QUESTIONABLE;
    }

    CEString sStem;
    if ((m_spLexeme->sSourceForm().bEndsWith (L"ать") || 
         m_spLexeme->sSourceForm().bEndsWith (L"ять")) &&
        14 != m_spInflection->iType())
    {
        sStem = m_spLexeme->sInfStem();
        if (2 == m_spInflection->iType() && m_spLexeme->bHasOAlternation())
        {
            if (sStem.uiLength() <= 3 || !sStem.bEndsWith (L"ева"))
            {
                assert(0);
                CEString sMsg (L"Stem with o-alternation does not end in 'eva'; lexeme = ");
                sMsg += m_spLexeme->sSourceForm();
                ERROR_LOG (sMsg);
                throw CException (H_ERROR_UNEXPECTED, sMsg);
            }
            sStem[sStem.uiLength()-3] = L'о';
        }

        sStem += L"нн";
    }

    if (4 == m_spInflection->iType() || 
        (5 == m_spInflection->iType() && m_spLexeme->sSourceForm().bEndsWith (L"еть")))
    {
        sStem = m_spLexeme->s1SgStem();
        if (m_spLexeme->bPartPastPassZhd())
        {
            assert(sStem.bEndsWith(L"ж"));
            sStem += L"д";
        }
        sStem += L"енн";
    }

    if (1 == m_spInflection->iType() && m_spLexeme->sSourceForm().bEndsWith (L"еть"))
    {
        if (m_spLexeme->sSourceForm().bEndsWith (L"одолеть") ||
            m_spLexeme->sSourceForm().bEndsWith (L"печатлеть"))
        {
            sStem = m_spLexeme->sSourceForm();
            sStem.sErase (m_spLexeme->sSourceForm().uiLength()-3);
            sStem += L"ённ";
        }
    }

    if (7 == m_spInflection->iType() || 8 == m_spInflection->iType())
    {
        sStem = m_spLexeme->s3SgStem();
        sStem += L"енн";
    }

    if (3 == m_spInflection->iType() || 10 == m_spInflection->iType())        // NB also 3 + circle
    {
        sStem = m_spLexeme->sInfStem();
        sStem += L"т";
    }

    if (9 == m_spInflection->iType() || 11 == m_spInflection->iType()  || 12 == m_spInflection->iType() || 
        14 == m_spInflection->iType() || 15 == m_spInflection->iType() || 16 == m_spInflection->iType())
    {
        CGramHasher sourceHasher(POS_VERB, SUBPARADIGM_PAST_TENSE, CASE_UNDEFINED, NUM_SG, GENDER_M,
            PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(),
            m_spLexeme->eIsReflexive());


        auto spPastM = make_shared<CWordForm>();
        int iNForms = m_spInflection->iFormCount (sourceHasher.sGramHash());
        for (int iWf = 0; iWf < iNForms; ++iWf)
        {
            rc = m_spInflection->eWordFormFromHash (sourceHasher.sGramHash(), iWf, spPastM);  
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
            if (nullptr == spPastM)
            {
                assert(0);
                ERROR_LOG (L"Failed to obtain past tense m. form.");
                return H_ERROR_POINTER;
            }

            sStem = spPastM->sWordForm();
            if (sStem.bEndsWith (L"л"))
            {
                sStem.sErase (sStem.uiLength()-1);
            }
            sStem += L"т";
        }

    }   //      if (9 == m_spInflection->iType() ...)

    ET_Subparadigm eSp (SUBPARADIGM_PART_PAST_PASS_LONG);
    vector<int> vecStress;
    rc = eGetParticipleStressPos (eSp, vecStress);
    if (rc != H_NO_ERROR || vecStress.empty())
    {
//        assert(0);
        CEString sMsg(L"Failed to obtain part pass past stress positions; lexeme = ");
        sMsg += m_spLexeme->sSourceForm();
        ERROR_LOG(sMsg);
        return H_ERROR_GENERAL;
    }

    if (sStem.bIsEmpty())
    {
        return H_ERROR_UNEXPECTED;
    }

    vector<int>::iterator itStress = vecStress.begin();
    for (; (H_NO_ERROR == rc) && (itStress != vecStress.end()); ++itStress)
    {
        if (4 == m_spInflection->iType() || 5 == m_spInflection->iType() || 
            7 == m_spInflection->iType() || 8 == m_spInflection->iType())
        {
            if (*itStress >= (int)sStem.uiLength())
            {
                assert(0);
                ERROR_LOG (L"Bad stress position.");
                rc = H_ERROR_GENERAL;
            }

            if (*itStress == sStem.uiNSyllables() - 1)
            {
                int iStressPos = sStem.uiGetVowelPos (*itStress);
                if (L'е' == sStem[iStressPos])
                {
                    sStem[iStressPos] = L'ё';
                }
            }
        }
        CFormBuilderLongAdj builder (m_spLexeme, m_spInflection, sStem, AT_A, eSp, *itStress, eStatus);
        rc = builder.eBuildParticiple();
    
    }   // for...

    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    //
    // Short forms
    //
    CGramHasher partPastPassLong (POS_VERB, SUBPARADIGM_PART_PAST_PASS_LONG, CASE_NOM, NUM_SG, 
                                  GENDER_M, PERSON_UNDEFINED, ANIM_NO, m_spLexeme->eAspect(), 
                                  m_spLexeme->eIsReflexive());
    auto spNSgMLong = make_shared<CWordForm>();
    int iNForms = m_spInflection->iFormCount (partPastPassLong.sGramHash());
    for (int iWf = 0; iWf < iNForms; ++iWf)
    {
        rc = m_spInflection->eWordFormFromHash (partPastPassLong.sGramHash(), iWf, spNSgMLong);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }
        if (nullptr == spNSgMLong)
        {
            assert(0);
            ERROR_LOG (L"Failed to obtain N Sg m of the long form.");
            return H_ERROR_POINTER;
        }

        CEString sNSgMLong (spNSgMLong->sWordForm());
//        int iPos = -1;
//        ET_StressType eType = ET_StressType::STRESS_TYPE_UNDEFINED;
        sNSgMLong.SetVowels (CEString::g_szRusVowels);
        if (sNSgMLong.bEndsWith (L"анный") || sNSgMLong.bEndsWith (L"янный") ||
            sNSgMLong.bEndsWith (L"енный"))
        {
//            auto itStress = spNSgMLong->m_mapStress.begin();
            auto& mapStress = spNSgMLong->mapGetStressPositions();
//            auto rcStress = spNSgMLong->eGetFirstStressSyll(iPos, eType);
//            for (; itStress != spNSgMLong->m_mapStress.end(); ++itStress)
            auto itStress = mapStress.begin();
//            while (H_NO_ERROR == rcStress)
            for (; itStress != mapStress.end(); ++itStress)
            {
                if (sNSgMLong.uiNSyllables() - 2 == (*itStress).first &&
                    STRESS_PRIMARY == (*itStress).second)
                {
                    ET_AccentType eAccentType = ET_AccentType::AT_UNDEFINED;
                    bool bYoAlternation = false;
                    bool bFleetingVowel = true;

                    //  -Анный/-Янный: rare, see GDRL p. 86 footnote 4
                    if (m_spLexeme->sSourceForm().uiNSyllables() == 1)
                    {
                        eAccentType = m_spInflection->eAccentType2();
                    }
                    else if ((m_spLexeme->sSourceForm().bEndsWithNoCase(L"ать")
                        || m_spLexeme->sSourceForm().bEndsWithNoCase(L"ять"))
                        && m_spInflection->bHasCommonDeviation(7))
                    {
                        eAccentType = AT_A;
                    }
                    else
                    {
                        ERROR_LOG(L"Unexpected part. past pass. ending in -Янный or -Анный");
                        return H_ERROR_UNEXPECTED;
                    }
                    CFormBuilderShortAdj shortAdj(m_spLexeme,
                        m_spInflection,
                        bYoAlternation,
                        spNSgMLong->sStem(),
                        SUBPARADIGM_PART_PAST_PASS_SHORT,
                        AT_A,
                        eAccentType,
                        vector<int>(1, (*itStress).first),
                        bFleetingVowel,
                        2,
                        eStatus);
                    if (STATUS_COMMON != eStatus)
                    {
                        shortAdj.SetUsageStatus(eStatus);
                    }
                    rc = shortAdj.eBuild();
                    if (rc != H_NO_ERROR)
                    {
                        return rc;
                    }

                }
                else
                {
                    bool bYoAlternation = false;
                    bool bFleetingVowel = true;
                    CFormBuilderShortAdj shortAdj(m_spLexeme,
                        m_spInflection,
                        bYoAlternation,
                        spNSgMLong->sStem(),
                        SUBPARADIGM_PART_PAST_PASS_SHORT,
                        AT_A,
                        AT_A,
                        vector<int>(1, (*itStress).first),
                        bFleetingVowel,
                        2,
                        eStatus);
                    if (STATUS_COMMON != eStatus)
                    {
                        shortAdj.SetUsageStatus(eStatus);
                    }
                    rc = shortAdj.eBuild();
                    if (rc != H_NO_ERROR)
                    {
                        return rc;
                    }
                }
            }
        }

        if (sNSgMLong.bEndsWith (L"ённый"))
        {
            if (spNSgMLong->iStressPositions() != 1)
            {
                assert(0);
                ERROR_LOG(L"WARNING: Unexpected number of stress positions.");
            }

            bool bYoAlternation = true;
            bool bFleetingVowel = true;
            int iPos = -1;
            ET_StressType eType = STRESS_TYPE_UNDEFINED;
            auto rcStress = spNSgMLong->eGetFirstStressSyll(iPos, eType);
            if (rcStress != H_NO_ERROR || eType != STRESS_PRIMARY || iPos != (int)sNSgMLong.uiGetNumOfSyllables()-2)
            {
                assert(0);
                ERROR_LOG (L"Unexpected stress position.");
                return H_ERROR_POINTER;
            }
            CFormBuilderShortAdj shortAdj (m_spLexeme, 
                                           m_spInflection,
                                           bYoAlternation, 
                                           spNSgMLong->sStem(),
                                           SUBPARADIGM_PART_PAST_PASS_SHORT, 
                                           AT_A, 
                                           AT_B,
                                           vector<int>(1, iPos),
                                           bFleetingVowel,
                                           2,
                                           eStatus);
            rc = shortAdj.eBuild();
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
        }

        if (sNSgMLong.bEndsWith (L"тый"))
        {
            if (spNSgMLong->iStressPositions() != 1)
            {
                assert(0);
                ERROR_LOG(L"WARNING: Unexpected number of stress positions.");
            }

            bool bYoAlternation = m_spLexeme->stGetProperties().bYoAlternation;
            bool bFleetingVowel = false;
//            auto itStress = spNSgMLong->m_mapStress.begin();
            int iPos = -1;
            ET_StressType eType = STRESS_TYPE_UNDEFINED;
            auto rcStress = spNSgMLong->eGetFirstStressSyll(iPos, eType);
            if (rcStress != H_NO_ERROR || eType != STRESS_PRIMARY)
            {
                assert(0);
                ERROR_LOG(L"Unexpected stress type.");
                return H_ERROR_POINTER;
            }

            ET_AccentType eAt2;
            if (AT_A == m_spInflection->eAccentType2() || AT_UNDEFINED == m_spInflection->eAccentType2())
            {
                eAt2 = AT_A;
            }
            else if (AT_C == m_spInflection->eAccentType2() || AT_C1 == m_spInflection->eAccentType2() 
                || AT_C2 == m_spInflection->eAccentType2())
            {
                eAt2 = m_spInflection->eAccentType2();
            }
            else
            {
                assert(0);
                ERROR_LOG (L"Unexpected accent type.");
                return H_ERROR_POINTER;
            }

            CFormBuilderShortAdj shortAdj (m_spLexeme, 
                                           m_spInflection,
                                           bYoAlternation, 
                                           sStem,
                                           SUBPARADIGM_PART_PAST_PASS_SHORT, 
                                           AT_A, 
                                           eAt2,
                                           vector<int>(1, iPos),
                                           bFleetingVowel);
            if (STATUS_COMMON != eStatus)
            {
                shortAdj.SetUsageStatus (eStatus);
            }
            rc = shortAdj.eBuild();
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
        }

    }   //  for (int iWf = 0 ...)

    return rc;

}   //  eBuildPastPassiveParticiple()

ET_ReturnCode CFormBuilderNonFinite::eBuildPresPassPartFromSourceForm(shared_ptr<CWordForm> sp1Pl)
{
    if (nullptr == sp1Pl)
    {
        return H_ERROR_POINTER;
    }

    auto&& sWordform = sp1Pl->sWordForm();
    auto uiLength = sWordform.uiLength();
    if (uiLength < 3)
    {
        ERROR_LOG(L"1st person plural must be at least three characters long.");
        return H_ERROR_UNEXPECTED;
    }

    int iGroup = -1;       // GDRL, p.86

    auto mapStress = sp1Pl->mapGetStressPositions();
    if (uiLength > 2 && sWordform.bEndsWith(L"ем") &&
        mapStress.find(uiLength-2) == mapStress.end() &&
        CEString::bIn(sWordform[uiLength-3], CEString::g_szRusVowels))
    {
        assert((1 == m_spInflection->iType() && AT_A == m_spInflection->eAccentType1()) ||
            (2 == m_spInflection->iType() && AT_A == m_spInflection->eAccentType1()) ||
            (12 == m_spInflection->iType() && AT_A == m_spInflection->eAccentType1()) ||
            (6 == m_spInflection->iType() && AT_A == m_spInflection->eAccentType1() &&
                m_spLexeme->sSourceForm().bEndsWith(L"ять")) || 13 == m_spInflection->iType());
        iGroup = 1;
    }
    else if (4 == m_spInflection->iType())
    {
        iGroup = 2;
    }
    else if (5 == m_spInflection->iType() ||
        (6 == m_spInflection->iType() && m_spLexeme->sSourceForm().bEndsWith(L"ать")) ||
        (6 == m_spInflection->iType() && 1 == m_spInflection->iStemAugment()) ||
        (7 == m_spInflection->iType()))
    {
        //        слышать видеть гнать (терпеть зреть?)
        if (5 == m_spInflection->iType())
        {
            vector<CEString> vecAllowed{ L"слышать", L"видеть", L"гнать" };     // Предисловие, с. 105
            auto sSourceForm = m_spLexeme->sSourceForm();
            if (!any_of(vecAllowed.begin(), vecAllowed.end(), [sSourceForm](CEString sAllowedForm) { return sSourceForm == sAllowedForm; }))
            {
                return H_FALSE;
            }
        }
        //        глаголать колебать колыхать
        if (6 == m_spInflection->iType())
        {
            vector<CEString> vecAllowed{ L"глаголать", L"колебать", L"колыхать", L"двигать" };
            auto sSourceForm = m_spLexeme->sSourceForm();
            if (!any_of(vecAllowed.begin(), vecAllowed.end(), [sSourceForm](CEString sAllowedForm) { return sSourceForm == sAllowedForm; }))
            {
                return H_FALSE;
            }
        }

        //        везти пасти нести вести
        if (7 == m_spInflection->iType())
        {
            vector<CEString> vecAllowed{ L"везти", L"пасти", L"нести", L"вести" };
            auto sSourceForm = m_spLexeme->sSourceForm();
            if (!any_of(vecAllowed.begin(), vecAllowed.end(), [sSourceForm](CEString sAllowedForm) { return sSourceForm == sAllowedForm; }))
            {
                return H_FALSE;
            }
        }

        iGroup = 3;
    }
    else if (14 == m_spInflection->iType() && 17 == m_spLexeme->stGetProperties().iSection)
    {
        iGroup = 1;
    }

    if (iGroup < 1)
    {
        return H_FALSE;
    }

    CEString sStem;
    if (13 == m_spInflection->iType())
    {
        sStem = m_spLexeme->sInfStem();
        sStem += L"ем";
    }
    else
    {
        sStem = sp1Pl->sWordForm();
        if (sStem.bEndsWith(L"ём"))
        {
            if (CEString::bIn(sStem[sStem.uiLength() - 3], CEString::g_szRusConsonants))
            {
                sStem.sReplace(sStem.uiLength() - 2, 2, L"ом");
            }
        }

        if (3 == iGroup && 6 == m_spInflection->iType() && m_spLexeme->sSourceForm() == L"двигать")
        {
            sStem = L"движим";
        }
    }

    ET_Status eStatus = (1 != iGroup) ? STATUS_RARE : STATUS_COMMON;

    ET_Subparadigm eSp(SUBPARADIGM_PART_PRES_PASS_LONG);
    vector<int> vecStress;
    auto rc = eGetParticipleStressPos(eSp, vecStress);
    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    vector<int>::iterator itStress = vecStress.begin();
    for (; (H_NO_ERROR == rc) && (itStress != vecStress.end()); ++itStress)
    {
        CFormBuilderLongAdj co_long(m_spLexeme, m_spInflection, sStem, AT_A, eSp, *itStress, eStatus);
        rc = co_long.eBuildParticiple();
        eSp = SUBPARADIGM_PART_PRES_PASS_SHORT;
        bool bYoAlternation = false;
        bool bFleetingVowel = false;
        CFormBuilderShortAdj shortAdj(m_spLexeme,
            m_spInflection,
            bYoAlternation,
            sStem,
            eSp,
            AT_A,
            AT_A,
            vector<int>(1, *itStress),
            bFleetingVowel);
        shortAdj.SetUsageStatus(eStatus);
        rc = shortAdj.eBuild();
    }

    m_spLexeme->SetHasPresPassParticiple(true);

    return H_NO_ERROR;

}       //  eBuildPresPassPartFromSourceForm()

ET_ReturnCode CFormBuilderNonFinite::eGetParticipleStressPos (ET_Subparadigm eSubparadigm, vector<int>& vecPositions)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    CEString sHash;
    vecPositions.clear();

    try
    {
        switch (eSubparadigm)
        {
            case SUBPARADIGM_PART_PRES_ACT:
            {
                CGramHasher hasher (POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_SG, 
                                    GENDER_UNDEFINED, PERSON_UNDEFINED, ANIM_UNDEFINED, 
                                    m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());
                if (4 == m_spInflection->iType() || 5 == m_spInflection->iType())
                {
                    hasher.m_ePerson = PERSON_1;
                }
                else
                {
                    hasher.m_ePerson = PERSON_3;
                }

                sHash = hasher.sGramHash();

                break;
            }
            case SUBPARADIGM_PART_PAST_ACT:
            {
                if (m_spInflection->bHasCommonDeviation(1) && (m_spInflection->iType() != 9))
                {
                    CGramHasher hasher (POS_VERB, SUBPARADIGM_INFINITIVE, CASE_UNDEFINED, NUM_UNDEFINED, 
                                        GENDER_UNDEFINED, PERSON_UNDEFINED, ANIM_UNDEFINED, 
                                        m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());
                    sHash = hasher.sGramHash();
                }
                else
                {
                    CGramHasher hasher (POS_VERB, SUBPARADIGM_PAST_TENSE, CASE_UNDEFINED, NUM_SG, GENDER_M, 
                                        PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(), 
                                        m_spLexeme->eIsReflexive());
                    sHash = hasher.sGramHash();
                }

                break;
            }
            case SUBPARADIGM_PART_PRES_PASS_LONG:
            {
                if (13 == m_spInflection->iType())
                {
                    CGramHasher hasher (POS_VERB, SUBPARADIGM_INFINITIVE, CASE_UNDEFINED, NUM_UNDEFINED, 
                                        GENDER_UNDEFINED, PERSON_UNDEFINED, ANIM_UNDEFINED, 
                                        m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());
                    sHash = hasher.sGramHash();
                }
                else
                {
                    if (4 == m_spInflection->iType() || 5 == m_spInflection->iType())
                    {
                        CGramHasher hasher (POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_SG, 
                                            GENDER_UNDEFINED, PERSON_1, ANIM_UNDEFINED, 
                                            m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());
                        sHash = hasher.sGramHash();
                    }
                    else
                    {
                        CGramHasher hasher (POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_PL, 
                                            GENDER_UNDEFINED, PERSON_1, ANIM_UNDEFINED, 
                                            m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());
                        sHash = hasher.sGramHash();
                    }
                }

                break;
            }
            case SUBPARADIGM_PART_PAST_PASS_LONG:
            {
                if ((m_spLexeme->sSourceForm().bEndsWith (L"ать") || m_spLexeme->sSourceForm().bEndsWith (L"ять")) &&
                    14 != m_spInflection->iType())
                {
                    int iStressPos = -1;
                    rc = m_spLexeme->eGetFirstStemStressPos(iStressPos);
                    while (H_NO_ERROR == rc)
                    {
                        if (iStressPos < 0)
                        {
                            assert(0);
                            CEString sMsg (L"Illegal stress position.");
                            ERROR_LOG (sMsg);
                            return H_ERROR_UNEXPECTED;
                        }

                        int iNSyll = m_spLexeme->sSourceForm().uiNSyllables();
                        if (iNSyll > 1 && iStressPos == iNSyll - 1)         // polysyllabic infinitive with 
                        {                                                   // stress on last syllable
                            vecPositions.push_back (iNSyll - 2);
                        }
                        else
                        {
                            vecPositions.push_back (iStressPos);
                        }
                        rc = m_spLexeme->eGetNextStemStressPos(iStressPos);
                    }
                }

//                int iPos = -1;
//                ET_StressType eType = ET_StressType::STRESS_TYPE_UNDEFINED;
                if (4 == m_spInflection->iType())    // same syllable (counting from right) as in 3 Sg Praes
                {
                    CGramHasher sg3Pres (POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_SG, 
                                         GENDER_UNDEFINED, PERSON_3, ANIM_UNDEFINED, 
                                         m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());
                    int iNForms = m_spInflection->iFormCount (sg3Pres.sGramHash());
                    auto sp3Sg = make_shared<CWordForm>();
                    for (int iF = 0; iF < iNForms; ++iF)
                    {
                        rc = m_spInflection->eWordFormFromHash (sg3Pres.sGramHash(), iF, sp3Sg);
                        if (rc != H_NO_ERROR)
                        {
                            return rc;
                        }
                        if (nullptr == sp3Sg)
                        {
                            assert(0);
                            ERROR_LOG (L"Failed to obtain template form.");
                            return H_ERROR_POINTER;
                        }

//                        auto itStressPos = sp3Sg->m_mapStress.begin();
//                        auto rcStress = sp3Sg->eGetFirstStressSyll(iPos, eType);
//                        for (; sp3Sg->m_mapStress.end() != itStressPos; ++itStressPos)
                        auto& mapStressSylls = sp3Sg->mapGetStressPositions();
//                        while (rcStress == H_NO_ERROR)
                        for (auto itStressSyll = mapStressSylls.begin(); itStressSyll != mapStressSylls.end(); ++itStressSyll)
                        {
                            if (STRESS_PRIMARY == itStressSyll->second)
                            {
                                vecPositions.push_back (itStressSyll->first);
                            }
                        }
                    }       //  for (int iF = 0; iF < iNForms; ++iF)

                }   //   if (4 == m_spInflection->iType())
                                                   
                if (5 == m_spInflection->iType() && m_spLexeme->sSourceForm().bEndsWith(L"еть"))
                {
                    int iStressPos = -1;
                    rc = m_spLexeme->eGetFirstStemStressPos(iStressPos);
                    while (H_NO_ERROR == rc)
                    {
                        if (iStressPos < 0)
                        {
                            assert(0);
                            CEString sMsg (L"Illegal stress position.");
                            ERROR_LOG (sMsg);
                            return H_ERROR_UNEXPECTED;
                        }

                        int iNSyll = m_spLexeme->sSourceForm().uiNSyllables();
                        if (iNSyll > 1)
                        {
                            vecPositions.push_back (iNSyll - 2);
                        }
                        else
                        {
                            vecPositions.push_back (iStressPos);
                        }

                        rc = m_spLexeme->eGetNextStemStressPos(iStressPos);

                        if (vecPositions.size() > 100)
                        {
                            assert(0);
                            CEString sMsg(L"Too many stress positions.");
                            ERROR_LOG(sMsg);
                            return H_ERROR_UNEXPECTED;
                        }
                    }
                }

                if (1 == m_spInflection->iType() && m_spLexeme->sSourceForm().bEndsWith(L"еть"))
                {                                                           // GDRL p. 86, footnote 3
                    vecPositions.push_back ((int)m_spLexeme->sSourceForm().uiGetNumOfSyllables() - 1);
                }

                if (7 == m_spInflection->iType() || 8 == m_spInflection->iType())
                {
                    CGramHasher pastF (POS_VERB, SUBPARADIGM_PAST_TENSE, CASE_UNDEFINED, NUM_SG, GENDER_F,
                                       PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(), 
                                       m_spLexeme->eIsReflexive());
                    auto spPastF = make_shared<CWordForm>();
                    int iNForms = m_spInflection->iFormCount (pastF.sGramHash());
                    for (int iF = 0; iF < iNForms; ++iF)
                    {
                        rc = m_spInflection->eWordFormFromHash (pastF.sGramHash(), iF, spPastF);
                        if (rc != H_NO_ERROR)
                        {
                            return rc;
                        }
                        if (nullptr == spPastF)
                        {
                            assert(0);
                            ERROR_LOG (L"Failed to obtain template form.");
                            return H_ERROR_POINTER;
                        }

//                        auto rcStress = spPastF->eGetFirstStressSyll(iPos, eType);
//                        auto itStressPos = spPastF->m_mapStress.begin();
                        auto mapStressSylls = spPastF->mapGetStressPositions();
                        for (auto itStressSyll = mapStressSylls.begin(); itStressSyll != mapStressSylls.end(); ++itStressSyll)
                        {
                            if (STRESS_PRIMARY == itStressSyll->second)
                            {
                                vecPositions.push_back (itStressSyll->first);
                            }
                        }
                    }       //  for (int iF = 0; iF < iNForms; ++iF)
                }

                if (3 == m_spInflection->iType() || 10 == m_spInflection->iType())        // NB also 3 + circle
                {
                    int iStressPos = -1;
                    rc = m_spLexeme->eGetFirstStemStressPos(iStressPos);
//                    while (H_NO_ERROR == rc)      // ???
                    if (H_NO_ERROR == rc)
                    {
                        if (iStressPos < 0)
                        {
                            assert(0);
                            CEString sMsg (L"Illegal stress position.");
                            ERROR_LOG (sMsg);
                            return H_ERROR_UNEXPECTED;
                        }

                        int iNSyll = m_spLexeme->sSourceForm().uiNSyllables();
                        if (iNSyll > 1)
                        {
                            if (iStressPos == iNSyll - 1)
                            {
                                vecPositions.push_back (iNSyll - 2);
                            }
                            else
                            {
                                vecPositions.push_back (iStressPos);
                            }
                        }
                        else
                        {
                            vecPositions.push_back (iStressPos);
                        }
                    }
                }

                if (9 == m_spInflection->iType() || 11 == m_spInflection->iType()  || 12 == m_spInflection->iType() || 
                    14 == m_spInflection->iType() || 15 == m_spInflection->iType() || 16 == m_spInflection->iType())
                {
                    CGramHasher pastM (POS_VERB, SUBPARADIGM_PAST_TENSE, CASE_UNDEFINED, NUM_SG, GENDER_M, 
                                       PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(), 
                                       m_spLexeme->eIsReflexive());
                    auto spPastM = make_shared<CWordForm>();
                    int iNForms = m_spInflection->iFormCount (pastM.sGramHash());
                    for (int iF = 0; iF < iNForms; ++iF)
                    {
                        rc = m_spInflection->eWordFormFromHash (pastM.sGramHash(), iF, spPastM);
                        if (rc != H_NO_ERROR)
                        {
                            return rc;
                        }
                        if (nullptr == spPastM)
                        {
                            assert(0);
                            ERROR_LOG (L"Failed to obtain template form.");
                            return H_ERROR_POINTER;
                        }

//                        auto itStressPos = spPastM->m_mapStress.begin();
//                        auto rcStress = spPastM->eGetFirstStressSyll(iPos, eType);
//                        while (H_NO_ERROR == rcStress)
                        auto& mapStressSylls = spPastM->mapGetStressPositions();
                        for (auto itStressSyll = mapStressSylls.begin(); itStressSyll != mapStressSylls.end(); ++itStressSyll)
                        {
                            if (STRESS_PRIMARY == itStressSyll->second)
                            {
                                vecPositions.push_back (itStressSyll->first);
                            }
                        }
                    }       //  for (int iF = 0; iF < iNForms; ++iF)
                }
                break;

            }   //  case SUBPARADIGM_PART_PAST_PASS
     
            default:
            {
                assert(0);
                ERROR_LOG (L"Bad subparadigm.");
                return H_ERROR_INVALID_ARG;
            }
        }       // switch

        int iPos = -1;
        ET_StressType eType = ET_StressType::STRESS_TYPE_UNDEFINED;
        if (SUBPARADIGM_PART_PAST_PASS_LONG != eSubparadigm)
        {
            auto spStressTemplate = make_shared<CWordForm>();
            rc = m_spInflection->eWordFormFromHash (sHash, 0, spStressTemplate);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
            if (nullptr == spStressTemplate)
            {
                assert(0);
                ERROR_LOG (L"Failed to obtain template form.");
                return H_ERROR_POINTER;
            }
//            auto itStressPos = spStressTemplate->m_mapStress.begin();
            auto rcStress = spStressTemplate->eGetFirstStressSyll(iPos, eType);
            // Skip secondary stress if any
//            for (; spStressTemplate->m_mapStress.end() != itStressPos; ++itStressPos)
            while (H_NO_ERROR == rcStress)
            {
                if (STRESS_PRIMARY == eType)
                {
                    vecPositions.push_back (iPos);
                }
                rcStress = spStressTemplate->eGetNextStressPos(iPos, eType);
            }

            if (rcStress != ET_ReturnCode::H_NO_MORE)
            {
                assert(0);
                ERROR_LOG(L"Unexpected return code.");
            }

            if (vecPositions.empty())
            {
                assert(0);
                ERROR_LOG (L"Can't find main stress.");
                return H_ERROR_GENERAL;
            }
        }
    }
    catch (CException& ex)
    {
        CEString sMsg (L"Exception: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG(sMsg);
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eGetParticipleStressPos (...)

//int CFormBuilderNonFinite::iStressPosFromTemplate (const CWordForm& wfTemplate, const CEString& sStem, int iAt)
//{
//    map<int, ET_StressType>::const_iterator itStressPos = wfTemplate.m_mapStress.begin();
//    int iMainStress = 0;
//    for (; itStressPos != wfTemplate.m_mapStress.end(); ++itStressPos)
//    {
//        if (STRESS_PRIMARY == (*itStressPos).second)     // main stress == true
//        {
//            if (iMainStress == iAt)
//            {
//                break;
//            }
//            ++iMainStress;
//        }
//    }
//
//    if (wfTemplate.m_mapStress.end() == itStressPos)
//    {
//        assert(0);
//        CEString sMsg (L"Can't find main stress.");
//        ERROR_LOG (sMsg);
//        throw CException (E_FAIL, sMsg);
//    }
//
//    return (*itStressPos).first;
//
//}   //  i_StressPosFromTemplate (...)

//
// Irregular forms
//
ET_ReturnCode CFormBuilderNonFinite::eDeriveIrregPresAdverbial()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        auto spWordForm = make_shared<CWordForm>(m_spInflection);
        if (nullptr == spWordForm)
        {
            assert(0);
            ERROR_LOG (L"Unable to instantiate CWordForm.");
            return H_ERROR_POINTER;
        }

//        spWordForm->m_spLexeme = m_spLexeme;
        spWordForm->SetPos(POS_VERB);
        spWordForm->SetSubparadigm(SUBPARADIGM_ADVERBIAL_PRESENT);
        spWordForm->SetAspect(m_spLexeme->eAspect());
        spWordForm->SetReflexivity(m_spLexeme->eIsReflexive());
        spWordForm->SetIrregular(true);
//        spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();
        spWordForm->SetInflectionId(m_spInflection->llInflectionId());
        
        CGramHasher pl3Hash (SUBPARADIGM_PRESENT_TENSE, 
                             NUM_PL, 
                             GENDER_UNDEFINED, 
                             PERSON_3, 
                             ANIM_UNDEFINED,
                             m_spLexeme->eAspect(),
                             CASE_UNDEFINED, 
                             m_spLexeme->eIsReflexive());
        
        vector<shared_ptr<CWordForm>> vecPl3Forms;        
        rc = eGetIrregularForms(pl3Hash.sGramHash(), vecPl3Forms);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        if (vecPl3Forms.empty())
        {
            return H_ERROR_UNEXPECTED;
        }

        CGramHasher sg1Hash (SUBPARADIGM_PRESENT_TENSE, 
                             NUM_SG, 
                             GENDER_UNDEFINED, 
                             PERSON_1, 
                             ANIM_UNDEFINED,
                             m_spLexeme->eAspect(),
                             CASE_UNDEFINED, 
                             m_spLexeme->eIsReflexive());

        vector<shared_ptr<CWordForm>> vecSg1Forms;
        rc = eGetIrregularForms(sg1Hash.sGramHash(), vecSg1Forms);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }
        if (vecSg1Forms.empty())
        {
            return H_ERROR_UNEXPECTED;
        }

        auto it3PlForm = vecPl3Forms.begin();
        for (; it3PlForm != vecPl3Forms.end(); ++it3PlForm)
        {
            if (it3PlForm != vecPl3Forms.begin())
            {
                auto spWfVariant = make_shared<CWordForm>();
//                CloneWordForm(spWordForm, spWfVariant);
                spWfVariant->eCloneFrom(spWordForm);
                spWordForm = spWfVariant;
            }

            CEString s3Pl ((*it3PlForm)->sWordForm());
            if (REFL_YES == m_spLexeme->eIsReflexive())
            {
                s3Pl.sErase (s3Pl.uiLength()-2);
            }

            if (!s3Pl.bEndsWith (L"ют") && !s3Pl.bEndsWith (L"ут") && !s3Pl.bEndsWith (L"ят") && 
                !s3Pl.bEndsWith (L"ат"))
            {
                assert(0);
                CEString sMsg (L"Unexpected 3 Pl ending: ");
                sMsg += s3Pl;
                ERROR_LOG (sMsg);
                return H_ERROR_UNEXPECTED;
            }

            m_spEndings = make_shared<CAdverbialEndings>(m_spLexeme, m_spInflection);
            CEString sStem(s3Pl);
            sStem.sErase (sStem.uiLength()-2);
            spWordForm->SetStem(sStem);
//            bool bIsReflexive = m_spLexeme->eIsReflexive() ? true : false;

            bool bHusher = false;
            if (sStem.bEndsWithOneOf(L"шжчщц"))
            {
                bHusher = true;
            }

            bool bVStem = false;    // ignored for pres.
            bool bIsVariant = false;
            static_pointer_cast<CAdverbialEndings>(m_spEndings)->eSelect(ET_Subparadigm::SUBPARADIGM_ADVERBIAL_PRESENT, bHusher, bVStem, bIsVariant);

            int64_t iNumEndings = m_spEndings->iCount();
            if (iNumEndings < 1)
            {
                ERROR_LOG(L"No ending or too many endings");
                return H_ERROR_UNEXPECTED;
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

                spWordForm->SetWordForm(spWordForm->sStem() + sEnding);
            }

            auto itSg1Form = vecSg1Forms.begin();
            spWordForm->AssignStress((*itSg1Form)->mapGetStressPositions());
            m_spInflection->AddWordForm (spWordForm);
            ++itSg1Form;

            // Will the ever happen??
            for (; itSg1Form != vecSg1Forms.end(); ++itSg1Form)
            {
                auto spNewWordForm = make_shared<CWordForm>();
//                CloneWordForm(spWordForm, spNewWordForm);
                spNewWordForm->eCloneFrom(spWordForm);
                spWordForm->AssignStress((*itSg1Form)->mapGetStressPositions());
                m_spInflection->AddWordForm (spWordForm);
            }
        }
    }
    catch (CException& ex)
    {
        CEString sMsg(L"Exception: ");
        sMsg += ex.szGetDescription();
        sMsg += L"; lexeme = ";
        sMsg += m_spLexeme->sSourceForm();
        ERROR_LOG(sMsg);
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eDeriveIrregPresAdverbial()

//
// "Abbreviated notation", GDRL pp. 88ff
//
ET_ReturnCode CFormBuilderNonFinite::eDeriveIrregPresActiveParticiple()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    CGramHasher pl3Hash (SUBPARADIGM_PRESENT_TENSE, 
                         NUM_PL, 
                         GENDER_UNDEFINED, 
                         PERSON_3, 
                         ANIM_UNDEFINED,
                         m_spLexeme->eAspect(),
                         CASE_UNDEFINED, 
                         m_spLexeme->eIsReflexive());
    map<shared_ptr<CWordForm>, bool> map3PlIrreg;
    rc = m_spInflection->eGetIrregularForms(pl3Hash.sGramHash(), map3PlIrreg);
    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    if (map3PlIrreg.empty())
    {
        assert(0);
        ERROR_LOG (L"Irregular 3 Pl form not found in database.");
        return H_ERROR_UNEXPECTED;
    }
    auto itWf = map3PlIrreg.begin();
    for (; itWf != map3PlIrreg.end(); ++itWf)
    {
        CEString sStem(itWf->first->sWordForm());
        if (REFL_YES == m_spLexeme->eIsReflexive())
        {
            sStem.sErase (sStem.uiLength()-2);
        }

        if (!sStem.bEndsWith (L"т"))
        {
//            assert(0);
            CEString sMsg (L"Unexpected 3 Pl ending: ");
            sMsg += itWf->first->sWordForm();
            ERROR_LOG (sMsg);
            return H_ERROR_UNEXPECTED;
        }

        sStem.sErase (sStem.uiLength()-1);
        
        int iPos = -1;
        ET_StressType eType = ET_StressType::STRESS_TYPE_UNDEFINED;
        vector<int> vecStressPos;
        if (sStem.bEndsWith (L"у") || sStem.bEndsWith (L"ю"))
        {
            auto rcStress = itWf->first->eGetFirstStressSyll(iPos, eType);
            while (H_NO_ERROR == rcStress)
//            map<int, ET_StressType>::iterator itStressPos = itWf->first->m_mapStress.begin();
//            for (; itStressPos != itWf->first->m_mapStress.end(); ++itStressPos)
            {
                if (STRESS_PRIMARY == eType)
                {
                    vecStressPos.push_back (iPos);
                }
                rcStress = itWf->first->eGetNextStressPos(iPos, eType);
            }
        }
        else if (sStem.bEndsWith (L"а") || sStem.bEndsWith (L"я"))
        {
            CGramHasher sg1Hash (SUBPARADIGM_PRESENT_TENSE, NUM_SG, GENDER_UNDEFINED, PERSON_1, 
                                 ANIM_UNDEFINED, m_spLexeme->eAspect(), CASE_UNDEFINED, 
                                 m_spLexeme->eIsReflexive());

            map<shared_ptr<CWordForm>, bool> mapSg1Irreg;
            rc = m_spInflection->eGetIrregularForms(sg1Hash.sGramHash(), mapSg1Irreg);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }

            if (mapSg1Irreg.empty())
            {
                assert(0);
                ERROR_LOG (L"Irregular 1 Sg form not found in database.");
                return H_ERROR_UNEXPECTED;
            }

            auto it = mapSg1Irreg.begin();
            for (; it != mapSg1Irreg.end(); ++it)
            {
//                map<int, ET_StressType>::iterator itStressPos = (*it).first->m_mapStress.begin();
//                for (; itStressPos != (*it).first->m_mapStress.end(); ++itStressPos)
                auto rcStress = (*it).first->eGetFirstStressSyll(iPos, eType);
                while (H_NO_ERROR == rcStress)
                {
                    if (STRESS_PRIMARY == eType)
                    {
                        vecStressPos.push_back (iPos);
                    }
                    rcStress = (*it).first->eGetNextStressPos(iPos, eType);
                }
            }
        }
        else
        {
            assert(0);
            CEString sMsg (L"Unexpected stem auslaut: ");
            sMsg += itWf->first->sWordForm();
            ERROR_LOG (sMsg);
            return H_ERROR_UNEXPECTED;
        }
        
        sStem += L'щ';

        ET_Subparadigm eSp (SUBPARADIGM_PART_PRES_ACT);
        vector<int>::iterator itStress = vecStressPos.begin();
        for (; itStress != vecStressPos.end(); ++itStress)
        {
            CFormBuilderLongAdj builder (m_spLexeme, m_spInflection, sStem, AT_A, eSp, *itStress);
            rc = builder.eBuildParticiple();
        }
    }
    return rc;

}   //  eDeriveIrregPresActiveParticiple()

//
// If present tense is irregular
//
ET_ReturnCode CFormBuilderNonFinite::eDeriveIrregPresPassiveParticiple()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    CGramHasher hasher (POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_PL, GENDER_UNDEFINED, 
                        PERSON_1, ANIM_UNDEFINED, m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());
    auto sp1Pl = make_shared<CWordForm>();
    auto n1Pl = m_spInflection->iFormCount(hasher.sGramHash());
    if (n1Pl < 1)
    {
        assert(0);
        ERROR_LOG(L"Failed to obtain 1st person plural form count.");
        return H_ERROR_UNEXPECTED;
    }

    for (auto nSourceForm = 0; nSourceForm < n1Pl; ++nSourceForm)
    {
        rc = m_spInflection->eWordFormFromHash(hasher.sGramHash(), nSourceForm, sp1Pl);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        if (nullptr == sp1Pl)
        {
            assert(0);
            ERROR_LOG(L"Failed to obtain 1st person plural form.");
            return H_ERROR_POINTER;
        }

        rc = eBuildPresPassPartFromSourceForm(sp1Pl);
    }
    
    if (H_NO_ERROR == rc)
    {
        m_spLexeme->SetHasPresPassParticiple(true);
    }

    return rc;

}   //  eDeriveIrregPresPassiveParticiple()

ET_ReturnCode CFormBuilderNonFinite::eDeriveIrregPastActPartAndAdverbial()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    if (!bHasIrregularPast())
    {
        assert(0);
        CEString sMsg (L"Irregular past tense expected.");
        ERROR_LOG (sMsg);
        return H_ERROR_UNEXPECTED;
    }

    int iPos = -1;
    ET_StressType eType = ET_StressType::STRESS_TYPE_UNDEFINED;

    //
    // Type 7 ending in -сти with -т- or -д- alternation mark:
    // 1 Sg stem present tense + -ший
    //
    CEString sStem;
    if (7 == m_spInflection->iType() &&
        (m_spLexeme->sSourceForm().bEndsWith(L"сти") || m_spLexeme->sSourceForm().bEndsWith(L"стись")) &&
        (L"т" == m_spLexeme->sVerbStemAlternation() || L"д" == m_spLexeme->sVerbStemAlternation()))
    {
        sStem = m_spLexeme->s1SgStem();
        sStem += L"ш";

        // Stress as inpast tense
        CGramHasher mSgHash(SUBPARADIGM_PAST_TENSE, NUM_SG, GENDER_M, PERSON_UNDEFINED, ANIM_UNDEFINED,
            m_spLexeme->eAspect(), CASE_UNDEFINED, m_spLexeme->eIsReflexive());

        map<shared_ptr<CWordForm>, bool> mapMSgIrreg;
        rc = m_spInflection->eGetIrregularForms(mSgHash.sGramHash(), mapMSgIrreg);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        if (mapMSgIrreg.empty())
        {
            assert(0);
            CEString sMsg(L"Unable to acquire M Sg Past of .");
            sMsg += m_spLexeme->sSourceForm();
            ERROR_LOG(sMsg);
            return H_ERROR_UNEXPECTED;
        }

        auto it = mapMSgIrreg.begin();
        for (; it != mapMSgIrreg.end(); ++it)
        {
//            map<int, ET_StressType>::iterator itStress = (*it).first->m_mapStress.begin();
//            for (; itStress != (*it).first->m_mapStress.end(); ++itStress)
            auto rcStress = (*it).first->eGetFirstStressSyll(iPos, eType);
            if (rcStress != H_NO_ERROR)
            {
                ERROR_LOG(L"Unable to retrieve stress syllable.");
                continue;
            }
            
            if (STRESS_PRIMARY == eType)
            {
                CFormBuilderLongAdj builder(m_spLexeme,
                    m_spInflection,
                    sStem,
                    AT_A,
                    SUBPARADIGM_PART_PAST_ACT,
                    iPos);
                builder.eBuildParticiple();
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
            }       //  if (STRESS_PRIMARY == eType)

            rcStress = (*it).first->eGetNextStressPos(iPos, eType);
            if (H_NO_ERROR == rcStress)
            {
                ERROR_LOG(L"Multiple stress positions.")
            }
        }   //  for (; it != mapMSgIrreg.end(); ++it)...

        return H_NO_ERROR;
    }       // Type 7 etc (special case)

    // Derive from past tense
    CGramHasher mSgHash (SUBPARADIGM_PAST_TENSE, NUM_SG, GENDER_M, PERSON_UNDEFINED, ANIM_UNDEFINED,
                         m_spLexeme->eAspect(), CASE_UNDEFINED, m_spLexeme->eIsReflexive());

    map<shared_ptr<CWordForm>, bool> mapMSgIrreg;
    rc = m_spInflection->eGetIrregularForms(mSgHash.sGramHash(), mapMSgIrreg);
    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    if (mapMSgIrreg.empty())
    {
        assert(0);
        CEString sMsg (L"Unable to acquire M Sg Past of .");
        sMsg += m_spLexeme->sSourceForm();
        ERROR_LOG (sMsg);
        return H_ERROR_UNEXPECTED;
    }

    auto it = mapMSgIrreg.begin();
    for (; it != mapMSgIrreg.end(); ++it)
    {
        CEString sStem = it->first->sWordForm();
        CEString sSuffix;

        int iCharsToRemove = 0;
        if (REFL_YES == m_spLexeme->eIsReflexive())
        {
            if (sStem.bEndsWith (L"лся"))
            {
                iCharsToRemove = 3;
                sSuffix = L"вш";
            }
            else
            {
                iCharsToRemove = 2;
                sSuffix = L"ш";
            }
        }
        else
        {
            if (sStem.bEndsWith (L"л"))
            {
                iCharsToRemove = 1;
                sSuffix = L"вш";
            }
            else
            {
                iCharsToRemove = 0;
                sSuffix = L"ш";
            }
        }

//        sStem.sErase (sStem.uiLength() - iCharsToRemove);
        sStem.sRemoveCharsFromEnd(iCharsToRemove);
        sStem += sSuffix;
    
//        map<int, ET_StressType>::iterator itStress = (*it).first->m_mapStress.begin();
//        for (; itStress != (*it).first->m_mapStress.end(); ++itStress)
        auto rcStress = (*it).first->eGetFirstStressSyll(iPos, eType);
        while (H_NO_ERROR == rcStress)
        {
            if (STRESS_PRIMARY == eType)
            {
                CFormBuilderLongAdj builder (m_spLexeme, 
                                             m_spInflection,
                                             sStem, 
                                             AT_A, 
                                             SUBPARADIGM_PART_PAST_ACT, 
                                             iPos);
                builder.eBuildParticiple();
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
            }       //  if (STRESS_PRIMARY == (*itStress).second)

            rcStress = (*it).first->eGetNextStressPos(iPos, eType);

        }       // stress pos. loop

    }   //  for (; it != mapMSgIrreg.end(); ++it)...

    return H_NO_ERROR;

}   //  eDeriveIrregPastActPartAndAdverbial()

ET_ReturnCode CFormBuilderNonFinite::eBuildIrregParticipialFormsLong (ET_Subparadigm eSp)   
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    if (SUBPARADIGM_PART_PRES_ACT != eSp && SUBPARADIGM_PART_PRES_PASS_LONG  != eSp && 
        SUBPARADIGM_PART_PAST_ACT != eSp && SUBPARADIGM_PART_PAST_PASS_LONG != eSp)
    {
        assert(0);
        return H_ERROR_UNEXPECTED;
    }

    CGramHasher irreg (POS_VERB, eSp, CASE_NOM, NUM_SG, GENDER_M, PERSON_UNDEFINED, 
                       ANIM_UNDEFINED, m_spLexeme->eAspect(), m_spLexeme->eIsReflexive());
    
    map<shared_ptr<CWordForm>, bool> mapIrregForms;
    rc = m_spInflection->eGetIrregularForms(irreg.sGramHash(), mapIrregForms);
    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    if (mapIrregForms.empty())
    {
        return H_NO_MORE;
    }

    auto it = mapIrregForms.begin();
    for (; it != mapIrregForms.end(); ++it)
    {
        auto spWf = (*it).first;
        CEString sWordForm = (*it).first->sWordForm();
        unsigned int uiCharsToRemove = (REFL_YES == m_spLexeme->eIsReflexive()) ? 4 : 2;
        if (sWordForm.uiLength() <= uiCharsToRemove)
        {
            assert(0);
            CEString sMsg (L"Irregular past active participle is too short: ");
            sMsg += sWordForm;
            ERROR_LOG (sMsg);
            return H_ERROR_UNEXPECTED;
        }

        CEString sStem = sWordForm.sErase (sWordForm.uiLength()-uiCharsToRemove, uiCharsToRemove);
        int iPos = -1;
        ET_StressType eType = ET_StressType::STRESS_TYPE_UNDEFINED;

        auto rcStress = spWf->eGetFirstStressSyll(iPos, eType);
        while (H_NO_ERROR == rcStress)
//        auto itStressPos = spWf->m_mapStress.begin();
//        for (; itStressPos != spWf->m_mapStress.end(); ++itStressPos)
        {
            if (STRESS_PRIMARY == eType)
            {
                CFormBuilderLongAdj builder (m_spLexeme, 
                                             m_spInflection,
                                             sStem,
                                             AT_A, 
                                             eSp, 
                                             iPos,
                                             STATUS_COMMON,     // don't know how to determine status
                                             true);             // irregular
                rc = builder.eBuildParticiple();
            }
            rcStress = spWf->eGetNextStressPos(iPos, eType);
        }
    
    }   //  for (; it != mapIrregForms.end(); ++it)

    return rc;

}       //  eBuildIrregParticipialFormsLong

/*
ET_ReturnCode CFormBuilderNonFinite::eDeriveIrregPartPresPassShort (ET_Status eStatus)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        CWordForm * pSource = nullptr;
        CEString sStem;

        CGramHasher nSgLong (POS_VERB, SUBPARADIGM_PART_PRES_PASS_LONG, CASE_NOM, NUM_SG, 
                             GENDER_M, PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(), 
                             m_spLexeme->eIsReflexive());

        auto nNSgLong = m_spInflection->iFormCount(nSgLong.sGramHash());
        if (nNSgLong < 1)
        {
            assert(0);
            ERROR_LOG(L"Failed to obtain long form count.");
            return H_ERROR_UNEXPECTED;
        }

        for (auto nSourceForm = 0; nSourceForm < nNSgLong; ++nSourceForm)
        {
            rc = m_spInflection->eWordFormFromHash(nSgLong.sGramHash(), nSourceForm, pSource);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
            if (nullptr == pSource)
            {
                assert(0);
                ERROR_LOG(L"Failed to obtain long form.");
                return H_ERROR_POINTER;
            }

            sStem = pSource->m_sWordForm;
            if (sStem.uiLength() <= 2)
            {
                assert(0);
                CEString sMsg(L"Long form not long enough.");
                ERROR_LOG(sMsg);
                return H_ERROR_UNEXPECTED;
            }

            sStem.sErase(sStem.uiLength() - 2, 2);

            bool bYoAlternation = false;
            bool bFleetingVowel = false;
            vector<int> vecStressPos;
            map<int, ET_StressType>::iterator itStressPos = pSource->m_mapStress.begin();
            for (; pSource->m_mapStress.end() != itStressPos; ++itStressPos)
            {
                if (STRESS_PRIMARY == (*itStressPos).second)
                {
                    vecStressPos.push_back((*itStressPos).first);
                }
            }
            CFormBuilderShortAdj shortAdj(m_spLexeme,
                bYoAlternation,
                sStem,
                SUBPARADIGM_PART_PRES_PASS_SHORT,
                AT_A,
                AT_A,
                vecStressPos,
                bFleetingVowel);
            shortAdj.SetUsageStatus(eStatus);   //  where  can we get this info for irregular forms?
            rc = shortAdj.eBuild();
        
        }       //  for (auto nSourceForm = 0; nSourceForm < nNSgLong; ++nSourceForm)
    }
    catch (CException& ex)
    {
        CEString sMsg (L"Exception: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG (sMsg);
        return H_EXCEPTION;
    }

    return rc;

}   //  eDeriveIrregPartPresPassShort()
*/

//
// Irregular short forms (as opposed to derived from irregular long)
//
ET_ReturnCode CFormBuilderNonFinite::eBuildIrregPartPastPassShort ([[maybe_unused]]bool& bIsVariant)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        for (ET_Gender eGender = GENDER_UNDEFINED; eGender <= GENDER_N; ++eGender)
        {
            ET_Number eNumber = (GENDER_UNDEFINED == eGender) ? NUM_PL : NUM_SG;

            auto spWordForm = make_shared<CWordForm>(m_spInflection);
            if (nullptr == spWordForm)
            {
                assert(0);
                CEString sMsg(L"Unable to instantiate CWordForm.");
                ERROR_LOG (L"Unable to instantiate CWordForm.");
                return H_ERROR_POINTER;
            }

//            spWordForm->m_spLexeme = m_spLexeme;
            spWordForm->SetPos(POS_VERB);
            spWordForm->SetSubparadigm(SUBPARADIGM_PART_PAST_PASS_SHORT);
            spWordForm->SetAspect(m_spLexeme->eAspect());
            spWordForm->SetReflexivity(m_spLexeme->eIsReflexive());
            spWordForm->SetGender(eGender);
            spWordForm->SetNumber(eNumber);
//            spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();
            spWordForm->SetInflectionId(m_spInflection->llInflectionId());

            bool bIsOptional = false;
            rc = m_spInflection->eGetFirstIrregularForm(spWordForm->sGramHash(), spWordForm, bIsOptional);
            if (rc != H_NO_ERROR && rc != H_FALSE && rc != H_NO_MORE)
            {
                assert(0);
                ERROR_LOG(L"Irregular form not found in database.");
                continue;;
            }

            // No irregular form for this hash?
            if (H_FALSE == rc)
            {
                continue;  // expected
            }

            m_spInflection->AddWordForm(spWordForm);

            // Continue for other irregular forms for this hash 
            // (which, in all likelyhood do not exist)
            do
            {
                bIsOptional = false;
                rc = m_spInflection->eGetNextIrregularForm(spWordForm, bIsOptional);
                if (rc != H_NO_ERROR && rc != H_NO_MORE)
                {
                    assert(0);
                    ERROR_LOG(L"Error accessing irregular m Sg Past form.");
                    continue;
                }

                if (H_NO_ERROR == rc)
                {
                    m_spInflection->AddWordForm(spWordForm);
                }

            } while (H_NO_ERROR == rc);
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

}   //  eBuildIrregPartPastPassShort (...)

ET_ReturnCode CFormBuilderNonFinite::eDeriveIrregPartPastPassShort()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    try
    {
        //
        // Part. pass long must be available
        //
        CGramHasher nSgLong (POS_VERB, SUBPARADIGM_PART_PAST_PASS_LONG, CASE_NOM, NUM_SG, GENDER_M, 
                             PERSON_UNDEFINED, ANIM_UNDEFINED, m_spLexeme->eAspect(), 
                             m_spLexeme->eIsReflexive());

        auto nPartPassLong = m_spInflection->iFormCount(nSgLong.sGramHash());
        if (nPartPassLong < 1)
        {
            assert(0);
            CEString sMsg(L"No irregular long forms.");
            ERROR_LOG(sMsg);
            return H_ERROR_POINTER;
        }

        for (auto nLongForm = 0; nLongForm < nPartPassLong; ++nLongForm)
        {
            auto spNSgLong = make_shared<CWordForm>();
            rc = m_spInflection->eWordFormFromHash(nSgLong.sGramHash(), nLongForm, spNSgLong);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
            if (nullptr == spNSgLong)
            {
                assert(0);
                CEString sMsg(L"Failed to obtain long form.");
                ERROR_LOG(sMsg);
                return H_ERROR_POINTER;
            }

            //
            // Short forms
            //
            map<ET_Gender, CEString> mapEndings;
            mapEndings[GENDER_M] = L"";
            mapEndings[GENDER_F] = L"а";
            mapEndings[GENDER_N] = L"о";
            mapEndings[GENDER_UNDEFINED] = (L"ы");

            ET_StressLocation eStressLocation = STRESS_LOCATION_UNDEFINED;

            CEString sLong(spNSgLong->sWordForm());
            CEString sStem;
            if (sLong.bEndsWith(L"енный") ||
                sLong.bEndsWith(L"анный") ||
                sLong.bEndsWith(L"янный"))
            {
                sStem = sLong.sSubstr(0, sLong.uiLength() - 3);
                eStressLocation = STRESS_LOCATION_STEM;
            }
            else if (sLong.bEndsWith(L"ённый"))
            {
                sStem = sLong.sSubstr(0, sLong.uiLength() - 3);
                eStressLocation = STRESS_LOCATION_ENDING;
            }
            else if (sLong.bEndsWith(L"тый"))
            {
                sStem = sLong.sSubstr(0, sLong.uiLength() - 2);
                eStressLocation = STRESS_LOCATION_STEM;
            }
            else
            {
                assert(0);
                CEString sMsg(L"Unable to deduce irregular short forms for ");
                sMsg += sLong;
                ERROR_LOG(sMsg);
                return H_ERROR_GENERAL;
            }

            for (ET_Gender eGender = GENDER_UNDEFINED; eGender <= GENDER_N; ++eGender)
            {
                ET_Number eNumber = (GENDER_UNDEFINED == eGender) ? NUM_PL : NUM_SG;

                auto spWordForm = make_shared<CWordForm>(m_spInflection);
                if (nullptr == spWordForm)
                {
                    assert(0);
                    ERROR_LOG(L"Unable to instantiate CWordForm.");
                    return H_ERROR_POINTER;
                }

//                spWordForm->m_spLexeme = m_spLexeme;
                spWordForm->SetPos(POS_VERB);
                spWordForm->SetSubparadigm(SUBPARADIGM_PART_PAST_PASS_SHORT);
                spWordForm->SetAspect(m_spLexeme->eAspect());
                spWordForm->SetReflexivity(m_spLexeme->eIsReflexive());
                spWordForm->SetStem(sStem);
                spWordForm->SetWordForm(sStem + mapEndings[eGender]);
                spWordForm->SetGender(eGender);
                spWordForm->SetNumber(eNumber);
//                spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();
                spWordForm->SetInflectionId(m_spInflection->llInflectionId());

                if (STRESS_LOCATION_STEM == eStressLocation)
                {
                    spWordForm->AssignStress(spNSgLong->mapGetStressPositions());
                }
                else if (STRESS_LOCATION_ENDING == eStressLocation)
                {
                    CEString sWf(spWordForm->sWordForm());
//                    spWordForm->m_mapStress[sWf.uiNSyllables()-1] = STRESS_PRIMARY;
                    spWordForm->SetStressPos(sWf.uiNSyllables()-1, STRESS_PRIMARY);
                    unsigned int uiYoPos = spWordForm->sWordForm().uiFindLastOf(L"ё");
                    sWf.SetVowels(CEString::g_szRusVowels);
                    int iStressPos = sWf.uiGetVowelPos(sWf.uiNSyllables()-1);
                    if (ecNotFound != uiYoPos)
                    {
                        if (uiYoPos != (unsigned int)iStressPos)
                        {
                            auto sWordForm = spWordForm->sWordForm();
//                            spWordForm->m_sWordForm[uiYoPos] = L'е';
                            sWordForm[uiYoPos] = L'е';
                            spWordForm->SetWordForm(sWordForm);
                        }
                    }
                }
                else
                {
                    assert(0);
                    ERROR_LOG(L"Illegal stress type.");
                    return H_ERROR_UNEXPECTED;
                }

                //multimap<int, StIrregularForm>& map_if = m_spLexeme->map_IrregularForms;
                //multimap<int, StIrregularForm>::iterator it_if = map_if.find (spWordForm->i_GramHash());
                //if (map_if.end() != it_if)
                //{
                //    spWordForm->m_mapStress = (*it_if).second.m_mapStress;
                //    spWordForm->m_sWordForm = (*it_if).second.m_sForm;
                //}

                //rc = eAssignSecondaryStress (spWordForm);
                //if (rc != H_NO_ERROR)
                //{
                //    return rc;
                //}

                m_spInflection->AddWordForm(spWordForm);
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

}   //  eDeriveIrregPartPastPassShort()

bool CFormBuilderNonFinite::bHasIrregPartPastPassShort()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    if (!m_spLexeme->bHasIrregularForms())
    {
        return false;
    }

    CGramHasher mSgHash (SUBPARADIGM_PART_PAST_PASS_SHORT, 
                         NUM_SG, 
                         GENDER_M, 
                         PERSON_UNDEFINED, 
                         ANIM_UNDEFINED,
                         m_spLexeme->eAspect(),
                         CASE_UNDEFINED, 
                         m_spLexeme->eIsReflexive());

    return m_spInflection->bHasIrregularForm(mSgHash.sGramHash());

}   // bHasIrregPartPastPassShort()
