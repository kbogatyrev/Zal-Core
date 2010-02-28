#include "StdAfx.h"
#include "FormBuilderVerbPast.h"

// &&&& TODO -- standard deviations not handled

HRESULT CT_FormBuilderPast::h_Build()
{
    HRESULT h_r = S_OK;

    vector<int>& vec_sourceStressPos = pco_Lexeme->vec_SourceStressPos;
 
    CT_ExtString xstr_lemma;
    h_r = h_BuildPastTenseStem (xstr_lemma);
    ET_AccentType eo_at = AT_UNDEFINED;
    if (AT_UNDEFINED == pco_Lexeme->eo_AccentType2)
    {
        eo_at = AT_A;
    }
    else
    {
        eo_at = pco_Lexeme->eo_AccentType2;
    }

    CT_ExtString xstr_ending;
    wchar_t arr_endings[][4] = { L"л", L"ла", L"ло", L"ли" };

    for (ET_Number eo_number = NUM_SG; eo_number < NUM_COUNT; ++eo_number)
        for (ET_Gender eo_gender = GENDER_M; eo_gender < GENDER_COUNT; ++eo_gender)
        {
            if (NUM_SG == eo_number)
            {
                if (GENDER_M == eo_gender && xstr_lemma.b_EndsWithOneOf (str_Consonants))
                {
                    xstr_ending = L"";
                }
                else
                {
                    xstr_ending = arr_endings[eo_gender-1];
                }
            }
            else
            {
                xstr_ending = arr_endings[3];
            }

            if (pco_Lexeme->b_Reflexive)
            {
                if (xstr_ending.b_EndsWithOneOf (str_Vowels))
                {
                    xstr_ending += L"сь";
                }
                else
                {
                    xstr_ending += L"ся";
                }
            }

            h_r = h_StressOnPastTenseEnding (eo_at, eo_number, eo_gender);
            vector<ET_EndingStressType> vec_stress;
            switch (h_r)
            {
                case S_FALSE:
                {
                    vec_stress.push_back (ENDING_STRESS_UNSTRESSED);
                    break;
                }
                case S_TRUE:
                {
                    vec_stress.push_back (ENDING_STRESS_STRESSED);
                    break;
                }
                case S_BOTH:
                {
                    vec_stress.push_back (ENDING_STRESS_UNSTRESSED);
                    vec_stress.push_back (ENDING_STRESS_UNSTRESSED);
                    break;
                }                
                default:
                {
                    ERROR_LOG (L"Unexpected past tense accent type.");
                    return h_r;
                }

            }   // switch (h_r)

            vector<ET_EndingStressType>::iterator it_stress = vec_stress.begin();
            for (; it_stress != vec_stress.end(); ++it_stress)
            {
                int i_stressPos = -1;
                if (ENDING_STRESS_UNSTRESSED == *it_stress)
                {
//                    h_r = h_GetVerbStemStressPosition (xstr_lemma, i_stressPos);
                }
                else if (ENDING_STRESS_STRESSED == *it_stress)
                {
//                    h_r = h_GetVerbEndingStressPosition (xstr_lemma, xstr_ending, i_stressPos);
                }
                else
                {
                    ATLASSERT (0);
                    ERROR_LOG (L"Illegal stress type.");
                    return E_INVALIDARG;
                }

                int i_iterations = (vec_sourceStressPos.size() > 1) ? vec_sourceStressPos.size() : 1;
                for (int i_wf = 0; i_wf < i_iterations; ++i_wf)
                {
                    CComObject<CT_WordForm> * pco_wordForm;
                    h_r = CComObject<CT_WordForm>::CreateInstance (&pco_wordForm);
                    if (S_OK != h_r)

                    pco_wordForm->eo_POS = pco_Lexeme->eo_PartOfSpeech;
                    pco_wordForm->eo_Subparadigm = SUBPARADIGM_PAST_TENSE;
                    pco_wordForm->eo_Reflexive = pco_Lexeme->b_Reflexive ? REFL_YES : REFL_NO;
                    pco_wordForm->str_Lemma = xstr_lemma;
                    pco_wordForm->str_WordForm = xstr_lemma + xstr_ending;
                    pco_wordForm->eo_Gender = eo_gender;
                    pco_wordForm->eo_Number = eo_number;
//                    pco_wordForm->i_Stress = (1 == i_iterations) ? i_stressPos : vec_SourceStressPos.size();

                    long l_gramHash = 0;
                    pco_wordForm->get_GramHash (&l_gramHash);
                    pco_Lexeme->v_AddWordForm (l_gramHash, pco_wordForm);
                }
            }
        }   // for ...(ET_Number / ET_Gender ... )

    return h_r;

}   //  h_Build()
