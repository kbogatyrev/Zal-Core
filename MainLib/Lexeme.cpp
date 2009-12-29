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
    *pl_id = i_DbKey;
    return S_OK;
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
                break;
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

    wstring arr_1stStage[] = 
        {L"б",  L"п",  L"в",  L"ф",  L"м",  L"з", L"с", L"д", L"т", L"ст", L"г", L"к", L"х", L"ск"};
    wstring arr_2ndStage[] = 
        {L"бл", L"пл", L"вл", L"фл", L"мл", L"ж", L"ш", L"ж", L"ч", L"щ",  L"ж", L"ч", L"ш", L"щ"}; 

    for (int i_sa = 0; i_sa < sizeof (arr_1stStage)/sizeof (wstring); ++i_sa)
    {
        map_StandardAlternations[arr_1stStage[i_sa]] = arr_2ndStage[i_sa];
    }

    wstring arr_preverbs[] = {L"в", L"над", L"об", L"от", L"под", L"пред", L"с"};
    for (int i_ap = 0; i_ap < sizeof (arr_preverbs)/sizeof (wstring); ++i_ap)
    {
        vec_AlternatingPreverbs.push_back (arr_preverbs[i_ap]);
    }

    wstring arr_preverbsV[] = {L"вс", L"вз", L"вос", L"воз", L"ис", L"из", L"нис", L"низ", L"рас", L"раз"};
    for (int i_ap = 0; i_ap < sizeof (arr_preverbs)/sizeof (wstring); ++i_ap)
    {
        vec_AlternatingPreverbsWithVoicing.push_back (arr_preverbs[i_ap]);
    }

    pco_Db = NULL;
    pco_Endings = NULL;

}   // v_Init()

void CT_Lexeme::v_SetDb (const wstring& str_dbPath)
{
    pco_Db = new CT_Sqlite (str_dbPath);
}

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
    str_select += str_ToString (ENDING_CLASS_NOUN);
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
    str_select += L";";

    pco_Endings = new CT_NounEndings;

    try
    {
        pco_Db->v_PrepareForSelect (str_select);

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
        
}   //  b_GetNounEndings()

