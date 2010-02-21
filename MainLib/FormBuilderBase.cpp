#include "StdAfx.h"
#include "FormBuilderBase.h"

struct fo_TypeEquals
{
    fo_TypeEquals (int i_type) : i_Type (i_type)
    {}

    bool operator() (const St_CommonDeviation& st_cd)
    {
        return st_cd.i_Type == i_Type;
    }

private:
    int i_Type;
};

HRESULT CT_FormBuilderBase::h_GetCommonDeviation (int i_type, St_CommonDeviation& st_data)
{
    HRESULT h_r = S_OK;

    vector<St_CommonDeviation>::iterator it_r = find_if (pco_Lexeme->vec_CommonDeviations.begin(), 
                                                         pco_Lexeme->vec_CommonDeviations.end(), 
                                                         fo_TypeEquals (i_type));
    if (pco_Lexeme->vec_CommonDeviations.end() != it_r)
    {
        st_data = *it_r;
        h_r = S_OK;
    }
    else
    {
        h_r = S_FALSE;
    }

    return h_r;
}

void CT_FormBuilderBase::v_ReporDbError()
{
    if (!pco_Db)
    {
        ATLASSERT(0);
        ERROR_LOG (L"No DB handle");
        return;
    }

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
}

HRESULT CT_FormBuilderBase::h_HandleDeclFleetingVowel (ET_Number eo_number,               // in
                                                       ET_Case eo_case,                   // in
                                                       ET_Gender eo_gender,               // in
                                                       ET_EndingStressType eo_stressType, // in
                                                       ET_Subparadigm eo_subparadigm,     // in
                                                       const wstring& str_ending,         // in
                                                       wstring& str_lemma)                // out
{
    HRESULT h_r = S_OK;

    if (SUBPARADIGM_LONG_ADJ != eo_subparadigm && 
        SUBPARADIGM_SHORT_ADJ != eo_subparadigm && 
        SUBPARADIGM_UNDEFINED != eo_subparadigm &&
        SUBPARADIGM_NOUN != eo_subparadigm)
    {
        ATLASSERT(0);
        ERROR_LOG (L"Illegal subparadigm.");
        return E_UNEXPECTED;
    }

    int i_type = pco_Lexeme->i_Type;
    wstring str_grStem = pco_Lexeme->str_GraphicStem;
    wstring str_inflection = pco_Lexeme->str_InflectionType;

    int i_lastVowel = str_grStem.find_last_of (str_Vowels);

    //
    // Fleeting vowel in initial form GDRL, p. 29-30
    //

    // ASSERT: last stem vowel is { о, е, ё, и } [also а, я "in several anomalous words"]
    if (GENDER_M == eo_gender ||
        (GENDER_F == eo_gender && 8 == pco_Lexeme->i_Type) ||
        L"мс" == str_inflection)
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
            if (GENDER_M != eo_gender)
            {
                ATLASSERT(0);
                ERROR_LOG (L"Enexpected gender for null ending");
                return E_UNEXPECTED;
            }
            return S_OK;
        }

        if (L"ь" == str_ending)
        {
            if ((L"мс" != str_inflection) || 
                (GENDER_F != eo_gender || 8 != i_type))
            {
                ATLASSERT(0);
                ERROR_LOG (L"Enexpected type for ending 'ь'");
                return E_UNEXPECTED;

            }
            return S_OK;
        }

        if (L"ью" == str_ending)
        {
            if (GENDER_F != eo_gender || 8 != i_type)
            {
                ATLASSERT(0);
                ERROR_LOG (L"Enexpected main symbol for ending 'ью'");
                return E_UNEXPECTED;
            }
            return S_OK;
        }

        if (L"й" == str_ending)
        {
            if (L"мс" != str_inflection)
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

        if (str_grStem[i_lastVowel] == L'о')   // сон, любовь
        {
            str_lemma.erase (i_lastVowel, 1);
            return S_OK;
        }

        if (str_grStem[i_lastVowel] == L'и')   // волчий
        {
            if ((L"мс" != str_inflection) || (6 != i_type))
            {
                ERROR_LOG (L"Unexpected inflection type.");
                str_lemma.clear();
                return E_UNEXPECTED;
            }
            str_lemma[i_lastVowel] = L'ь';
            return S_OK;
        }

        if ((str_grStem[i_lastVowel] == L'е') || 
            (str_grStem[i_lastVowel] == L'ё'))
        {
            if (i_lastVowel > 0)
            {
                if (wstring::npos != str_Vowels.find (str_grStem[i_lastVowel-1]))
                {
                    str_lemma[i_lastVowel] = L'й';   // боец, паек
                    return S_OK;
                }
            }
            if (GENDER_M == eo_gender)
            {
                if (6 == i_type)                    // улей
                {
                    str_lemma = str_grStem;
                    str_lemma[i_lastVowel] = L'ь';
                    return S_OK;
                }
                if (3 == i_type)
                {
                    if ((i_lastVowel > 0) && 
                        (wstring::npos == wstring (str_ShSounds + L'ц').find (str_grStem[i_lastVowel-1])))
                    {
                        str_lemma[i_lastVowel] = L'ь';   // зверек
                        return S_OK;
                    }
                }
                if ((i_lastVowel > 0) && (L'л' == str_grStem[i_lastVowel-1]))
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
    if ((GENDER_F == eo_gender && 8 != i_type && NUM_PL == eo_number) || 
        (GENDER_N == eo_gender && NUM_PL == eo_number) || 
        (L"п" == str_inflection || L"мн." == pco_Lexeme->str_MainSymbol))
    {
        if (POS_NOUN == eo_subparadigm)
        {
            if (eo_case == CASE_ACC)
            {

                if (pco_Endings->i_Count (ST_EndingDescriptor (GENDER_UNDEFINED, 
                                                               NUM_PL, 
                                                               CASE_ACC, 
                                                               ANIM_UNDEFINED, 
                                                               eo_stressType)) != 1)
                {
                    ERROR_LOG (L"Wrong number of endings.");
                }
                std::wstring str_endingPlAcc;
                h_r = pco_Endings->h_GetEnding (ST_EndingDescriptor (GENDER_UNDEFINED, 
                                                                     NUM_PL, 
                                                                     eo_case, 
                                                                     ANIM_UNDEFINED, 
                                                                     eo_stressType), 
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
                                                                     eo_stressType),
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

        if (L"п" == str_inflection)
        {
            if (SUBPARADIGM_SHORT_ADJ != eo_subparadigm || GENDER_M != eo_gender)
            {
                return S_OK;
            }
        }

        if (((GENDER_F == eo_gender) || (GENDER_N == eo_gender)) && (6 == i_type))
        {
            if (ENDING_STRESS_STRESSED == eo_stressType)
            {
                str_lemma[str_lemma.length()-1] = L'е';   // статей, питей
            }
            else
            {
                str_lemma[str_lemma.length()-1] = L'и';   // гостий, ущелий
            }
            return S_OK;
        }

        int i_lastConsonant = str_grStem.find_last_of (str_Consonants);
        if (i_lastConsonant > 0)
        {
            if ((L'ь' == str_grStem[i_lastConsonant-1]) || 
                (L'й' == str_grStem[i_lastConsonant-1]))
            {
                if (L'ц' == str_grStem[i_lastConsonant])
                {
                    str_lemma[i_lastConsonant-1] = L'е';    // колец
                    return S_OK;
                }

                if (ENDING_STRESS_STRESSED == eo_stressType)
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
            if (L'к' == str_grStem[i_lastConsonant-1] || 
                L'г' == str_grStem[i_lastConsonant-1] ||
                L'х' == str_grStem[i_lastConsonant-1])
            {
                str_lemma.insert (i_lastConsonant, 1, L'о');  // кукол, окон, мягок
                return S_OK;
            }
            if (L'к' == str_grStem[i_lastConsonant] ||
                L'г' == str_grStem[i_lastConsonant] ||
                L'х' == str_grStem[i_lastConsonant])
            {
                if (3 != i_type)
                {
                    ERROR_LOG (_T("Unexpected type."));
                    return E_FAIL;
                }
                if (wstring::npos == wstring (str_ShSounds + L'ц').find (str_grStem[i_lastConsonant-1]))
                {
                    str_lemma.insert (i_lastConsonant, 1, L'о');  // сказок, ведерок, краток, долог
                    return S_OK;
                }
            }
            if (L'ц' == str_grStem[i_lastConsonant])
            {
                str_lemma.insert (i_lastConsonant, 1, L'е');      // овец
                return S_OK;
            }

            if (ENDING_STRESS_STRESSED == eo_stressType)
            {
                if (wstring::npos != str_ShSounds.find (str_grStem[i_lastConsonant-1]))
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

}   //  h_HandleDeclFleetingVowel (...)

//
// This is only needed when the initial form has ending stress
//
HRESULT CT_FormBuilderBase::h_GetDeclStemStressPosition (const wstring& str_lemma, 
                                                         vector<int>& vec_iPositions, 
                                                         ET_Subparadigm eo_subParadigm)
{
    HRESULT h_r = S_OK;

    vector<int>::iterator it_pos = pco_Lexeme->vec_SourceStressPos.begin();
    for (; it_pos != pco_Lexeme->vec_SourceStressPos.end() && 
           *it_pos >= 0 && 
           *it_pos < (int)pco_Lexeme->str_GraphicStem.length(); 
           ++it_pos)
    {
        vec_iPositions.push_back (*it_pos);
    }

    if (pco_Lexeme->vec_SourceStressPos.end() == it_pos)
    {
        return S_OK;
    }

    CT_ExtString xstr_stem (pco_Lexeme->str_GraphicStem);
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
        vec_iPositions.push_back (i_lastVowelPos);
        return S_OK;
    }

    //
    // 2 or more syllables
    //
    ET_AccentType eo_accentType = (SUBPARADIGM_SHORT_ADJ == eo_subParadigm) 
                                    ? pco_Lexeme->eo_AccentType2 
                                    : pco_Lexeme->eo_AccentType1;
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
            vec_iPositions.push_back (i_lastVowelPos);
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
            vec_iPositions.push_back (i_lastVowelPos);
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
            vec_iPositions.push_back ((int)ui_firstVowel);
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

}   //  h_GetDeclStemStressPosition (...)

HRESULT CT_FormBuilderBase::h_GetDeclEndingStressPosition (const wstring& str_lemma, 
                                                           const wstring& str_ending,
                                                           int& i_position)
{
    int i_endingVowel = str_ending.find_first_of (str_Vowels);
    if (wstring::npos != i_endingVowel)
    {
        i_position = str_lemma.length() + i_endingVowel;
    }
// &&&& pronominal decl -ogo -ego -omu -emu + samoyo, eyo, neyo
    else
    {
        int i_position = str_lemma.find_last_of (str_Vowels);
        if (wstring::npos == i_position)
        {
            ERROR_LOG (L"Warning: can't find stressed vowel.");
            i_position = -1;
            return E_FAIL;                        
        }
    }

    return S_OK;

}   //  h_GetDeclEndingStressPosition (...)

HRESULT CT_FormBuilderBase::h_HandleDeclYoAlternation (ET_EndingStressType eo_stressType, 
                                                       int i_stressPos,
                                                       ET_Subparadigm eo_subParadigm, 
                                                       wstring& str_lemma)
{
    HRESULT h_r = S_OK;

// TODO: do we need the test below??
//    if (!b_YoAlternation && !b_OAlternation)
//    {
//        ERROR_LOG (L"No o/yo alternation.");
//        return E_UNEXPECTED;                        
//    }

    if (ENDING_STRESS_UNDEFINED == eo_stressType)
    {
        ATLASSERT(0);
        ERROR_LOG (L"Undefined stress type.");
        return E_INVALIDARG;
    }

    int i_yoOffset = pco_Lexeme->str_GraphicStem.find (_T("ё"));
    if (std::wstring::npos != i_yoOffset)
    {
        // Graphic stem contains yo: replace with e if stress is on ending
        if (ENDING_STRESS_STRESSED == eo_stressType)
        {
            str_lemma[i_yoOffset] = L'е';        //  yo --> e: ежа, щелочей, тяжела
        }
    }
    else
    {
        // Graphic stem has no yo (must have at least on e)
        if (ENDING_STRESS_UNSTRESSED == eo_stressType)
        {
            int i_eOffset = pco_Lexeme->str_GraphicStem.rfind (_T("е"));    // last "e" in graphic stem (?)
            if (wstring::npos == i_eOffset)
            {
                ATLASSERT(0);
                ERROR_LOG (L"Unstressed stem with yo alternation has no e.");
                return E_FAIL;
            }

            // Replace stressed e with yo:
            if (i_eOffset == i_stressPos)
            {
                str_lemma[i_eOffset] = L'ё';
            }
        }
    }

    return h_r;

}   //  h_HandleDeclYoAlternation (...)

HRESULT CT_FormBuilderBase::h_GetVerbStemStressPosition (const wstring& str_lemma, 
                                                         vector<int>& vec_iPosition)
{
    //
    // Find the sequence # of the stressed vowel in infinitive
    //
    vector<int>& vec_sourceStressPos = pco_Lexeme->vec_SourceStressPos;

    vector<int>::iterator it_sourceStressPos = vec_sourceStressPos.begin();
    for (; it_sourceStressPos != vec_sourceStressPos.end(); ++it_sourceStressPos)
    {
        int i_stressPos = -1;
        if (*it_sourceStressPos >= (int)pco_Lexeme->str_SourceForm.length())
        {
            ATLASSERT (0);
            ERROR_LOG (L"Illegal source stress position.");
            return E_FAIL;
        }

        CT_ExtString xstr_infinitive (pco_Lexeme->str_SourceForm);
        xstr_infinitive.v_SetVowels (str_Vowels);
        int i_syllables = xstr_infinitive.i_NSyllables();
        int i_stressedSyllable = 0;
        for (; i_stressedSyllable < i_syllables; ++i_stressedSyllable)
        {
            i_stressPos = xstr_infinitive.i_GetVowelPos (i_stressedSyllable);
            if (*it_sourceStressPos == i_stressPos)
            {
                break;
            }
        }

        if (i_stressedSyllable == i_syllables)
        {
            ATLASSERT (0);
            i_stressPos = -1;
            ERROR_LOG (L"Illegal source stress position.");
            return E_FAIL;
        }

        CT_ExtString xstr_lemma (str_lemma);
        xstr_lemma.v_SetVowels (str_Vowels);
        i_syllables = xstr_lemma.i_NSyllables();
        if (i_syllables < 1)
        {
            ATLASSERT (0);
            i_stressPos = -1;
            ERROR_LOG (L"No vowels in verb stem.");
            return E_FAIL;
        }

        if (i_stressedSyllable > i_syllables)
        {
            i_stressPos = xstr_lemma.i_GetVowelPos (i_syllables-1);  // last syllable
        }
        else
        {
            i_stressPos = xstr_lemma.i_GetVowelPos (i_stressedSyllable); // same syll as in infinitive
        }

        vec_iPosition.push_back (i_stressPos);

    }   //  for (; it_sourceStressPos != vec_SourceStressPos.end(); ... )

    return S_OK;

}   //  h_GetVerbStemStressPosition (...)

HRESULT CT_FormBuilderBase::h_GetVerbEndingStressPosition (const wstring& str_lemma, 
                                                           const wstring& str_ending,
                                                           int& i_position)
{
   HRESULT h_r = S_OK;

    if (POS_VERB != pco_Lexeme->eo_PartOfSpeech)
    {
        ATLASSERT (0);
        ERROR_LOG (L"Unexpected part of speech.");
        return E_UNEXPECTED;
    }

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
            ERROR_LOG (L"Warning: can't find stressed vowel.");
            i_position = -1;
            return E_FAIL;                        
        }
// &&&& Past masc. + -sya + c'': sorvalsyA
    }

    return h_r;

}   //  h_GetVerbEndingStressPosition (...)

HRESULT CT_FormBuilderBase::h_BuildPastTenseStem (CT_ExtString& xstr_lemma)
{
    HRESULT h_r = S_OK;

    int i_type = pco_Lexeme->i_Type;
    CT_ExtString& xstr_1SgStem = pco_Lexeme->xstr_1SgStem;
    CT_ExtString& xstr_infStem = pco_Lexeme->xstr_InfStem;

    if (7 == i_type || 8 == i_type)
    {
        xstr_lemma = xstr_1SgStem;
        if (xstr_lemma.b_EndsWithOneOf (L"тд"))
        {
            xstr_lemma = xstr_lemma.erase (xstr_lemma.length()-1);
        }
    }
    else
    {
        xstr_lemma = xstr_infStem;
    }
    if (9 == i_type)
    {
        ATLASSERT (xstr_lemma.b_EndsWith (L"е"));
        xstr_lemma = xstr_lemma.erase (xstr_lemma.length()-1);
    }
    if (3 == i_type && pco_Lexeme->b_StemAugment)
    {
        ATLASSERT (xstr_lemma.b_EndsWith (L"ну"));
        xstr_lemma = xstr_lemma.erase (xstr_lemma.length()-2);
    }

    return h_r;

}   //  h_BuildPastTenseStem (...)

HRESULT CT_FormBuilderBase::h_StressOnPastTenseEnding (ET_AccentType eo_accentType,
                                                       ET_Number eo_number, 
                                                       ET_Gender eo_gender)
{
    HRESULT h_r = S_OK;

    if (POS_VERB != pco_Lexeme->eo_PartOfSpeech)
    {
        ATLASSERT(0);
        ERROR_LOG (L"Unexpected part of speech value.");
        return E_FAIL;
    }

    vector<ET_EndingStressType> vec_stress;
    switch (eo_accentType)
    {
        case AT_A:
        {
            return S_FALSE;
        }
        case AT_B:
        {
            return S_TRUE;
        }
        case AT_C:
        {
            if (NUM_SG == eo_number && GENDER_N == eo_gender)
            {
                return S_FALSE;
            }
            if (NUM_PL == eo_number)
            {
                return S_FALSE;
            }
            return S_TRUE;
        }
        case AT_C2:
        {
            if (!pco_Lexeme->b_Reflexive)
            {
                ATLASSERT(0);
                ERROR_LOG (L"Accent type C2 with a non-reflexive verb.");
                return h_r;
            }
            if (!(GENDER_F == eo_gender && NUM_SG == eo_number))
            {
                return S_BOTH;
            }
            return S_TRUE;
        }
        default:
        {
            ERROR_LOG (L"Unexpected past tense accent type.");
            return h_r;
        }

        return E_FAIL;

    }   // switch (eo_accentType)

}   //  h_StressOnPastTenseEnding (...)

HRESULT CT_FormBuilderBase::h_CloneWordForm (CComObject<CT_WordForm> * p_source, 
                                             CComObject<CT_WordForm> *& p_clone)
{
    HRESULT h_r = S_OK;

    h_r = CComObject<CT_WordForm>::CreateInstance (&p_clone);
    if (S_OK != h_r)
    {
        ATLASSERT(0);
        return h_r;
    }
    p_clone->str_WordForm = p_source->str_WordForm;
    p_clone->str_Lemma = p_source->str_Lemma;
    p_clone->ll_Lexeme_id = p_source->ll_Lexeme_id;
    p_clone->vec_Stress = p_source->vec_Stress;
    p_clone->eo_POS = p_source->eo_POS;
    p_clone->eo_Case = p_source->eo_Case;
    p_clone->eo_Number = p_source->eo_Number;
    p_clone->eo_Subparadigm = p_source->eo_Subparadigm;
    p_clone->eo_Gender = p_source->eo_Gender;
    p_clone->eo_Tense = p_source->eo_Tense;
    p_clone->eo_Person = p_source->eo_Person;
    p_clone->eo_Animacy = p_source->eo_Animacy;
    p_clone->eo_Reflexive = p_source->eo_Reflexive;
    p_clone->eo_Voice = p_source->eo_Voice;

    return S_OK;
}
