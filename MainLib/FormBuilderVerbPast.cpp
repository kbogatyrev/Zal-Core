#include "StdAfx.h"
#include "FormBuilderVerbPast.h"

static wchar_t arr_endings[][4] = { L"л", L"ла", L"ло", L"ли" };

HRESULT CT_FormBuilderPast::h_GetEnding (wstring& str_lemma, 
                                         ET_Number eo_number, 
                                         ET_Gender eo_gender, 
                                         wstring& str_ending)
{
    HRESULT h_r = S_OK;

    if (NUM_SG == eo_number)
    {
        if (GENDER_M == eo_gender && CT_ExtString(str_lemma).b_EndsWithOneOf (str_Consonants))
        {
            str_ending = L"";
        }
        else
        {
            str_ending = arr_endings[eo_gender-1];
        }
    }
    else
    {
        str_ending = arr_endings[3];
    }

    return S_OK;

}   //  h_GetEnding (...)

HRESULT CT_FormBuilderPast::h_GetStressPositions (const wstring& str_lemma,
                                                  const wstring& str_ending,
                                                  ET_Number eo_number, 
                                                  ET_Gender eo_gender, 
                                                  vector<int>& vec_iStressPositions)
{
    HRESULT h_r = S_OK;

    ET_AccentType eo_accentType = AT_UNDEFINED;
    if (AT_UNDEFINED == pco_Lexeme->eo_AccentType2)
    {
        eo_accentType = AT_A;
    }
    else
    {
        eo_accentType = pco_Lexeme->eo_AccentType2;
    }

    vector<ET_StressLocation> vec_eoStressType;
    h_GetPastTenseStressTypes (eo_accentType, eo_number, eo_gender, vec_eoStressType);

    vector<ET_StressLocation>::iterator it_stressType = vec_eoStressType.begin();
    for (; it_stressType != vec_eoStressType.end(); ++it_stressType)
    {
        if (STRESS_LOCATION_STEM == *it_stressType)
        {
            h_GetStemStressPositions (str_lemma, vec_iStressPositions);
        }

        if (STRESS_LOCATION_ENDING == *it_stressType)
        {
            if (AT_C2 == eo_accentType && GENDER_M == eo_gender && NUM_SG == eo_number)
            {
                if (REFL_NO == pco_Lexeme->eo_Reflexive)
                {
                    ATLASSERT(0);
                    wstring str_msg (L"Unexpected part of speech value.");
                    ERROR_LOG (str_msg);
                    throw CT_Exception (E_FAIL, str_msg);
                }
                // This is or is becoming obsolete? (GDRL, p. 80-81)
                vec_iStressPositions.push_back (CT_ExtString (str_lemma).i_NSyllables());
            }
            else
            {
                int i_pos = -1;
                h_GetEndingStressPosition (str_lemma, str_ending, i_pos);
                vec_iStressPositions.push_back (i_pos);
            }
        }
    }

    return S_OK;

}   //  h_GetStressPositions (...)

HRESULT CT_FormBuilderPast::h_CreateFormTemplate (const wstring& str_lemma,
                                                  const wstring& str_ending,
                                                  ET_Number eo_number, 
                                                  ET_Gender eo_gender, 
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
        throw CT_Exception (h_r, (LPCTSTR)cs_msg);
    }

    pco_wordForm->pco_Lexeme = pco_Lexeme;
    pco_wordForm->eo_POS = pco_Lexeme->eo_PartOfSpeech;
    pco_wordForm->eo_Subparadigm = SUBPARADIGM_PAST_TENSE;
    pco_wordForm->eo_Reflexive = pco_Lexeme->eo_Reflexive;
    pco_wordForm->str_Lemma = str_lemma;
    pco_wordForm->str_WordForm = str_lemma + str_ending;
    pco_wordForm->eo_Gender = eo_gender;
    pco_wordForm->eo_Number = eo_number;

    v_AssignSecondaryStress (pco_wordForm);

    return S_OK;

}   //  h_CreateFormTemplate (...)