HRESULT CT_Lexeme::h_GetLongFormEndings()
{
    HRESULT h_r = S_OK;

    if (POS_ADJ != eo_PartOfSpeech)
    {
        ATLASSERT(0);
        return E_INVALIDARG;
    }

    wstring str_select 
        (L"SELECT DISTINCT ending, gender, number, case_value, animacy, stress FROM endings");
    str_select += L" WHERE inflection_class = ";
    if (L"п" == str_InflectionType)
    {
//        str_select += L"2";
        str_select += str_ToString (ENDING_CLASS_ADJECTIVE);
    }
    else
    {
        if (L"мс" == str_InflectionType)
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
    str_select += str_ToString (i_Type);
    str_select += L";";

    pco_Endings = new CT_AdjLongEndings;
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
        
}   //  b_GetLongFormEndings()

HRESULT CT_Lexeme::h_GetShortFormEndings()
{
    HRESULT h_r = S_OK;

    if (POS_ADJ != eo_PartOfSpeech)
    {
        return E_INVALIDARG;
    }

    wstring str_select (L"SELECT DISTINCT ending, gender, number, stress FROM endings");
    str_select += L" WHERE inflection_class = ";
    str_select += str_ToString (ENDING_CLASS_SHORT_ADJECTIVE);
    str_select += L" AND inflection_type = ";
    str_select += str_ToString (i_Type);
    str_select += L";";

    pco_Endings = new CT_AdjShortEndings;
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
        
}   //  h_GetShortFormEndings()

HRESULT CT_Lexeme::h_GetPersonalEndings()
{
    HRESULT h_r = S_OK;

    if (POS_VERB != eo_PartOfSpeech)
    {
        return E_INVALIDARG;
    }

    int i_conjugation = ((4 == i_Type) || (5 == i_Type)) ? 2 : 1;

    wstring str_select (L"SELECT DISTINCT ending, person, number, stress FROM endings");
    str_select += L" WHERE inflection_class = 5 AND inflection_type = ";
    str_select += str_ToString (i_conjugation);
    str_select += L";";

    pco_Endings = new CT_PersonalEndings;

    try
    {
        pco_Db->v_PrepareForSelect (str_select);

        while (pco_Db->b_GetRow())
        {
            wstring str_ending;
            ST_EndingDescriptor st_d;
            pco_Db->v_GetData (0, str_ending);
            pco_Db->v_GetData (1, (int&)st_d.eo_Person);
            pco_Db->v_GetData (2, (int&)st_d.eo_Number);
            pco_Db->v_GetData (3, (int&)st_d.eo_Stress);
            pco_Endings->h_AddEnding (str_ending, st_d);
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

}   //  h_GetPersonalEndings()

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
        case POS_VERB:
        {
            h_r = h_BuildVerbForms();
            break;
        }

        default:
        {
            return E_NOTIMPL;
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

            h_r = h_StressOnNounEnding (eo_number, eo_case);
            ET_EndingStressType eo_stress = (S_TRUE == h_r) 
                ? ENDING_STRESS_STRESSED 
                : ENDING_STRESS_UNSTRESSED;

            if (b_YoAlternation)
            {
                h_r = h_HandleYoAlternation (eo_stress, false, str_lemma);
                if (S_OK != h_r)
                {
                    return h_r;
                }
            }

            ET_Case eo_endingCase = eo_case;

            if (CASE_ACC == eo_case)
            {
                if (NUM_SG == eo_number)
                {
                    if (GENDER_M == map_MainSymbolToGender[str_InflectionType])
                    {
                        ANIM_YES == map_MainSymbolToAnimacy[str_InflectionType]
                            ? eo_endingCase = CASE_GEN
                            : eo_endingCase = CASE_NOM;
                    }
                    if (GENDER_N == map_MainSymbolToGender[str_InflectionType])
                    {
                        eo_endingCase = CASE_NOM;
                    }
                }
                if (NUM_PL == eo_number)
                {
                    ANIM_YES == map_MainSymbolToAnimacy[str_InflectionType]
                        ? eo_endingCase = CASE_GEN
                        : eo_endingCase = CASE_NOM;
                }
            }

            int i_numEndings = pco_Endings->i_Count (ST_EndingDescriptor (eo_number, eo_endingCase, eo_stress));
            if (i_numEndings < 1)
            {
//                ATLASSERT(0);
                ERROR_LOG (L"No endings");
                continue;
            }

            for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)
            {
                std::wstring str_ending;
                h_r = pco_Endings->h_GetEnding (ST_EndingDescriptor (eo_number, eo_endingCase, eo_stress),
                                                i_ending,
                                                str_ending);
                if (S_OK != h_r)
                {
                    ERROR_LOG (L"Error getting ending from hash.");
                    continue;
                }

                if (8 == i_Type)
                {
                    if (wstring::npos == wstring (str_ShSounds + L'ц').find (str_lemma[str_lemma.length()-1]))
                    {
                        if (str_ending.length() > 0)
                        {
                            if (L'а' == str_ending[0])
                            {
                                continue;
                            }
                        }
                    }
                    else
                    {
                        if (str_ending.length() > 0)
                        {
                            if (L'я' == str_ending[0])
                            {
                                continue;
                            }
                        }
                    }
                }

                if (b_FleetingVowel)
                {
                    h_r = h_HandleFleetingVowel (eo_number, 
                                                 eo_endingCase,
                                                 GENDER_UNDEFINED, 
                                                 ADJ_FORM_UNDEFINED,
                                                 str_ending,
                                                 str_lemma);
                }
                
                int i_stressPos = -1;
                h_r = h_GetStressPosition (str_lemma, str_ending, eo_stress, i_stressPos, false);
                if (S_OK != h_r)                                                          // ^- not a short form  
                {
                    ERROR_LOG (L"Error getting stress position.");
//                    continue;
                }

                CComObject<CT_WordForm> * sp_wordForm;
                h_r = CComObject<CT_WordForm>::CreateInstance (&sp_wordForm);
                if (S_OK != h_r)
                {
                    return h_r;
                }
                sp_wordForm->eo_POS = eo_PartOfSpeech;
                sp_wordForm->str_Lemma = str_lemma;
                sp_wordForm->str_WordForm = str_lemma + str_ending;
                sp_wordForm->eo_Case = eo_case;
                sp_wordForm->eo_Number = eo_number;
                sp_wordForm->eo_Animacy = map_MainSymbolToAnimacy[str_InflectionType];
                sp_wordForm->i_Stress = i_stressPos;
                
                m_coll.push_back (sp_wordForm);
            }
        }
    }

    return S_OK;

}    //  h_BuildNounForms()

HRESULT CT_Lexeme::h_BuildAdjForms()
{
    HRESULT h_r = S_OK;

    h_r = h_GetLongFormEndings();
    if (S_OK != h_r)
    {
        return h_r;
    }

    h_r = h_BuildLongForms();
    ATLASSERT (S_OK == h_r);

    if (L"мс" == str_InflectionType)
    {
        return S_OK;
    }

    if (!b_NoShortForms)
    {
        h_r = h_BuildShortForms();
        ATLASSERT (S_OK == h_r);
    }

    if (b_NoComparative)
    {
        return S_OK;
    }

    h_r = h_BuildComparative();
    ATLASSERT (S_OK == h_r);

    return S_OK;

}   // h_BuildAdjForms (...)

