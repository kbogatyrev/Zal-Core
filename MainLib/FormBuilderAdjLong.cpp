#include "StdAfx.h"
#include "Endings.h"
#include "FormBuilderAdjLong.h"

HRESULT CT_FormBuilderLongAdj::h_GetEndings()
{
    HRESULT h_r = S_OK;

    int i_type = -1;

    wstring str_select 
        (L"SELECT DISTINCT ending, gender, number, case_value, animacy, stress FROM endings");
    str_select += L" WHERE inflection_class = ";
    if (L"п" == pco_Lexeme->str_InflectionType)
    {
//        str_select += L"2";
        str_select += str_ToString (ENDING_CLASS_ADJECTIVE);
    }
    else
    {
        if (L"мс" == pco_Lexeme->str_InflectionType)
        {
            str_select += L"3";
        }
        else
        {
            ATLASSERT(0);
            return E_INVALIDARG;
        }
    }
    str_select += L" AND inflection_type = ";
    str_select += str_ToString (pco_Lexeme->i_Type);
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
        v_ReporDbError();    
        return E_FAIL;
    }
    
    return S_OK;
        
}   //  b_GetEndings()

HRESULT CT_FormBuilderLongAdj::h_Build()
{
    HRESULT h_r = S_OK;

    h_r = h_GetEndings();
    if (S_OK != h_r)
    {
        return h_r;
    }

    for (ET_Gender eo_gender = GENDER_M; eo_gender <= GENDER_N; ++eo_gender)
        for (ET_Number eo_number = NUM_SG; eo_number <= NUM_PL; ++eo_number)
            for (ET_Case eo_case = CASE_NOM; eo_case < CASE_NUM; ++eo_case)
                for (ET_Animacy eo_animacy = ANIM_YES; eo_animacy < ANIM_COUNT; ++eo_animacy)
                {
                    ET_EndingStressType eo_stressType = ENDING_STRESS_UNDEFINED;
                    if (AT_A == eo_AccentType|| AT_A1 == eo_AccentType)
                    {
                        eo_stressType = ENDING_STRESS_UNSTRESSED;
                    }
                    else
                    {
                        if (AT_B == eo_AccentType)
                        {
                            eo_stressType = ENDING_STRESS_STRESSED;
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
                        h_r = pco_Endings->h_GetEnding (st_d, i_ending, str_ending);
                        if (S_OK != h_r)
                        {
                            ERROR_LOG (L"Error getting ending from hash.");
                            continue;
                        }

                        if (L"мс" == pco_Lexeme->str_InflectionType && pco_Lexeme->b_FleetingVowel)
                        {
                            h_r = h_HandleDeclFleetingVowel (eo_number, 
                                                             eo_case,
                                                             eo_gender,
                                                             eo_stressType,
                                                             SUBPARADIGM_LONG_ADJ,
                                                             str_ending,
                                                             str_Lemma);
                        }

                        vector<int> vec_iStressPos;
                        if (ENDING_STRESS_UNSTRESSED == eo_stressType)
                        {
                            h_r = h_GetDeclStemStressPosition (str_Lemma, vec_iStressPos, eo_Subparadigm);
                        }
                        else if (ENDING_STRESS_STRESSED == eo_stressType)
                        {
                            int i_stressPos = -1;
                            h_r = h_GetDeclEndingStressPosition (str_Lemma, str_ending, i_stressPos);
                            vec_iStressPos.push_back (i_stressPos);
                        }
                        else
                        {
                            ATLASSERT (0);
                            ERROR_LOG (L"Illegal stress type.");
                            return E_INVALIDARG;
                        }
                        if (S_OK != h_r)
                        {
                            ERROR_LOG (L"Error getting stress position.");
                        }

                        CComObject<CT_WordForm> * pco_wordForm;
                        h_r = CComObject<CT_WordForm>::CreateInstance (&pco_wordForm);
                        if (S_OK != h_r)
                        {
                            return h_r;
                        }

                        pco_wordForm->eo_POS = pco_Lexeme->eo_PartOfSpeech;
                        pco_wordForm->eo_Subparadigm = SUBPARADIGM_LONG_ADJ;
                        pco_wordForm->str_Lemma = str_Lemma;
                        pco_wordForm->str_WordForm = str_Lemma + str_ending;
                        pco_wordForm->eo_Gender = eo_gender;
                        pco_wordForm->eo_Case = eo_case;
                        pco_wordForm->eo_Number = eo_number;
                        pco_wordForm->eo_Animacy = eo_animacy;
                        pco_wordForm->vec_Stress = vec_iStressPos;

                        pco_Lexeme->m_coll.push_back (pco_wordForm);

                    }   //  for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)
                }   //  for (ET_Animacy ...)

    return S_OK;

}   //  h_Build (...)
