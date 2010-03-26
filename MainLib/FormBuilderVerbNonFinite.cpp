#include "StdAfx.h"
#include "Endings.h"
#include "FormBuilderAdjLong.h"
#include "FormBuilderVerbNonFinite.h"

HRESULT CT_FormBuilderNonFinite::h_Build()
{
    HRESULT h_r = S_OK;

    try
    {
        h_BuildInfinitive();
        h_BuildPresentActiveParticiple();
        h_BuildPresentAdverbial();
        h_BuildPastActiveParticiple();
    }
    catch (CT_Exception co_ex)
    {
        return co_ex.i_GetErrorCode();  // logging should be always done by callee
    }
    
    return S_OK;
}

HRESULT CT_FormBuilderNonFinite::h_BuildInfinitive()
{
    HRESULT h_r = S_OK;

    CComObject<CT_WordForm> * pco_wordForm = NULL;
    h_r = CComObject<CT_WordForm>::CreateInstance (&pco_wordForm);
    if (S_OK != h_r)
    {
        ATLASSERT(0);
        CString cs_msg;
        cs_msg.Format (L"CreateInstance() failed, error %x.", h_r);
        ERROR_LOG ((LPCTSTR)cs_msg);
        throw CT_Exception (h_r, (LPCTSTR)cs_msg);
    }

    pco_wordForm->eo_POS = POS_VERB;
    pco_wordForm->eo_Subparadigm = SUBPARADIGM_INFINITIVE;
    pco_wordForm->eo_Reflexive = pco_Lexeme->b_Reflexive ? REFL_YES : REFL_NO;
    pco_wordForm->str_Lemma = pco_Lexeme->xstr_InfStem.c_str();
    pco_wordForm->str_WordForm = pco_Lexeme->xstr_Infinitive;

    vector<int> vec_iStress = pco_Lexeme->vec_SourceStressPos;
    v_AssignStress (pco_wordForm, pco_Lexeme->vec_SecondaryStressPos, false);

    int i_gramHash = pco_wordForm->i_GramHash();

    if (1 == vec_iStress.size() || b_MultiStress (pco_wordForm->str_Lemma, vec_iStress))
    {
        vector<int>::iterator it_stressPos = vec_iStress.begin();
        for (; it_stressPos != vec_iStress.end(); ++it_stressPos)
        {
            pco_wordForm->map_Stress[*it_stressPos] = true;  // primary
        }
        pco_Lexeme->h_AddWordForm (i_gramHash, pco_wordForm);
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
            pco_Lexeme->h_AddWordForm (i_gramHash, pco_wordForm);
        }
    }

    return S_OK;

}   //  h_BuildInfinitive()

