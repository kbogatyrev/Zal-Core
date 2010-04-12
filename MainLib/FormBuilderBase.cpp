#include "StdAfx.h"
#include "FormBuilderBase.h"

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

void CT_FormBuilderBase::v_AssignSecondaryStress (CComObject<CT_WordForm> * pco_wf)
{
    CT_ExtString xstr_grStem (pco_Lexeme->str_GraphicStem);
    vector<int>::iterator it_pos = pco_Lexeme->vec_SecondaryStressPos.begin();
    for (; it_pos != pco_Lexeme->vec_SecondaryStressPos.end(); ++it_pos)
    {
        int i_stressedSyll = xstr_grStem.i_GetSyllableFromVowelPos (*it_pos);
        if (i_stressedSyll < 0)
        {
            ATLASSERT(0);
            wstring str_msg (L"Stressed syllable not found.");
            ERROR_LOG (str_msg);
            throw CT_Exception (E_FAIL, str_msg);
        }
        pco_wf->map_Stress[i_stressedSyll] = false;
    }
}

//
// GDRL p. 11: a dash in a word with two main stress marks means a compound form
// with two main stresses; otherwise 
//
bool CT_FormBuilderBase::b_MultiStress (const wstring& str_lemma, vector<int>& vec_iStressPos)
{
    if (vec_iStressPos.size() < 2)
    {
        return false;
    }

    int i_dashPos = (int)str_lemma.find (L'-');
    if (i_dashPos == wstring::npos || i_dashPos < 1)
    {
        return false;
    }

    // find any two that are separated by dash
    CT_ExtString xstr_lemma (str_lemma);
    vector<int>::iterator it_left = vec_iStressPos.begin();
    int i_leftVowelPos = xstr_lemma.i_GetVowelPos (*it_left);
    if (i_leftVowelPos < 0)
    {
        ATLASSERT(0);
        wstring str_msg (L"Can't find stressed vowel.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }
    if (i_leftVowelPos > i_dashPos)
    {
        return false;
    }

    vector<int>::iterator it_right = it_left + 1;
    for (; it_right != vec_iStressPos.end(); ++it_right)
    {
        if (xstr_lemma.i_GetVowelPos (*it_right) > i_dashPos)
        {
            return true;
        }
    }

    return false;

}   //  b_MultiStress (...)

HRESULT CT_FormBuilderBase::h_CloneWordForm (const CComObject<CT_WordForm> * p_source, 
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
    p_clone->eo_Status = p_source->eo_Status;

    return S_OK;

}  // h_CloneWordForm (...)

HRESULT CT_FormBuilderBase::h_CloneWordForm (const  CT_WordForm * p_source, 
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
    p_clone->eo_Status = p_source->eo_Status;

    return S_OK;

}  // h_CloneWordForm (...)

HRESULT CT_FormBuilderBase::h_WordFormFromHash (int i_hash, int i_at, CT_WordForm *& pco_wf)
{
    pair<multimap<int, CComVariant>::iterator, multimap<int, CComVariant>::iterator> pair_range;
    pair_range = pco_Lexeme->m_coll.equal_range (i_hash);
    if (pair_range.first == pair_range.second)
    {
        ATLASSERT(0);
        wstring str_msg (L"Unable to locate word form in collection.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_FAIL, str_msg);
    }

    if (distance (pair_range.first, pair_range.second) - 1 < i_at)
    {
        ATLASSERT(0);
        wstring str_msg (L"Bad form number.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_INVALIDARG, str_msg);
    }

    multimap<int, CComVariant>::iterator it_wf (pair_range.first);
    advance (it_wf, i_at);
    if ((*it_wf).first != i_hash)
    {
        ATLASSERT(0);
        wstring str_msg (L"Error extracting map element.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_INVALIDARG, str_msg);
    }

    CComQIPtr<IWordForm> sp_wf = (*it_wf).second.pdispVal;
    if (NULL == sp_wf)
    {
        ATLASSERT(0);
        wstring str_msg (L"QI for IWordForm failed.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_POINTER, str_msg);
    }

    pco_wf = dynamic_cast<CT_WordForm *> (sp_wf.p);
    if (NULL == pco_wf)
    {
        ATLASSERT(0);
        wstring str_msg (L"Failed to obtain 3 Pl. word form.");
        ERROR_LOG (str_msg);
        throw CT_Exception (E_POINTER, str_msg);
    }

    return S_OK;

}   //  h_WordFormFromHash (...)

int CT_FormBuilderBase::i_NForms (int i_hash)
{
    return pco_Lexeme->m_coll.count (i_hash);
}
