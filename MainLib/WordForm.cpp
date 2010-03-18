#include "StdAfx.h"
#include "WordForm.h"

HRESULT CT_WordForm::get_Wordform (BSTR * pbstr_wordForm)
{
    if (str_WordForm.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_WordForm.c_str());
    *pbstr_wordForm = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_WordForm::get_Lemma (BSTR * pbstr_lemma)
{
    if (str_Lemma.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_Lemma.c_str());
    *pbstr_lemma = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_WordForm::get_LexemeId (LONG *)
{
    return E_NOTIMPL;
}

HRESULT CT_WordForm::get_PartOfSpeech (ET_PartOfSpeech * pe_partOfSpeech)
{
    *pe_partOfSpeech = (ET_PartOfSpeech)eo_POS;
    return S_OK;
}

HRESULT CT_WordForm::get_Subparadigm (ET_Subparadigm * pe_subparadigm)
{
    *pe_subparadigm = (ET_Subparadigm)eo_Subparadigm;
    return S_OK;
}

HRESULT CT_WordForm::get_Case (ET_Case * pe_case)
{
    *pe_case = (ET_Case)eo_Case;
    return S_OK;
}

HRESULT CT_WordForm::get_Number (ET_Number * pe_number)
{
    *pe_number = (ET_Number)eo_Number;
    return S_OK;
}

HRESULT CT_WordForm::get_Gender (ET_Gender * pe_gender)
{
    *pe_gender = (ET_Gender)eo_Gender;
    return S_OK;
}

HRESULT CT_WordForm::get_Tense (ET_Tense * pe_tense)
{
    *pe_tense = (ET_Tense)eo_Tense;
    return S_OK;
}

HRESULT CT_WordForm::get_Person (ET_Person * pe_person)
{
    *pe_person = (ET_Person)eo_Person;
    return S_OK;
}

HRESULT CT_WordForm::get_IsReflexive (ET_Reflexive * pe_reflexive)
{
    *pe_reflexive = eo_Reflexive;
    return S_OK;
}

HRESULT CT_WordForm::get_Animacy (ET_Animacy * pe_animacy)
{
    *pe_animacy = (ET_Animacy)eo_Animacy;
    return S_OK;
}

HRESULT CT_WordForm::get_Voice (ET_Voice * pe_voice)
{
    *pe_voice = (ET_Voice)eo_Voice;
    return S_OK;
}

HRESULT CT_WordForm::get_SpecialForm (ET_SpecialForm * pe_specialForm)
{
    *pe_specialForm = (ET_SpecialForm)eo_SpecialForm;
    return S_OK;
}

HRESULT CT_WordForm::get_Aspect (ET_Aspect * pe_aspect)
{
    *pe_aspect = (ET_Aspect)eo_Aspect;
    return S_OK;
}

HRESULT CT_WordForm::get_Stress (LONG * pl_stressPos)
{
    map<int, bool>::iterator it_pos = map_Stress.begin();
    while (!it_pos->second)
    {
        ++it_pos;
    }

    if (map_Stress.end() == it_pos)
    {
        return E_FAIL;
    }

    *pl_stressPos = it_pos->first;

    if (map_Stress.size() > 1)
    {
        return S_MORE;
    }
    else
    {
        return S_OK;
    }

}   //  get_Stress (...)

HRESULT CT_WordForm::GetStressPos (LONG l_At, LONG * pl_StressPosition, BOOL * pb_IsPrimary)
{
    map<int, bool>::iterator it_stressPos = map_Stress.find (l_At);
    if (map_Stress.end() != it_stressPos)
    {
        *pl_StressPosition = it_stressPos->first;
        *pb_IsPrimary = it_stressPos->second;
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

HRESULT CT_WordForm::get_StressCount (LONG * pl_val)
{
    *pl_val = map_Stress.size();
    return S_OK;
}

HRESULT CT_WordForm::get_IsPrimaryStress (LONG l_pos, BOOL * pb_type)
{
    map<int, bool>::iterator it_ = map_Stress.find (l_pos);
    if (map_Stress.end() == it_)
    {
        return E_INVALIDARG;
    }
    else
    {
        *pb_type = it_->second ? TRUE : FALSE;
        return S_OK;
    }
}

/////////////////////////////////////////////////////////////////

int CT_WordForm::i_GramHash()
{
    CT_Hasher co_h (*this);
    return co_h.i_GramHash();
}

HRESULT CT_WordForm::h_DecodeHash (int i_hash)
{
    try
    {
        CT_Hasher co_h (*this);
        return co_h.h_DecodeHash (i_hash);
    }
    catch (CT_Exception co_ex)
    {
        return co_ex.i_GetErrorCode();  // logging should be always done by callee
    }
}


/*
HRESULT CT_WordForm::get_Stress (bool * pl_pos, bool * pb_isPrimary)
{
    if ((i_at < 0) || (vec_Stress.size() <= (unsigned int)i_at))
    {
        *pl_stressPos = -1;
        return E_INVALIDARG;
    }

    *pl_stressPos = vec_Stress[i_at];
    if (vec_Stress.size() > (unsigned int)i_at + 1)
    {
        return S_MORE;
    }
    else
    {
        return S_OK;
    }
}
*/

// -----------------  Tim's code  --------------------

/*
HRESULT CT_WordForm::GramClear ()
{
    eo_POS = POS_UNDEFINED;
    eo_Case = CASE_UNDEFINED;
    eo_Number = NUM_UNDEFINED;
    eo_Subparadigm = SUBPARADIGM_UNDEFINED;
    eo_Gender = GENDER_UNDEFINED;
    eo_Tense = TENSE_UNDEFINED;
    eo_Person = PERSON_UNDEFINED;
    eo_Animacy = ANIM_UNDEFINED;
    eo_Reflexive = REFL_UNDEFINED;
    eo_Voice = VOICE_UNDEFINED;
    eo_SpecialForm = SPECIAL_UNDEFINED;
    eo_Aspect = ASPECT_UNDEFINED;
    return S_OK;
}

HRESULT CT_WordForm::Clear()
{
    GramClear();
    str_WordForm = L"";
    str_Lemma = L"";
    ll_Lexeme_id = -1;
    map_Stress.clear();

    return S_OK;
}

HRESULT CT_WordForm::get_GramHash (long * pl_hash)
{
    // Ranges:
    //          0-999: Nouns
    //      1000-1999: Adjectives
    //      2000-2999: Numerals
    //      3000-3999: Pronouns
    //     4000-19999: Verbs
    // 20000-infinity: all the rest (unchangeable Parts of Speech)

    long l_shift = 0;

    if (eo_POS == POS_UNDEFINED || eo_POS == POS_ADV || eo_POS == POS_PREP ||
        eo_POS == POS_CONJ || eo_POS == POS_PARTICLE || eo_POS == POS_COMPAR ||
        eo_POS == POS_INTERJ || eo_POS == POS_PARENTH)
    {
        *pl_hash = MIN_UNCHANGEABLE + (long)eo_POS;
        return S_OK;
    }
    long l_Case;
    if (eo_POS == POS_NOUN) // 40 possibilities
    {
        *pl_hash = MIN_NOUN + eo_Case * NUM_COUNT + eo_Number;
        return S_OK;
    }
    if (eo_POS == POS_ADJ || eo_POS == POS_NUM || eo_POS == POS_PRONOUN) // 264 possibilities
    {
        if (eo_Case != CASE_ACC || eo_Animacy != ANIM_YES)
        {
            l_Case = (long)eo_Case;
        }
        else
        {
            l_Case = CASE_COUNT; // In fact, it means Accusative case of an animate adjective
        }
        if (eo_POS == POS_ADJ)
        {
            l_shift = MIN_ADJ;
        }
        else if (eo_POS == POS_NUM)
        {
            l_shift = MIN_NUM;
        }
        else if (eo_POS == POS_PRONOUN)
        {
            l_shift = MIN_PRONOUN;
        }
        *pl_hash = l_shift + eo_Subparadigm * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT
                + l_Case * NUM_COUNT * GENDER_COUNT + eo_Number * GENDER_COUNT + eo_Gender;
        return S_OK;
    }
    if (eo_POS == POS_VERB)
    {
        if (eo_SpecialForm == SPECIAL_UNDEFINED)
        {
            return E_FAIL;
        }
        if (eo_SpecialForm == SPECIAL_NORMAL) // 144 possibilities
        {
            // It is assumed that GENDER_COUNT == PERSON_COUNT
            if (eo_Tense != TENSE_PAST)
            {
                *pl_hash = MIN_VERB + eo_Tense * NUM_COUNT * PERSON_COUNT * REFL_COUNT
                        + eo_Number * PERSON_COUNT * REFL_COUNT + eo_Person * REFL_COUNT + eo_Reflexive;
                return S_OK;
            }
            else
            {
                *pl_hash = MIN_VERB + TENSE_PAST * NUM_COUNT * GENDER_COUNT * REFL_COUNT 
                        + eo_Number * GENDER_COUNT * REFL_COUNT + eo_Gender * REFL_COUNT + eo_Reflexive;
                return S_OK;
            }
        }
        if (eo_SpecialForm == SPECIAL_IMPERATIVE) // 36 possibilities
        {
            *pl_hash = MIN_VERB + eo_Number * PERSON_COUNT * REFL_COUNT + eo_Person * REFL_COUNT + eo_Reflexive
                    + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT;
            return S_OK;
        }
        if (eo_SpecialForm == SPECIAL_PARTICIPLE) // 4752 possibilities
        {
            if (eo_Case != CASE_ACC || eo_Animacy != ANIM_YES)
            {
                l_Case = (long)eo_Case;
            }
            else           
            {
                l_Case = CASE_COUNT; // In fact, it means Accusative case of an animate participle
            }
            *pl_hash = MIN_VERB +
                eo_Tense * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT + 
                l_Case * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT + 
                eo_Number * GENDER_COUNT * VOICE_COUNT * REFL_COUNT + 
                eo_Gender * VOICE_COUNT * REFL_COUNT + 
                eo_Voice * REFL_COUNT + 
                eo_Reflexive + 
                TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
            return S_OK;
        }
        if (eo_SpecialForm == SPECIAL_CONVERB) // 12 possibilities
        {
            *pl_hash = MIN_VERB + eo_Tense * REFL_COUNT + eo_Reflexive 
                    + TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                    + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                    + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
            return S_OK;
        }
        if (eo_SpecialForm == SPECIAL_INFINITIVE) // 2 possibilities
        {
            *pl_hash = MIN_VERB + eo_Reflexive + TENSE_COUNT * REFL_COUNT 
                    + TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                    + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                    + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
            return S_OK;
        }
    }
    
    if (eo_SpecialForm == SPECIAL_COMPARATIVE) // 1 possibility
    {
        *pl_hash = MIN_VERB + REFL_COUNT + TENSE_COUNT * REFL_COUNT 
                + TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
        return S_OK;
        //In the original version, this used to give 4947;
    }
    return E_FAIL;
} // get_GramHash

HRESULT CT_WordForm::DecodeHash (long l_hash)
{
    // Ranges:
    //          0-999: Nouns
    //      1000-1999: Adjectives
    //      2000-2999: Numerals
    //      3000-3999: Pronouns
    //     4000-19999: Verbs
    // 20000-infinity: all the rest (unchangeable Parts of Speech)

    long l_shift = 0;
    GramClear();
    if (l_hash >= MIN_NOUN && l_hash < MIN_ADJ)
    {
        eo_POS = POS_NOUN;
    }
    else if (l_hash >= MIN_ADJ && l_hash < MIN_NUM)
    {
        eo_POS = POS_ADJ;
        l_shift = MIN_ADJ;
    }
    else if (l_hash >= MIN_NUM && l_hash < MIN_PRONOUN)
    {
        eo_POS = POS_NUM;
        l_shift = MIN_NUM;
    }
    else if (l_hash >= MIN_PRONOUN && l_hash < MIN_VERB)
    {
        eo_POS = POS_PRONOUN;
        l_shift = MIN_PRONOUN;
    }
    else if (l_hash >= MIN_VERB && l_hash < MIN_UNCHANGEABLE)
    {
        eo_POS = POS_VERB;
        l_shift = MIN_VERB;
    }
    else if (l_hash >= MIN_UNCHANGEABLE && l_hash < MIN_UNCHANGEABLE + POS_COUNT)
    {
        eo_POS = (ET_PartOfSpeech)(l_hash - MIN_UNCHANGEABLE);
        return S_OK;
    }
    l_hash -= l_shift;
    long l_hash_max = REFL_COUNT + TENSE_COUNT * REFL_COUNT 
                + TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
    if (l_hash < 0 || l_hash > l_hash_max)
    {
        return E_FAIL;
    }
    
    if (eo_POS == POS_UNDEFINED || eo_POS == POS_ADV || eo_POS == POS_PREP ||
        eo_POS == POS_CONJ || eo_POS == POS_PARTICLE || eo_POS == POS_COMPAR ||
        eo_POS == POS_INTERJ || eo_POS == POS_PARENTH)
    {
        return S_OK;
    }
    if (eo_POS == POS_NOUN) // 40 possibilities
    {
        eo_Number = (ET_Number)(l_hash % NUM_COUNT);
        eo_Case = (ET_Case)((l_hash - eo_Number) / NUM_COUNT);
        return S_OK;
    }
    if (eo_POS == POS_ADJ || eo_POS == POS_NUM || eo_POS == POS_PRONOUN) // 264 possibilities
    {
        eo_Gender = (ET_Gender)(l_hash % GENDER_COUNT);
        l_hash = (l_hash - eo_Gender) / GENDER_COUNT;
        eo_Number = (ET_Number)(l_hash % NUM_COUNT);
        l_hash = (l_hash - eo_Number) / NUM_COUNT;
        long l_tmp_case = l_hash % (CASE_COUNT + 1);
        if (l_tmp_case == CASE_COUNT)
        {
            eo_Case = CASE_ACC;
            eo_Animacy = ANIM_YES;
        }
        else
        {
            eo_Case = (ET_Case)l_tmp_case;
            if (eo_Case != CASE_UNDEFINED)
            {
                eo_Animacy = ANIM_NO;
            }           
        }
        l_hash = (l_hash - l_tmp_case) / (CASE_COUNT + 1);
        eo_Subparadigm = (ET_Subparadigm)l_hash;   
    }
    if (eo_POS == POS_VERB)
    {
        if (l_hash < TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT) // 144 possibilities
        {
            eo_SpecialForm = SPECIAL_NORMAL;
            eo_Reflexive = (ET_Reflexive)(l_hash % REFL_COUNT);
            l_hash = (l_hash - eo_Reflexive) / REFL_COUNT;
            if (l_hash < TENSE_PAST * NUM_COUNT * GENDER_COUNT * REFL_COUNT)
            {
                eo_Person = (ET_Person)(l_hash % PERSON_COUNT);
                l_hash = (l_hash - eo_Person) / PERSON_COUNT;
                eo_Number = (ET_Number)(l_hash % NUM_COUNT);
                l_hash = (l_hash - eo_Number) / NUM_COUNT;
                eo_Tense = (ET_Tense)l_hash;
                return S_OK;
            }           
            else
            {
                eo_Gender = (ET_Gender)(l_hash % GENDER_COUNT);
                l_hash = (l_hash - eo_Gender) / GENDER_COUNT;
                eo_Number = (ET_Number)(l_hash % NUM_COUNT);
                eo_Tense = TENSE_PAST;
                return S_OK;
            }
        }
       
        if (l_hash >= TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
            && l_hash < TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                        + NUM_COUNT * PERSON_COUNT * REFL_COUNT) // 36 possibilities
        {
            l_hash -= TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT;
            eo_SpecialForm = SPECIAL_IMPERATIVE;
            eo_Reflexive = (ET_Reflexive)(l_hash % REFL_COUNT);
            l_hash = (l_hash - eo_Reflexive) / REFL_COUNT;
            eo_Person = (ET_Person)(l_hash % PERSON_COUNT);
            l_hash = (l_hash - eo_Person) / PERSON_COUNT;
            eo_Number = (ET_Number)l_hash;
            return S_OK;
        }
        if (l_hash >= TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                    + NUM_COUNT * PERSON_COUNT * REFL_COUNT
                && l_hash < TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                    + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                    + NUM_COUNT * PERSON_COUNT * REFL_COUNT) // 4752 possibilities
        {
            l_hash -= TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                        + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
            eo_SpecialForm = SPECIAL_PARTICIPLE;
            eo_Reflexive = (ET_Reflexive)(l_hash % REFL_COUNT);
            l_hash = (l_hash - eo_Reflexive) / REFL_COUNT;
            eo_Voice = (ET_Voice)(l_hash % VOICE_COUNT);
            l_hash = (l_hash - eo_Voice) / VOICE_COUNT;
            eo_Gender = (ET_Gender)(l_hash % GENDER_COUNT);
            l_hash = (l_hash - eo_Gender) / GENDER_COUNT;
            eo_Number = (ET_Number)(l_hash % NUM_COUNT);
            l_hash = (l_hash - eo_Number) / NUM_COUNT;
            long l_tmp_case = l_hash % (CASE_COUNT + 1);
            if (l_tmp_case == CASE_COUNT)
            {
                eo_Case = CASE_ACC;
                eo_Animacy = ANIM_YES;
            }
            else
            {
                eo_Case = (ET_Case)l_tmp_case;
                if (eo_Case != CASE_UNDEFINED)
                {
                    eo_Animacy = ANIM_NO;
                }
            }
            l_hash = (l_hash - l_tmp_case) / (CASE_COUNT + 1);
            eo_Tense = (ET_Tense)l_hash;
            return S_OK;
        }
        if (l_hash >= TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                    + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                    + NUM_COUNT * PERSON_COUNT * REFL_COUNT
                && l_hash < l_hash_max - REFL_COUNT) // 12 possibilities
        {
            l_hash -= TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                    + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                    + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
            eo_SpecialForm = SPECIAL_CONVERB;
            eo_Reflexive = (ET_Reflexive)(l_hash % REFL_COUNT);
            l_hash = (l_hash - eo_Reflexive) / REFL_COUNT;
            eo_Tense = (ET_Tense)l_hash;
            return S_OK;
        }
        if (l_hash >= l_hash_max - REFL_COUNT && l_hash < l_hash_max) // 2 possibilities
        {
            l_hash -= l_hash_max - REFL_COUNT;
            eo_SpecialForm = SPECIAL_INFINITIVE;
            eo_Reflexive = (ET_Reflexive)l_hash;
            return S_OK;
        }
    }
    if (l_hash == l_hash_max)
    {
        eo_SpecialForm = SPECIAL_COMPARATIVE;
        return S_OK;
    }
    
    return E_FAIL;
}   // DecodeHash (long l_hash)

HRESULT CT_WordForm::DecodeString (BSTR bstr_gram)
{
    // Partially depends on POS.
   
    // Warning!
    // 1. The list may be incomplete.
    // 2. Verb reflexivity is recognized only if str_lemma is present.

    USES_CONVERSION;

    wstring str_gram = OLE2W(bstr_gram);
    if (str_gram.length() <= 0)
    {
        return E_INVALIDARG;
    }
    if (eo_POS == POS_UNDEFINED)
    {
        return E_FAIL;
    }

    GramClear();
    wsmatch result;
    wstring str_parameter;
    bool b_match;

    while (str_gram.length() > 0)
    {
        b_match = regex_match(str_gram, result, (const wregex)L"\\s*([^\\s\\.]*\\.?)\\s*(.*)");
        str_parameter = result[1];
        str_gram = result[2];
        if (str_parameter == L"1")
        {
            eo_Person = PERSON_1;
        }
        if (str_parameter == L"2")
        {
            eo_Person = PERSON_2;
        }
        if (str_parameter == L"3")
        {
            eo_Person = PERSON_3;
        }
        if (str_parameter == L"ед.")
        {
            eo_Number = NUM_SG;
        }
        if (str_parameter == L"мн.")
        {
            eo_Number = NUM_PL;
        }
        if (str_parameter == L"И.")
        {
            eo_Case = CASE_NOM;
        }
        if (str_parameter == L"Р.")
        {
            eo_Case = CASE_GEN;
        }
        if (str_parameter == L"Д.")
        {
            eo_Case = CASE_DAT;
        }
        if (str_parameter == L"В.")
        {
            eo_Case = CASE_ACC;
        }
        if (str_parameter == L"Т.")
        {
            eo_Case = CASE_INST;
        }
        if (str_parameter == L"П.")
        {
            eo_Case = CASE_PREP;
        }
        if (str_parameter == L"Р2")       
        {
            eo_Case = CASE_PART;
        }
        if (str_parameter == L"П2")
        {
            eo_Case = CASE_LOC;          
        }
        if (str_parameter == L"кф")
        {
            eo_Subparadigm = SUBPARADIGM_SHORT_ADJ;
        }
        if (str_parameter == L"буд.")
        {
            eo_Tense = TENSE_FUTURE;
        }
        if (str_parameter == L"наст.")
        {
            eo_Tense = TENSE_PRESENT;
        }
        if (str_parameter == L"прош.")
        {
            eo_Tense = TENSE_PAST;
        }
        if (str_parameter == L"прич.")
        {
            eo_SpecialForm = SPECIAL_PARTICIPLE;
        }
        if (str_parameter == L"деепр.")
        {
            eo_SpecialForm = SPECIAL_CONVERB;
        }
        if (str_parameter == L"повел.")
        {
            eo_SpecialForm = SPECIAL_IMPERATIVE;
        }
        if (str_parameter == L"страд.")
        {
            eo_Voice = VOICE_PASSIVE;
        }
        if (str_parameter == L"м")
        {
            eo_Gender = GENDER_M;
        }
        if (str_parameter == L"ж")
        {
            eo_Gender = GENDER_F;
        }
        if (str_parameter == L"с")
        {
            eo_Gender = GENDER_N;
        }
        if (str_parameter == L"одуш.")
        {
            eo_Animacy = ANIM_YES;
        }
        if (str_parameter == L"сравн.")
        {
            eo_SpecialForm = SPECIAL_COMPARATIVE;
        }
    }
    if (eo_POS == POS_ADV)
    {
        if (eo_Subparadigm == SUBPARADIGM_UNDEFINED)
        {
            eo_Subparadigm = SUBPARADIGM_LONG_ADJ;
        }
        if (eo_Case == CASE_ACC && eo_Animacy == ANIM_UNDEFINED)
        {
            eo_Animacy = ANIM_NO;
        }
    }
    if (eo_POS == POS_VERB)
    {
        if (eo_SpecialForm == SPECIAL_UNDEFINED)
        {
            eo_SpecialForm = SPECIAL_NORMAL;
        }
        if (eo_SpecialForm == SPECIAL_IMPERATIVE && eo_Person == PERSON_UNDEFINED)
        {
            eo_Person = PERSON_2;
        }
        if (eo_SpecialForm == SPECIAL_PARTICIPLE && eo_Case == CASE_ACC && eo_Animacy == ANIM_UNDEFINED)
        {
            eo_Animacy = ANIM_NO;
        }
        if (eo_SpecialForm == SPECIAL_PARTICIPLE && eo_Voice == VOICE_UNDEFINED)
        {
            eo_Voice = VOICE_ACTIVE;
        }
        b_match = regex_match(str_Lemma, result, (const wregex)L".*(ся|сь)");
        if (b_match == true)
        {
            eo_Reflexive = REFL_YES;
        }
        else
        {
            eo_Reflexive = REFL_NO;
        }
    }
   
    return S_OK;
} // DecodeString (BSTR bstr_gram)
*/
