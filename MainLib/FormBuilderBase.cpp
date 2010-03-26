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

void CT_FormBuilderBase::v_AssignStress (CComObject<CT_WordForm> * pco_wf, 
                                         vector<int>& vec_iStressPos, 
                                         bool b_isPrimary)
{
    vector<int>::iterator it_pos = vec_iStressPos.begin();
    for (; it_pos != vec_iStressPos.end(); ++it_pos)
    {
        pco_wf->map_Stress[*it_pos] = b_isPrimary;
    }
}

//
// GDRL p. 11: a dash in a word with two main stress marks means a compound form
// with two main stresses; otherwise 
//
int I_DashPos = -1;
bool b_PredLeftOfDash (int i_stressPos)
{
    return (i_stressPos < I_DashPos);
}

bool b_PredRightOfDash (int i_stressPos)
{
    return (i_stressPos > I_DashPos);
}

bool CT_FormBuilderBase::b_MultiStress (const wstring& str_lemma, vector<int>& vec_iStressPos)
{
    if (vec_iStressPos.size() < 2)
    {
        return false;
    }

    I_DashPos = (int)str_lemma.find (L'-');
    if (I_DashPos != wstring::npos && I_DashPos > 0)
    {
        // find any two that are separated by dash
        vector<int>::iterator it_left = find_if (vec_iStressPos.begin(), vec_iStressPos.end(), b_PredLeftOfDash);
        if (vec_iStressPos.end() == it_left)
        {
            return false;
        }
        vector<int>::iterator it_right = find_if (vec_iStressPos.begin(), vec_iStressPos.end(), b_PredRightOfDash);
        if (vec_iStressPos.end() == it_right)
        {
            return false;
        }

        if (!((*it_left < I_DashPos) && (*it_right > I_DashPos)))
        {
            ATLASSERT(0);
            wstring str_msg (L"Bad polytonic stress positions.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_FAIL, str_msg);
        }

        return true;
    }

    return false;

}   //  b_MultiStress (...)

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
    p_clone->map_Stress = p_source->map_Stress;
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

}  // h_CloneWordForm (...)

HRESULT CT_FormBuilderBase::h_WordFormFromHash (int i_hash, CComQIPtr<IWordForm>& sp_wf)
{
    multimap<int, CComVariant>::iterator it_wordForm = pco_Lexeme->m_coll.find (i_hash);
    if (pco_Lexeme->m_coll.end() == it_wordForm)
    {
        ATLASSERT(0);
        wstring str_msg (L"Unable to locate word form in collection.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }

    sp_wf = (*it_wordForm).second.pdispVal;
    if (NULL == sp_wf)
    {
        ATLASSERT(0);
        wstring str_msg (L"QI for IWordForm failed.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_POINTER, str_msg);
    }

    return S_OK;

}   //  h_WordFormFromHash (...)
