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
    map<int, ET_StressType>::iterator it_pos = map_Stress.begin();
    while (it_pos->second != STRESS_PRIMARY)
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
    map<int, ET_StressType>::iterator it_stressPos = map_Stress.find (l_At);
    if (map_Stress.end() != it_stressPos)
    {
        CT_ExtString xstr_wf (str_WordForm);
        xstr_wf.v_SetVowels (str_Vowels);
        *pl_StressPosition = xstr_wf.i_GetVowelPos (it_stressPos->first);
        *pb_IsPrimary = (STRESS_PRIMARY == it_stressPos->second) ? TRUE : FALSE;

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

    map<int, ET_StressType>::iterator it_ = map_Stress.find (i_syll);
    if (map_Stress.end() == it_)
    {
        ERROR_LOG (L"Unable to find stress position.");
        return E_INVALIDARG;
    }
    else
    {
        *pb_type =  (STRESS_PRIMARY == it_->second) ? TRUE : FALSE;
        return S_OK;
    }
}

HRESULT CT_WordForm::get_Status (ET_Status * pe_status)
{
    *pe_status = (ET_Status)eo_Status;
    return S_OK;
}

HRESULT CT_WordForm::get_Hash (LONG * pl_hash)
{
    HRESULT h_r = S_OK;

    *pl_hash = i_GramHash();

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

        map<int, ET_StressType>::iterator it_stress = map_Stress.begin();
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
