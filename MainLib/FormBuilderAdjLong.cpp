#include "StdAfx.h"
#include "Endings.h"
#include "FormBuilderAdjLong.h"

HRESULT CT_FormBuilderLongAdj::h_GetEndings()
{
    HRESULT h_r = S_OK;

    int i_type = -1;

    wstring str_select (L"SELECT DISTINCT ending, gender, number, case_value, animacy, stress FROM endings");
    str_select += L" WHERE inflection_class = ";

    if (L"мс" == pco_Lexeme->str_InflectionType)
    {
        str_select += str_ToString (ENDING_CLASS_PRONOUN);
    }
    else
    {
        str_select += str_ToString (ENDING_CLASS_ADJECTIVE);
    }

    str_select += L" AND inflection_type = ";
    if (SUBPARADIGM_PART_PRES_ACT == eo_Subparadigm ||
        SUBPARADIGM_PART_PAST_ACT == eo_Subparadigm)
    {
        str_select += L"4";
    }
    else if (SUBPARADIGM_PART_PRES_PASS_LONG == eo_Subparadigm ||
             SUBPARADIGM_PART_PAST_PASS_LONG == eo_Subparadigm)
    {
        str_select += L"1";
    }
    else
    {
        str_select += str_ToString (pco_Lexeme->i_Type);
    }
    str_select += L";";

    try
    {
        pco_Db->v_PrepareForSelect (str_select);

        while (pco_Db->b_GetRow())
        {
            wstring str_ending;
            ST_EndingDescriptor st_d;
            pco_Db->v_GetData (0, str_ending);
            pco_Db->v_GetData (1, (int&)st_d.eo_Gender);
            pco_Db->v_GetData (2, (int&)st_d.eo_Number);
            pco_Db->v_GetData (3, (int&)st_d.eo_Case);
            pco_Db->v_GetData (4, (int&)st_d.eo_Animacy);
            pco_Db->v_GetData (5, (int&)st_d.eo_Stress);
            HRESULT h_r = pco_Endings->h_AddEnding (str_ending, st_d);
        }
    }
    catch (...)
    {
        v_ReportDbError();    
        throw CT_Exception (E_FAIL, L"Error getting endings.");
    }
    
    return S_OK;
        
}   //  b_GetEndings()