HRESULT CT_Lexeme::h_BuildVerbStems()
{
    HRESULT h_r = S_OK;
    
    xstr_Infinitive = str_SourceForm;
    xstr_InfStem = str_GraphicStem;

    switch (i_Type)
    {
        case 1:         // делать, читать(ся), верстать, терять, жалеть, читать, стараться
        {
            xstr_1SgStem = xstr_3SgStem = str_GraphicStem;
            xstr_1SgEnding = L"ю";
            break;
        }

        case 2:         // требовать, рисовать(ся), тушевать, малевать, ковать, жевать, клевать, радоваться
        {
            xstr_1SgStem = xstr_3SgStem = wstring (str_GraphicStem).erase (xstr_InfStem.length()-3, 3);
            if (xstr_Infinitive.b_EndsWith (L"овать"))
            {
                xstr_1SgStem += L"у";
                xstr_3SgStem += L"у";
            }
            else if (xstr_Infinitive.b_EndsWith (L"евать"))
            {
                if (xstr_InfStem.b_EndsWithOneOf (str_ShSounds + L'ц'))
                {
                    xstr_1SgStem += L"у";
                    xstr_3SgStem += L"у";
                }
                else
                {
                    xstr_1SgStem += L"ю";
                    xstr_3SgStem += L"ю";
                }
            }
            xstr_1SgEnding = L"ю";
            break;
        }

        case 3:         // Without circle: (вы)тянуть, тронуть, стукнуть, гнуть, повернуть, тянуть
        {               // With circle: вымокнуть, (за)вянуть, (по)гибнуть, стыть//стынуть, достигнуть 
                        //              (достичь), свыкнуться
            xstr_1SgStem = xstr_3SgStem = wstring (str_GraphicStem).erase (xstr_InfStem.length()-1, 1);
            xstr_1SgEnding = L"у";
            break;
        }

        case 4:         // строить(ся), множить, грабить, тратить, смолить, крушить, томить, щадить, хоронить, 
                        // точить, ловить, просить, молиться

        case 5:         // (вы)стоять, выглядеть, слышать, видеть, кричать, бежать, звенеть, шуметь, висеть
        {
            h_r = h_StandardAlternation (xstr_1SgStem);
            if (S_OK != h_r)
            {
                return h_r;
            }

            xstr_3SgStem = wstring (str_GraphicStem).erase (xstr_InfStem.length()-1, 1);
            if (xstr_1SgStem.b_EndsWithOneOf (str_ShSounds))
            {
                xstr_1SgEnding = L"у";
            }
            else
            {
                xstr_1SgEnding = L"ю";
            }
            break;
        }

        case 6:         // Without circle: сеять(ся), глаголать, колебать, сыпать, прятать, смеяться, ржать, 
                        //                 слать, стлать, трепать, скакать, казаться
                        // With circle: (вы)сосать(ся), жаждать, рвать(ся), ткать, лгать, брать, звать, стонать
        {
            h_r = h_StandardAlternation (xstr_1SgStem);
            if (S_OK != h_r)
            {
                return h_r;
            }            

            xstr_3SgStem = xstr_1SgStem;
            if (xstr_1SgStem.b_EndsWithOneOf (str_ShSounds))
            {
                xstr_1SgEnding = L"у";
            }
            else
            {
                xstr_1SgEnding = L"ю";
            }
            break;
        }

        case 7:         // (вы)грызть, лезть, сесть, красть(ся), ползти, везти, пасти, нести, блюсти, вести, 
                        // мести(сь), (у)честь, расти, грести
        {
            if (xstr_InfStem.b_EndsWith (L"с"))
            {
                if (str_VerbStemAlternation.empty())
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Missing stem alternation for type 7");
                    return E_FAIL;
                }

                xstr_1SgStem = xstr_InfStem.substr (0, xstr_InfStem.length()-1) + str_VerbStemAlternation;
            }
            else
            {
                ATLASSERT (xstr_InfStem.b_EndsWith (L"з"));
                xstr_1SgStem = xstr_InfStem;
            }

            xstr_3SgStem = xstr_1SgStem;
            xstr_1SgEnding = L"у";
            break;
        }

        case 8:         // (вы)стричь, лечь, запрячь, беречь(ся), волочь, печь(ся), жечь, толочь, мочь
        {
            if (str_VerbStemAlternation.empty())
            {
                ATLASSERT(0);
                ERROR_LOG (L"Missing stem alternation for type 8");
                return E_FAIL;
            }

            xstr_1SgStem = xstr_InfStem += str_VerbStemAlternation;
            if (L"к" == str_VerbStemAlternation)
            {
                xstr_3SgStem = xstr_InfStem + L"ч";
            }
            else
            {
                if (L"г" == str_VerbStemAlternation)
                {
                    xstr_3SgStem = xstr_InfStem + L"ж";
                }
                else
                {
                    ATLASSERT(0);
                    ERROR_LOG (L"Bad stem finale");
                    return E_FAIL;
                }
            }
            xstr_1SgEnding = L"у";
            break;
        }

        case 9:             // (вы-, с)тереть(ся), запереть, умереть, отпереть
        {
            xstr_1SgStem = xstr_3SgStem = wstring (str_GraphicStem).erase (xstr_InfStem.length()-1, 1);
            xstr_1SgEnding = L"у";
            break;
        }
        case 10:            // (вы)пороть, (у)колоть(ся), молоть, бороться
        {
            xstr_1SgStem = xstr_3SgStem = wstring (str_GraphicStem).erase (xstr_InfStem.length()-1, 1);
            xstr_1SgEnding = L"ю";
            break;
        }
        case 11:            // шить(ся), (вы-, до)бить(ся), лить(ся), пить(ся)
        {
            xstr_1SgStem = xstr_3SgStem = wstring (str_GraphicStem).erase (xstr_InfStem.length()-1, 1);
            xstr_1SgEnding = L"ю";
            break;
        }
        case 12:            // выть, крыть, мыть, ныть,...
        {
            if (xstr_InfStem.b_EndsWith (L"ы"))
            {
                xstr_1SgStem = xstr_3SgStem = wstring (xstr_InfStem).replace (xstr_InfStem.length()-1, 1, L'о', 1);
            }
            xstr_1SgEnding = L"ю";
            break;
        }
        case 13:            // давать(ся), оставаться
        {
            xstr_1SgStem = xstr_3SgStem = xstr_InfStem;
            xstr_1SgEnding = L"ю";
            break;
        }
        case 14:            // мять(ся), жать(ся), сжать, отнять, занять, ...
        {
//&&&& #17
            if (str_VerbStemAlternation.empty())
            {
                    ATLASSERT(0);
                    ERROR_LOG (L"Missing stem alternation for type 14");
                    return E_FAIL;
            }

            if (L"н" != str_VerbStemAlternation && 
                L"м" != str_VerbStemAlternation && 
                L"им" != str_VerbStemAlternation)
            {
                xstr_1SgStem = xstr_3SgStem = str_VerbStemAlternation;
            }
            else
            {
                xstr_1SgStem = xstr_3SgStem = wstring (str_GraphicStem).erase (xstr_InfStem.length()-1, 1);
            }
            xstr_1SgEnding = L"у";
            break;
        }
        case 15:            // одеть, достать, остаться
        case 16:            // жить(ся), плыть, слыть
        {
            xstr_1SgStem = xstr_3SgStem = xstr_InfStem;            
            xstr_1SgEnding = L"у";
            break;
        }
        default:
        {
            ATLASSERT(0);
            ERROR_LOG (L"Unrecognized conjugation type.");
            return E_FAIL;
        }

    }

    if (S_OK != h_r)
    {
        return h_r;
    }

    return S_OK;

}    //  h_BuildVerbStems()

