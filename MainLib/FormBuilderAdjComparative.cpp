#include "StdAfx.h"

#include "WordForm.h"
#include "Lexeme.h"
#include "FormBuilderAdjComparative.h"

using namespace Hlib;

ET_ReturnCode CFormBuilderComparative::eCreateFormTemplate (const CEString& sLemma, CWordForm *& pWordForm)
{
    pWordForm = new CWordForm;
    if (NULL == pWordForm)
    {
        return H_ERROR_POINTER;
    }

    pWordForm->m_pLexeme = m_pLexeme;
    pWordForm->m_ePos = m_pLexeme->ePartOfSpeech();
    pWordForm->m_eSubparadigm = SUBPARADIGM_COMPARATIVE;
    pWordForm->m_sLemma = sLemma;

    return H_NO_ERROR;

}   //  eCreateFormTemplate (...)

ET_ReturnCode CFormBuilderComparative::eHandleStressAndAdd(CWordForm * pWordForm, 
                                                           vector<int>& vecStress, 
                                                           ET_StressLocation eStressType, 
                                                           CEString& sStem, 
                                                           const CEString& sEnding)
{
    ET_ReturnCode rc = H_NO_ERROR;

    ASSERT(m_pLexeme);   // we assume base class ctor took care of this

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

    ASSERT(m_pLexeme);   // we assume base class ctor took care of this

    try
    {
        CEString sLemma(m_pLexeme->sGraphicStem());
        CGramHasher hash(SUBPARADIGM_COMPARATIVE, NUM_UNDEFINED, GENDER_UNDEFINED, ANIM_UNDEFINED, CASE_UNDEFINED);
        map<CWordForm *, bool> mapIrreg;
        rc = m_pLexeme->eGetIrregularForms(hash.sGramHash(), mapIrreg);
        if (rc != H_NO_ERROR)
        {
            return rc;
        }

        bool bNoRegularForms = mapIrreg.empty() ? false : true;

        map<CWordForm *, bool>::iterator it = mapIrreg.begin();
        for (; it != mapIrreg.end(); ++it)
        {
            if ((*it).second)   // optional?
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

        CEString sGraphicStem = m_pLexeme->sGraphicStem();
        if (sGraphicStem.bEndsWithOneOf (L"кгх"))
        {
            if (sGraphicStem.bEndsWith (L"к"))
            {
                sLemma[sLemma.uiLength()-1] = L'ч';
            }
            if (sGraphicStem.bEndsWith (L"г"))
            {
                sLemma[sLemma.uiLength()-1] = L'ж';
            }
            if (sGraphicStem.bEndsWith (L"х"))
            {
                sLemma[sLemma.uiLength()-1] = L'ш';
            }

            CWordForm * pWordForm = NULL;
            rc = eCreateFormTemplate(sLemma, pWordForm);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
            pWordForm->m_sLemma = sLemma;
            pWordForm->m_sWordForm = sLemma + L"е";

            unsigned int uiLastStemVowel = sLemma.uiFindLastOf (g_szRusVowels);
            if (ecNotFound != uiLastStemVowel)
            {
                unsigned int uiStressedSyllable = sLemma.uiGetSyllableFromVowelPos(uiLastStemVowel);
                pWordForm->m_mapStress[uiStressedSyllable] = STRESS_PRIMARY;
            }
            else
            {
                ERROR_LOG (L"Warning: can't find stressed vowel in comparative.");
            }

//            int iGramHash = pWordForm->sGramHash();
            m_pLexeme->AddWordForm (pWordForm);
        }
        else
        {
            vector<int> vecStress;
            ET_StressLocation eStressLocation = STRESS_LOCATION_UNDEFINED;
            if ((AT_A == m_pLexeme->eAccentType1()) && (AT_UNDEFINED == m_pLexeme->eAccentType2()))
            {
                // &&&& Never has fleeting vowel???
                eStressLocation = STRESS_LOCATION_STEM;
                rc = eGetStemStressPositions(sLemma, SUBPARADIGM_COMPARATIVE, vecStress);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
            }
            else
            {
                eStressLocation = STRESS_LOCATION_ENDING;
                int iStress = sLemma.uiNSyllables();
                vecStress.push_back(iStress);
            }

            CWordForm * pWordForm1 = NULL;
            rc = eCreateFormTemplate (sLemma, pWordForm1);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }

            rc = eHandleStressAndAdd(pWordForm1, vecStress, eStressLocation, sLemma, L"ей");
            if (rc != H_NO_ERROR)
            {
                return rc;
            }

            CWordForm * pWordForm2 = NULL;
            rc = eCreateFormTemplate (sLemma, pWordForm2);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }

            rc = eHandleStressAndAdd(pWordForm2, vecStress, eStressLocation, sLemma, L"ее");
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
        }
    }
    catch (CException ex)
    {
        return H_EXCEPTION;  // logging should be always done by callee
    }

    return H_NO_ERROR;

}   //  eBuild()
