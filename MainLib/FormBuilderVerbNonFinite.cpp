#include "StdAfx.h"
#include "Endings.h"
#include "FormBuilderAdjLong.h"
#include "FormBuilderVerbNonFinite.h"

HRESULT CT_FormBuilderNonFinite::h_Build()
{
    HRESULT h_r = S_OK;

    try
    {
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
                             TENSE_PRESENT, 
                             PERSON_3, 
                             ANIM_UNDEFINED, 
                             REFL_UNDEFINED, 
                             VOICE_UNDEFINED, 
//                             SPECIAL_UNDEFINED, 
                             ASPECT_UNDEFINED);

    multimap<int, CComVariant>::iterator it_wordForm = pco_Lexeme->m_coll.find (co_hasher.i_GramHash());
    if (pco_Lexeme->m_coll.end() == it_wordForm)
    {
        wstring str_msg (L"Unable to locate 3 Pl. praes form.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }

    CComQIPtr<IWordForm> sp_wordFormItf = (*it_wordForm).second.pdispVal;
    if (NULL == sp_wordFormItf)
    {
        ATLASSERT(0);
        wstring str_msg (L"QI for IWordForm failed.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_POINTER, str_msg);
    }

    CT_WordForm * pco_wordForm = dynamic_cast<CT_WordForm *> (sp_wordFormItf.p);
    str_lemma = pco_wordForm->str_WordForm;
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

//    multimap<int, CComVariant>::iterator it_wordForm = pco_Lexeme->m_coll.begin();
/*
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
        if (SUBPARADIGM_PRESENT_TENSE == pco_wordForm->eo_Subparadigm)
        {
            if (NUM_PL == pco_wordForm->eo_Number && PERSON_3 == pco_wordForm->eo_Person)
            {
                str_lemma = pco_wordForm->str_WordForm;
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
            }
            else if (NUM_SG == pco_wordForm->eo_Number)
            {
                if (4 == i_type || 5 == i_type)
                {
                    if (PERSON_1 == pco_wordForm->eo_Person)
                    {
//                        i_stressPos = pco_wordForm->vec_Stress[0];
                    }
                }
                else
                {
                    if (PERSON_3 == pco_wordForm->eo_Person)
                    {
//                        i_stressPos = pco_wordForm->vec_Stress[0];
                    }
                }
            }

            if (!str_lemma.empty() && i_stressPos >= 0)
            {
                break;
            }
        }
    }       // for ...

    if (it_wordForm == pco_Lexeme->m_coll.end())
    {
        ATLASSERT(0);
        wstring str_msg (L"Unable to find 3 Pl form.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }
*/
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

    vector<int>& vec_sourceStressPos = pco_Lexeme->vec_SourceStressPos;

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

    vector<int> vec_stress;
    CT_ExtString xstr_lemma;
    CT_ExtString xstr_ending (L"я");     // for the lack of a better term...
    if (13 == i_type)
    {
        xstr_lemma = pco_Lexeme->xstr_InfStem;
        if ((vec_sourceStressPos.size() > 1) || vec_sourceStressPos[0] >= (int)xstr_lemma.length())
        {
            ATLASSERT(0);
            wstring str_msg (L"Bad stress position.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_FAIL, str_msg);
        }
        vec_stress = pco_Lexeme->vec_SourceStressPos;
    }
    else
    {
        bool b_stressFound = false;
        bool b_lemmaFound = false;
        multimap<int, CComVariant>::iterator it_wordForm = pco_Lexeme->m_coll.begin();
        for (; it_wordForm != pco_Lexeme->m_coll.end(); ++it_wordForm)
        {
            CComQIPtr<IWordForm> sp_wfItf = (*it_wordForm).second.pdispVal;
            if (NULL == sp_wfItf)
            {
                ATLASSERT(0);
                wstring str_msg (L"QI for IWordForm failed.");
                ERROR_LOG (str_msg);
                throw CT_Exception (E_POINTER, str_msg);
            }

            CT_WordForm * pco_wf = dynamic_cast<CT_WordForm *> (sp_wfItf.p);
            if (SUBPARADIGM_PRESENT_TENSE == pco_wf->eo_Subparadigm)
            {
                if (NUM_SG == pco_wf->eo_Number && PERSON_1 == pco_wf->eo_Person)
                {
//                    vec_stress = pco_wf->vec_Stress;
                    b_stressFound = true;
                }
                if (NUM_PL == pco_wf->eo_Number && PERSON_3 == pco_wf->eo_Person)
                {
                    xstr_lemma = pco_wf->str_Lemma;
                    b_lemmaFound = true;
                }
            }
            if (b_lemmaFound && b_stressFound)
            {
                break;
            }
        }       //  for (; ... )

        if (it_wordForm == pco_Lexeme->m_coll.end())
        {
            ATLASSERT(0);
            wstring str_msg (L"Unable to find present tense form.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_FAIL, str_msg);
        }

        if (xstr_lemma.b_EndsWithOneOf (str_ShSounds))
        {
            xstr_ending = L"а";
        }
    }       // else

    CComObject<CT_WordForm> * pco_wordForm;
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
    pco_wordForm->eo_Subparadigm = SUBPARADIGM_ADVERBIAL_PRESENT;
    pco_wordForm->str_Lemma = xstr_lemma;
    pco_wordForm->str_WordForm = xstr_lemma + xstr_ending;
//    pco_wordForm->i_Stress = i_stressPos;

    int i_GramHash = pco_wordForm->i_GramHash();
    pco_Lexeme->h_AddWordForm (i_GramHash, pco_wordForm);

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