HRESULT CT_Lexeme::h_BuildVerbForms()
{
    HRESULT h_r = S_OK;
    
    for (ET_Number eo_number = NUM_SG; eo_number <= NUM_PL; ++eo_number)
    {
        for (ET_Person eo_person = PERSON_1; eo_person <= PERSON_3; ++eo_person)
        {
            ET_EndingStressType eo_stress = ENDING_STRESS_UNDEFINED;
            switch (eo_AccentType1)
            {
                case AT_A:
                {
                    eo_stress = ENDING_STRESS_UNSTRESSED;
                    break;
                }
                case AT_B:
                {
                    eo_stress = ENDING_STRESS_STRESSED;
                    break;
                }
                case AT_C:
                {
                    if (NUM_SG == eo_number && PERSON_1 == eo_person)
                    {
                        eo_stress = ENDING_STRESS_STRESSED;
                    }
                    else
                    {
                        eo_stress = ENDING_STRESS_UNSTRESSED;
                    }
                    break;
                }
                default:
                {
                    ERROR_LOG (L"Incompatible stress type.");
                    continue;
                }
            }
        
            h_r = h_BuildVerbStems();
            if (S_OK != h_r)
            {
                continue;
            }

            h_r = h_GetPersonalEndings();
            if (S_OK != h_r)
            {
                continue;
            }

            ST_EndingDescriptor st_d (eo_person, eo_number, eo_stress);
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
                    
                }

                CT_ExtString xstr_lemma;
                if (PERSON_1 == eo_person && NUM_SG == eo_number)
                {
                    xstr_lemma = xstr_1SgStem;
                }
                else
                {
                    if (4 == i_Type || 5 == i_Type)
                    {
                        xstr_lemma = xstr_3SgStem;
                    }
                    else
                    {
                        if (PERSON_3 == eo_person && NUM_PL == eo_number)
                        {
                            xstr_lemma = xstr_1SgStem;
                        }
                    }
                }

                CComObject<CT_WordForm> * sp_wordForm;
                h_r = CComObject<CT_WordForm>::CreateInstance (&sp_wordForm);
                if (S_OK != h_r)
                {
                    return h_r;
                }

                sp_wordForm->str_Lemma = xstr_lemma;
                CT_ExtString xstr_wf = xstr_lemma + str_ending;
                if (b_FleetingVowel)
                {
                    h_r = h_HandleFleetingVowel (xstr_wf);
                    if (S_OK != h_r)
                    {
                        continue;
                    }
                }
                sp_wordForm->eo_POS = eo_PartOfSpeech;
                sp_wordForm->str_WordForm = xstr_wf;
                sp_wordForm->eo_Person = eo_person;
                sp_wordForm->eo_Number = eo_number;
//                sp_wordForm->i_Stress = i_stressPos;

                m_coll.push_back (sp_wordForm);
            }
        }
    }

    return S_OK;

}   //  h_BuildVerbForms()

