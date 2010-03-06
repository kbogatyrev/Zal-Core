#include "StdAfx.h"
#include "Endings.h"
#include "FormBuilderAdjShort.h"

HRESULT CT_FormBuilderShortAdj::h_GetEndings()
{
    HRESULT h_r = S_OK;

    if (POS_ADJ != pco_Lexeme->eo_PartOfSpeech)
    {
        return E_INVALIDARG;
    }

    wstring str_select (L"SELECT DISTINCT ending, gender, number, stress FROM endings");
    str_select += L" WHERE inflection_class = ";
    str_select += str_ToString (ENDING_CLASS_SHORT_ADJECTIVE);
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
            pco_Db->v_GetData (3, (int&)st_d.eo_Stress);
            HRESULT h_r = pco_Endings->h_AddEnding (str_ending, st_d);
        }
    }
    catch (...)
    {
        v_ReportDbError();    
        return E_FAIL;
    }
    
    return S_OK;
        
}   //  h_GetShortFormEndings()

HRESULT CT_FormBuilderShortAdj::h_GetStressTypes (ET_Number eo_number, 
                                                  ET_Gender eo_gender, 
                                                  vector<ET_StressLocation>& vec_eoStressType)
{
    HRESULT h_r = S_OK;

    if (POS_ADJ != pco_Lexeme->eo_PartOfSpeech)
    {
        ATLASSERT(0);
        ERROR_LOG (L"Unexpected part of speech value.");
        return E_FAIL;
    }

    if (NUM_PL == eo_number && GENDER_UNDEFINED != eo_gender)
    {
        ATLASSERT(0);
        ERROR_LOG (L"Unexpected gender/number values.");
        return E_INVALIDARG;
    }

    if (NUM_SG == eo_number && GENDER_UNDEFINED == eo_gender)
    {
        ATLASSERT(0);
        ERROR_LOG (L"Unexpected gender/number values.");
        return E_INVALIDARG;
    }

    if (GENDER_M == eo_gender)
    {
        ATLASSERT (NUM_SG == eo_number);
        vec_eoStressType.push_back (STRESS_LOCATION_STEM);
        return S_OK;
    }

    ET_AccentType eo_at = AT_UNDEFINED;
    if (AT_UNDEFINED == pco_Lexeme->eo_AccentType2)
    {
        eo_at = pco_Lexeme->eo_AccentType1;
    }
    else
    {
        eo_at = pco_Lexeme->eo_AccentType2;
    }

    switch (eo_at)
    {
        case AT_UNDEFINED:
        {
            ATLASSERT(0);
            ERROR_LOG (L"Undefined accent type.");
            return E_FAIL;
        }
        case AT_A:
        {
            vec_eoStressType.push_back (STRESS_LOCATION_STEM);
            return S_OK;
        }
        case AT_A1:
        {
            if (GENDER_F == eo_gender)
            {
                ATLASSERT (NUM_SG == eo_number);
                vec_eoStressType.push_back (STRESS_LOCATION_STEM);
                vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            if (GENDER_N == eo_gender)
            {
                ATLASSERT (NUM_SG == eo_number);
                vec_eoStressType.push_back (STRESS_LOCATION_STEM);
                return S_OK;
            }
            if (NUM_PL == eo_number)
            {
                ATLASSERT (NUM_SG == eo_number);
                vec_eoStressType.push_back (STRESS_LOCATION_STEM);
                return S_OK;
            }
            ATLASSERT(0);
            ERROR_LOG (L"Bad arguments.");
            return E_INVALIDARG;
        }
        case AT_B:
        {
            return S_TRUE;
        }
        case AT_B1:
        {
            if (GENDER_F == eo_gender || GENDER_N == eo_gender)
            {
                ATLASSERT (NUM_SG == eo_number);
                vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            if (NUM_PL == eo_number)
            {
                ATLASSERT (GENDER_UNDEFINED == eo_gender);
                vec_eoStressType.push_back (STRESS_LOCATION_STEM);
                vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }

            ATLASSERT(0);
            ERROR_LOG (L"Bad arguments.");
            return E_INVALIDARG;
        }
        case AT_C:
        {
            if (GENDER_F == eo_gender)
            {
                ATLASSERT (NUM_SG == eo_number);
                vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            if (GENDER_N == eo_gender || NUM_PL == eo_number)
            {
                vec_eoStressType.push_back (STRESS_LOCATION_STEM);
                return S_OK;
            }

            ATLASSERT (0);
            ERROR_LOG (L"Bad arguments.");
            return E_INVALIDARG;
        }
        case AT_C1:
        {
            if (GENDER_F == eo_gender)
            {
                ATLASSERT (NUM_SG == eo_number);
                vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            if (GENDER_N == eo_gender)
            {
                ATLASSERT (NUM_SG == eo_number);
                vec_eoStressType.push_back (STRESS_LOCATION_STEM);
                return S_OK;
            }
            if (NUM_PL == eo_number)
            {
                ATLASSERT (GENDER_UNDEFINED == eo_gender);
                vec_eoStressType.push_back (STRESS_LOCATION_STEM);
                vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            ATLASSERT (0);
            ERROR_LOG (L"Bad arguments.");
            return E_INVALIDARG;
        }
        case AT_C2:
        {
            if (GENDER_F == eo_gender)
            {
                ATLASSERT (NUM_SG == eo_number);
                vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            if (GENDER_N == eo_gender)
            {
                ATLASSERT (NUM_SG == eo_number);
                vec_eoStressType.push_back (STRESS_LOCATION_STEM);
                vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            if (NUM_PL == eo_number)
            {
                ATLASSERT (GENDER_UNDEFINED == eo_gender);
                vec_eoStressType.push_back (STRESS_LOCATION_STEM);
                vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
                return S_OK;
            }
            ATLASSERT (0);
            ERROR_LOG (L"Bad arguments.");
            return E_INVALIDARG;
        }
        default:
        {
            ERROR_LOG (L"Illegal accent type.");
            return E_FAIL;
        }
    }

    return E_INVALIDARG;

}   // h_GetStressType()

HRESULT CT_FormBuilderShortAdj::h_CreateWordForm (ET_Number eo_number, 
                                                  ET_Gender eo_gender, 
                                                  ET_StressLocation eo_stressType,
                                                  const wstring& str_ending,
                                                  wstring& str_lemma)
{
    HRESULT h_r = S_OK;

    if (pco_Lexeme->b_FleetingVowel)
    {
        h_r = h_HandleFleetingVowel (eo_number, 
                                     CASE_UNDEFINED, 
                                     eo_gender,
                                     eo_stressType,
                                     SUBPARADIGM_SHORT_ADJ,
                                     str_ending,
                                     str_lemma);
    }

    vector<int> vec_iStressPos;
    if (STRESS_LOCATION_STEM == eo_stressType)
    {
        h_r = h_GetStemStressPositions (str_lemma, SUBPARADIGM_SHORT_ADJ, vec_iStressPos);
        if (S_OK != h_r)
        {
            ERROR_LOG (L"Error getting stress position.");
            return h_r;
        }
    }
    else if (STRESS_LOCATION_ENDING == eo_stressType)
    {
        int i_stressPos = -1;
        h_r = h_GetEndingStressPosition (str_lemma, str_ending, i_stressPos);
        if (S_OK != h_r)
        {
            ERROR_LOG (L"Error getting stress position.");
            return h_r;
        }
        vec_iStressPos.push_back (i_stressPos);
    }
    else
    {
        ATLASSERT (0);
        ERROR_LOG (L"Illegal stress type.");
        h_r = E_INVALIDARG;
    }

    vector<int>::iterator it_stressPos = vec_iStressPos.begin();
    for (; it_stressPos != vec_iStressPos.end(); ++it_stressPos)
    {
        if (pco_Lexeme->b_YoAlternation)
        {
            h_r = h_HandleYoAlternation (eo_stressType, *it_stressPos, SUBPARADIGM_SHORT_ADJ, str_lemma);
            if (S_OK != h_r)
            {
                return h_r;
            }
        }
                
        CComObject<CT_WordForm> * pco_wordForm;
        h_r = CComObject<CT_WordForm>::CreateInstance (&pco_wordForm);
        if (S_OK != h_r)
        {
            return h_r;
        }

        pco_wordForm->eo_POS = pco_Lexeme->eo_PartOfSpeech;
        pco_wordForm->eo_Subparadigm = SUBPARADIGM_SHORT_ADJ;
        pco_wordForm->str_Lemma = str_lemma;
        pco_wordForm->str_WordForm = str_lemma + str_ending;
        pco_wordForm->eo_Gender = eo_gender;
        pco_wordForm->eo_Number = eo_number;
        pco_wordForm->vec_Stress = vec_iStressPos;

        long l_gramHash = 0;
        pco_wordForm->get_GramHash (&l_gramHash);
        pco_Lexeme->v_AddWordForm (l_gramHash, pco_wordForm);
    }

    return S_OK;

}   //  h_CreateWordForm (...)

HRESULT CT_FormBuilderShortAdj::h_Build()
{
    HRESULT h_r = S_OK;

    h_r = h_GetEndings();
    if (S_OK != h_r)
    {
        return h_r;
    }

    for (ET_Gender eo_gender = GENDER_UNDEFINED; eo_gender <= GENDER_N; ++eo_gender)
    {
        ET_Number eo_number = (GENDER_UNDEFINED == eo_gender) ? NUM_PL : NUM_SG;

        vector<ET_StressLocation> vec_eoStressType;
        h_r = h_GetStressTypes (eo_number, eo_gender, vec_eoStressType);
        if (S_OK != h_r)
        {
            return h_r;
        }

        wstring str_lemma (pco_Lexeme->str_GraphicStem);

        vector<ET_StressLocation>::iterator it_stressType = vec_eoStressType.begin();
        for (; it_stressType != vec_eoStressType.end(); ++it_stressType)
        {
            ST_EndingDescriptor st_d (eo_gender, eo_number, *it_stressType);
            int i_numEndings = pco_Endings->i_Count (st_d);
            if (i_numEndings < 1)
            {
                ERROR_LOG (L"No endings");
                continue;
            }

            for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)
            {
                str_lemma = pco_Lexeme->str_GraphicStem;
                std::wstring str_ending;
                h_r = pco_Endings->h_GetEnding (st_d, i_ending, str_ending);
                if (S_OK != h_r)
                {
                    ERROR_LOG (L"Error getting ending from hash.");
                    continue;
                }

                h_r = h_CreateWordForm (eo_number, eo_gender, *it_stressType, str_ending, str_lemma);

            }   //  for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)

        }   //  for (; it_stress != vec_eoStress.end(); ++it_stress)

    }   //  for (ET_Gender eo_gender = GENDER_UNDEFINED; ...

    return S_OK;

}   //  h_Build()
