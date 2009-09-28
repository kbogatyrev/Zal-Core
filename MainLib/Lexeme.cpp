#include "StdAfx.h"
#include "Lexeme.h"

//
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

//&&&&
    h_GenerateParadigm();

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
template <typename T>
wstring static str_ToString (T from)
{
    wstringstream io_;
	io_ << from;
	return io_.str();
};

void CT_Lexeme::v_Init()
{
    i_DbKey = -1;
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
                map_MainSymbolToGender[str_ms] = G_M;
                break;
            }
            case MS_MO:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_YES;
                map_MainSymbolToGender[str_ms] = G_M;
                break;
            }
            case MS_ZH:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_NO;
                map_MainSymbolToGender[str_ms] = G_F;
                break;
            }
            case MS_ZHO:
            case MS_MO_ZHO:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_YES;
                map_MainSymbolToGender[str_ms] = G_F;
                break;
            }
            case MS_S:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_NO;
                map_MainSymbolToGender[str_ms] = G_N;
            }
            case MS_SO:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_YES;
                map_MainSymbolToGender[str_ms] = G_N;
                break;
            }
            case MS_MN:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_UNDEFINED;
                map_MainSymbolToGender[str_ms] = G_UNDEFINED;
                break;
            }
            case MS_MN_NEOD:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_NO;
                map_MainSymbolToGender[str_ms] = G_UNDEFINED;
                break;
            }
            case MS_MN_ODUSH:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_YES;
                map_MainSymbolToGender[str_ms] = G_UNDEFINED;
                break;
            }
            case MS_MN_OT:
            {
                map_MainSymbolToAnimacy[str_ms] = ANIM_NO;
                map_MainSymbolToGender[str_ms] = G_UNDEFINED;
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
                map_MainSymbolToGender[str_ms] = G_UNDEFINED;
                return;
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

HRESULT CT_Lexeme::h_GetEndings (ET_Number eo_number, ET_Case eo_case)
{
    HRESULT h_r = S_OK;

//    ET_Gender eo_gender = map_MainSymbolToGender[str_MainSymbol];
    ET_Gender eo_gender = map_MainSymbolToGender[str_InflectionType];
    //if (G_UNDEFINED == eo_gender)
    //{
    //    if (str_InflectionType.empty())
    //    {
    //        ATLASSERT(0);
    //        return E_FAIL;
    //    }
    //    eo_gender = map_MainSymbolToGender[str_InflectionType];
    //}

    ET_Animacy eo_animacy = map_MainSymbolToAnimacy[str_InflectionType];
    //if (ANIM_UNDEFINED == eo_animacy)
    //{
    //    if (str_InflectionType.empty())
    //    {
    //        ATLASSERT(0);
    //        return E_FAIL;
    //    }
    //    eo_animacy = map_MainSymbolToAnimacy[str_InflectionType];
    //}

    wstring str_select 
(L"select distinct ending, case_value, person, is_plural from endings where inflection_class = ");
    str_select += str_ToString (eo_PartOfSpeech);
    str_select += (L" and (inflection_type = ");
    str_select += str_ToString (i_Type);
    str_select += (L" or inflection_type = -1)");
    str_select += (L" and (gender = ");
    str_select += str_ToString (eo_gender);
    str_select += (L" or gender = 0)");
    str_select += (L" and (is_animate = ");
    str_select += str_ToString (eo_animacy);
    str_select += (L" or is_animate = 0)");
    str_select += (L" and stem_augment = ");
    str_select += b_StemAugment ? L"1" : L"0";
    str_select += (L" and common_deviation = ");
    str_select += str_ToString (i_CommonDeviation);
    str_select += (L" and (case_value = ");
    str_select += str_ToString (eo_case);
    str_select += (L" or case_value = 0)");
    str_select += (L" and is_plural = ");
    str_select += (NUM_PL == eo_number) ? L"1" : L"0";

    ST_Ending st_ending;
    int i_id = 0;

    vec_Endings.clear();

    try
    {
        pco_Db->v_PrepareForSelect (str_select);

        while (pco_Db->b_GetRow())
        {
//            pco_Db->v_GetData (0, i_id);
            pco_Db->v_GetData (0, st_ending.str_Ending);
//            pco_Db->v_GetData (2, (int&)st_ending.eo_Class);
//            pco_Db->v_GetData (3, st_ending.i_Type);
            pco_Db->v_GetData (1, (int&)st_ending.eo_Case);
            pco_Db->v_GetData (2, st_ending.i_Person);
//            pco_Db->v_GetData (6, (int&)st_ending.eo_Gender);
//            pco_Db->v_GetData (7, st_ending.b_Animate);
//            pco_Db->v_GetData (8, st_ending.b_Stressed);
            pco_Db->v_GetData (3, st_ending.b_Plural);

            vec_Endings.push_back (st_ending);
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
    
    if (POS_NOUN == eo_PartOfSpeech)
    {
        int i_hash = 0;
        std::vector<ST_Ending>::iterator it_ending = vec_Endings.begin();
        for (; vec_Endings.end() != it_ending; ++it_ending)
        {
            i_hash = (*it_ending).b_Plural ? 10 : 0;
            i_hash += (int)(*it_ending).eo_Case;
        }
        map_NumberCaseToEnding[i_hash] = vec_Endings.size()-1;
    }

    return S_OK;
        
}   //  b_GetEndings (...)

HRESULT CT_Lexeme::h_GenerateParadigm()
{
    HRESULT h_r = S_OK;

    switch (eo_PartOfSpeech)
    {
        case POS_NOUN:
        {
            h_r = h_BuildNounForms();
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

            h_r = h_GetEndings (eo_number, eo_case);
            if (S_OK != h_r)
            {
                return h_r;
            }
// TODO: alternative forms; error if vec_Endings is empty
// handle fleeting vowel 

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

            if (vec_Endings.size() > 0)
            {
                sp_wordForm->str_WordForm = str_GraphicStem + vec_Endings[0].str_Ending;
            }
            sp_wordForm->eo_Case = eo_case;
//            sp_wordForm->eo_Animacy = map_MainSymbolToAnimacy[str_MainSymbol];
            sp_wordForm->eo_Animacy = map_MainSymbolToAnimacy[str_InflectionType];
            m_coll.push_back (sp_wordForm);
        }
    }

    return S_OK;
}

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
            if (G_F != map_MainSymbolToGender[str_InflectionType])
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
            if (G_F != map_MainSymbolToGender[str_InflectionType])
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
            if (G_F != map_MainSymbolToGender[str_InflectionType] || 8 != i_Type)
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

}   // h_StressOnEnding()

HRESULT CT_Lexeme::h_HandleFleetingVowel (ET_Number eo_number,         // in
                                          ET_Case eo_case,             // in
                                          ET_Gender eo_gender,         // in, adj only
                                          ET_AdjForm eo_longOrShort,   // in, adj only
                                          const wstring& str_ending,   // in
                                          wstring& str_lemma)          // out
{
    str_lemma = str_GraphicStem;
    int i_lastVowel = str_GraphicStem.find_last_of (str_Vowels);

    //
    // Fleeting vowel in initial form GDRL, p. 29-30
    //

    // ASSERT: last stem vowel is { о, е, ё, и } [also а, я "in several anomalous words"]
    if ((G_M == map_MainSymbolToGender[str_InflectionType]) ||
        ((G_F == map_MainSymbolToGender[str_InflectionType]) && (8 == i_Type)) ||
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
            if (G_M != map_MainSymbolToGender[str_InflectionType])
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
                (G_F != map_MainSymbolToGender[str_InflectionType] || 8 != i_Type))
            {
                ATLASSERT(0);
                ERROR_LOG (L"Enexpected type for ending 'ь'");
                return E_UNEXPECTED;

            }
            return S_OK;
        }

        if (L"ью" == str_ending)
        {
            if (G_F != map_MainSymbolToGender[str_InflectionType] || 8 != i_Type)
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
            if (G_M == map_MainSymbolToGender[str_InflectionType])
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
    if (((G_F == map_MainSymbolToGender[str_InflectionType]) && (8 != i_Type)) || 
        (G_N == map_MainSymbolToGender[str_InflectionType]) || 
        (L"п" == str_InflectionType) || (L"мн." == str_MainSymbol))
    {
        if (POS_NOUN == eo_PartOfSpeech)
        {
            if (eo_case != CASE_GEN)
            {
                return S_OK;
            }
            if (eo_case == CASE_ACC)
            {
                if (str_GetNounEnding (NUM_PL, CASE_ACC) == str_GetNounEnding (NUM_PL, CASE_GEN))
                {
                    return S_OK;
                }
            }
        }

        if (L"п" == str_InflectionType)
        {
            if (FORM_SHORT != eo_longOrShort || G_M != eo_gender)
            {
                return S_OK;
            }
        }

        if (((G_F == map_MainSymbolToGender[str_InflectionType]) ||
            (G_N == map_MainSymbolToGender[str_InflectionType])) &&
            (6 == i_Type))
        {
            if (S_OK == h_StressOnNounEnding (eo_number, eo_case))
            {
                str_lemma[str_lemma.length()-1] = L'е';   // статей, питей
            }
            else
            {
                str_lemma[str_lemma.length()-1] = L'и';   // гостий, ущелий
            }
            return S_OK;
        }
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
        if (S_TRUE == h_StressOnNounEnding (eo_number, eo_case))
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

    return S_OK;

}   //  h_HandleFleetingVowel (...)

wstring CT_Lexeme::str_GetNounEnding (ET_Number eo_number, ET_Case eo_case)
{
    int i_hash = (NUM_SG == eo_number) ? 0 : 10;
    i_hash += (int)eo_case;
    int i_ending = map_NumberCaseToEnding[i_hash];
    return vec_Endings[i_ending].str_Ending;
    

}   //  str_GetNounEnding (...)