HRESULT CT_Lexeme::h_BuildLongForms()
{
    HRESULT h_r = S_OK;

    for (ET_Gender eo_gender = GENDER_M; eo_gender <= GENDER_N; ++eo_gender)
        for (ET_Number eo_number = NUM_SG; eo_number <= NUM_PL; ++eo_number)
            for (ET_Case eo_case = CASE_NOM; eo_case < CASE_NUM; ++eo_case)
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

                    ST_EndingDescriptor st_d (eo_gender, eo_number, eo_case, eo_animacy, eo_stress);
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

                        if (L"мс" == str_InflectionType && b_FleetingVowel)
                        {
                            h_r = h_HandleFleetingVowel (eo_number, 
                                                         eo_case, 
                                                         eo_gender, 
                                                         ADJ_FORM_LONG,
                                                         str_ending,
                                                         str_lemma);
                        }

                        int i_stressPos = -1;
                        h_r = h_GetStressPosition (str_lemma, 
                                                   str_ending, 
                                                   eo_stress, 
                                                   i_stressPos, 
                                                   false);   // not a short form
                        if (S_OK != h_r)
                        {
                            ERROR_LOG (L"Error getting stress position.");
                        }

                        CComObject<CT_WordForm> * sp_wordForm;
                        h_r = CComObject<CT_WordForm>::CreateInstance (&sp_wordForm);
                        if (S_OK != h_r)
                        {
                            return h_r;
                        }

                        sp_wordForm->eo_POS = eo_PartOfSpeech;
                        sp_wordForm->eo_AdjForm = ADJ_FORM_LONG;
                        sp_wordForm->str_Lemma = str_lemma;
                        sp_wordForm->str_WordForm = str_lemma + str_ending;
                        sp_wordForm->eo_Gender = eo_gender;
                        sp_wordForm->eo_Case = eo_case;
                        sp_wordForm->eo_Number = eo_number;
                        sp_wordForm->eo_Animacy = eo_animacy;
                        sp_wordForm->i_Stress = i_stressPos;

                        m_coll.push_back (sp_wordForm);

                    }   //  for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)
                }   //  for (ET_Animacy ...)

    return S_OK;

}   //  h_BuildLongForms()

