#include "StdAfx.h"
#include "Endings.h"
#include "FormBuilderAdjLong.h"
#include "FormBuilderVerbNonFinite.h"

HRESULT CT_FormBuilderNonFinite::h_Build()
{
    HRESULT h_r = S_OK;

    h_r = h_BuildPresentActiveParticiple();
    h_r = h_BuildPresentAdverbial();
    h_r = h_BuildPastActiveParticiple();

    return h_r;
}

HRESULT CT_FormBuilderNonFinite::h_BuildPresentActiveParticiple()
{
    HRESULT h_r = S_OK;

    if (L"св" == pco_Lexeme->str_MainSymbol)
    {
        ATLASSERT(0);
        ERROR_LOG (L"Attempt to build present tense active participle off of a perfective verb.");
        return E_FAIL;
    }

    int i_type = pco_Lexeme->i_Type;

    wstring str_lemma;
    int i_stressPos = -1;
    multimap<int, CComVariant>::iterator it_wordForm = pco_Lexeme->m_coll.begin();
    for (; it_wordForm != pco_Lexeme->m_coll.end(); ++it_wordForm)
    {
        CComQIPtr<IWordForm> sp_wordFormItf = (*it_wordForm).second.pdispVal;
        if (NULL == sp_wordFormItf)
        {
            ATLASSERT(0);
            ERROR_LOG (L"QI for IWordForm failed.");
            return E_POINTER;
        }

        CT_WordForm * pco_wordForm = dynamic_cast<CT_WordForm *> (sp_wordFormItf.p);
        if (SUBPARADIGM_PRESENT_TENSE == pco_wordForm->eo_Subparadigm)
        {
            if (NUM_PL == pco_wordForm->eo_Number && PERSON_3 == pco_wordForm->eo_Person)
            {
                str_lemma = pco_wordForm->str_WordForm;
                str_lemma.erase (str_lemma.length()-1, 1);
                str_lemma += L'щ';
                i_stressPos = pco_wordForm->vec_Stress[0];
            }
            else if (NUM_SG == pco_wordForm->eo_Number)
            {
                if (4 == i_type || 5 == i_type)
                {
                    if (PERSON_1 == pco_wordForm->eo_Person)
                    {
                        i_stressPos = pco_wordForm->vec_Stress[0];
                    }
                    else if (PERSON_3 == pco_wordForm->eo_Person)
                    {
                        i_stressPos = pco_wordForm->vec_Stress[0];      //NB!!!
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
        ERROR_LOG (L"Unable to find 3 Pl form.");
        return E_FAIL;
    }

    CT_FormBuilderLongAdj co_builder (pco_Lexeme, str_lemma, AT_A, SUBPARADIGM_PARTICIPLE_PRESENT_ACTIVE);
    co_builder.h_Build();

    return h_r;

}   //  h_BuildPresentActiveParticiple()

HRESULT CT_FormBuilderNonFinite::h_BuildPresentAdverbial()
{
    HRESULT h_r = S_OK;

    if (L"св" == pco_Lexeme->str_MainSymbol)
    {
        ERROR_LOG (L"Attempt to build present tense adverbial off of a perfective verb.");
        return E_FAIL;
    }

    vector<int>& vec_sourceStressPos = pco_Lexeme->vec_SourceStressPos;

    int i_type = pco_Lexeme->i_Type;

    if (3 == i_type || 8 == i_type || 9 == i_type || 11 == i_type || 14 == i_type || 15 == i_type)
    {
        wstring str_msg (L"Attempt to build present tense adverbial off of a type ");
        str_msg += str_ToString (i_type);
        str_msg += L" verb.";
        ERROR_LOG (str_msg);
        return E_FAIL;
    }

    vector<int> vec_stress;
    CT_ExtString xstr_lemma;
    CT_ExtString xstr_ending (L"я");     // for the lack of a better term...
//    int i_stressPos = -1;
    if (13 == i_type)
    {
        xstr_lemma = pco_Lexeme->xstr_InfStem;
        if ((vec_sourceStressPos.size() > 1) || vec_sourceStressPos[0] >= (int)xstr_lemma.length())
        {
            ERROR_LOG (L"Bad stress position.");
            return E_FAIL;
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
                ERROR_LOG (L"QI for IWordForm failed.");
                return E_POINTER;
            }

            CT_WordForm * pco_wf = dynamic_cast<CT_WordForm *> (sp_wfItf.p);
            if (SUBPARADIGM_PRESENT_TENSE == pco_wf->eo_Subparadigm)
            {
                if (NUM_SG == pco_wf->eo_Number && PERSON_1 == pco_wf->eo_Person)
                {
                    vec_stress = pco_wf->vec_Stress;
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
            ERROR_LOG (L"Unable to find present tense form.");
            return E_FAIL;
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
        return h_r;
    }

    pco_wordForm->eo_POS = pco_Lexeme->eo_PartOfSpeech;
    pco_wordForm->eo_Subparadigm = SUBPARADIGM_ADVERBIAL_PRESENT;
    pco_wordForm->str_Lemma = xstr_lemma;
    pco_wordForm->str_WordForm = xstr_lemma + xstr_ending;
//    pco_wordForm->i_Stress = i_stressPos;

    long l_GramHash = 0;
    pco_wordForm->get_GramHash (&l_GramHash);
//    pco_Lexeme->m_coll.push_back (pco_wordForm);
    pco_Lexeme->v_AddWordForm (l_GramHash, pco_wordForm);

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
                ERROR_LOG (L"QI for IWordForm failed.");
                return E_POINTER;
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
            ERROR_LOG (L"Unable to find S Pl form.");
            return E_FAIL;
        }


        h_r = h_BuildPastTenseStem (xstr_lemma);

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
            ERROR_LOG (L"Multiple stress positions are not allowed here.");
            return E_FAIL;
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
        h_r = h_GetPastTenseStressTypes (eo_at, NUM_SG, GENDER_M, vec_eoStressType);
        if (S_OK != h_r)
        {
            return h_r;
        }
            
        vector<int> vec_iStressPos;
        vector<ET_StressLocation>::iterator it_stressType = vec_eoStressType.begin();
        for (; it_stressType != vec_eoStressType.end(); ++ it_stressType)
        {
            if (STRESS_LOCATION_STEM == *it_stressType)
            {
                h_r = h_GetStemStressPositions (xstr_lemma, vec_iStressPos);
                if (S_OK != h_r)
                {
                    return h_r;
                }
            }
            else if (STRESS_LOCATION_ENDING == *it_stressType)
            {
                int i_stressPos = -1;
                h_r = h_GetEndingStressPosition (xstr_lemma, str_ending, i_stressPos);
                if (S_OK != h_r)
                {
                    return h_r;
                }
            }
            else
            {
                ATLASSERT (0);
                ERROR_LOG (L"Illegal stress type.");
                return E_INVALIDARG;
            }
        }
//    }

        
    return h_r;

}   //  h_BuildPastActiveParticiple()
