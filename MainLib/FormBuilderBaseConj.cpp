#include "StdAfx.h"
#include "FormBuilderBaseConj.h"

HRESULT CT_FormBuilderBaseConj::h_GetPastTenseStressTypes (ET_AccentType eo_accentType,
                                                           ET_Number eo_number, 
                                                           ET_Gender eo_gender,
                                                           vector<ET_StressLocation>& vec_eoStressType)
{
    HRESULT h_r = S_OK;

    if (POS_VERB != pco_Lexeme->eo_PartOfSpeech)
    {
        ATLASSERT(0);
        wstring str_msg (L"Unexpected part of speech value.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }

    vector<ET_StressLocation> vec_stress;
    switch (eo_accentType)
    {
        case AT_A:
        {
            vec_eoStressType.push_back (STRESS_LOCATION_STEM);
            break;
        }
        case AT_B:
        {
            vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
            break;
        }
        case AT_C:
        {
            if (NUM_SG == eo_number && GENDER_N == eo_gender)
            {
                vec_eoStressType.push_back (STRESS_LOCATION_STEM);
            }
            else if (NUM_PL == eo_number)
            {
                vec_eoStressType.push_back (STRESS_LOCATION_STEM);
                break;
            }
            else
            {
                vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
            }
            break;
        }
        case AT_C2:
        {
            if (REFL_NO == pco_Lexeme->eo_Reflexive)
            {
                ATLASSERT(0);
                wstring str_msg (L"Unexpected part of speech value.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_FAIL, str_msg);
            }
            else if (!(GENDER_F == eo_gender && NUM_SG == eo_number))
            {
                vec_eoStressType.push_back (STRESS_LOCATION_STEM);
                vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
            }
            else
            {
                vec_eoStressType.push_back (STRESS_LOCATION_ENDING);
            }
            break;
        }
        default:
        {
            ATLASSERT(0);
            wstring str_msg (L"Unexpected past tense accent type.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_FAIL, str_msg);
        }

    }   // switch (eo_accentType)

    return S_OK;

}   //  h_GetPastTenseStressType (...)

HRESULT CT_FormBuilderBaseConj::h_GetStemStressPositions (const wstring& str_lemma, 
                                                          vector<int>& vec_iPosition)
{
    //
    // Find the sequence # of the stressed vowel in infinitive
    //
    vector<int>& vec_infStress = pco_Lexeme->vec_SourceStressPos; // alias for readability
    vector<int>::iterator it_infStresPos = vec_infStress.begin();
    for (; it_infStresPos != vec_infStress.end(); ++it_infStresPos)
    {
        CT_ExtString xstr_lemma (str_lemma);
        xstr_lemma.v_SetVowels (str_Vowels);
        int i_stemSyllables = xstr_lemma.i_NSyllables();
        if (i_stemSyllables < 1)
        {
            ATLASSERT(0);
            wstring str_msg (L"No vowels in verb stem.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_FAIL, str_msg);
        }

        if (*it_infStresPos >= i_stemSyllables)
        {
            vec_iPosition.push_back (i_stemSyllables - 1);  // last stem syllable
        }
        else
        {
            vec_iPosition.push_back (*it_infStresPos);    // same syllable as in infinitive
        }

    }   //  for (; it_sourceStressPos != vec_SourceStressPos.end(); ... )

    return S_OK;

}   //  h_GetStemStressPositions (...)

HRESULT CT_FormBuilderBaseConj::h_GetEndingStressPosition (const wstring& str_lemma, 
                                                           const wstring& str_ending,
                                                           int& i_position)
{
   HRESULT h_r = S_OK;

    if (POS_VERB != pco_Lexeme->eo_PartOfSpeech)
    {
        ATLASSERT(0);
        wstring str_msg (L"Unexpected part of speech.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_UNEXPECTED, str_msg);
    }

    bool b_retract = false;
    CT_ExtString xstr_ending (str_ending);
    xstr_ending.v_SetVowels (str_Vowels);
    if (0 == xstr_ending.i_NSyllables())
    {
        if (!(xstr_ending.length() == 0 || 
              L"ь" == xstr_ending || 
              L"й" == xstr_ending || 
              L"л" == xstr_ending))
        {
            ATLASSERT(0);
            wstring str_msg (L"Unexpected non-syllabic ending.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_FAIL, str_msg);
        }
        b_retract = true;
    }
    else
    {
        if (L"те" == xstr_ending || L"ся" == xstr_ending || L"тесь" == xstr_ending)
        {
            b_retract = true;
        }
        else if (xstr_ending.b_EndsWith (L"те") || 
                 xstr_ending.b_EndsWith (L"ся") || 
                 xstr_ending.b_EndsWith (L"тесь"))
        {
            if (L'ь' == xstr_ending[0] || L'й' == xstr_ending[0])
            {
                b_retract = true;
            }
        }
    }

    CT_ExtString xstr_lemma (str_lemma);
    xstr_lemma.v_SetVowels (str_Vowels);
//    int i_endingVowel = str_ending.find_first_of (str_Vowels);
//    if (wstring::npos != i_endingVowel)
//    {
//        i_position = str_lemma.length() + i_endingVowel;
    if (!b_retract)
    {
        i_position = xstr_lemma.i_NSyllables();
    }
    else
    {
        i_position = xstr_lemma.i_NSyllables() - 1;
//        int i_position = str_lemma.find_last_of (str_Vowels);
//        if (wstring::npos == i_position)
//        {
//            ATLASSERT(0);
//            i_position = -1;
//            wstring str_msg (L"Warning: can't find stressed vowel.");
//            ERROR_LOG (str_msg);
//            throw CT_Exception (E_FAIL, str_msg);
//        }
//// &&&& TODO Past masc. + -sya + c'': sorvalsyA
    }

    return h_r;

}   //  h_GetEndingStressPosition (...)

HRESULT CT_FormBuilderBaseConj::h_BuildPastTenseStem (wstring& str_lemma)
{
    HRESULT h_r = S_OK;

    CT_ExtString xstr_lemma;

    int i_type = pco_Lexeme->i_Type;
    if (7 == i_type || 8 == i_type)
    {
        xstr_lemma = pco_Lexeme->xstr_1SgStem;
        if (xstr_lemma.b_EndsWithOneOf (L"тд"))
        {
            xstr_lemma = xstr_lemma.erase (xstr_lemma.length()-1);
        }
    }
    else
    {
        xstr_lemma = pco_Lexeme->xstr_InfStem;
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

    str_lemma = xstr_lemma.c_str();

    return h_r;

}   //  h_BuildPastTenseStem (...)

HRESULT CT_FormBuilderBaseConj::h_HandleYoAlternation (int i_stressPos,
                                                       ET_Subparadigm eo_subParadigm, 
                                                       wstring& str_lemma)
{
    HRESULT h_r = S_OK;

    if (!pco_Lexeme->b_YoAlternation)
    {
        return S_OK;
    }

    if (SUBPARADIGM_PAST_TENSE != eo_subParadigm && 
        SUBPARADIGM_PART_PAST_ACT != eo_subParadigm &&
        SUBPARADIGM_PART_PAST_PASS_LONG != eo_subParadigm &&
        SUBPARADIGM_PART_PAST_PASS_SHORT != eo_subParadigm &&
        SUBPARADIGM_ADVERBIAL_PAST != eo_subParadigm &&
        SUBPARADIGM_PART_PRES_PASS_LONG != eo_subParadigm &&
        SUBPARADIGM_PART_PRES_PASS_SHORT != eo_subParadigm )
    {
        return S_OK;
    }

    if (SUBPARADIGM_PART_PAST_ACT == eo_subParadigm)
    {
        ATLASSERT (7 == pco_Lexeme->i_Type &&                  // GDRL, p. 85
        CT_ExtString (pco_Lexeme->xstr_SourceForm).b_EndsWith (L"сти") &&
        (L"т" == pco_Lexeme->str_VerbStemAlternation || 
         L"д" == pco_Lexeme->str_VerbStemAlternation));
    }

    int i_eOffset = str_lemma.rfind (_T("е"));    // last "e" in graphic stem (?)
    if (wstring::npos == i_eOffset)
    {
        ATLASSERT(0);
        wstring str_msg (L"Unstressed stem with yo alternation has no e.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }

    if (i_stressPos == i_eOffset)
    {
        str_lemma[i_stressPos] = L'ё';
    }

    return S_OK;

}   //  h_HandleYoAlternation (...)