HRESULT CT_FormBuilderLongAdj::h_GetStressPositions (const wstring& str_ending,
                                                     ET_StressLocation eo_stressType,
                                                     vector<int>& vec_iStressPos)
{
    HRESULT h_r = S_OK;

    CT_ExtString xstr_lemma (str_Lemma);
    xstr_lemma.v_SetVowels (str_Vowels);

    int i_stressPos = -1;

    if (STRESS_LOCATION_STEM == eo_stressType)
    {
        h_GetStemStressPositions (str_Lemma, eo_Subparadigm, vec_iStressPos);
    }
    else if (STRESS_LOCATION_ENDING == eo_stressType)
    {
        if (CT_ExtString (str_ending).i_NSyllables() < 1)
        {
            i_stressPos = xstr_lemma.i_NSyllables() - 1;
        }
        else
        {
            i_stressPos = xstr_lemma.i_NSyllables();
        }

//        h_GetEndingStressPosition (str_Lemma, str_ending, i_stressPos);
        vec_iStressPos.push_back (i_stressPos);
    }
    else
    {
        ATLASSERT (0);
        wstring str_msg (L"Illegal stress type.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_INVALIDARG, str_msg);
    }
    if (S_OK != h_r)
    {
        wstring str_msg (L"Error getting stress positions.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_INVALIDARG, str_msg);
    }

    return S_OK;

}   //  h_GetStressPositions (...)

HRESULT CT_FormBuilderLongAdj::h_CreateFormTemplate (ET_Gender eo_gender,
                                                     ET_Number eo_number, 
                                                     ET_Case eo_case, 
                                                     ET_Animacy eo_animacy,
                                                     const wstring& str_ending,
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

    pco_wordForm->eo_POS = pco_Lexeme->eo_PartOfSpeech;
    pco_wordForm->eo_Subparadigm = eo_Subparadigm;
    pco_wordForm->str_Lemma = str_Lemma;
    pco_wordForm->eo_Gender = eo_gender;
    pco_wordForm->eo_Case = eo_case;
    pco_wordForm->eo_Number = eo_number;
    pco_wordForm->eo_Animacy = eo_animacy;
    pco_wordForm->eo_Reflexive = pco_Lexeme->eo_Reflexive;
    pco_wordForm->str_WordForm = str_Lemma + str_ending;
    if (REFL_YES == pco_Lexeme->eo_Reflexive)
    {
        pco_wordForm->str_WordForm += L"ся";
    }

    v_AssignSecondaryStress (pco_wordForm);

    return S_OK;

}   //  h_CreateFormTemplate (...)

HRESULT CT_FormBuilderLongAdj::h_HandleCommonDeviations (CT_WordForm * pco_wordForm)
{
    HRESULT h_r = S_OK;

    if (b_HasCommonDeviation (4) && SUBPARADIGM_PART_PRES_ACT == eo_Subparadigm)
    {
        if (b_DeviationOptional (4))
        {
            CComObject<CT_WordForm> * pco_variant = NULL;
            h_CloneWordForm (pco_wordForm, pco_variant);
            pco_Lexeme->h_AddWordForm (pco_variant);    // store both versions
            pco_wordForm = pco_variant;
        }

        map<int, bool> map_correctedStress;
        map<int, bool>::iterator it_stressPos = pco_wordForm->map_Stress.begin();
        for (; it_stressPos != pco_wordForm->map_Stress.end(); ++it_stressPos)
        {
            if (!(*it_stressPos).second)
            {
                map_correctedStress[(*it_stressPos).first] = false;
                continue;
            }
            if ((*it_stressPos).first < 1)
            {
                ATLASSERT(0);
                wstring str_msg (L"Unexpected stress position in cd-4 participle.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_FAIL, str_msg);
            }
            map_correctedStress[(*it_stressPos).first-1] = true;
        }
        pco_wordForm->map_Stress = map_correctedStress;        

    }   //  if (b_HasCommonDeviation (4) ...

    if (b_HasCommonDeviation (6) && 
        SUBPARADIGM_PART_PAST_ACT == eo_Subparadigm &&
        3 == pco_Lexeme->i_Type &&
        pco_Lexeme->b_StemAugment)
    {
        wstring str_ending = pco_wordForm->str_WordForm.substr (pco_wordForm->str_Lemma.length());
        CT_ExtString xstr_lemma = pco_wordForm->str_Lemma;
        int i_length = -1;
        if (xstr_lemma.b_EndsWith (L"вш"))
        {
            i_length = xstr_lemma.length()-2;
        }
        else
        {
            i_length = xstr_lemma.length()-1;
        }

        xstr_lemma = xstr_lemma.substr (0, i_length);

        if (xstr_lemma.b_EndsWith (L"ну"))
        {
            return S_FALSE;
        }

        if (b_DeviationOptional (6))
        {
            CComObject<CT_WordForm> * pco_variant = NULL;
            h_CloneWordForm (pco_wordForm, pco_variant);
            pco_Lexeme->h_AddWordForm (pco_variant);    // store both versions
            pco_wordForm = pco_variant;
        }

        xstr_lemma += L"нувш";        
        pco_wordForm->str_WordForm = xstr_lemma + str_ending;

    }   //  if (b_HasCommonDeviation (6) ... )

    int i_cd = -1;
    if (b_HasCommonDeviation (7))
    {
        i_cd = 7;
    }
    else if (b_HasCommonDeviation (8))
    {
        i_cd = 8;
    }

    if (i_cd > 0 && SUBPARADIGM_PART_PAST_PASS_LONG == eo_Subparadigm)
    {
        CT_ExtString xstr_lemma (pco_wordForm->str_Lemma);
        int i_nSyll = xstr_lemma.i_NSyllables();
        if (i_nSyll < 1)        //  < 2??
        {
            ATLASSERT(0);
            wstring str_msg (L"No syllables in part pass past stem.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_FAIL, str_msg);
        }

        if (b_DeviationOptional (i_cd))
        {
            CComObject<CT_WordForm> * pco_variant = NULL;
            h_CloneWordForm (pco_wordForm, pco_variant);
            pco_Lexeme->h_AddWordForm (pco_variant);    // store both versions
            pco_wordForm = pco_variant;
        }

        map<int, bool> map_correctedStress;
        map<int, bool>::iterator it_stressPos = pco_wordForm->map_Stress.begin();
        int i_newStressPos = -1;
        for (; it_stressPos != pco_wordForm->map_Stress.end(); ++it_stressPos)
        {
            if (!(*it_stressPos).second)
            {
                map_correctedStress[(*it_stressPos).first] = false;
                continue;
            }
            if (8 == i_cd && (*it_stressPos).first < 1)
            {
                ATLASSERT(0);
                wstring str_msg (L"Unexpected stress position in cd-7 or cd-8 participle.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_FAIL, str_msg);
            }

            if (7 == i_cd)
            {
                i_newStressPos = (*it_stressPos).first + 1;
            }
            else
            {
                if (8 == i_cd)
                {
                    i_newStressPos = CT_ExtString (pco_wordForm->str_Lemma).i_NSyllables()-2;
                }
                else
                {
                    ATLASSERT(0);
                    wstring str_msg (L"Unexpected cd number.");
                    ERROR_LOG (str_msg);
                    throw CT_Exception (E_FAIL, str_msg);
                }
            }

            map_correctedStress[i_newStressPos] = true;

            // Remove yo at old stress pos
            int i_at = xstr_lemma.i_GetVowelPos ((*it_stressPos).first);
            if (L'ё' == pco_wordForm->str_WordForm[i_at])
            {
                pco_wordForm->str_Lemma[i_at] = pco_wordForm->str_WordForm[i_at] = L'е';
            }

            // E --> yo at new stress pos
            i_at = xstr_lemma.i_GetVowelPos (i_newStressPos);
            if (L'е' == pco_wordForm->str_WordForm[i_at])
            {
                pco_wordForm->str_Lemma[i_at] = pco_wordForm->str_WordForm[i_at] = L'ё';
            }
        }
        pco_wordForm->map_Stress = map_correctedStress;
    }

    return S_OK;

}   //  h_HandleCommonDeviations (...)

HRESULT CT_FormBuilderLongAdj::h_Build()
{
    HRESULT h_r = S_OK;

    try
    {
        h_GetEndings();

        for (ET_Gender eo_gender = GENDER_M; eo_gender <= GENDER_N; ++eo_gender)
            for (ET_Number eo_number = NUM_SG; eo_number <= NUM_PL; ++eo_number)
                for (ET_Case eo_case = CASE_NOM; eo_case < CASE_NUM; ++eo_case)
                    for (ET_Animacy eo_animacy = ANIM_YES; eo_animacy < ANIM_COUNT; ++eo_animacy)
                    {
                        ET_StressLocation eo_stressType = STRESS_LOCATION_UNDEFINED;
                        if (AT_A == eo_AccentType|| AT_A1 == eo_AccentType)
                        {
                            eo_stressType = STRESS_LOCATION_STEM;
                        }
                        else
                        {
                            if (AT_B == eo_AccentType)
                            {
                                eo_stressType = STRESS_LOCATION_ENDING;
                            }
                            else
                            {
                                ERROR_LOG (L"Unknown stress type.");
                                continue;
                            }
                        }

                        ST_EndingDescriptor st_d (eo_gender, eo_number, eo_case, eo_animacy, eo_stressType);
                        int i_numEndings = pco_Endings->i_Count (st_d);
                        if (i_numEndings < 1)
                        {
                            ERROR_LOG (L"No endings");
                            continue;
                        }

                        for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)
                        {
                            std::wstring str_ending;
                            pco_Endings->h_GetEnding (st_d, i_ending, str_ending);

                            if (L"мс" == pco_Lexeme->str_InflectionType && pco_Lexeme->b_FleetingVowel)
                            {
                                h_FleetingVowelCheck (eo_number, eo_case, eo_gender, eo_stressType,
                                                      SUBPARADIGM_LONG_ADJ, str_ending, str_Lemma);
                            }

                            CComObject<CT_WordForm> * pco_wordForm = NULL;
                            h_CreateFormTemplate (eo_gender, 
                                                  eo_number, 
                                                  eo_case, 
                                                  eo_animacy, 
                                                  str_ending, 
                                                  pco_wordForm);
//                            pco_wordForm->str_WordForm = str_Lemma + str_ending;

                            vector<int> vec_iStress;
                            h_GetStressPositions (str_ending, eo_stressType, vec_iStress);
                            if (1 == vec_iStress.size() || b_MultiStress (str_Lemma, vec_iStress))
                            {
                                vector<int>::iterator it_stressPos = vec_iStress.begin();
                                for (; it_stressPos != vec_iStress.end(); ++it_stressPos)
                                {
                                    pco_wordForm->map_Stress[*it_stressPos] = true;  // primary
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
                                    pco_wordForm->map_Stress[*it_stressPos] = true;  // primary
                                    pco_Lexeme->h_AddWordForm (pco_wordForm);
                                }
                            }

                        }   //  for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)
                    }   //  for (ET_Animacy ...)
    }
    catch (CT_Exception co_ex)
    {
        return co_ex.i_GetErrorCode();  // logging should be always done by callee
    }

    return S_OK;

}   //  h_Build (...)

HRESULT CT_FormBuilderLongAdj::h_BuildParticiple()
{
    HRESULT h_r = S_OK;

    if (-1 == i_StemStressPos)
    {
        ATLASSERT(0);
        wstring str_msg (L"No stress position for a participle.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }

    try
    {
        h_GetEndings();

        for (ET_Gender eo_gender = GENDER_M; eo_gender <= GENDER_N; ++eo_gender)
            for (ET_Number eo_number = NUM_SG; eo_number <= NUM_PL; ++eo_number)
                for (ET_Case eo_case = CASE_NOM; eo_case < CASE_NUM; ++eo_case)
                    for (ET_Animacy eo_animacy = ANIM_YES; eo_animacy < ANIM_COUNT; ++eo_animacy)
                    {
                        ST_EndingDescriptor st_d (eo_gender, eo_number, eo_case, eo_animacy, STRESS_LOCATION_STEM);
                        int i_numEndings = pco_Endings->i_Count (st_d);
                        if (i_numEndings < 1)
                        {
                            ERROR_LOG (L"No endings");
                            continue;
                        }

                        for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)
                        {
                            std::wstring str_ending;
                            pco_Endings->h_GetEnding (st_d, i_ending, str_ending);

                            CT_WordForm * pco_stressTemplate = NULL;
                            CComObject<CT_WordForm> * pco_wordForm = NULL;
                            h_CreateFormTemplate (eo_gender, 
                                                  eo_number, 
                                                  eo_case, 
                                                  eo_animacy, 
                                                  str_ending, 
                                                  pco_wordForm);
                            pco_wordForm->map_Stress[i_StemStressPos] = true;
                            h_HandleYoAlternation (STRESS_LOCATION_STEM, i_StemStressPos, str_Lemma);
                            h_HandleCommonDeviations (pco_wordForm);
                            pco_Lexeme->h_AddWordForm (pco_wordForm);

                        }   //  for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)

                    }   //  for (ET_Animacy ...)
    }
    catch (CT_Exception co_ex)
    {
        return co_ex.i_GetErrorCode();  // logging should be always done by callee
    }

    return S_OK;

}   //  h_BuildParticiple (...)
