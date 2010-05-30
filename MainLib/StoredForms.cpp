#include "StdAfx.h"
#include "StoredForms.h"
#include "WordForm.h"

HRESULT CT_StoredForms::get_LexemeId (LONG * pl_id)
{
    *pl_id = i_LexemeId;

    return S_OK;
}

HRESULT CT_StoredForms::get_Headword (BSTR * pbstr_source)
{
    CComBSTR sp_bstr (str_Headword.c_str());
    *pbstr_source = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_StoredForms::GetForms()
{
    HRESULT h_r = S_OK;

    if (NULL == pco_Db)
    {
        ERROR_LOG (L"DB pointer is NULL.");
        return E_POINTER;
    }

    wstring str_select (L"SELECT DISTINCT hash, wordform FROM test_data");
    str_select += L" WHERE lexeme_id = ";
    str_select += str_ToString (i_LexemeId);
    str_select += L";";

    m_coll.clear();

    try
    {
        pco_Db->v_PrepareForSelect (str_select);

        while (pco_Db->b_GetRow())
        {
            CComObject<CT_WordForm> * pco_wf;
            h_r = CComObject<CT_WordForm>::CreateInstance (&pco_wf);
            if (S_OK != h_r)
            {
                ERROR_LOG (L"CreateInstance failed on CT_WordForm.");
                return h_r;
            }

//            sp_lexeme->v_SetDb (str_DbPath);

            int i_gramHash = -1;
            pco_Db->v_GetData (0, i_gramHash);
            pco_Db->v_GetData (1, pco_wf->str_WordForm);

            CT_Hasher co_hasher;
            co_hasher.v_FromHash (i_gramHash, *pco_wf);

            CComQIPtr<IWordForm> spqi_wf (pco_wf);
            if (!spqi_wf)
            {
                ATLASSERT(0);
                ERROR_LOG (L"QI for IWordForm failed");
                return E_NOINTERFACE;
            }

            pair<int, CComVariant> co_pair (i_gramHash, CComVariant (spqi_wf));
            m_coll.insert (co_pair);
        }
    }
    catch (...)
    {
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
 
        return E_FAIL;
    }

    return S_OK;

}   //  GetForms()


HRESULT CT_StoredForms::get_VariantCount (LONG l_gramHash, LONG l_at, LONG * pl_count)
{
    *pl_count = m_coll.count (l_gramHash);

    return S_OK;
}

HRESULT CT_StoredForms::get_Source (long l_at, BSTR * pbstr_source)
{
    CComBSTR sp_bstr (str_Headword.c_str());
    *pbstr_source = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_StoredForms::get_WordForm (long l_gramHash, long l_at, IWordForm ** ppVal)
{
    HRESULT h_r = S_OK;

    pair<multimap<int, CComVariant>::iterator, multimap<int, CComVariant>::iterator> pair_range;
    pair_range = m_coll.equal_range (l_gramHash);
    if (pair_range.first == pair_range.second)
    {
        ATLASSERT(0);
        wstring str_msg (L"Unable to locate word form in collection.");
        ERROR_LOG (str_msg);
        return E_FAIL;
    }

    if (distance (pair_range.first, pair_range.second) - 1 < l_at)
    {
        ATLASSERT(0);
        wstring str_msg (L"Bad form number.");
        ERROR_LOG (str_msg);
        return E_INVALIDARG;
    }

    multimap<int, CComVariant>::iterator it_wf (pair_range.first);
    advance (it_wf, l_at);
    if ((*it_wf).first != l_gramHash)
    {
        ATLASSERT(0);
        wstring str_msg (L"Error extracting map element.");
        ERROR_LOG (str_msg);
        return E_INVALIDARG;
    }

    CComQIPtr<IWordForm> sp_wf = (*it_wf).second.pdispVal;
    if (NULL == sp_wf)
    {
        ATLASSERT(0);
        wstring str_msg (L"QI for IWordForm failed.");
        ERROR_LOG (str_msg);
        return E_POINTER;
    }

    *ppVal = sp_wf.Detach();

// TODO stress

    return E_NOTIMPL;
}