HRESULT CT_FormBuilderNonFinite::h_BuildPresentActiveParticiple()
{
    HRESULT h_r = S_OK;

    if (L"св" == pco_Lexeme->str_MainSymbol)
    {
        wstring str_msg (L"Attempt to build present tense active participle off of a perfective verb.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }

    int i_type = pco_Lexeme->i_Type;

    wstring str_lemma;
    int i_stressPos = -1;

    CT_GramHasher co_hasher (POS_VERB, 
                             SUBPARADIGM_PRESENT_TENSE, 
                             CASE_UNDEFINED, 
                             NUM_PL, 
                             GENDER_UNDEFINED, 
                             TENSE_UNDEFINED, 
                             PERSON_3, 
                             ANIM_UNDEFINED, 
                             pco_Lexeme->b_Reflexive ? REFL_YES : REFL_NO,
                             VOICE_UNDEFINED, 
                             ASPECT_UNDEFINED);

    CComQIPtr<IWordForm> sp_3PlWf;
    h_WordFormFromHash (co_hasher.i_GramHash(), sp_3PlWf);
    CT_WordForm * pco_3PlWf = dynamic_cast<CT_WordForm *> (sp_3PlWf.p);
    if (NULL == pco_3PlWf)
    {
        ATLASSERT(0);
        wstring str_msg (L"Failed to obtain 3 Pl. word form.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_POINTER, str_msg);
    }

    str_lemma = pco_3PlWf->str_WordForm;
    int i_erase = pco_Lexeme->b_Reflexive ? 3 : 1;
    if (i_erase >= (int)str_lemma.length())
    {
        ATLASSERT(0);
        wstring str_msg (L"Illegal stem length.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }
    str_lemma = str_lemma.erase (str_lemma.length() - i_erase);
    str_lemma += L'щ';

    CT_FormBuilderLongAdj co_builder (pco_Lexeme, str_lemma, AT_A, SUBPARADIGM_PARTICIPLE_PRESENT_ACTIVE);
    co_builder.h_Build();

    return h_r;

}   //  h_BuildPresentActiveParticiple()

HRESULT CT_FormBuilderNonFinite::h_BuildPresentAdverbial()
{
    HRESULT h_r = S_OK;

    if (L"св" == pco_Lexeme->str_MainSymbol)
    {
        ATLASSERT(0);
        wstring str_msg (L"Attempt to build present tense adverbial off of a perfective verb.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }

    int i_type = pco_Lexeme->i_Type;

    if (3 == i_type || 8 == i_type || 9 == i_type || 11 == i_type || 14 == i_type || 15 == i_type)
    {
        ATLASSERT(0);
        wstring str_msg (L"Attempt to build present tense adverbial off of a type ");
        str_msg += str_ToString (i_type);
        str_msg += L" verb.";
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }

    CComObject<CT_WordForm> * pco_wordForm = NULL;
    h_r = CComObject<CT_WordForm>::CreateInstance (&pco_wordForm);
    if (S_OK != h_r)
    {
        ATLASSERT(0);
        CString cs_msg;
        cs_msg.Format (L"CreateInstance() failed, error %x.", h_r);
        ERROR_LOG ((LPCTSTR)cs_msg);
        throw CT_Exception (h_r, (LPCTSTR)cs_msg);
    }

    pco_wordForm->eo_POS = POS_VERB;
    pco_wordForm->eo_Subparadigm = SUBPARADIGM_ADVERBIAL_PRESENT;
    pco_wordForm->eo_Reflexive = pco_Lexeme->b_Reflexive ? REFL_YES : REFL_NO;
    v_AssignStress (pco_wordForm, pco_Lexeme->vec_SecondaryStressPos, false);

    int i_gramHash = pco_wordForm->i_GramHash();

    vector<int> vec_iStress;
    wstring str_lemma;
    wstring str_ending (L"я");     // for the lack of a better term...
    if (13 == i_type)
    {
        CT_GramHasher co_hasher (POS_VERB, 
                                 SUBPARADIGM_INFINITIVE, 
                                 CASE_UNDEFINED, 
                                 NUM_UNDEFINED, 
                                 GENDER_UNDEFINED, 
                                 TENSE_UNDEFINED, 
                                 PERSON_UNDEFINED, 
                                 ANIM_UNDEFINED, 
                                 pco_Lexeme->b_Reflexive ? REFL_YES : REFL_NO,
                                 VOICE_UNDEFINED, 
                                 ASPECT_UNDEFINED);
        CComQIPtr<IWordForm> sp_infinitive;
        h_WordFormFromHash (co_hasher.i_GramHash(), sp_infinitive);
        CT_WordForm * pco_infinitive = dynamic_cast<CT_WordForm *> (sp_infinitive.p);
        if (NULL == pco_infinitive)
        {
            ATLASSERT(0);
            wstring str_msg (L"Failed to obtain infinitive.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_POINTER, str_msg);
        }

        pco_wordForm->str_Lemma = pco_infinitive->str_Lemma;
        pco_wordForm->str_WordForm = pco_wordForm->str_Lemma + str_ending;

        vec_iStress = pco_Lexeme->vec_SourceStressPos;
        if (1 == vec_iStress.size() || b_MultiStress (pco_wordForm->str_Lemma, vec_iStress))
        {
            vector<int>::iterator it_stressPos = vec_iStress.begin();
            for (; it_stressPos != vec_iStress.end(); ++it_stressPos)
            {
                pco_wordForm->map_Stress[*it_stressPos] = true;  // primary
            }
            pco_Lexeme->h_AddWordForm (i_gramHash, pco_wordForm);
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
                pco_Lexeme->h_AddWordForm (i_gramHash, pco_wordForm);
            }
        }
    }
    else
    {
        CT_GramHasher co_hasher3Sg (POS_VERB, 
                                    SUBPARADIGM_PRESENT_TENSE, 
                                    CASE_UNDEFINED, 
                                    NUM_PL, 
                                    GENDER_UNDEFINED, 
                                    TENSE_UNDEFINED, 
                                    PERSON_3, 
                                    ANIM_UNDEFINED, 
                                    pco_Lexeme->b_Reflexive ? REFL_YES : REFL_NO,
                                    VOICE_UNDEFINED, 
                                    ASPECT_UNDEFINED);
        CComQIPtr<IWordForm> sp_3PlWf;
        h_WordFormFromHash (co_hasher3Sg.i_GramHash(), sp_3PlWf);
        CT_WordForm * pco_3PlWf = dynamic_cast<CT_WordForm *> (sp_3PlWf.p);
        if (NULL == pco_3PlWf)
        {
            ATLASSERT(0);
            wstring str_msg (L"Failed to obtain 3 Pl word form.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_POINTER, str_msg);
        }

        CT_GramHasher co_hasher1Sg (co_hasher3Sg);
        co_hasher1Sg.eo_Number = NUM_SG;
        co_hasher1Sg.eo_Person = PERSON_1;
        CComQIPtr<IWordForm> sp_1SgWf;
        h_WordFormFromHash (co_hasher1Sg.i_GramHash(), sp_1SgWf);
        CT_WordForm * pco_1SgWf = dynamic_cast<CT_WordForm *> (sp_1SgWf.p);
        if (NULL == pco_1SgWf)
        {
            ATLASSERT(0);
            wstring str_msg (L"Failed to obtain 1 Sg word form.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_POINTER, str_msg);
        }

        if (CT_ExtString (str_lemma).b_EndsWithOneOf (str_ShSounds))
        {
            str_ending = L"а";
        }
        pco_wordForm->str_Lemma = pco_3PlWf->str_Lemma;
        pco_wordForm->str_WordForm = pco_3PlWf->str_Lemma + str_ending;
        pco_wordForm->map_Stress = pco_1SgWf->map_Stress;
        pco_Lexeme->h_AddWordForm (i_gramHash, pco_wordForm);
    }

    return h_r;

}   //  h_BuildPresentAdverbial()

HRESULT CT_FormBuilderNonFinite::h_BuildPastActiveParticiple()
{
    HRESULT h_r = S_OK;

    int i_type = pco_Lexeme->i_Type;
    vector<int>& vec_sourceStressPos = pco_Lexeme->vec_SourceStressPos;

    CT_ExtString xstr_lemma;
    int i_stressPos = -1;
    wstring str_ending;     // for the lack of a better term...

    //
    // Type 7 ending in -сти with -т- or -д- alternation mark:
    //     1 Sg stem present tense + -ший
    //
    if (7 == i_type && 
        CT_ExtString (pco_Lexeme->str_SourceForm).b_EndsWith (L"сти") &&
        (L"т" == pco_Lexeme->str_VerbStemAlternation || L"д" == pco_Lexeme->str_VerbStemAlternation))
    {
        xstr_lemma = pco_Lexeme->xstr_1SgStem;
        str_ending = L"ший";
    }

    //
    // All other types: derived from Past Sg. m 
    //
    else
    {
        multimap<int, CComVariant>::iterator it_wordForm = pco_Lexeme->m_coll.begin();
        for (; it_wordForm != pco_Lexeme->m_coll.end(); ++it_wordForm)
        {
            CComQIPtr<IWordForm> sp_wordFormItf = (*it_wordForm).second.pdispVal;
            if (NULL == sp_wordFormItf)
            {
                ATLASSERT(0);
                wstring str_msg (L"QI for IWordForm failed.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_POINTER, str_msg);
            }

            CT_WordForm * pco_wordForm = dynamic_cast<CT_WordForm *> (sp_wordFormItf.p);
            if (SUBPARADIGM_PAST_TENSE == pco_wordForm->eo_Subparadigm)
            {
                if (NUM_SG == pco_wordForm->eo_Number && GENDER_M == pco_wordForm->eo_Gender)
                {
                    xstr_lemma = pco_wordForm->str_Lemma;
//                    i_stressPos = pco_wordForm->vec_Stress.push_back&&&&
                    break;
                }
            }
        }

        if (it_wordForm == pco_Lexeme->m_coll.end())
        {
            ATLASSERT(0);
            wstring str_msg (L"Unable to find S Pl form.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_FAIL, str_msg);
        }

        h_BuildPastTenseStem (xstr_lemma);

        //
        // If Pst Sg. m ends in -л, replace it with -вший, otherwise add -ший
        //
        if (xstr_lemma.b_EndsWithOneOf (str_Consonants))
        {
            str_ending = L"ший";
        }
        else
        {
            str_ending = L"вший";
        }
    }

//    if (1 == pco_Lexeme->i_CommonDeviation)
//    {
        if (vec_sourceStressPos.size() > 1)
        {
            ATLASSERT(0);
            wstring str_msg (L"Multiple stress positions are not allowed here.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_FAIL, str_msg);
        }
        i_stressPos = vec_sourceStressPos[0];
//    }
//    else
//    {
        ET_AccentType eo_at = AT_UNDEFINED;
        if (AT_UNDEFINED == pco_Lexeme->eo_AccentType2)
        {
            eo_at = AT_A;
        }
        else
        {
            eo_at = pco_Lexeme->eo_AccentType2;
        }

        vector<ET_StressLocation> vec_eoStressType;
        h_GetPastTenseStressTypes (eo_at, NUM_SG, GENDER_M, vec_eoStressType);
            
        vector<int> vec_iStressPos;
        vector<ET_StressLocation>::iterator it_stressType = vec_eoStressType.begin();
        for (; it_stressType != vec_eoStressType.end(); ++ it_stressType)
        {
            if (STRESS_LOCATION_STEM == *it_stressType)
            {
                h_GetStemStressPositions (xstr_lemma, vec_iStressPos);
            }
            else if (STRESS_LOCATION_ENDING == *it_stressType)
            {
                int i_stressPos = -1;
                h_GetEndingStressPosition (xstr_lemma, str_ending, i_stressPos);
            }
            else
            {
                ATLASSERT(0);
                wstring str_msg (L"Illegal stress type.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_INVALIDARG, str_msg);
            }
        }
//    }

    return h_r;

}   //  h_BuildPastActiveParticiple()
