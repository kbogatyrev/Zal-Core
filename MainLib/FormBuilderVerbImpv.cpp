#include "StdAfx.h"
#include "FormBuilderVerbImpv.h"

HRESULT CT_FormBuilderImperative::h_Build()
{
    HRESULT h_r = S_OK;
 
    int i_type = pco_Lexeme->i_Type;
    CT_ExtString& xstr_1SgStem = pco_Lexeme->xstr_1SgStem;
    CT_ExtString& xstr_3SgStem = pco_Lexeme->xstr_3SgStem;
    ET_AccentType eo_accentType1 = pco_Lexeme->eo_AccentType1;
    ET_AccentType eo_accentType2 = pco_Lexeme->eo_AccentType2;
    vector<int>& vec_sourceStressPos = pco_Lexeme->vec_SourceStressPos;

    CT_ExtString xstr_lemma;
    
    if (4 == i_type || 5 == i_type)
    {
        xstr_lemma = xstr_3SgStem;    
    }
    else
    {
        xstr_lemma = xstr_1SgStem;
    }

    wstring str_ending;
    if (11 == i_type)
    {
        ATLASSERT (xstr_lemma.b_EndsWith (L"ь"));
        str_ending = L"ей";
    }
    else if (xstr_lemma.b_EndsWithOneOf (str_Vowels))
    {
        str_ending = L"й";
        if (4 == i_type)
        {
            if (AT_B == pco_Lexeme->eo_AccentType1 || 
                AT_C == pco_Lexeme->eo_AccentType1)   // кроить, поить
            {
                str_ending = L"и";
            }
            if (AT_A == eo_accentType1 && xstr_lemma.b_StartsWith (L"вы"))
            {
                str_ending = L"и";
            }
        }
    }
    else
    {
        if (AT_B == eo_accentType1 || AT_C == eo_accentType1)
        {
            str_ending = L"и";
        }
        if (AT_A == eo_accentType1)
        {
            if (xstr_lemma.b_StartsWith (L"вы"))
            {
                str_ending = L"и";
            }
            else if (xstr_lemma.b_EndsWith (L"щ"))      // 2 consonants or щ or CьC
            {
                str_ending = L"и";
            }
            else if (xstr_lemma.length() >= 2 && 
                str_Consonants.find (xstr_lemma[xstr_lemma.length()-1]) != wstring::npos &&
                str_Consonants.find (xstr_lemma[xstr_lemma.length()-2]) != wstring::npos)
            {
                str_ending = L"и";
            }
            else if (xstr_lemma.length() >= 3 && 
                     str_Consonants.find (xstr_lemma[xstr_lemma.length()-1]) != wstring::npos &&
                     L'ь' == xstr_lemma[xstr_lemma.length()-2] &&
                     str_Consonants.find (xstr_lemma[xstr_lemma.length()-3]) != wstring::npos)
            {
                str_ending = L"и";
            }
            else
            {
                str_ending = L"ь";
            }
        }
    }

    ET_EndingStressType eo_stress = ENDING_STRESS_UNDEFINED;
    switch (eo_accentType1)
    {
        case AT_A:
        {
            eo_stress = ENDING_STRESS_UNSTRESSED;
            break;
        }
        case AT_B:
        case AT_C:
        {
            eo_stress = ENDING_STRESS_STRESSED;
            break;
        }
        default:
        {
            ERROR_LOG (L"Illegal accent type.");
            return E_FAIL;
        }
    }

    int i_stressPos = -1;
//    h_r = h_GetStressPosition (xstr_lemma, str_ending, eo_stress, i_stressPos);
    if (ENDING_STRESS_UNSTRESSED == eo_stress)
    {
//        h_r = h_GetStemStressPosition (xstr_lemma, i_stressPos);
    }
    else if (ENDING_STRESS_STRESSED == eo_stress)
    {
        h_r = h_GetEndingStressPosition (xstr_lemma, str_ending, i_stressPos);
    }
    else
    {
        ATLASSERT (0);
        ERROR_LOG (L"Illegal stress type.");
        return E_INVALIDARG;
    }
    
    for (ET_Number eo_number = NUM_SG; eo_number < NUM_COUNT; ++eo_number)
    {
        int i_iterations = (vec_sourceStressPos.size() > 1) ? vec_sourceStressPos.size() : 1;
        for (int i_wf = 0; i_wf < i_iterations; ++i_wf)
        {
            CComObject<CT_WordForm> * pco_wordForm;
            h_r = CComObject<CT_WordForm>::CreateInstance (&pco_wordForm);
            if (S_OK != h_r)
            {
                return h_r;
            }

            pco_wordForm->eo_POS = pco_Lexeme->eo_PartOfSpeech;
            pco_wordForm->eo_Subparadigm = SUBPARADIGM_IMPERATIVE;
            pco_wordForm->eo_Reflexive = pco_Lexeme->b_Reflexive ? REFL_YES : REFL_NO;
            pco_wordForm->str_Lemma = xstr_lemma;
            if (NUM_PL == eo_number)
            {
                str_ending += L"те";
            }
            pco_wordForm->str_WordForm = xstr_lemma + str_ending;
            pco_wordForm->eo_Number = eo_number;
//            pco_wordForm->i_Stress = (1 == i_iterations) ? i_stressPos : vec_SourceStressPos[i_wf];

            long l_gramHash = 0;
            pco_wordForm->get_GramHash (&l_gramHash);
            pco_Lexeme->v_AddWordForm (l_gramHash, pco_wordForm);
        }
    }

    return h_r;

}   //  h_BuildImperativeForms()
