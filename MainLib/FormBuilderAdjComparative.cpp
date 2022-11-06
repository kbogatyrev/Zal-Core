#include <assert.h>

#include "Logging.h"
#include "WordForm.h"
#include "Lexeme.h"
#include "FormBuilderAdjComparative.h"

using namespace Hlib;

ET_ReturnCode CFormBuilderComparative::eCreateFormTemplate (const CEString& sStem, CWordForm *& pWordForm)
{
    pWordForm = new CWordForm;
    if (NULL == pWordForm)
    {
        return H_ERROR_POINTER;
    }

    pWordForm->m_pLexeme = m_pLexeme;
    pWordForm->m_ePos = m_pLexeme->ePartOfSpeech();
    pWordForm->m_eSubparadigm = SUBPARADIGM_COMPARATIVE;
    pWordForm->m_sStem = sStem;
    pWordForm->m_llLexemeId = m_pLexeme->llLexemeId();

    return H_NO_ERROR;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderComparative::eHandleStressAndAdd(CWordForm * pWordForm, 
                                                           vector<int>& vecStress, 
                                                           ET_StressLocation eStressType, 
                                                           CEString& sStem, 
                                                           const CEString& sEnding,
                                                           long long llEndingKey)
{
    ET_ReturnCode rc = H_NO_ERROR;

    assert(m_pLexeme);   // we assume base class ctor took care of this

    CEString sGramHash = pWordForm->sGramHash();

    try
    {
        if (1 == vecStress.size() || m_pLexeme->bIsMultistressedCompound())
        {
            vector<int>::iterator itStressPos = vecStress.begin();
            for (; itStressPos != vecStress.end(); ++itStressPos)
            {
                pWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                rc = eHandleYoAlternation(eStressType, *itStressPos, sStem, sEnding);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
            }

            pWordForm->m_sStem = sStem;
            pWordForm->m_sEnding = sEnding;
            pWordForm->m_llEndingDataId = llEndingKey;
            pWordForm->m_sWordForm = sStem + sEnding;

            m_pLexeme->AddWordForm(pWordForm);
        }
        else
        {
            vector<int>::iterator itStressPos = vecStress.begin();
            for (; itStressPos != vecStress.end(); ++itStressPos)
            {
                if (itStressPos != vecStress.begin())
                {
                    CWordForm * pWfVariant = NULL;
                    CloneWordForm (pWordForm, pWfVariant);
                    pWordForm = pWfVariant;
                }
                pWordForm->m_mapStress[*itStressPos] = STRESS_PRIMARY;
                rc = eHandleYoAlternation(eStressType, *itStressPos, sStem, sEnding);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                pWordForm->m_sStem = sStem;
                pWordForm->m_sEnding = sEnding;
                pWordForm->m_llEndingDataId = llEndingKey;
                pWordForm->m_sWordForm = sStem + sEnding;

                m_pLexeme->AddWordForm(pWordForm);
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

    assert(m_pLexeme);   // we assume base class ctor took care of this

    if (m_pLexeme->bHasMissingForms() && m_pLexeme->eFormExists(L"AdjComp"))
    {
        return H_NO_ERROR;
    }

    bool bIsVariant = false;

    try
    {
        CEString sStem(m_pLexeme->sGraphicStem());
        CGramHasher hash(SUBPARADIGM_COMPARATIVE, NUM_UNDEFINED, GENDER_UNDEFINED, ANIM_UNDEFINED, CASE_UNDEFINED);
        map<CWordForm *, bool> mapIrreg;
        rc = m_pLexeme->eGetIrregularForms(hash.sGramHash(), mapIrreg);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }


        // Irregular forms
        bool bNoRegularForms = mapIrreg.empty() ? false : true;

        map<CWordForm *, bool>::iterator it = mapIrreg.begin();
        for (; it != mapIrreg.end(); ++it)
        {
            if ((*it).second)   // optional?  TODO: needs work; this DOES NOT indicate that a regular variant exists
            {
                bNoRegularForms = false;
            }

            CWordForm * pWordForm = new CWordForm((*it).first);
            m_pLexeme->AddWordForm(pWordForm);
        }

        if (bNoRegularForms)
        {
            return H_NO_MORE;
        }

        bool bVelarStemAuslaut = false;

        // Regular forms
        m_pEndings = new CComparativeEndings(m_pLexeme);
        if (NULL == m_pEndings)
        {
            return H_ERROR_POINTER;
        }

        CEString sGraphicStem = m_pLexeme->sGraphicStem();

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

            CWordForm * pWordForm = NULL;
            rc = eCreateFormTemplate(sStem, pWordForm);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
            pWordForm->m_sStem = sStem;
            
            ((CComparativeEndings*)m_pEndings)->eSelect(bVelarStemAuslaut, bIsVariant);
            int iNumEndings = m_pEndings->iCount();
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
            rc = m_pEndings->eGetEnding(0, sEnding, llEndingKey);
            if (rc != H_NO_ERROR)
            {
                return H_ERROR_UNEXPECTED;
            }

            pWordForm->m_sStem = sStem;
            pWordForm->m_sEnding = sEnding;
            pWordForm->m_llEndingDataId = llEndingKey;
            pWordForm->m_sWordForm = sStem + sEnding;

            unsigned int uiLastStemVowel = sStem.uiFindLastOf (CEString::g_szRusVowels);
            if (ecNotFound != uiLastStemVowel)
            {
                unsigned int uiStressedSyllable = sStem.uiGetSyllableFromVowelPos(uiLastStemVowel);
                pWordForm->m_mapStress[uiStressedSyllable] = STRESS_PRIMARY;
            }
            else
            {
                ERROR_LOG (L"Warning: can't find stressed vowel in comparative.");
            }

            m_pLexeme->AddWordForm (pWordForm);
        }
        else
        {
            bVelarStemAuslaut = false;

            vector<int> vecStress;
            ET_StressLocation eStressLocation = STRESS_LOCATION_UNDEFINED;

            if (m_pLexeme->bHasCommonDeviation(2))
            {
                assert(sStem.bEndsWith(L"нн"));

                eStressLocation = STRESS_LOCATION_STEM;

                int iPos = -1;
                rc = m_pLexeme->eGetFirstStemStressPos(iPos);
                auto nSyllablesInStem = sStem.uiNSyllables();;
                while (H_NO_ERROR == rc && iPos < nSyllablesInStem)
                {
                    assert(iPos >= 0);
                    vecStress.push_back(iPos);
                    rc = m_pLexeme->eGetNextStemStressPos(iPos);
                }
            }
            else if ((AT_A == m_pLexeme->eAccentType1()) && (AT_UNDEFINED == m_pLexeme->eAccentType2()))
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

            ((CComparativeEndings*)m_pEndings)->eSelect(bVelarStemAuslaut, bIsVariant);
            int iNumEndings = m_pEndings->iCount();
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
                CWordForm* pWordForm = NULL;
                rc = eCreateFormTemplate(sStem, pWordForm);
                if (rc != H_NO_ERROR)
                {
                    continue;
                }

                CEString sEnding;
                int64_t llEndingKey = -1;
                rc = m_pEndings->eGetEnding(iEnding, sEnding, llEndingKey);
                if (rc != H_NO_ERROR)
                {
                    continue;
                }

                rc = eHandleStressAndAdd(pWordForm, vecStress, eStressLocation, sStem, sEnding, llEndingKey);
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