HRESULT CT_FormBuilderPast::h_Assemble (CT_WordForm * pco_wordForm, 
                                        int i_stressPos, 
                                        wstring& str_lemma, 
                                        wstring& str_ending)
{
    pco_wordForm->map_Stress[i_stressPos] = true;  // primary
    pco_wordForm->str_Lemma = str_lemma;
    h_HandleYoAlternation (i_stressPos, SUBPARADIGM_PAST_TENSE, pco_wordForm->str_Lemma);
    pco_wordForm->str_WordForm = pco_wordForm->str_Lemma + str_ending;
    if (b_HasCommonDeviation (1) && 
        !(NUM_SG == pco_wordForm->eo_Number && 
          GENDER_F == pco_wordForm->eo_Gender))
    {
        h_RetractStressToPreverb (pco_wordForm, b_DeviationOptional (1));
    }

    if (b_HasCommonDeviation (5) && 
        3 == pco_Lexeme->i_Type && 
        pco_Lexeme->b_StemAugment && 
        NUM_SG == pco_wordForm->eo_Number &&
        GENDER_M == pco_wordForm->eo_Gender)
    {
        if (b_DeviationOptional (5))
        {
            CComObject<CT_WordForm> * pco_variant = NULL;
            h_CloneWordForm (pco_wordForm, pco_variant);
            pco_Lexeme->h_AddWordForm (pco_variant);    // store both versions
            pco_wordForm = pco_variant;
        }

        pco_wordForm->str_WordForm = str_lemma + L"нул";
    }

    return S_OK;

}   // h_Assemble (...)

HRESULT CT_FormBuilderPast::h_Build()
{
    HRESULT h_r = S_OK;

    try
    {
        vector<int>& vec_sourceStressPos = pco_Lexeme->vec_SourceStressPos;
     
        wstring str_lemma;
        h_BuildPastTenseStem (str_lemma);
        ET_AccentType eo_at = AT_UNDEFINED;
        if (AT_UNDEFINED == pco_Lexeme->eo_AccentType2)
        {
            eo_at = AT_A;
        }
        else
        {
            eo_at = pco_Lexeme->eo_AccentType2;
        }

        wstring str_ending;
        for (ET_Number eo_number = NUM_SG; eo_number < NUM_COUNT; ++eo_number)
        {
            for (ET_Gender eo_gender = GENDER_UNDEFINED; eo_gender < GENDER_COUNT; ++eo_gender)
            {
                if (NUM_PL == eo_number && eo_gender != GENDER_UNDEFINED)
                {
                    continue;
                }
                if (NUM_SG == eo_number && eo_gender == GENDER_UNDEFINED)
                {
                    continue;
                }

                h_GetEnding (str_lemma, eo_number, eo_gender, str_ending);

                if (REFL_YES == pco_Lexeme->eo_Reflexive)
                {
                    if (CT_ExtString (str_ending).b_EndsWithOneOf (str_Vowels))
                    {
                        str_ending += L"сь";
                    }
                    else
                    {
                        str_ending += L"ся";
                    }
                }

                vector<int> vec_iStress;
                h_GetStressPositions (str_lemma, str_ending, eo_number, eo_gender, vec_iStress);

                vector<int>::iterator it_stress = vec_iStress.begin();
                for (; it_stress != vec_iStress.end(); ++it_stress)
                {
                    CComObject<CT_WordForm> * pco_wordForm = NULL;
                    h_CreateFormTemplate (str_lemma, str_ending, eo_number, eo_gender, pco_wordForm);
                    int i_gramHash = pco_wordForm->i_GramHash();
                    if (1 == vec_iStress.size() || b_MultiStress (str_lemma, vec_iStress))
                    {
                        vector<int>::iterator it_stressPos = vec_iStress.begin();
                        for (; it_stressPos != vec_iStress.end(); ++it_stressPos)
                        {
                            h_Assemble (pco_wordForm, *it_stressPos, str_lemma, str_ending);
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
                                pco_wfVariant->map_Stress.clear();
                                pco_wordForm = pco_wfVariant;
                            }
                            h_Assemble (pco_wordForm, *it_stressPos, str_lemma, str_ending);
                            pco_Lexeme->h_AddWordForm (pco_wordForm);
                        }
                    }
                }   // for (; it_stress != vec_iStress.end(); ++it_stress)
            }   //  for (ET_Gender eo_gender = ... )
        }   // for ...(ET_Number = ... )
    }
    catch (CT_Exception co_ex)
    {
        return co_ex.i_GetErrorCode();  // logging should be always done by callee
    }

    return h_r;

}   //  h_Build()
