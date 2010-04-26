#include "StdAfx.h"
#include "WordForm.h"
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

HRESULT CT_FormBuilderBaseConj::h_HandleCommonDeviations()
{
    HRESULT h_r = S_OK;

    if (pco_Lexeme->map_CommonDeviations.empty())
    {
        return S_OK;
    }

    map<int, bool>::iterator it_cd = pco_Lexeme->map_CommonDeviations.begin();
    switch ((*it_cd).first)
    {
        case 1:
        {
            h_CommonDeviation_1 ((*it_cd).second);
            break;
        }
        case 2:
        case 3:
        {
            h_CommonDeviations_2_3 ((*it_cd).first, (*it_cd).second);
            break;
        }
        default:
        {
            ATLASSERT(0);
            wstring str_msg (L"Verb common deviation not recognized.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_FAIL, str_msg);
        }
    }

    return S_OK;

}   //  h_HandleCommonDeviations()

//
// Stress shifts left by one syllable in past tense (except fem.) and passive participle
// (except short form fem.)
//
HRESULT CT_FormBuilderBaseConj::h_CommonDeviation_1 (bool b_optional)
{
    HRESULT h_r = S_OK;

    //
    // Past tense
    //
    for (ET_Number eo_num = NUM_SG; eo_num < NUM_COUNT; ++eo_num)
    {
        for (ET_Gender eo_gender = GENDER_UNDEFINED; eo_gender < GENDER_COUNT; ++eo_gender)
        {
            if (NUM_PL == eo_num && eo_gender != GENDER_UNDEFINED)
            {
                continue;
            }
            if (NUM_SG == eo_num && (GENDER_UNDEFINED == eo_gender || GENDER_F == eo_gender))
            {
                continue;
            }

            CT_GramHasher co_hasher (POS_VERB, SUBPARADIGM_PAST_TENSE, CASE_UNDEFINED, 
                                     eo_num, eo_gender, TENSE_UNDEFINED, PERSON_UNDEFINED, 
                                     ANIM_UNDEFINED, pco_Lexeme->eo_Reflexive, VOICE_UNDEFINED, 
                                     ASPECT_UNDEFINED);

            h_RetractStress (co_hasher.i_GramHash(), b_optional);

        }   //  for (ET_Gender eo_gender = GENDER_UNDEFINED; ...)

    }   //  for (ET_Number eo_num = NUM_SG; ...)

    //
    // Past passive part long
    //
    for (ET_Number eo_num = NUM_SG; eo_num < NUM_COUNT; ++eo_num)
        for (ET_Gender eo_gender = GENDER_M; eo_gender < GENDER_COUNT; ++eo_gender)
            for (ET_Case eo_case = CASE_NOM; eo_case < CASE_COUNT; ++eo_case)
            {
                CT_GramHasher co_hasher (POS_VERB, SUBPARADIGM_PART_PRES_PASS_LONG, eo_case, 
                                         eo_num, eo_gender, TENSE_UNDEFINED, PERSON_UNDEFINED, 
                                         ANIM_UNDEFINED, pco_Lexeme->eo_Reflexive, VOICE_UNDEFINED, 
                                         ASPECT_UNDEFINED);

                h_RetractStress (co_hasher.i_GramHash(), b_optional);
            }

    //
    // Past passive part short
    //
    for (ET_Number eo_num = NUM_SG; eo_num < NUM_COUNT; ++eo_num)
    {
        for (ET_Gender eo_gender = GENDER_UNDEFINED; eo_gender < GENDER_COUNT; ++eo_gender)
        {
            if (NUM_PL == eo_num && eo_gender != GENDER_UNDEFINED)
            {
                continue;
            }
            if (NUM_SG == eo_num && (GENDER_UNDEFINED == eo_gender || GENDER_F == eo_gender))
            {
                continue;
            }
            CT_GramHasher co_hasher (POS_VERB, SUBPARADIGM_PART_PRES_PASS_SHORT, CASE_UNDEFINED,
                                     eo_num, eo_gender, TENSE_UNDEFINED, PERSON_UNDEFINED, 
                                     ANIM_UNDEFINED, pco_Lexeme->eo_Reflexive, VOICE_UNDEFINED, 
                                     ASPECT_UNDEFINED);

            h_RetractStress (co_hasher.i_GramHash(), b_optional);
        }
    }

    return S_OK;

}   // h_CommonDeviation1 (...)

HRESULT CT_FormBuilderBaseConj::h_RetractStress (int i_hash, bool b_isOptional)
{
    HRESULT h_r = S_OK;

    CT_WordForm * pco_wf = NULL;
    int i_nForms = i_NForms (i_hash);
    for (int i_wf = 0; i_wf < i_nForms; ++i_wf)
    {
        h_WordFormFromHash (i_hash, i_wf, pco_wf);
        if (NULL == pco_wf)
        {
            ATLASSERT(0);
            wstring str_msg (L"Failed to obtain word form.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_POINTER, str_msg);
        }

        if (b_isOptional)   // store both forms
        {
            CComObject<CT_WordForm> * pco_variant = NULL;
            h_CloneWordForm (pco_wf, pco_variant);
            pco_Lexeme->h_AddWordForm (pco_variant);
            pco_wf = pco_variant;
        }

        map<int, bool> map_correctedStress;
        map<int, bool>::iterator it_stressPos = pco_wf->map_Stress.begin();
        for (; it_stressPos != pco_wf->map_Stress.begin(); ++it_stressPos)
        {
            if (!(*it_stressPos).second)
            {
                map_correctedStress[(*it_stressPos).first] = false;
                continue;
            }
            if ((*it_stressPos).first < 1)
            {
                ATLASSERT(0);
                wstring str_msg (L"Unexpected stress position in cd-1 verb.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_FAIL, str_msg);
            }
            if (CT_ExtString (pco_wf->str_WordForm).b_StartsWith (L"пере"))
            {
                if ((*it_stressPos).first < 2)
                {
                    ATLASSERT(0);
                    wstring str_msg (L"Unexpected stress position in cd-1 verb.");
                    ERROR_LOG (str_msg);
                    throw CT_Exception (E_FAIL, str_msg);
                }
                map_correctedStress[(*it_stressPos).first-2] = true;
            }
            else
            {
                map_correctedStress[(*it_stressPos).first-1] = true;
            }
        }
        pco_wf->map_Stress = map_correctedStress;
    }

    return S_OK;

}   //  h_RetractStress (...)

HRESULT CT_FormBuilderBaseConj::h_CommonDeviations_2_3 (int i_deviation, bool b_isOptional)
{
    HRESULT h_r = S_OK;

    for (ET_Number eo_num = NUM_SG; eo_num < NUM_COUNT; ++eo_num)
    {
        CT_GramHasher co_hasher (POS_VERB, SUBPARADIGM_IMPERATIVE, CASE_UNDEFINED, 
                                 eo_num, GENDER_UNDEFINED, TENSE_UNDEFINED, PERSON_UNDEFINED, 
                                 ANIM_UNDEFINED, pco_Lexeme->eo_Reflexive, VOICE_UNDEFINED, 
                                 ASPECT_UNDEFINED);

        CT_WordForm * pco_wf = NULL;
        int i_nForms = i_NForms (co_hasher.i_GramHash());
        for (int i_wf = 0; i_wf < i_nForms; ++i_wf)
        {
            h_WordFormFromHash (co_hasher.i_GramHash(), i_wf, pco_wf);
            if (NULL == pco_wf)
            {
                ATLASSERT(0);
                wstring str_msg (L"Failed to obtain word form.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_POINTER, str_msg);
            }

            if (b_isOptional)   // store both forms
            {
                CComObject<CT_WordForm> * pco_variant = NULL;
                h_CloneWordForm (pco_wf, pco_variant);
                pco_Lexeme->h_AddWordForm (pco_variant);
                pco_wf = pco_variant;
            }

            int i_endingOffset = (REFL_NO == pco_Lexeme->eo_Reflexive)
                ? pco_wf->str_WordForm.length() - 1
                : pco_wf->str_WordForm.length() - 3;

            if (i_endingOffset < 1)
            {
                ATLASSERT(0);
                wstring str_msg (L"Bad ending offset.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_POINTER, str_msg);
            }

            if (2 == i_deviation)
            {
                if (str_Vowels.find (pco_wf->str_WordForm[i_endingOffset-1]) != wstring.npos)
                {
                    pco_wf->str_WordForm[i_endingOffset] = L'й';
                }
                else
                {
                    pco_wf->str_WordForm[i_endingOffset] = L'ь';
                }
            }

            if (3 == i_deviation)
            {
                if (NUM_SG == eo_num)
                {
                    pco_wf->str_WordForm[i_endingOffset] = L'и';
                }
                else
                {
                    pco_wf->str_WordForm[i_endingOffset] = L'ь';
                }
            }
        }
    }   //  for (ET_Number eo_num = NUM_SG; ...)

    return S_OK;

}   //  h_CommonDeviations_2_3 (...)