HRESULT CT_Lexeme::h_BuildShortForms()
{
    HRESULT h_r = S_OK;

    h_r = h_GetShortFormEndings();
    if (S_OK != h_r)
    {
        return h_r;
    }

    for (ET_Gender eo_gender = GENDER_UNDEFINED; eo_gender <= GENDER_N; ++eo_gender)
    {
        ET_Number eo_number = (GENDER_UNDEFINED == eo_gender) ? NUM_PL : NUM_SG;

        vector<ET_EndingStressType> vec_eoStress;
        h_r = h_StressOnShortFormEnding (eo_number, eo_gender);
        switch (h_r)
        {
            case S_TRUE:
            {
                vec_eoStress.push_back (ENDING_STRESS_STRESSED);
                break;
            }
            case S_FALSE:
            {
                vec_eoStress.push_back (ENDING_STRESS_UNSTRESSED);
                break;
            }
            case S_BOTH:
            {
                vec_eoStress.push_back (ENDING_STRESS_STRESSED);
                vec_eoStress.push_back (ENDING_STRESS_UNSTRESSED);
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

        vector<ET_EndingStressType>::iterator it_stress = vec_eoStress.begin();
        for (; it_stress != vec_eoStress.end(); ++it_stress)
        {
            ST_EndingDescriptor st_d (eo_gender, eo_number, *it_stress);
            int i_numEndings = pco_Endings->i_Count (st_d);
            if (i_numEndings < 1)
            {
                ERROR_LOG (L"No endings");
                continue;
            }

            for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)
            {
                str_lemma = str_GraphicStem;
                std::wstring str_ending;
                h_r = pco_Endings->h_GetEnding (st_d, i_ending, str_ending);
                if (S_OK != h_r)
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

                int i_stressPos = -1;
                h_r = h_GetStressPosition (str_lemma, str_ending, *it_stress, i_stressPos, true);
                if (S_OK != h_r)                                                           // ^-- short form
                {
                    ERROR_LOG (L"Error getting stress position.");
                }

                if (b_YoAlternation)
                {
                    h_r = h_HandleYoAlternation (*it_stress, true, str_lemma);
                    if (S_OK != h_r)                        // ^-- short form
                    {
                        return h_r;
                    }
                }

                CComObject<CT_WordForm> * sp_wordForm;
                h_r = CComObject<CT_WordForm>::CreateInstance (&sp_wordForm);
                if (S_OK != h_r)
                {
                    return h_r;
                }

                sp_wordForm->eo_POS = eo_PartOfSpeech;
                sp_wordForm->eo_AdjForm = ADJ_FORM_SHORT;
                sp_wordForm->str_Lemma = str_lemma;
                sp_wordForm->str_WordForm = str_lemma + str_ending;
                sp_wordForm->eo_Gender = eo_gender;
                sp_wordForm->eo_Number = eo_number;
                sp_wordForm->i_Stress = i_stressPos;

                m_coll.push_back (sp_wordForm);

            }   //  for (int i_ending = 0; i_ending < i_numEndings; ++i_ending)

        }   //  for (; it_stress != vec_eoStress.end(); ++it_stress)

    }   //  for (ET_Gender eo_gender = GENDER_UNDEFINED; ...

    return S_OK;

}   //  h_BuildShortForms()

HRESULT CT_Lexeme::h_BuildComparative()
{
    HRESULT h_r = S_OK;

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

        sp_wordForm->eo_POS = eo_PartOfSpeech;
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
            i_stressPos = -1;
            h_r = h_GetStemStressPosition (str_lemma, i_stressPos);
            if (S_OK != h_r || i_stressPos < 0 || i_stressPos >= (int)str_lemma.length())
            {
                ATLASSERT(0);
                ERROR_LOG (L"h_GetStemStressPosition() failed");
                return E_FAIL;
            }
        }
        else
        {
            i_stressPos = str_lemma.length();
        }

        sp_wordForm1->eo_POS = eo_PartOfSpeech;
        sp_wordForm1->eo_AdjForm = ADJ_FORM_COMPARATIVE;
        sp_wordForm1->str_Lemma = str_lemma;
        sp_wordForm1->str_WordForm = str_lemma + L"ей";
        sp_wordForm1->i_Stress = i_stressPos;

        m_coll.push_back (sp_wordForm1);

        sp_wordForm1->eo_POS = eo_PartOfSpeech;
        sp_wordForm2->eo_AdjForm = ADJ_FORM_COMPARATIVE;
        sp_wordForm2->str_Lemma = str_lemma;
        sp_wordForm2->str_WordForm = str_lemma + L"ее";
        sp_wordForm2->i_Stress = i_stressPos;

        m_coll.push_back (sp_wordForm2);
    }

    return S_OK;

}   //  h_BuildComparative()

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
                    str_lemma[i_lastVowel] = L'ь';
                    return S_OK;
                }
                if (3 == i_Type)
                {
                    if ((i_lastVowel > 0) && 
                        (wstring::npos == wstring (str_ShSounds + L'ц').find (str_GraphicStem[i_lastVowel-1])))
                    {
                        str_lemma[i_lastVowel] = L'ь';   // зверек
                        return S_OK;
                    }
                }
                if ((i_lastVowel > 0) && (L'л' == str_GraphicStem[i_lastVowel-1]))
                {
                    str_lemma[i_lastVowel] = L'ь';       // лед, палец
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

                if (pco_Endings->i_Count (ST_EndingDescriptor (GENDER_UNDEFINED, 
                                                               NUM_PL, 
                                                               CASE_ACC, 
                                                               ANIM_UNDEFINED, 
                                                               eo_stress)) != 1)
                {
                    ERROR_LOG (L"Wrong number of endings.");
                }
                std::wstring str_endingPlAcc;
                h_r = pco_Endings->h_GetEnding (ST_EndingDescriptor (GENDER_UNDEFINED, 
                                                                     NUM_PL, 
                                                                     eo_case, 
                                                                     ANIM_UNDEFINED, 
                                                                     eo_stress), 
                                                str_endingPlAcc);
                if (S_OK != h_r)
                {
                    ERROR_LOG (L"GetEnding() failed.");
                }

                std::wstring str_endingPlGen;
                h_r = pco_Endings->h_GetEnding (ST_EndingDescriptor (GENDER_UNDEFINED, 
                                                                     NUM_PL, 
                                                                     CASE_GEN, 
                                                                     ANIM_UNDEFINED, 
                                                                     eo_stress),
                                                str_endingPlGen);
                if (S_OK != h_r)
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
                if (wstring::npos == wstring (str_ShSounds + L'ц').find (str_GraphicStem[i_lastConsonant-1]))
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
                if (wstring::npos != str_ShSounds.find (str_GraphicStem[i_lastConsonant-1]))
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

HRESULT CT_Lexeme::h_HandleFleetingVowel (CT_ExtString& xstr_verbForm)
{
// types 5, 6, 7, 8, 9, 11, 14
    bool b_preverb = false;
    bool b_voicing = false;

    vector<wstring>::iterator it_p = vec_AlternatingPreverbs.begin();
    for (; it_p != vec_AlternatingPreverbs.end()&&!b_preverb; ++it_p)
    {
        if (xstr_verbForm.b_StartsWith (*it_p))
        {
            b_preverb = true;
        }
    }

    if (!b_preverb)
    {
        it_p = vec_AlternatingPreverbsWithVoicing.begin();
        for (; it_p != vec_AlternatingPreverbsWithVoicing.end()&&!b_preverb; ++it_p)
        {
            if (xstr_verbForm.b_StartsWith (*it_p))
            {
                b_preverb = true;
                b_voicing = true;
            }
        }
    }

    if (!b_preverb)
    {
        return S_FALSE;
    }

    wstring str_preverb = *it_p;

    if (xstr_verbForm.length() < str_preverb.length() + 2)
    {
        ATLASSERT(0);
        ERROR_LOG (L"Stem too short.");
        return E_INVALIDARG;
    }

    // o after prefix?
    if (xstr_verbForm[str_preverb.length()-1] == L'о')
    {
        // Remove o if followed by CV
        if ((str_Consonants.find (xstr_verbForm[str_preverb.length()]) != wstring::npos) &&
            (str_Vowels.find (xstr_verbForm[str_preverb.length()+1]) != wstring::npos))
        {
            xstr_verbForm.erase (xstr_verbForm.begin() + str_preverb.length()-1);
        }
    }
    else
    {
        // Insert o if followed by CC or Cь
        if ((str_Consonants.find (xstr_verbForm[str_preverb.length()]) != wstring::npos) &&
            (wstring (str_Consonants + L'ь').find (xstr_verbForm[str_preverb.length()+1]) != wstring::npos))
        {
            xstr_verbForm.insert (xstr_verbForm.begin() + str_preverb.length(), L'о');
        }
    }

    return S_OK;

}   //  h_HandleFleetingVowel()

HRESULT CT_Lexeme::h_HandleYoAlternation (ET_EndingStressType eo_stress, 
                                          bool b_shortForm, 
                                          wstring& str_lemma)
{
    HRESULT h_r = S_OK;

    if (ENDING_STRESS_UNDEFINED == eo_stress)
    {
        ATLASSERT(0);
        ERROR_LOG (L"Undefined stress type.");
        return E_INVALIDARG;
    }

    int i_yoOffset = str_GraphicStem.find (_T("ё"));
    if (std::wstring::npos != i_yoOffset)
    {
        // Graphic stem contains yo: replace with e if stress is on ending
        if (ENDING_STRESS_STRESSED == eo_stress)
        {
            str_lemma[i_yoOffset] = L'е';        //  yo --> e: ежа, щелочей, тяжела
        }
    }
    else
    {
        // Graphic stem has no yo (must have at least on e)
        if (ENDING_STRESS_UNSTRESSED == eo_stress)
        {
            int i_eOffset = str_GraphicStem.rfind (_T("е"));    // last "e" in graphic stem (?)
            if (wstring::npos == i_eOffset)
            {
                ATLASSERT(0);
                ERROR_LOG (L"Unstressed stem with yo alternation has no e.");
                return E_FAIL;
            }

            int i_stress = -1;
            h_r = h_GetStemStressPosition (str_lemma, i_stress);
            if (S_OK != h_r)
            {
                ATLASSERT(0);
                ERROR_LOG (L"h_GetStemStressPosition() failed or bad stress position");
                return E_FAIL;
            }

            // Replace stressed e with yo:
            if (i_eOffset == i_stress)
            {
                str_lemma[i_eOffset] = L'ё';
            }
        }
    }

    return h_r;

}   //  h_HandleYoAlternation (...)

HRESULT CT_Lexeme::h_GetStressPosition (const wstring& str_lemma,
                                        const wstring& str_ending, 
                                        ET_EndingStressType eo_type, 
                                        int& i_position,
                                        bool b_shortForm)
{
    HRESULT h_r = S_OK;

    switch (eo_type)
    {
        case ENDING_STRESS_UNSTRESSED:
        {
            h_r = h_GetStemStressPosition (str_lemma, i_position, b_shortForm);
            ATLASSERT (str_Vowels.find (str_lemma[i_position]) != wstring::npos);
            break;
        }
        case ENDING_STRESS_STRESSED:
        {
            if (wstring::npos == str_ending.find_first_of (str_Vowels))
            {
                i_position = str_lemma.find_last_of (str_Vowels);
                if (wstring::npos == i_position)
                {
                    ERROR_LOG (L"Warning: can't find stressed vowel in comparative.");
                    i_position = -1;
                    return E_FAIL;
                }
            }
            else
            {
                int i_endingVowel = str_ending.find_first_of (str_Vowels);
                if (wstring::npos != i_endingVowel)
                {
                    i_position = str_lemma.length() + i_endingVowel;
                }
                else
                {
                    int i_position = str_lemma.find_last_of (str_Vowels);
                    if (wstring::npos == i_position)
                    {
                        ERROR_LOG (L"Warning: can't find stressed vowel in comparative.");
                        i_position = -1;
                        return E_FAIL;
                        
                    }
                }
            }
            break;
        }
        default:
        {
            ATLASSERT (0);
            ERROR_LOG (L"Illegal stress type.");
            return E_INVALIDARG;
        }
    }

    return h_r;

}   //  h_GetStressPosition (...) 

//
// This is only needed when the initial form has ending stress
//
HRESULT CT_Lexeme::h_GetStemStressPosition (const wstring& str_lemma, int& i_position, bool b_shortForm)
{
    HRESULT h_r = S_OK;

    if (i_SourceStressPos < (int)str_GraphicStem.length() && i_SourceStressPos >= 0)
    {
        i_position = i_SourceStressPos;
        return S_OK;
    }

    CT_ExtString xstr_stem (str_GraphicStem);
    xstr_stem.v_SetVowels (str_Vowels);
    int i_syllables = xstr_stem.i_NSyllables();
    if (i_syllables < 1)
    {
        ATLASSERT (0);
        ERROR_LOG (L"i_NSyllables() failed.");
        return E_FAIL;
    }

    int i_lastVowelPos = xstr_stem.i_GetVowelPos (i_syllables - 1);

    //
    // Trivial
    //
    if (1 == i_syllables)
    {
        i_position = i_lastVowelPos;
        return S_OK;
    }

    //
    // 2 or more syllables
    //
    ET_AccentType eo_accentType = b_shortForm ? eo_AccentType2 : eo_AccentType1;
    switch (eo_accentType)
    {
        case AT_A:
        case AT_A1:
        case AT_B:
        {
            ATLASSERT (0);
            ERROR_LOG (L"Unexpected accent type.");
            return E_UNEXPECTED;
        }
        case AT_B1:
        {
            i_position = i_lastVowelPos;
            return S_OK;
        }
        case AT_C:
        case AT_C1:
        {
            ATLASSERT (0);
            ERROR_LOG (L"Unexpected accent type.");
            return E_UNEXPECTED;
        }
        case AT_D:
        case AT_D1:
        {
            i_position = i_lastVowelPos;
            return S_OK;
        }
        case AT_E:
        {
            ATLASSERT (0);
            ERROR_LOG (L"Unexpected accent type.");
            return E_UNEXPECTED;
        }
        case AT_F:
        case AT_F1:
        case AT_F2:
        {
            unsigned int ui_firstVowel = xstr_stem.find_first_of (str_Vowels, 0);
            if (wstring::npos == ui_firstVowel)
            {
                ATLASSERT (0);
                ERROR_LOG (L"No vowels.");
                return E_FAIL;
            }
            i_position = ui_firstVowel;
            return S_OK;
        }
        default:
        {
            ATLASSERT (0);
            ERROR_LOG (L"Unexpected accent type.");
            return E_UNEXPECTED;
        }
    }

    return S_OK;

}   //  h_GetStemStressPosition (...)

HRESULT CT_Lexeme::h_StandardAlternation (CT_ExtString& xstr_presentStem)
{
    if (str_GraphicStem.length() <= 3)
    {
        ATLASSERT(0);
        ERROR_LOG (L"Graphic stem too short.");
        return E_INVALIDARG;
    }

    xstr_presentStem = str_GraphicStem;

    wstring str_finale;
    if (xstr_presentStem.b_EndsWith (L"ск"))
    {
        str_finale = L"ск";
    }
    else
    {
        if (xstr_presentStem.b_EndsWith (L"ст"))
        {
            str_finale = L"ст";
        }
        else
        {
            str_finale = xstr_presentStem[xstr_presentStem.length()-2];
        }
    }
    map<wstring, wstring>::iterator it_alt = map_StandardAlternations.find (str_finale);
    if (map_StandardAlternations.end() != it_alt)
    {
        if (L"щ" == str_VerbStemAlternation)
        {
            ATLASSERT (L"т" == str_finale);
            xstr_presentStem = str_VerbStemAlternation;
        }
        else
        {
            xstr_presentStem = xstr_presentStem.substr (0, xstr_presentStem.length()-2) + (*it_alt).second;
        }
    }

    return S_OK;

}   //  h_StandardAlternation()
