#include "StdAfx.h"
#include "FormBuilderAdjComparative.h"

HRESULT CT_FormBuilderComparative::h_CreateFormTemplate (const wstring& str_lemma, 
                                                         CComObject<CT_WordForm> *& pco_wordForm)
{
    HRESULT h_r = S_OK;

    h_r = CComObject<CT_WordForm>::CreateInstance (&pco_wordForm);
    if (S_OK != h_r)
    {
        ATLASSERT(0);
        CString cs_msg;
        cs_msg.Format (L"CreateInstance() failed, error %x.", h_r);
        ERROR_LOG ((LPCTSTR)cs_msg);
        throw CT_Exception (E_INVALIDARG, (LPCTSTR)cs_msg);
    }

    pco_wordForm->pco_Lexeme = pco_Lexeme;
    pco_wordForm->eo_POS = pco_Lexeme->eo_PartOfSpeech;
    pco_wordForm->eo_Subparadigm = SUBPARADIGM_COMPARATIVE;
    pco_wordForm->str_Lemma = str_lemma;
    v_AssignSecondaryStress (pco_wordForm);
                                                                    //  ^-- only secondary   
    return S_OK;

}   //  h_CreateFormTemplate (...)

HRESULT CT_FormBuilderComparative::h_HandleStressAndAdd (CComObject<CT_WordForm> * pco_wordForm,
                                                         vector<int>& vec_iStress)
{
    HRESULT h_r = S_OK;

    long l_gramHash = pco_wordForm->i_GramHash();

    if (1 == vec_iStress.size() || b_MultiStress (pco_wordForm->str_Lemma, vec_iStress))
    {
        vector<int>::iterator it_stressPos = vec_iStress.begin();
        for (; it_stressPos != vec_iStress.end(); ++it_stressPos)
        {
            pco_wordForm->map_Stress[*it_stressPos] = STRESS_PRIMARY;
        }
        pco_Lexeme->h_AddWordForm (pco_wordForm);
    }
    else
    {
        vector<int>::iterator it_stressPos = vec_iStress.begin();
        for (; it_stressPos != vec_iStress.end(); ++it_stressPos)
        {
            if (it_stressPos != vec_iStress.begin())
            {
                CComObject<CT_WordForm> * pco_wfVariant = NULL;
                h_CloneWordForm (pco_wordForm, pco_wfVariant);
                pco_wordForm = pco_wfVariant;
            }
            pco_wordForm->map_Stress[*it_stressPos] = STRESS_PRIMARY;
            pco_Lexeme->h_AddWordForm (pco_wordForm);
        }
    }

    return S_OK;

}   //  h_HandleStress (...)

HRESULT CT_FormBuilderComparative::h_Build()
{
    HRESULT h_r = S_OK;

    try
    {
        CT_ExtString& xstr_graphicStem = pco_Lexeme->xstr_GraphicStem;
        wstring str_lemma (pco_Lexeme->xstr_GraphicStem.c_str());

        CComObject<CT_WordForm> * pco_wordForm = NULL;
        h_CreateFormTemplate (str_lemma, pco_wordForm);
        bool b_isVariant = false;
        h_r = h_HandleIrregularForms (pco_wordForm, b_isVariant);
        if (FAILED (h_r))
        {
            ATLASSERT (0);
            CString cs_msg;
            cs_msg.Format (L"h_HandleIrregularForms() failed, error %x.", h_r);
            ERROR_LOG ((LPCTSTR)cs_msg);
            return h_r;
        }

        if ((S_OK == h_r) && !b_isVariant)
        {
            return S_OK;
        }

        if (xstr_graphicStem.b_EndsWithOneOf (L"кгх"))
        {
            if (xstr_graphicStem.b_EndsWith (L"к"))
            {
                str_lemma[str_lemma.length()-1] = L'ч';
            }
            if (xstr_graphicStem.b_EndsWith (L"г"))
            {
                str_lemma[str_lemma.length()-1] = L'ж';
            }
            if (xstr_graphicStem.b_EndsWith (L"х"))
            {
                str_lemma[str_lemma.length()-1] = L'ш';
            }

//            CComObject<CT_WordForm> * pco_wordForm = NULL;
//            h_CreateFormTemplate (str_lemma, pco_wordForm);
            pco_wordForm->str_Lemma = str_lemma;
            pco_wordForm->str_WordForm = str_lemma + L"е";

            int i_lastStemVowel = str_lemma.find_last_of (str_Vowels);
            if (wstring::npos != i_lastStemVowel)
            {
                pco_wordForm->map_Stress[i_lastStemVowel] = STRESS_PRIMARY;
            }
            else
            {
                ERROR_LOG (L"Warning: can't find stressed vowel in comparative.");
            }

            int i_gramHash = pco_wordForm->i_GramHash();
            pco_Lexeme->h_AddWordForm (pco_wordForm);
        }
        else
        {
//            CComObject<CT_WordForm> * pco_wordForm = NULL;
//            h_CreateFormTemplate (str_lemma, pco_wordForm);
            pco_wordForm->str_Lemma = str_lemma;
            pco_wordForm->str_WordForm = str_lemma + L"ей";

            vector<int> vec_iStress;
            if ((AT_A == pco_Lexeme->eo_AccentType1) && (AT_UNDEFINED == pco_Lexeme->eo_AccentType2))
            {
                // &&&& Never has fleeting vowel???
                h_r = h_GetStemStressPositions (str_lemma, SUBPARADIGM_COMPARATIVE, vec_iStress);
            }
            else
            {
                int i_stress = CT_ExtString (str_lemma).i_NSyllables();
                vec_iStress.push_back (i_stress);
//                vec_iStress.push_back ((int)str_lemma.length());
            }

            CComObject<CT_WordForm> * pco_wordForm1 = NULL;
            h_CreateFormTemplate (str_lemma, pco_wordForm1);
            pco_wordForm1->str_WordForm = str_lemma + L"ей";
            h_HandleStressAndAdd (pco_wordForm1, vec_iStress);

            CComObject<CT_WordForm> * pco_wordForm2 = NULL;
            h_CreateFormTemplate (str_lemma, pco_wordForm2);
            pco_wordForm2->str_WordForm = str_lemma + L"ее";
            h_HandleStressAndAdd (pco_wordForm2, vec_iStress);

        }
    }
    catch (CT_Exception co_ex)
    {
        return co_ex.i_GetErrorCode();  // logging should be always done by callee
    }

    return S_OK;

}   //  h_Build()
