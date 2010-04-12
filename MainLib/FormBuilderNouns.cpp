#include "StdAfx.h"
#include "Endings.h"
#include "FormBuilderNouns.h"

HRESULT CT_FormBuilderNouns::h_GetEndings()
{
    HRESULT h_r = S_OK;

    if (NULL == pco_Lexeme)
    {
        ATLASSERT(0);
        wstring str_msg (L"Null pointer to lexeme object.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_POINTER, str_msg);
    }

    if (NULL == pco_Endings)
    {
        ATLASSERT(0);
        wstring str_msg (L"Null pointer to endings object.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_POINTER, str_msg);
    }

    wstring str_select (L"SELECT DISTINCT ending, number, case_value, stress FROM endings");
    str_select += L" WHERE inflection_class = ";
    str_select += str_ToString (ENDING_CLASS_NOUN);
    str_select += L" AND inflection_type = ";
    str_select += str_ToString (pco_Lexeme->i_Type);
    str_select += L" AND (gender = ";
    str_select += str_ToString (pco_Lexeme->eo_GetGender());
    str_select += L" OR gender = ";
    str_select += str_ToString (GENDER_UNDEFINED) + L")";
    str_select += L" AND (animacy = ";
    str_select += str_ToString (pco_Lexeme->eo_GetAnimacy());
    str_select += L" OR animacy = ";
    str_select += str_ToString (ANIM_UNDEFINED) + L")";
    str_select += L" AND stem_augment = ";
    str_select += pco_Lexeme->b_StemAugment ? L"1" : L"0";

    try
    {
        wstring str_selectBase (str_select);
        str_selectBase += L" AND common_deviation = 0;";

        pco_Db->v_PrepareForSelect (str_selectBase);

        while (pco_Db->b_GetRow())
        {
            wstring str_ending;
            ST_EndingDescriptor st_d;
            pco_Db->v_GetData (0, str_ending);
            pco_Db->v_GetData (1, (int&)st_d.eo_Number);
            pco_Db->v_GetData (2, (int&)st_d.eo_Case);
            pco_Db->v_GetData (3, (int&)st_d.eo_Stress);

            map<int, bool>::iterator it_cd = pco_Lexeme->map_CommonDeviations.find (1);
            if (pco_Lexeme->map_CommonDeviations.end() != it_cd)
            {
                if (NUM_PL == st_d.eo_Number && CASE_NOM == st_d.eo_Case && GENDER_F != st_d.eo_Gender)
                {
                    if (!pco_Lexeme->map_CommonDeviations[1]) // is optional?
                    {
                        continue;
                    }
                }
            }

            it_cd = pco_Lexeme->map_CommonDeviations.find (2);
            if (pco_Lexeme->map_CommonDeviations.end() != it_cd)
            {
                if (NUM_PL == st_d.eo_Number && CASE_GEN == st_d.eo_Case)
                {
                    if (!pco_Lexeme->map_CommonDeviations[1]) // is optional?
                    {
                        continue;
                    }
                }
            }
            pco_Endings->h_AddEnding (str_ending, st_d);
        }
    }
    catch (...)
    {
        v_ReportDbError();    
        throw CT_Exception (E_FAIL, L"DB error");
    }

    for (int i_cd = 1; i_cd <= 2; ++i_cd)
    {
        map<int, bool>::iterator it_cd = pco_Lexeme->map_CommonDeviations.find (i_cd);
        if (pco_Lexeme->map_CommonDeviations.end() == it_cd)
        {
            continue;
        }

        wstring str_selectCD (str_select);
        str_selectCD += L" AND common_deviation = ";
        str_selectCD += str_ToString (i_cd);
        str_select += L";";

        try
        {
            pco_Db->v_PrepareForSelect (str_selectCD);

            while (pco_Db->b_GetRow())
            {
                wstring str_ending;
                ST_EndingDescriptor st_d;
                pco_Db->v_GetData (0, str_ending);
                pco_Db->v_GetData (1, (int&)st_d.eo_Number);
                pco_Db->v_GetData (2, (int&)st_d.eo_Case);
                pco_Db->v_GetData (3, (int&)st_d.eo_Stress);
                pco_Endings->h_AddEnding (str_ending, st_d);
            }
        }
        catch (...)
        {
            v_ReportDbError();        
            throw CT_Exception (E_FAIL, L"DB error");
        }

    }   // for ...

    return S_OK;

}   //  h_GetEndings()

HRESULT CT_FormBuilderNouns::h_HandleStemAugment (wstring& str_lemma)
{
    if (!pco_Lexeme->b_StemAugment)
    {
        return S_OK;
    }

    if (1 == pco_Lexeme->i_Type)
    {
        str_lemma.erase (str_lemma.length()-2, 2);  // римлянин, южанин, армянин
    }
    if (3 == pco_Lexeme->i_Type)
    {
        CT_ExtString xstr_gs (pco_Lexeme->str_GraphicStem);
        if (xstr_gs.b_EndsWith (L"ёнок"))
        {
            str_lemma.erase (str_lemma.length()-4, 4);  // цыплёнок, опёнок
        }
        if (xstr_gs.b_EndsWith (L"онок"))
        {
            str_lemma.erase (str_lemma.length()-4, 4);  // мышонок
        }
        if (xstr_gs.b_EndsWith (L"ёночек"))
        {
            str_lemma.erase (str_lemma.length()-6, 6);  // цыплёночек
        }
        if (xstr_gs.b_EndsWith (L"оночек"))
        {
            str_lemma.erase (str_lemma.length()-6, 6);  // мышоночек
        }
    }

    return S_OK;

}   //  h_HandleStemAugment (...)

HRESULT CT_FormBuilderNouns::h_GetStressType (ET_Number eo_number, ET_Case eo_case, ET_StressLocation& eo_stressType)
{
    if (NUM_UNDEFINED == eo_number)
    {
        ATLASSERT(0);
        wstring str_msg (L"Undefined number.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_INVALIDARG, str_msg);
    }

// &&&& why default??
    eo_stressType = STRESS_LOCATION_STEM;

    switch (pco_Lexeme->eo_AccentType1)
    {
        case AT_A:
        {
            eo_stressType = STRESS_LOCATION_STEM;
            break;
        }
        case AT_B:
        {
            eo_stressType = STRESS_LOCATION_ENDING;
            break;
        }
        case AT_B1:
        {
            if (NUM_SG == eo_number && CASE_INST == eo_case)
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            else
            {
                eo_stressType = STRESS_LOCATION_ENDING;
            }
            break;
        }
        case AT_C:
        {
            if (NUM_SG == eo_number)
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            else
            {
                eo_stressType = STRESS_LOCATION_ENDING;
            }
            break;
        }
        case AT_D:
        {
            if (NUM_SG == eo_number)
            {
                eo_stressType = STRESS_LOCATION_ENDING;
            }
            else
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            break;
        }
        case AT_D1:
        {
            if (GENDER_F != pco_Lexeme->eo_GetGender())
            {
                ATLASSERT(0);
                wstring str_msg (L"Non-feminine noun with D1 stress.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_FAIL, str_msg);
            }
            if (NUM_SG == eo_number && CASE_ACC == eo_case) // assume fem -a stems only?
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            else if (NUM_SG == eo_number)
            {
                eo_stressType = STRESS_LOCATION_ENDING;
            }
            else
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            break;
        }
        case AT_E:
        {
            if (NUM_SG == eo_number)
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            else if (CASE_NOM == eo_case)
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            else if (CASE_ACC == eo_case && ANIM_NO == pco_Lexeme->eo_GetAnimacy())
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            else
            {
                eo_stressType = STRESS_LOCATION_ENDING;
            }
            break;
        }

        case AT_F:
        {
            if (NUM_SG == eo_number)
            {
                eo_stressType = STRESS_LOCATION_ENDING;
            }
            else if (CASE_NOM == eo_case)
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            else if (CASE_ACC == eo_case && ANIM_NO == pco_Lexeme->eo_GetAnimacy())
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            else
            {
                eo_stressType = STRESS_LOCATION_ENDING;
            }
            break;
        }
        case AT_F1:
        {
            if (GENDER_F != pco_Lexeme->eo_GetGender())
            {
                ATLASSERT(0);
                wstring str_msg (L"Non-feminine noun with F1 stress.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_FAIL, str_msg);
            }
            if (NUM_PL == eo_number && CASE_NOM == eo_case)
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            else if (NUM_SG == eo_number && CASE_ACC == eo_case) // fem only?
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            else
            {
                eo_stressType = STRESS_LOCATION_ENDING;
            }
            break;
        }
        case AT_F2:
        {
            if (GENDER_F != pco_Lexeme->eo_GetGender() || 8 != pco_Lexeme->i_Type)
            {
                ATLASSERT(0);   // assume f -i stems only?
                wstring str_msg (L"Non-feminine/type 8 noun with F2 stress.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_FAIL, str_msg);
            }
            if (ANIM_YES == pco_Lexeme->eo_GetAnimacy())
            {
                ATLASSERT(0);                   // inanimate only?
                wstring str_msg (L"Animate noun with F2 stress.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_FAIL, str_msg);
            }
            if (NUM_PL == eo_number && (CASE_NOM == eo_case || CASE_ACC == eo_case))
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            else if (NUM_SG == eo_number && CASE_INST == eo_case)
            {
                eo_stressType = STRESS_LOCATION_STEM;
            }
            else
            {
                eo_stressType = STRESS_LOCATION_ENDING;
            }
            break;
        }
        default:
        {
            ATLASSERT(0);
            wstring str_msg (L"Illegal accent type.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_FAIL, str_msg);
        }
    }

    return S_OK;

}   // h_GetStressType()

HRESULT CT_FormBuilderNouns::h_HandleAccEnding (ET_Number eo_number, ET_Case& eo_case)
{
    if (NUM_SG == eo_number)
    {
        if (GENDER_M == pco_Lexeme->eo_GetGender())
        {
            ANIM_YES == pco_Lexeme->eo_GetAnimacy()
                ? eo_case = CASE_GEN
                : eo_case = CASE_NOM;
        }
        if (GENDER_N == pco_Lexeme->eo_GetGender())
        {
            eo_case = CASE_NOM;
        }
    }
    if (NUM_PL == eo_number)
    {
        ANIM_YES == pco_Lexeme->eo_GetAnimacy()
            ? eo_case = CASE_GEN
            : eo_case = CASE_NOM;
    }

    return S_OK;

}   //  h_HandleAccEnding

HRESULT CT_FormBuilderNouns::h_GetStressPositions (const wstring& str_lemma, 
                                                   const wstring& str_ending,
                                                   ET_StressLocation eo_stressType,
                                                   bool b_hasFleetingVowel,
                                                   vector<int>& vec_iStressPos)
{
    HRESULT h_r = S_OK;

    int i_stressPos = -1;
    if (STRESS_LOCATION_STEM == eo_stressType)
    {
        h_GetStemStressPositions (str_lemma, SUBPARADIGM_NOUN, b_hasFleetingVowel, vec_iStressPos);
    }
    else if (STRESS_LOCATION_ENDING == eo_stressType)
    {
        h_GetEndingStressPosition (str_lemma, str_ending, i_stressPos);
        vec_iStressPos.push_back (i_stressPos);
    }
    else
    {
        ATLASSERT(0);
        wstring str_msg (L"Bad stress type.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }

    if (vec_iStressPos.empty())
    {
        vec_iStressPos.push_back (-1);
    }

    return S_OK;

}   //  h_GetStressPositions (...)

HRESULT CT_FormBuilderNouns::h_CreateFormTemplate (ET_Number eo_number, 
                                                   ET_Case eo_case, 
                                                   const wstring& str_lemma,
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
    pco_wordForm->eo_Case = eo_case;
    pco_wordForm->str_Lemma = str_lemma;
    pco_wordForm->eo_Number = eo_number;
    pco_wordForm->eo_Animacy = pco_Lexeme->eo_GetAnimacy();
    v_AssignSecondaryStress (pco_wordForm);

    return h_r;

}   //  h_CreateFormTemplate (...)

HRESULT CT_FormBuilderNouns::h_Build()
{
    HRESULT h_r = S_OK;

    try
    {
        h_r = h_GetEndings();

        for (ET_Number eo_number = NUM_SG; eo_number <= NUM_PL; ++eo_number)
        {
            for (ET_Case eo_case = CASE_NOM; eo_case < CASE_NUM; ++eo_case)
            {
                wstring str_lemma (pco_Lexeme->str_GraphicStem);

                h_r = h_HandleStemAugment (str_lemma);

                ET_StressLocation eo_stress = STRESS_LOCATION_UNDEFINED;
                h_GetStressType (eo_number, eo_case, eo_stress);

                // Handle acc ending
                ET_Case eo_endingCase = eo_case;
                if (CASE_ACC == eo_case)
                {
                    h_HandleAccEnding (eo_number, eo_endingCase);
                }

                int i_numEndings = pco_Endings->i_Count (ST_EndingDescriptor (eo_number, 
                                                                              eo_endingCase, 
                                                                              eo_stress));
                if (i_numEndings < 1)
                {
    //                ATLASSERT(0);
                    ERROR_LOG (L"No endings");
                    continue;
                }

                for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)
                {
                    // Get ending and modify as necessary
                    std::wstring str_ending;
                    h_r = pco_Endings->h_GetEnding (ST_EndingDescriptor (eo_number, eo_endingCase, eo_stress),
                                                    i_ending,
                                                    str_ending);
                    if (8 == pco_Lexeme->i_Type)
                    {
                        if (wstring::npos == wstring (str_ShSounds + L'ц').find (str_lemma[str_lemma.length()-1]))
                        {
                            if (CT_ExtString (str_ending).b_StartsWith (L"а"))
                            {
                                continue;
                            }
                        }
                        else
                        {
                            if (CT_ExtString (str_ending).b_StartsWith (L"я"))
                            {
                                continue;
                            }
                        }
                    }

                    bool b_hasFleetingVowel = false;
                    h_r = h_FleetingVowelCheck (eo_number, 
                                                eo_endingCase,
                                                pco_Lexeme->eo_GetGender(), 
                                                eo_stress,
                                                SUBPARADIGM_NOUN,
                                                str_ending,
                                                str_lemma,
                                                b_hasFleetingVowel);

                    vector<int> vec_iStress;
                    h_GetStressPositions (str_lemma, str_ending, eo_stress, b_hasFleetingVowel, vec_iStress);
                    CComObject<CT_WordForm> * pco_wordForm = NULL;
                    h_CreateFormTemplate (eo_number, eo_case, str_lemma, pco_wordForm);
                    if (1 == vec_iStress.size() || b_MultiStress (str_lemma, vec_iStress))
                    {
                        vector<int>::iterator it_stressPos = vec_iStress.begin();
                        for (; it_stressPos != vec_iStress.end(); ++it_stressPos)
                        {
                            pco_wordForm->map_Stress[*it_stressPos] = true;  // primary
                            h_HandleYoAlternation (eo_stress, *it_stressPos, SUBPARADIGM_NOUN, pco_wordForm->str_Lemma);
                            pco_wordForm->str_WordForm = pco_wordForm->str_Lemma + str_ending;
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
                            h_HandleYoAlternation (eo_stress, *it_stressPos, SUBPARADIGM_NOUN, pco_wordForm->str_Lemma);
                            pco_wordForm->str_WordForm = pco_wordForm->str_Lemma + str_ending;
                            pco_Lexeme->h_AddWordForm (pco_wordForm);
                        }
                    }

                }   //  for (int i_ending = 0; ... )

            }   //  for (ET_Case eo_case = CASE_NOM; ... )

        }   //  for (ET_Number eo_number = NUM_SG; ... )
    }
    catch (CT_Exception co_ex)
    {
        return co_ex.i_GetErrorCode();  // logging should be always done by callee
    }

    return S_OK;

}    //  h_BuildNounForms()
