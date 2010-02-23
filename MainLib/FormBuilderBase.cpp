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

void CT_FormBuilderBase::v_ReportDbError()
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
