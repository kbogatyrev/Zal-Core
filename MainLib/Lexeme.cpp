#include "StdAfx.h"
#include "Lexeme.h"

//
HRESULT CT_Lexeme::GenerateWordForms()
{
    HRESULT h_r = h_GenerateParadigm();
    return h_r;
}

HRESULT CT_Lexeme::get_LexemeId (LONG * pl_id)
{
    return E_NOTIMPL;
}

HRESULT CT_Lexeme::get_GraphicStem (BSTR * pbstr_lemma)
{
    if (str_GraphicStem.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_GraphicStem.c_str());
    *pbstr_lemma = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_Lexeme::get_InitialForm (BSTR * pbstr_sourceForm)
{
    if (str_SourceForm.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_SourceForm.c_str());
    *pbstr_sourceForm = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_Lexeme::get_IsVariant (BOOL * pb_variant)
{
    *pb_variant = b_IsVariant ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_MainSymbol (BSTR * pbstr_mainSymbol)
{
    if (str_MainSymbol.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_MainSymbol.c_str());
    *pbstr_mainSymbol = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_Lexeme::get_IsPluralOf (BOOL * pui_isPluralOf)
{
    *pui_isPluralOf = b_IsPluralOf ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_IsTransitive (BOOL * pui_isTransitive)
{
    *pui_isTransitive = b_Transitive;
    return S_OK;
}

HRESULT CT_Lexeme::get_MainSymbolPluralOf (BSTR * pbstr_msPluralOf)
{
    if (str_MainSymbolPluralOf.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_MainSymbolPluralOf.c_str());
    *pbstr_msPluralOf = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_Lexeme::get_AltMainSymbol (BSTR * pbstr_altMainSymbol)
{
    if (str_AltMainSymbol.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_AltMainSymbol.c_str());
    *pbstr_altMainSymbol = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_Lexeme::get_InflectionType (BSTR * pbstr_inflectionType)
{
    if (str_InflectionType.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_InflectionType.c_str());
    *pbstr_inflectionType = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_Lexeme::get_PartOfSpeech (ET_PartOfSpeech * pe_pos)
{
    *pe_pos = eo_PartOfSpeech;
    return S_OK;
}

HRESULT CT_Lexeme::get_Comment (BSTR * pbstr_comment)
{
    if (str_Comment.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_Comment.c_str());
    *pbstr_comment = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_Lexeme::get_AltMainSymbolComment (BSTR * pbstr_altMsComment)
{
    if (str_AltMainSymbolComment.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_AltMainSymbolComment.c_str());
    *pbstr_altMsComment = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_Lexeme::get_AltInflectionComment (BSTR * pbstr_altInflectionComment)
{
    if (str_AltInflectionComment.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_AltInflectionComment.c_str());
    *pbstr_altInflectionComment = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_Lexeme::get_VerbStemAlternation (BSTR * bstr_verbStemAlternation)
{
    if (str_VerbStemAlternation.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_VerbStemAlternation.c_str());
    *bstr_verbStemAlternation = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_Lexeme::get_Section (LONG * pl_section)
{
    *pl_section = (long)i_Section;
    return S_OK;
}

HRESULT CT_Lexeme::get_NoComparative (BOOL * pui_noComparative)
{
    *pui_noComparative = b_NoComparative ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_HasAssumedForms (BOOL * pui_hasAssumedForms)
{
    *pui_hasAssumedForms = b_AssumedForms ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_HasYoAlternation (BOOL * pui_hasYoAlteration)
{
    *pui_hasYoAlteration = b_YoAlternation ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_HasOAlternation (BOOL * pui_hasOAlteration)
{
    *pui_hasOAlteration = b_OAlternation ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_HasSecondGenitive (BOOL * pui_hasSecondaryGenetive)
{
    *pui_hasSecondaryGenetive = b_SecondGenitive ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_QuestionableForms (BSTR *)
{
    return E_NOTIMPL;
}

HRESULT CT_Lexeme::get_IrregularForms (BSTR *)
{
    return E_NOTIMPL;
}

HRESULT CT_Lexeme::get_RestrictedForms (BSTR *)
{
    return E_NOTIMPL;
}

HRESULT CT_Lexeme::get_Contexts (BSTR *)
{
    return E_NOTIMPL;
}

HRESULT CT_Lexeme::get_TrailingComment (BSTR * pbstr_trailingComment)
{
    if (str_TrailingComment.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_TrailingComment.c_str());
    *pbstr_trailingComment = sp_bstr.Detach();

    return S_OK;
}


// From Inflection table:
HRESULT CT_Lexeme::get_IsPrimaryInflectionGroup (BOOL * pui_isPrimaryInflectionGroup)
{
    *pui_isPrimaryInflectionGroup = b_PrimaryInflectionGroup ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_Type (LONG * pl_type)
{
    *pl_type = i_Type;
    return S_OK;
}

HRESULT CT_Lexeme::get_AccentType1 (ET_AccentType * pe_accentType1)
{
    *pe_accentType1 = eo_AccentType1;
    return S_OK;
}

HRESULT CT_Lexeme::get_AccentType2 (ET_AccentType * pe_accentType2)
{
    *pe_accentType2 = eo_AccentType2;
    return S_OK;
}

HRESULT CT_Lexeme::get_ShortFormsAreRestricted (BOOL * pui_shortFormsAreRestricted)
{
    *pui_shortFormsAreRestricted = b_ShortFormsRestricted ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_PastParticipleIsRestricted (BOOL * pui_pastParticipleIsRestricted)
{
    *pui_pastParticipleIsRestricted = b_PastParticipleRestricted ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_NoShortForms (BOOL * pui_noShortForms)
{
    *pui_noShortForms = b_NoShortForms ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_NoPastParticiple (BOOL * pui_noPastParticiple)
{
    *pui_noPastParticiple = b_NoPastParticiple ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_HasFleetingVowel (BOOL * pui_hasFleetingVowel)
{
    *pui_hasFleetingVowel = b_FleetingVowel ? TRUE : FALSE;
    return S_OK;
}

HRESULT CT_Lexeme::get_HasStemAugment (BOOL * pui_hasStemAugment)
{
    *pui_hasStemAugment = b_StemAugment ? TRUE :FALSE;
    return S_OK;
}


//
//
//
/*
template <typename T>
wstring static str_ToString (T from)
{
    wstringstream io_;
    io_ << from;
    return io_.str();
};
*/

void CT_Lexeme::v_Init()
{
    i_DbKey = -1;
    i_SourceStressPos = -1;
    i_SecondarySourceStressPos = -1;
    b_IsVariant = false;
    b_IsPluralOf = false;
    b_Transitive = false;
    eo_PartOfSpeech = POS_UNDEFINED;
    i_Section = -1;
    b_NoComparative = false;
    b_AssumedForms = false;
    b_YoAlternation = false;
    b_SecondGenitive = false;
    b_PrimaryInflectionGroup = false;
    i_Type = -1;
    eo_AccentType1 = AT_UNDEFINED;
    eo_AccentType2 = AT_UNDEFINED;
    b_ShortFormsRestricted = false;
    b_PastParticipleRestricted = false;
    b_NoShortForms = false;
    b_NoPastParticiple = false;
    b_FleetingVowel = false;
    b_StemAugment = false;
    i_CommonDeviation = -1;

    wstring arr_strMainSymbol[] = { L"м", L"мо", L"ж", L"жо", L"с", L"со", L"мо-жо", L"мн.",
     L"мн. неод.", L"мн. одуш.", L"мн. _от_", L"п", L"мс", L"мс-п", L"числ.", L"числ.-п", 
     L"св", L"нсв", L"св-нсв", L"н", L"предл.", L"союз", L"предик.", L"вводн.", L"сравн.", 
     L"част.", L"межд." };

    for (ET_MainSymbol eo_ms = MS_START; eo_ms < MS_END; ++eo_ms)
    {
        map_MainSymbol[arr_strMainSymbol[eo_ms]] = eo_ms; 
    }

    for (int i_ms = 0; i_ms < (int)MS_END; ++i_ms)
    {
        wstring str_ms = arr_strMainSymbol[i_ms];
        ET_MainSymbol eo_ms = map_MainSymbol[str_ms];
        switch (eo_ms)
        {
            case MS_M:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_NO;
                map_MainSymbolToGender[str_ms] = GENDER_M;
                break;
            }
            case MS_MO:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_YES;
                map_MainSymbolToGender[str_ms] = GENDER_M;
                break;
            }
            case MS_ZH:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_NO;
                map_MainSymbolToGender[str_ms] = GENDER_F;
                break;
            }
            case MS_ZHO:
            case MS_MO_ZHO:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_YES;
                map_MainSymbolToGender[str_ms] = GENDER_F;
                break;
            }
            case MS_S:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_NO;
                map_MainSymbolToGender[str_ms] = GENDER_N;
            }
            case MS_SO:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_YES;
                map_MainSymbolToGender[str_ms] = GENDER_N;
                break;
            }
            case MS_MN:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_UNDEFINED;
                map_MainSymbolToGender[str_ms] = GENDER_UNDEFINED;
                break;
            }
            case MS_MN_NEOD:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_NO;
                map_MainSymbolToGender[str_ms] = GENDER_UNDEFINED;
                break;
            }
            case MS_MN_ODUSH:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_YES;
                map_MainSymbolToGender[str_ms] = GENDER_UNDEFINED;
                break;
            }
            case MS_MN_OT:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_NO;
                map_MainSymbolToGender[str_ms] = GENDER_UNDEFINED;
                break;
            }
            case MS_SV:
            case MS_NSV:
            case MS_SV_NSV:
            case MS_P:
            case MS_MS:
            case MS_MS_P:
            case MS_N:
            case MS_CHISL:
            case MS_CHISL_P:
            case MS_CHAST:
            case MS_PREDL:
            case MS_SOJUZ:
            case MS_VVODN:
            case MS_PREDIK:
            case MS_MEZHD:
            case MS_SRAVN:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_UNDEFINED;
                map_MainSymbolToGender[str_ms] = GENDER_UNDEFINED;
                break;
            }
            default:
            {
                ATLASSERT(0);
                ERROR_LOG (L"Unrecognized ending.");
                return;
            }

        }    // switch

    }   //  for (int i_ms = 0; ... )

}   // v_Init()

HRESULT CT_Lexeme::h_GetNounEndings()
{
    HRESULT h_r = S_OK;

    if (POS_NOUN != eo_PartOfSpeech)
    {
        return E_INVALIDARG;
    }

    ET_Gender eo_gender = map_MainSymbolToGender[str_InflectionType];
    ET_Animacy eo_animacy = map_MainSymbolToAnimacy[str_InflectionType];

    wstring str_select (L"SELECT DISTINCT ending, number, case_value, stress FROM endings");
    str_select += L" WHERE inflection_class = ";
    str_select += str_ToString (eo_PartOfSpeech);
    str_select += L" AND inflection_type = ";
    str_select += str_ToString (i_Type);
    str_select += L" AND (gender = ";
    str_select += str_ToString (eo_gender);
    str_select += L" OR gender = ";
    str_select += str_ToString (GENDER_UNDEFINED) + L")";
    str_select += L" AND (animacy = ";
    str_select += str_ToString (eo_animacy);
    str_select += L" OR animacy = ";
    str_select += str_ToString (ANIM_UNDEFINED) + L")";
    str_select += L" AND stem_augment = ";
    str_select += b_StemAugment ? L"1" : L"0";
    str_select += L" AND common_deviation = ";              // case "1" and "2" ??
    str_select += str_ToString (i_CommonDeviation);


    co_Endings.v_Reset();

    try
    {
        pco_Db->v_PrepareForSelect (str_select);

        while (pco_Db->b_GetRow())
        {
            vector<ST_Ending> vec_endings;
            wstring str_ending;
            ET_Gender eo_gender = GENDER_UNDEFINED;
            ET_Number eo_number = NUM_UNDEFINED;
            ET_Case eo_case = CASE_UNDEFINED;
            ET_EndingStressType eo_stress = ENDING_STRESS_UNDEFINED;

            pco_Db->v_GetData (0, str_ending);
            pco_Db->v_GetData (1, (int&)eo_number);
            pco_Db->v_GetData (2, (int&)eo_case);
            pco_Db->v_GetData (3, (int&)eo_stress);

            co_Endings.h_AddEnding (str_ending, eo_number, eo_case, eo_stress);
        }
    }
    catch (...)
    {
        wstring str_msg;
        try
        {
            wstring str_error;
            pco_Db->v_GetLastError (str_error);
            str_msg = L"DB error %d: ";
            str_msg += str_error;
        }
        catch (...)
        {
            str_msg = L"Apparent DB error ";
        }
    
        CString cs_msg;
        cs_msg.Format (str_msg.c_str(), pco_Db->i_GetLastError());
        ERROR_LOG ((LPCTSTR)cs_msg);
    
        return E_FAIL;
    }
    
    return S_OK;
        
}   //  b_GetNounEndings (...)

HRESULT CT_Lexeme::h_GetAdjEndings()
{
    HRESULT h_r = S_OK;

    if (POS_ADJ != eo_PartOfSpeech)
    {
        return E_INVALIDARG;
    }

    wstring str_select 
        (L"SELECT DISTINCT ending, gender, number, case_value, animacy, stress FROM endings");
    str_select += L" WHERE inflection_class = 2 AND inflection_type = ";
    str_select += str_ToString (i_Type);

    co_Endings.v_Reset();

    try
    {
        pco_Db->v_PrepareForSelect (str_select);

        while (pco_Db->b_GetRow())
        {
            vector<ST_Ending> vec_endings;
            wstring str_ending;
            ET_Gender eo_gender = GENDER_UNDEFINED;
            ET_Number eo_number = NUM_UNDEFINED;
            ET_Case eo_case = CASE_UNDEFINED;
            ET_Animacy eo_animacy = ANIM_UNDEFINED;
            ET_EndingStressType eo_stress = ENDING_STRESS_UNDEFINED;

            pco_Db->v_GetData (0, str_ending);
            pco_Db->v_GetData (1, (int&)eo_gender);
            pco_Db->v_GetData (2, (int&)eo_number);
            pco_Db->v_GetData (3, (int&)eo_case);
            pco_Db->v_GetData (4, (int&)eo_animacy);
            pco_Db->v_GetData (5, (int&)eo_stress);

            HRESULT h_r = co_Endings.h_AddEnding (str_ending, eo_gender, eo_number, eo_case, eo_animacy, eo_stress);
        }
    }
    catch (...)
    {
        wstring str_msg;
        try
        {
            wstring str_error;
            pco_Db->v_GetLastError (str_error);
            str_msg = L"DB error %d: ";
            str_msg += str_error;
        }
        catch (...)
        {
            str_msg = L"Apparent DB error ";
        }
    
        CString cs_msg;
        cs_msg.Format (str_msg.c_str(), pco_Db->i_GetLastError());
        ERROR_LOG ((LPCTSTR)cs_msg);
    
        return E_FAIL;
    }
    
    return S_OK;
        
}   //  b_GetAdjEndings (...)

HRESULT CT_Lexeme::h_GetShortFormEndings()
{
    HRESULT h_r = S_OK;

    if (POS_ADJ != eo_PartOfSpeech)
    {
        return E_INVALIDARG;
    }

    wstring str_select (L"SELECT DISTINCT ending, gender, number, stress FROM endings");
    str_select += L" WHERE inflection_class = 4 AND inflection_type = ";
    str_select += str_ToString (i_Type);

    co_Endings.v_Reset();

    try
    {
        pco_Db->v_PrepareForSelect (str_select);

        while (pco_Db->b_GetRow())
        {
            vector<ST_Ending> vec_endings;
            wstring str_ending;
            ET_Gender eo_gender = GENDER_UNDEFINED;
            ET_Number eo_number = NUM_UNDEFINED;
            ET_Case eo_case = CASE_UNDEFINED;
            ET_Animacy eo_animacy = ANIM_UNDEFINED;
            ET_EndingStressType eo_stress = ENDING_STRESS_UNDEFINED;

            pco_Db->v_GetData (0, str_ending);
            pco_Db->v_GetData (1, (int&)eo_gender);
            pco_Db->v_GetData (2, (int&)eo_number);
            pco_Db->v_GetData (3, (int&)eo_stress);

            HRESULT h_r = co_Endings.h_AddEnding (str_ending, eo_gender, eo_number, eo_stress);
        }
    }
    catch (...)
    {
        wstring str_msg;
        try
        {
            wstring str_error;
            pco_Db->v_GetLastError (str_error);
            str_msg = L"DB error %d: ";
            str_msg += str_error;
        }
        catch (...)
        {
            str_msg = L"Apparent DB error ";
        }
    
        CString cs_msg;
        cs_msg.Format (str_msg.c_str(), pco_Db->i_GetLastError());
        ERROR_LOG ((LPCTSTR)cs_msg);
    
        return E_FAIL;
    }
    
    return S_OK;
        
}   //  h_GetShortFormEndings (...)

HRESULT CT_Lexeme::h_GenerateParadigm()
{
    HRESULT h_r = S_OK;

    m_coll.clear();

    switch (eo_PartOfSpeech)
    {
        case POS_NOUN:
        {
            h_r = h_BuildNounForms();
            break;
        }
        case POS_ADJ:
        {
            h_r = h_BuildAdjForms();
            break;
        }

        default:
        {
            return E_FAIL;
        }
    }

    return S_OK;

}   // h_GenerateParadigm()

HRESULT CT_Lexeme::h_BuildNounForms()
{
    HRESULT h_r = S_OK;
    
    h_r = h_GetNounEndings();
    if (S_OK != h_r)
    {
        return h_r;
    }

    for (ET_Number eo_number = NUM_SG; eo_number <= NUM_PL; ++eo_number)
    {
        for (ET_Case eo_case = CASE_NOM; eo_case < CASE_NUM; ++eo_case)
        {
            CComObject<CT_WordForm> * sp_wordForm;
            h_r = CComObject<CT_WordForm>::CreateInstance (&sp_wordForm);
            if (S_OK != h_r)
            {
                return h_r;
            }

            wstring str_lemma (str_GraphicStem);

            if (b_StemAugment)
            {
                if (1 == i_Type)
                {
                    str_lemma.erase (str_lemma.length()-2, 2);  // римлянин, южанин, армянин
                }
                if (3 == i_Type)
                {
                    CT_ExtString xstr_gs (str_GraphicStem);
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
            }

            if (b_YoAlternation)
            {
                int i_yoOffset = str_GraphicStem.find (_T("ё"));
                if (std::wstring::npos != i_yoOffset)
                {
                    if (S_TRUE == h_StressOnNounEnding (eo_number, eo_case))
                    {
                        str_lemma[i_yoOffset] = L'е';        //  ежа, щелочей, тяжела
                    }
                    else
                    {
                        int i_eOffset = str_GraphicStem.rfind (_T("е"));
                        if (std::wstring::npos == i_eOffset)
                        {
                            ATLASSERT(0);
                            ERROR_LOG (L"Unrecognized ending.");
                            return E_FAIL;
                        }
                        if (i_eOffset == i_SourceStressPos)
                        {
                            str_lemma[i_eOffset] = L'ё';
                        }
                    }
                }
            }

            h_r = h_StressOnNounEnding (eo_number, eo_case);
            ET_EndingStressType eo_stress = (S_TRUE == h_r) 
                ? ENDING_STRESS_STRESSED 
                : ENDING_STRESS_UNSTRESSED;

            int i_numEndings = co_Endings.i_GetNumOfEndings (GENDER_UNDEFINED, 
                                                             eo_number, 
                                                             eo_case,
                                                             ANIM_UNDEFINED,
                                                             eo_stress);
            if (i_numEndings < 1)
            {
                ERROR_LOG (L"No endings");
                continue;
            }

            for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)
            {
                std::wstring str_ending;
                bool b_ = co_Endings.b_GetEnding (str_ending,
                                                  GENDER_UNDEFINED,
                                                  eo_number, 
                                                  eo_case,
                                                  ANIM_UNDEFINED,                    
                                                  eo_stress, 
                                                  i_ending);
                if (!b_)
                {
                    ERROR_LOG (L"Error getting ending from hash.");
                    continue;
                }

                if (b_FleetingVowel)
                {
                    h_r = h_HandleFleetingVowel (eo_number, 
                                                 eo_case, 
                                                 GENDER_UNDEFINED, 
                                                 ADJ_FORM_UNDEFINED,
                                                 str_ending,
                                                 str_lemma);
                }
                
                sp_wordForm->str_Lemma = str_lemma;
                sp_wordForm->str_WordForm = str_lemma + str_ending;
                sp_wordForm->eo_Case = eo_case;
                sp_wordForm->eo_Number = eo_number;
                sp_wordForm->eo_Animacy = map_MainSymbolToAnimacy[str_InflectionType];
                
                m_coll.push_back (sp_wordForm);
            }
        }
    }

    return S_OK;

}    //  h_BuildNounForms()

HRESULT CT_Lexeme::h_BuildAdjForms()
{
    HRESULT h_r = S_OK;

    h_r = h_GetAdjEndings();
    if (S_OK != h_r)
    {
        return h_r;
    }

    for (ET_Gender eo_gender = GENDER_M; eo_gender <= GENDER_N; ++eo_gender)
    {
        for (ET_Number eo_number = NUM_SG; eo_number <= NUM_PL; ++eo_number)
        {
            for (ET_Case eo_case = CASE_NOM; eo_case < CASE_NUM; ++eo_case)
            {
                for (ET_Animacy eo_animacy = ANIM_YES; eo_animacy < ANIM_COUNT; ++eo_animacy)
                {
                    wstring str_lemma (str_GraphicStem);

                    ET_EndingStressType eo_stress = ENDING_STRESS_UNDEFINED;
                    if (AT_A == eo_AccentType1 || AT_A1 == eo_AccentType1)
                    {
                        eo_stress = ENDING_STRESS_UNSTRESSED;
                    }
                    else
                    {
                        if (AT_B == eo_AccentType1)
                        {
                            eo_stress = ENDING_STRESS_STRESSED;
                        }
                        else
                        {
                            ERROR_LOG (L"Unknown stress type.");
                            continue;
                        }
                    }
                    int i_numEndings = co_Endings.i_GetNumOfEndings (eo_gender,
                                                                     eo_number, 
                                                                     eo_case,
                                                                     eo_animacy, 
                                                                     eo_stress);
                    if (i_numEndings < 1)
                    {
                        ERROR_LOG (L"No endings");
                        continue;
                    }

                    for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)
                    {
                        std::wstring str_ending;
                        bool b_ = co_Endings.b_GetEnding (str_ending,
                                                          eo_gender, 
                                                          eo_number, 
                                                          eo_case, 
                                                          eo_animacy,
                                                          eo_stress, 
                                                          i_ending);
                        if (!b_)
                        {
                            ERROR_LOG (L"Error getting ending from hash.");
                            continue;
                        }

                        CComObject<CT_WordForm> * sp_wordForm;
                        h_r = CComObject<CT_WordForm>::CreateInstance (&sp_wordForm);
                        if (S_OK != h_r)
                        {
                            return h_r;
                        }

                        sp_wordForm->eo_AdjForm = ADJ_FORM_LONG;
                        sp_wordForm->str_Lemma = str_lemma;
                        sp_wordForm->str_WordForm = str_lemma + str_ending;
                        sp_wordForm->eo_Gender = eo_gender;
                        sp_wordForm->eo_Case = eo_case;
                        sp_wordForm->eo_Number = eo_number;
                        sp_wordForm->eo_Animacy = eo_animacy;
                        m_coll.push_back (sp_wordForm);
                    }
                }
            }
        }
    }

    //
    // Short forms
    //
    h_r = h_GetShortFormEndings();
    if (S_OK != h_r)
    {
        return h_r;
    }

    for (ET_Gender eo_gender = GENDER_UNDEFINED; eo_gender <= GENDER_N; ++eo_gender)
    {
        CComObject<CT_WordForm> * sp_wordForm;
        h_r = CComObject<CT_WordForm>::CreateInstance (&sp_wordForm);
        if (S_OK != h_r)
        {
            return h_r;
        }

        ET_Number eo_number = (GENDER_UNDEFINED == eo_gender) ? NUM_PL : NUM_SG;

        ET_EndingStressType eo_stress = ENDING_STRESS_UNDEFINED;
        h_r = h_StressOnShortFormEnding (eo_number, eo_gender);
        switch (h_r)
        {
            case S_TRUE:
            {
                eo_stress = ENDING_STRESS_STRESSED;
                break;
            }
            case S_FALSE:
            {
                eo_stress = ENDING_STRESS_UNSTRESSED;
                break;
            }
            case S_BOTH:
            {
                eo_stress = ENDING_STRESS_UNDEFINED;
                break;
            }
            default:
            {
                ATLASSERT(0);
                ERROR_LOG (L"h_StressOnShortFormEnding() failed");
                return h_r;
            }
        }

        wstring str_lemma (str_GraphicStem);

        int i_numEndings = co_Endings.i_GetNumOfEndings (eo_gender,
                                                         eo_number, 
                                                         CASE_UNDEFINED,
                                                         ANIM_UNDEFINED, 
                                                         eo_stress);
        if (i_numEndings < 1)
        {
            ERROR_LOG (L"No endings");
            continue;
        }

        for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)
        {
            std::wstring str_ending;
            bool b_ = co_Endings.b_GetEnding (str_ending,
                                              eo_gender, 
                                              eo_number, 
                                              CASE_UNDEFINED, 
                                              ANIM_UNDEFINED,
                                              eo_stress, 
                                              i_ending);
            if (!b_)
            {
                ERROR_LOG (L"Error getting ending from hash.");
                continue;
            }

            if (b_FleetingVowel)
            {
                h_r = h_HandleFleetingVowel (eo_number, 
                                             CASE_UNDEFINED, 
                                             eo_gender, 
                                             ADJ_FORM_SHORT,
                                             str_ending,
                                             str_lemma);
            }

            sp_wordForm->eo_AdjForm = ADJ_FORM_SHORT;
            sp_wordForm->str_Lemma = str_lemma;
            sp_wordForm->str_WordForm = str_lemma + str_ending;
            sp_wordForm->eo_Gender = eo_gender;
            sp_wordForm->eo_Number = eo_number;
            m_coll.push_back (sp_wordForm);
        }

    }   //  for (ET_Gender eo_gender = GENDER_UNDEFINED; ...

    //
    // Comparative
    //
    if (b_NoComparative)
    {
        return S_OK;
    }

    CT_ExtString xstr_graphicStem (str_GraphicStem);
    wstring str_lemma (str_GraphicStem);

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

        CComObject<CT_WordForm> * sp_wordForm;
        h_r = CComObject<CT_WordForm>::CreateInstance (&sp_wordForm);
        if (S_OK != h_r)
        {
            return h_r;
        }
        sp_wordForm->eo_AdjForm = ADJ_FORM_COMPARATIVE;
        sp_wordForm->str_Lemma = str_lemma;
        sp_wordForm->str_WordForm = str_lemma + L"е";

        int i_lastStemVowel = str_lemma.find_last_of (str_Vowels);
        if (wstring::npos != i_lastStemVowel)
        {
            sp_wordForm->i_Stress = i_lastStemVowel;
        }
        else
        {
            ERROR_LOG (L"Warning: can't find stressed vowel in comparative.");
            sp_wordForm->i_Stress = -1;
        }


        m_coll.push_back (sp_wordForm);
    }
    else
    {
        CComObject<CT_WordForm> * sp_wordForm1;
        h_r = CComObject<CT_WordForm>::CreateInstance (&sp_wordForm1);
        if (S_OK != h_r)
        {
            return h_r;
        }
        CComObject<CT_WordForm> * sp_wordForm2;
        h_r = CComObject<CT_WordForm>::CreateInstance (&sp_wordForm2);
        if (S_OK != h_r)
        {
            return h_r;
        }

        int i_stressPos = -1;
        if ((AT_A == eo_AccentType1) && (AT_UNDEFINED == eo_AccentType2))
        {
            i_stressPos = i_SourceStressPos;
        }
        else
        {
            i_stressPos = str_lemma.length();
        }

        sp_wordForm1->eo_AdjForm = ADJ_FORM_COMPARATIVE;
        sp_wordForm1->str_Lemma = str_lemma;
        sp_wordForm1->str_WordForm = str_lemma + L"ей";
        sp_wordForm1->i_Stress = i_stressPos;

        m_coll.push_back (sp_wordForm1);

        sp_wordForm2->eo_AdjForm = ADJ_FORM_COMPARATIVE;
        sp_wordForm2->str_Lemma = str_lemma;
        sp_wordForm2->str_WordForm = str_lemma + L"ее";
        sp_wordForm2->i_Stress = i_stressPos;

        m_coll.push_back (sp_wordForm2);
    }

    return S_OK;

}   // h_BuildAdjForms (...)

HRESULT CT_Lexeme::h_StressOnNounEnding (ET_Number eo_number, ET_Case eo_case)
{
    if (POS_NOUN != eo_PartOfSpeech)
    {
        ATLASSERT(0);
        ERROR_LOG (L"Unexpected part of speech value.");
        return E_FAIL;
    }

    if (NUM_UNDEFINED == eo_number)
    {
        ATLASSERT(0);
        return E_INVALIDARG;
    }

    switch (eo_AccentType1)
    {
        case AT_A:
        {
            return S_FALSE;
        }
        case AT_B:
        {
            return S_TRUE;
        }
        case AT_B1:
        {
            if (NUM_SG == eo_number && CASE_INST == eo_case)
            {
                return S_FALSE;
            }
            return S_TRUE;
        }
        case AT_C:
        {
            if (NUM_SG == eo_number)
            {
                return S_FALSE;
            }
            return S_TRUE;
        }
        case AT_D:
        {
            if (NUM_SG == eo_number)
            {
                return S_TRUE;
            }
            return S_FALSE;
        }
        case AT_D1:
        {
            if (GENDER_F != map_MainSymbolToGender[str_InflectionType])
            {
                ATLASSERT(0);
                ERROR_LOG (L"Non-fem. noun has a.p. D1.");
                return E_FAIL;
            }
            if (NUM_SG == eo_number && CASE_ACC == eo_case) // assume fem -a stems only?
            {
                return S_FALSE;
            }
            if (NUM_SG == eo_number)
            {
                return S_TRUE;
            }
            return S_FALSE;
        }
        case AT_E:
        {
            if (NUM_SG == eo_number)
            {
                return S_FALSE;
            }
            if (CASE_NOM == eo_case)
            {
                return S_FALSE;
            }
            if (CASE_ACC == eo_case && ANIM_NO == map_MainSymbolToAnimacy[str_InflectionType])
            {
                return S_FALSE;
            }
            return S_TRUE;
        }
        case AT_F:
        {
            if (NUM_SG == eo_number)
            {
                return S_TRUE;
            }

            if (CASE_NOM == eo_case)
            {
                return S_FALSE;
            }
            if (CASE_ACC == eo_case && ANIM_NO == map_MainSymbolToAnimacy[str_InflectionType])
            {
                return S_FALSE;
            }
            return S_TRUE;
        }
        case AT_F1:
        {
            if (GENDER_F != map_MainSymbolToGender[str_InflectionType])
            {
                ATLASSERT(0);                   // assume f -a stems only?
                ERROR_LOG (L"Non-fem. noun has a.p. F1.");
                return E_FAIL;
            }
            if (NUM_PL == eo_number && CASE_NOM == eo_case)
            {
                return S_FALSE;
            }
            if (NUM_SG == eo_number && CASE_ACC == eo_case) // fem only?
            {
                return S_FALSE;
            }
            return S_TRUE;
        }
        case AT_F2:
        {
            if (GENDER_F != map_MainSymbolToGender[str_InflectionType] || 8 != i_Type)
            {
                ATLASSERT(0);                   // assume f -i stems only?
                ERROR_LOG (L"Non-fem./type 8 noun has a.p. F2.");
                return E_FAIL;
            }
            if (ANIM_YES == map_MainSymbolToAnimacy[str_InflectionType])
            {
                ATLASSERT(0);                   // inanimate only?
                ERROR_LOG (L"Animate noun has a.p. F2.");
                return E_FAIL;
            }
            if (NUM_PL == eo_number && (CASE_NOM == eo_case || CASE_ACC == eo_case))
            {
                return S_FALSE;
            }
            if (NUM_SG == eo_number && CASE_INST == eo_case)
            {
                return S_FALSE;
            }
            return S_TRUE;
        }
        default:
        {
            ERROR_LOG (L"Illegal accent type.");
            return E_FAIL;
        }
    }

    return S_FALSE;

}   // h_StressOnNounEnding()

HRESULT CT_Lexeme::h_StressOnShortFormEnding (ET_Number eo_number, ET_Gender eo_gender)
{
    if (POS_ADJ != eo_PartOfSpeech)
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
        return S_FALSE;
    }

    ET_AccentType eo_at = AT_UNDEFINED;
    if (AT_UNDEFINED == eo_AccentType2)
    {
        eo_at = eo_AccentType1;
    }
    else
    {
        eo_at = eo_AccentType2;
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
            return S_FALSE;
        }
        case AT_A1:
        {
            if (GENDER_F == eo_gender)
            {
                ATLASSERT (NUM_SG == eo_number);
                return S_BOTH;
            }
            if (GENDER_N == eo_gender)
            {
                ATLASSERT (NUM_SG == eo_number);
                return S_FALSE;
            }
            if (NUM_PL == eo_number)
            {
                ATLASSERT (NUM_SG == eo_number);
                return S_FALSE;
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
                return S_TRUE;
            }
            if (NUM_PL == eo_number)
            {
                ATLASSERT (GENDER_UNDEFINED == eo_gender);
                return S_BOTH;
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
                return S_TRUE;
            }
            if (GENDER_N == eo_gender || NUM_PL == eo_number)
            {
                return S_FALSE;
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
                return S_TRUE;
            }
            if (GENDER_N == eo_gender)
            {
                ATLASSERT (NUM_SG == eo_number);
                return S_FALSE;
            }
            if (NUM_PL == eo_number)
            {
                ATLASSERT (GENDER_UNDEFINED == eo_gender);
                return S_BOTH;
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
                return S_TRUE;
            }
            if (GENDER_N == eo_gender)
            {
                ATLASSERT (NUM_SG == eo_number);
                return S_BOTH;
            }
            if (NUM_PL == eo_number)
            {
                ATLASSERT (GENDER_UNDEFINED == eo_gender);
                return S_BOTH;
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

    return S_FALSE;

}   // h_StressOnShortFormEnding()

HRESULT CT_Lexeme::h_HandleFleetingVowel (ET_Number eo_number,         // in
                                          ET_Case eo_case,             // in
                                          ET_Gender eo_gender,         // in, adj only
                                          ET_AdjForm eo_longOrShort,   // in, adj only
                                          const wstring& str_ending,   // in
                                          wstring& str_lemma)          // out
{
    HRESULT h_r = S_OK;

    str_lemma = str_GraphicStem;
    int i_lastVowel = str_GraphicStem.find_last_of (str_Vowels);

    //
    // Fleeting vowel in initial form GDRL, p. 29-30
    //

    // ASSERT: last stem vowel is { о, е, ё, и } [also а, я "in several anomalous words"]
    if ((GENDER_M == map_MainSymbolToGender[str_InflectionType]) ||
        ((GENDER_F == map_MainSymbolToGender[str_InflectionType]) && (8 == i_Type)) ||
        (L"мс" == str_InflectionType))
    {
// endings that preserve the fleeting vowel (all from the initial form except -ью):
// m.: NULL (сон, конец)
// f.: -ь, -ью (любовь)
// pron.: NULL, -ь, -й (весь, волчий)

        //
        // No vowel in ending: keep the fleeting vowel
        //
        if (str_ending.empty())
        {
            if (GENDER_M != map_MainSymbolToGender[str_InflectionType])
            {
                ATLASSERT(0);
                ERROR_LOG (L"Enexpected gender for null ending");
                return E_UNEXPECTED;
            }
            return S_OK;
        }

        if (L"ь" == str_ending)
        {
            if ((L"мс" != str_InflectionType) || 
                (GENDER_F != map_MainSymbolToGender[str_InflectionType] || 8 != i_Type))
            {
                ATLASSERT(0);
                ERROR_LOG (L"Enexpected type for ending 'ь'");
                return E_UNEXPECTED;

            }
            return S_OK;
        }

        if (L"ью" == str_ending)
        {
            if (GENDER_F != map_MainSymbolToGender[str_InflectionType] || 8 != i_Type)
            {
                ATLASSERT(0);
                ERROR_LOG (L"Enexpected main symbol for ending 'ью'");
                return E_UNEXPECTED;
            }
            return S_OK;
        }

        if (L"й" == str_ending)
        {
            if (L"мс" != str_InflectionType)
            {
                ATLASSERT(0);
                ERROR_LOG (L"Enexpected main symbol for ending 'й'");
                return E_UNEXPECTED;
            }
            return S_OK;
        }

        if (wstring::npos == i_lastVowel)
        {
            ERROR_LOG (L"Vowel expected.");
            str_lemma.clear();
            return E_UNEXPECTED;
        }

        if (str_GraphicStem[i_lastVowel] == L'о')   // сон, любовь
        {
            str_lemma.erase (i_lastVowel, 1);
            return S_OK;
        }

        if (str_GraphicStem[i_lastVowel] == L'и')   // волчий
        {
            if ((L"мс" != str_InflectionType) || (6 != i_Type))
            {
                ERROR_LOG (L"Unexpected inflection type.");
                str_lemma.clear();
                return E_UNEXPECTED;
            }
            str_lemma[i_lastVowel] = L'ь';
            return S_OK;
        }

        if ((str_GraphicStem[i_lastVowel] == L'е') || 
            (str_GraphicStem[i_lastVowel] == L'ё'))
        {
            if (i_lastVowel > 0)
            {
                if (wstring::npos != str_Vowels.find (str_GraphicStem[i_lastVowel-1]))
                {
                    str_lemma[i_lastVowel] = L'й';   // боец, паек
                    return S_OK;
                }
            }
            if (GENDER_M == map_MainSymbolToGender[str_InflectionType])
            {
                if (6 == i_Type)                    // улей
                {
                    str_lemma = str_GraphicStem;
                    str_lemma[i_lastVowel] = L'й';
                    return S_OK;
                }
                if (3 == i_Type)
                {
                    if ((i_lastVowel > 0) && 
                        (wstring::npos == wstring (L"шжчщц").find (str_GraphicStem[i_lastVowel-1])))
                    {
                        str_lemma[i_lastVowel] = L'й';   // зверек
                        return S_OK;
                    }
                }
                if ((i_lastVowel > 0) && (L'л' == str_GraphicStem[i_lastVowel-1]))
                {
                    str_lemma[i_lastVowel] = L'й';       // лед, палец
                    return S_OK;
                }
            }
            
            str_lemma.erase (i_lastVowel, 1);   // default -- just remove the vowel

            return S_OK;
        
        }   // е or ё
    }

    //
    // Fleeting vowel is NOT in the initial form
    //
// fem nouns except i-stems: G Pl (and A PL if animate)
// adjectives (non-pronominal paradigm) short form m
// nouns with main symbol "mn."
    if ((GENDER_F == map_MainSymbolToGender[str_InflectionType] && 8 != i_Type && NUM_PL == eo_number) || 
        (GENDER_N == map_MainSymbolToGender[str_InflectionType] && NUM_PL == eo_number) || 
        (L"п" == str_InflectionType || L"мн." == str_MainSymbol))
    {
        if (POS_NOUN == eo_PartOfSpeech)
        {
            if (eo_case == CASE_ACC)
            {
                ET_EndingStressType eo_stress = (h_StressOnNounEnding (NUM_PL, eo_case) == S_TRUE) 
                    ? ENDING_STRESS_STRESSED 
                    : ENDING_STRESS_UNSTRESSED;
                if (co_Endings.i_GetNumOfEndings (GENDER_UNDEFINED, 
                                                  NUM_PL, 
                                                  CASE_ACC, 
                                                  ANIM_UNDEFINED, 
                                                  eo_stress) != 1)
                {
                    ERROR_LOG (L"Wrong number of endings.");
                }
                std::wstring str_endingPlAcc;
                bool b_ = co_Endings.b_GetEnding (str_endingPlAcc, 
                                                  GENDER_UNDEFINED, 
                                                  NUM_PL, 
                                                  eo_case, 
                                                  ANIM_UNDEFINED, 
                                                  eo_stress);
                if (!b_)
                {
                    ERROR_LOG (L"GetEnding() failed.");
                }
                std::wstring str_endingPlGen;
                b_ = co_Endings.b_GetEnding (str_endingPlGen, 
                                             GENDER_UNDEFINED,
                                             NUM_PL, 
                                             CASE_GEN,
                                             ANIM_UNDEFINED,
                                             eo_stress);
                if (!b_)
                {
                    ERROR_LOG (L"GetEnding() failed.");
                }

                if (str_endingPlAcc != str_endingPlGen)
                {
                    return S_OK;
                }
            }
            else
            {
                if (eo_case != CASE_GEN)
                {
                    return S_OK;
                }
            }
        }

        if (L"п" == str_InflectionType)
        {
            if (ADJ_FORM_SHORT != eo_longOrShort || GENDER_M != eo_gender)
            {
                return S_OK;
            }
        }

        if (((GENDER_F == map_MainSymbolToGender[str_InflectionType]) ||
            (GENDER_N == map_MainSymbolToGender[str_InflectionType])) &&
            (6 == i_Type))
        {
            if (S_TRUE == h_StressOnNounEnding (eo_number, eo_case))
            {
                str_lemma[str_lemma.length()-1] = L'е';   // статей, питей
            }
            else
            {
                str_lemma[str_lemma.length()-1] = L'и';   // гостий, ущелий
            }
            return S_OK;
        }

        int i_lastConsonant = str_GraphicStem.find_last_of (str_Consonants);
        if (i_lastConsonant > 0)
        {
            if ((L'ь' == str_GraphicStem[i_lastConsonant-1]) || 
                (L'й' == str_GraphicStem[i_lastConsonant-1]))
            {
                if (L'ц' == str_GraphicStem[i_lastConsonant])
                {
                    str_lemma[i_lastConsonant-1] = L'е';    // колец
                    return S_OK;
                }

                if (S_TRUE == h_StressOnNounEnding (eo_number, eo_case))
                {
                    str_lemma[i_lastConsonant-1] = L'ё';     // серёг, каём
                    return S_OK;
                }
                else
                {
                    str_lemma[i_lastConsonant] = L'е';       // шпилек, чаек, писем
                    return S_OK;
                }
            }
        }
        if (i_lastConsonant > 0)
        {
            if (L'к' == str_GraphicStem[i_lastConsonant-1] || 
                L'г' == str_GraphicStem[i_lastConsonant-1] ||
                L'х' == str_GraphicStem[i_lastConsonant-1])
            {
                str_lemma.insert (i_lastConsonant, 1, L'о');  // кукол, окон, мягок
                return S_OK;
            }
            if (L'к' == str_GraphicStem[i_lastConsonant] ||
                L'г' == str_GraphicStem[i_lastConsonant] ||
                L'х' == str_GraphicStem[i_lastConsonant])
            {
                if (3 != i_Type)
                {
                    ERROR_LOG (_T("Unexpected type."));
                    return E_FAIL;
                }
                if (L'ш' != str_GraphicStem[i_lastConsonant-1] &&
                    L'ж' != str_GraphicStem[i_lastConsonant-1] &&
                    L'ч' != str_GraphicStem[i_lastConsonant-1] &&
                    L'щ' != str_GraphicStem[i_lastConsonant-1] &&
                    L'ц' != str_GraphicStem[i_lastConsonant-1])
                {
                    str_lemma.insert (i_lastConsonant, 1, L'о');  // сказок, ведерок, краток, долог
                    return S_OK;
                }
            }
            if (L'ц' == str_GraphicStem[i_lastConsonant])
            {
                str_lemma.insert (i_lastConsonant, 1, L'е');      // овец
                return S_OK;
            }

            if (POS_NOUN == eo_PartOfSpeech)
            {
                h_r = h_StressOnNounEnding (eo_number, eo_case);
            }
            else
            {
                h_r = h_StressOnShortFormEnding (eo_number, eo_gender);
            }

            if (S_TRUE == h_r)
            {
                if (L'ш' == str_GraphicStem[i_lastConsonant-1] &&
                    L'ж' == str_GraphicStem[i_lastConsonant-1] &&
                    L'ч' == str_GraphicStem[i_lastConsonant-1] &&
                    L'щ' == str_GraphicStem[i_lastConsonant-1])
                {
                    str_lemma.insert (i_lastConsonant, 1, L'о');    // кишок
                    return S_OK;
                }
                else
                {
                    str_lemma.insert (i_lastConsonant, 1, L'ё');    // сестёр, хитёр
                    return S_OK;
                }
            }
            else
            {
                str_lemma.insert (i_lastConsonant, 1, L'е');       // сосен, чисел, ножен, верен
            }
        }
    }
    return S_OK;

}   //  h_HandleFleetingVowel (...)
