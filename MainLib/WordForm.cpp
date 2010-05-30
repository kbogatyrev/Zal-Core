#include "StdAfx.h"
#include "WordForm.h"
#include "Lexeme.h"

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
        ERROR_LOG (L"Unable to find stress position.");
        return E_FAIL;
    }

//    *pl_stressPos = it_pos->first;
    CT_ExtString xstr_wf (str_WordForm);
    xstr_wf.v_SetVowels (str_Vowels);
    *pl_stressPos = xstr_wf.i_GetVowelPos (it_pos->first);

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
        CT_ExtString xstr_wf (str_WordForm);
        xstr_wf.v_SetVowels (str_Vowels);
        *pl_StressPosition = xstr_wf.i_GetVowelPos (it_stressPos->first);
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
    CT_ExtString xstr_wf (str_WordForm);
    xstr_wf.v_SetVowels (str_Vowels);
    int i_syll = xstr_wf.i_GetSyllableFromVowelPos (l_pos);

    map<int, bool>::iterator it_ = map_Stress.find (i_syll);
    if (map_Stress.end() == it_)
    {
        ERROR_LOG (L"Unable to find stress position.");
        return E_INVALIDARG;
    }
    else
    {
        *pb_type = it_->second ? TRUE : FALSE;
        return S_OK;
    }
}

HRESULT CT_WordForm::get_Status (ET_Status * pe_status)
{
    *pe_status = (ET_Status)eo_Status;
    return S_OK;
}

HRESULT CT_WordForm::SaveTestData()
{
    HRESULT h_r = S_OK;

    CT_Sqlite& co_db = *pco_Lexeme->pco_Db;
    int i_hash = i_GramHash();
    try
    {
        co_db.v_PrepareForInsert (L"test_data", 3);
        co_db.v_Bind (1, pco_Lexeme->i_DbKey);  // descriptor id
        co_db.v_Bind (2, i_hash);
        co_db.v_Bind (3, str_WordForm);
        co_db.v_InsertRow();
        co_db.v_Finalize();
        
        __int64 ll_wordFormId = co_db.ll_GetLastKey();

        map<int, bool>::iterator it_stress = map_Stress.begin();
        for (; it_stress != map_Stress.end(); ++it_stress)
        {
            co_db.v_PrepareForInsert (L"test_data_stress", 3);
            co_db.v_Bind (1, ll_wordFormId);
            co_db.v_Bind (2, (*it_stress).first);
            co_db.v_Bind (3, (*it_stress).second);
            co_db.v_InsertRow();
            co_db.v_Finalize();
        }
    }
    catch (CT_Exception& co_exc)
    {
        wstring str_msg (co_exc.str_GetDescription());
        wstring str_error;
        try
        {
            co_db.v_GetLastError (str_error);
            str_msg += wstring (L", error %d: ");
            str_msg += str_error;
        }
        catch (...)
        {
            str_msg = L"Apparent DB error ";
        }
    
        CString cs_msg;
        cs_msg.Format (str_msg.c_str(), co_db.i_GetLastError());
        ERROR_LOG ((LPCTSTR)cs_msg);

        return MAKE_HRESULT (SEVERITY_ERROR, FACILITY_ITF, 0xF000 + co_db.i_GetLastError());
    }

    return S_OK;

}   //  SaveTestData()

HRESULT CT_WordForm::Verify (long l_storedLexemeId, 
                             ET_TestResult * peo_result, 
                             IWordForm ** pp_generatedWf)
{
    HRESULT h_r = S_OK;

    if (!pco_Lexeme->pco_Db)
    {
        ERROR_LOG (L"No DB handle.");
        ATLASSERT(0);
        return E_POINTER;
    }

    if (pco_Lexeme->m_coll.empty())
    {
        ERROR_LOG (L"No word forms in lexeme object.");
        ATLASSERT(0);
        return E_UNEXPECTED;
    }

    wstring str_query (L"SELECT DISTINCT id, wordform FROM test_data");
    str_query += L" WHERE lexeme_id = ";
    str_query += str_ToString (l_storedLexemeId);
    str_query += L" AND hash = ";
    str_query += str_ToString (i_GramHash());
    str_query += L";";

    try
    {
        pco_Lexeme->pco_Db->v_PrepareForSelect (str_query);

        while (pco_Lexeme->pco_Db->b_GetRow())
        {
            int i_formId = -1;
            wstring str_savedWf;
            pco_Lexeme->pco_Db->v_GetData (0, i_formId);
            pco_Lexeme->pco_Db->v_GetData (1, str_savedWf);

            CComObject<CT_WordForm> * pco_savedWf;
            h_r = CComObject<CT_WordForm>::CreateInstance (&pco_savedWf);
            if (S_OK != h_r)
            {
                ERROR_LOG (L"CreateInstance failed on CT_WordForm.");
                return h_r;
            }

            pco_savedWf->pco_Lexeme = pco_Lexeme;

            CT_Hasher co_hasher;
            co_hasher.v_FromHash (i_GramHash(), *pco_savedWf);
            pco_savedWf->str_WordForm = str_savedWf;

            wstring str_stressQuery 
                (L"SELECT position, is_primary FROM test_data_stress WHERE form_id = ");
            str_stressQuery += str_ToString (i_formId);
            unsigned int ui_stressHandle = pco_Lexeme->pco_Db->ui_PrepareForSelect (str_stressQuery);
            map<int, bool> map_savedStress;
            while (pco_Lexeme->pco_Db->b_GetRow (ui_stressHandle))
            {
                int i_pos = -1;
                bool b_primary = false;
                pco_Lexeme->pco_Db->v_GetData (0, i_pos, ui_stressHandle);
                pco_Lexeme->pco_Db->v_GetData (1, b_primary, ui_stressHandle);

                map_savedStress[i_pos] = b_primary;
            }

            pco_savedWf->map_Stress = map_savedStress;

            //
            // More than one word form may be generated for a single hash; if there is
            // a generated form that matches the saved form, we mark this as a
            // success and exit; if none of the generated forms matched the saved form,
            // we return fail.
            //
            if (str_WordForm != str_savedWf || map_Stress != map_savedStress)
            {
                *peo_result = TEST_RESULT_FAIL;
            }
            else
            {
                *peo_result = TEST_RESULT_OK;
                return S_OK;
            }
        }
    }
    catch (...)
    {
        wstring str_msg;
        try
        {
            wstring str_error;
            pco_Lexeme->pco_Db->v_GetLastError (str_error);
            str_msg = L"DB error %d: ";
            str_msg += str_error;
        }
        catch (...)
        {
            str_msg = L"Apparent DB error ";
        }
    
        CString cs_msg;
        cs_msg.Format (str_msg.c_str(), pco_Lexeme->pco_Db->i_GetLastError());
        ERROR_LOG ((LPCTSTR)cs_msg);
 
        return E_FAIL;
    }

    return S_OK;

}   //  Verify (...)

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
        HRESULT h_r = co_h.h_DecodeHash (i_hash);
        this->eo_Animacy = co_h.eo_Animacy;
        this->eo_Aspect = co_h.eo_Aspect;
        this->eo_Case = co_h.eo_Case;
        this->eo_Gender = co_h.eo_Gender;
        this->eo_Number = co_h.eo_Number;
        this->eo_Person = co_h.eo_Person;
        this->eo_POS = co_h.eo_POS;
        this->eo_Reflexive = co_h.eo_Reflexive;
        this->eo_Subparadigm = co_h.eo_Subparadigm;
        return h_r;
    }
    catch (CT_Exception co_ex)
    {
        return co_ex.i_GetErrorCode();  // logging should be always done by callee
    }
}
