#include <assert.h>

#include "Logging.h"
#include "WordForm.h"
#include "Lexeme.h"
#include "Inflection.h"
#include "FormBuilderAdjComparative.h"

using namespace Hlib;

ET_ReturnCode CFormBuilderComparative::eCreateFormTemplate (const CEString& sStem, shared_ptr<CWordForm>& spWordForm)
{
    spWordForm = make_shared<CWordForm>(m_spInflection);
    if (nullptr == spWordForm)
    {
        return H_ERROR_POINTER;
    }

    spWordForm->SetInflection(m_spInflection);
    spWordForm->SetPos(POS_ADJ);
    spWordForm->SetSubparadigm(SUBPARADIGM_COMPARATIVE);
    spWordForm->SetStem(sStem);
//    spWordForm->m_llLexemeId = m_spLexeme->llLexemeId();
    spWordForm->SetInflectionId(m_spInflection->llInflectionId());

    return H_NO_ERROR;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderComparative::eHandleStressAndAdd(shared_ptr<CWordForm> spWordForm, 
                                                           vector<int>& vecStress, 
                                                           ET_StressLocation eStressType, 
                                                           CEString& sStem, 
                                                           const CEString& sEnding,
                                                           long long llEndingKey)
{
    ET_ReturnCode rc = H_NO_ERROR;

    assert(m_spLexeme);   // we assume base class ctor took care of this

    CEString sGramHash = spWordForm->sGramHash();

    try
    {
        if (1 == vecStress.size() || m_spInflection->bIsMultistressedCompound())
        {
            vector<int>::iterator itStressPos = vecStress.begin();
            for (; itStressPos != vecStress.end(); ++itStressPos)
            {
//                spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
                rc = eHandleYoAlternation(eStressType, *itStressPos, sStem, sEnding);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
            }

            spWordForm->SetStem(sStem);
            spWordForm->SetEnding(sEnding);
            spWordForm->SetEndingDataId(llEndingKey);
            spWordForm->SetWordForm(sStem + sEnding);

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
//                    CloneWordForm (spWordForm, spWfVariant);
                    spWordForm = spWfVariant;
                }
//                spWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
                rc = eHandleYoAlternation(eStressType, *itStressPos, sStem, sEnding);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                spWordForm->SetStem(sStem);
                spWordForm->SetEnding(sEnding);
                spWordForm->SetEndingDataId(llEndingKey);
                spWordForm->SetWordForm(sStem + sEnding);

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

}   //  eHandleStressAndAdd (...)

ET_ReturnCode CFormBuilderComparative::eBuild()
{
    ET_ReturnCode rc = H_NO_ERROR;

    assert(m_spLexeme);   // we assume base class ctor took care of this

    if (m_spLexeme->bHasMissingForms() && m_spInflection->eFormExists(L"AdjComp"))
    {
        return H_NO_ERROR;
    }

    bool bIsVariant = false;

    try
    {
        CEString sStem(m_spLexeme->sGraphicStem());
        CGramHasher hash(SUBPARADIGM_COMPARATIVE, NUM_UNDEFINED, GENDER_UNDEFINED, ANIM_UNDEFINED, CASE_UNDEFINED);
        map<shared_ptr<CWordForm>, bool> mapIrreg;
        rc = m_spInflection->eGetIrregularForms(hash.sGramHash(), mapIrreg);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }


        // Irregular forms
        bool bNoRegularForms = mapIrreg.empty() ? false : true;

        auto it = mapIrreg.begin();
        for (; it != mapIrreg.end(); ++it)
        {
            if ((*it).second)   // optional?  TODO: needs work; this DOES NOT indicate that a regular variant exists
            {
                bNoRegularForms = false;
            }

            auto spSource = it->first;
            shared_ptr<CWordForm> spWordForm = make_shared<CWordForm>();
            spWordForm->Copy(*spSource);
            m_spInflection->AddWordForm(spWordForm);
        }

        if (bNoRegularForms)
        {
            return H_NO_MORE;
        }

        bool bVelarStemAuslaut = false;

        // Regular forms
        m_spEndings = make_shared<CComparativeEndings>(m_spLexeme, m_spInflection);
        if (nullptr == m_spEndings)
        {
            return H_ERROR_POINTER;
        }

        CEString sGraphicStem = m_spLexeme->sGraphicStem();

        if (sGraphicStem.bEndsWithOneOf (L"кгх"))
        {
            bVelarStemAuslaut = true;

            if (sGraphicStem.bEndsWith (L"к"))
            {
                sStem[sStem.uiLength()-1] = L'ч';
            }
            if (sGraphicStem.bEndsWith (L"г"))
            {
                sStem[sStem.uiLength()-1] = L'ж';
            }
            if (sGraphicStem.bEndsWith (L"х"))
            {
                sStem[sStem.uiLength()-1] = L'ш';
            }

            auto spWordForm = make_shared<CWordForm>();
            rc = eCreateFormTemplate(sStem, spWordForm);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
            spWordForm->SetStem(sStem);
            
            static_pointer_cast<CComparativeEndings>(m_spEndings)->eSelect(bVelarStemAuslaut, bIsVariant);
            int iNumEndings = m_spEndings->iCount();
            if (iNumEndings < 1)
            {
                ERROR_LOG(L"No endings");
                return H_ERROR_UNEXPECTED;
            }
            if (iNumEndings > 1)
            {
                ERROR_LOG(L"More than one ending with a regular velar auslaut.");
            }

            int64_t llEndingKey = -1;
            CEString sEnding;
            rc = m_spEndings->eGetEnding(0, sEnding, llEndingKey);
            if (rc != H_NO_ERROR)
            {
                return H_ERROR_UNEXPECTED;
            }

            spWordForm->SetStem(sStem);
            spWordForm->SetEnding(sEnding);
            spWordForm->SetEndingDataId(llEndingKey);
            spWordForm->SetWordForm(sStem + sEnding);

            unsigned int uiLastStemVowel = sStem.uiFindLastOf (CEString::g_szRusVowels);
            if (ecNotFound != uiLastStemVowel)
            {
                unsigned int uiStressedSyllable = sStem.uiGetSyllableFromVowelPos(uiLastStemVowel);
//                spWordForm->m_mapStress[uiStressedSyllable] = STRESS_PRIMARY;
                spWordForm->SetStressPos(uiStressedSyllable, STRESS_PRIMARY);
            }
            else
            {
                ERROR_LOG (L"Warning: can't find stressed vowel in comparative.");
            }

            m_spInflection->AddWordForm (spWordForm);
        }
        else
        {
            bVelarStemAuslaut = false;

            vector<int> vecStress;
            ET_StressLocation eStressLocation = STRESS_LOCATION_UNDEFINED;

            if (m_spInflection->bHasCommonDeviation(2))
            {
                assert(sStem.bEndsWith(L"нн"));

                eStressLocation = STRESS_LOCATION_STEM;

                int iPos = -1;
                rc = m_spLexeme->eGetFirstStemStressPos(iPos);
                auto nSyllablesInStem = sStem.uiNSyllables();;
                while (H_NO_ERROR == rc && iPos < nSyllablesInStem)
                {
                    assert(iPos >= 0);
                    vecStress.push_back(iPos);
                    rc = m_spLexeme->eGetNextStemStressPos(iPos);
                }
            }
            else if ((AT_A == m_spInflection->eAccentType1()) && (AT_UNDEFINED == m_spInflection->eAccentType2()))
            {
                // &&&& Never has fleeting vowel???
                eStressLocation = STRESS_LOCATION_STEM;
                rc = eGetStemStressPositions(sStem, SUBPARADIGM_COMPARATIVE, NUM_UNDEFINED, vecStress);
                                                                                // Number not needed for this subparadigm
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
            }
            else
            {
                eStressLocation = STRESS_LOCATION_ENDING;
                int iStress = sStem.uiNSyllables();
                vecStress.push_back(iStress);
            }

            static_pointer_cast<CComparativeEndings>(m_spEndings)->eSelect(bVelarStemAuslaut, bIsVariant);
            int iNumEndings = m_spEndings->iCount();
            if (iNumEndings < 1)
            {
                ERROR_LOG(L"No endings");
                return H_ERROR_UNEXPECTED;
            }
            if (iNumEndings > 2)
            {
                ERROR_LOG(L"More than one ending with a regular non-velar auslaut.");
            }

            for (int iEnding = 0; iEnding < iNumEndings; ++iEnding)
            {
                shared_ptr<CWordForm> spWordForm;
                rc = eCreateFormTemplate(sStem, spWordForm);
                if (rc != H_NO_ERROR)
                {
                    continue;
                }

                CEString sEnding;
                int64_t llEndingKey = -1;
                rc = m_spEndings->eGetEnding(iEnding, sEnding, llEndingKey);
                if (rc != H_NO_ERROR)
                {
                    continue;
                }

                rc = eHandleStressAndAdd(spWordForm, vecStress, eStressLocation, sStem, sEnding, llEndingKey);
 //               if (rc != H_NO_ERROR)
 //               {
 //                   continue;
 //               }
            }
        }
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;  // logging should be always done by callee
    }

    return H_NO_ERROR;

}   //  eBuild()
