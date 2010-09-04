#include "StdAfx.h"
#include "SqliteWrapper.h"
#include "SqliteComWrapper.h"

int CT_StatusUpdate::operator()(int i_percentDone) const
{
    pco_Parent->StatusUpdate (i_percentDone);
    return i_percentDone; 
}

HRESULT CT_SqliteComWrapper::StatusUpdate (int i_progress)
{
    HRESULT h_r = S_OK;

    int i_nConnections = m_vec.GetSize();
    for (int i_c = 0; i_c < i_nConnections; ++i_c)
    {
        Lock();
        CComPtr<IUnknown> sp_unkConnection = m_vec.GetAt (i_c);
        CComQIPtr<IZalNotification> sp_qiConnection = sp_unkConnection;
        Unlock();
        if (sp_qiConnection)
        {
            h_r = sp_qiConnection->ProgressNotification (i_progress);
        }
    }

    return h_r;

}   //  StatusUpdate (...)

HRESULT CT_SqliteComWrapper::put_DbPath (BSTR bstr_dbPath)
{
    USES_CONVERSION;

    str_DbPath = OLE2W (bstr_dbPath);

    if (pco_Db)
    {
        delete pco_Db;
    }

    pco_Db = new CT_Sqlite (str_DbPath);
    if (!pco_Db)
    {
        return E_FAIL;
    }

    return S_OK;

}   //  put_DbPath (...)

HRESULT CT_SqliteComWrapper::TableExists (BSTR bstr_table, BOOL * b_exists)
{
    USES_CONVERSION;

    HRESULT h_r = S_OK;

    if (NULL == pco_Db)
    {
        ATLASSERT(0);
        ERROR_LOG (L"DB pointer is NULL.");
        return E_POINTER;
    }

    try
    {
        *b_exists = pco_Db->b_TableExists (OLE2W (bstr_table));
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

}   //  TableExists (...)

HRESULT CT_SqliteComWrapper::ExportTable (BSTR bstr_path, BSTR bstr_table)
{
    USES_CONVERSION;

    HRESULT h_r = S_OK;

    if (NULL == pco_Db)
    {
        ATLASSERT(0);
        ERROR_LOG (L"DB pointer is NULL.");
        return E_POINTER;
    }

    try
    {
        CT_StatusUpdate co_progress;
        co_progress.pco_Parent = this;
        bool b_ret = pco_Db->b_ExportTable (OLE2W (bstr_path), OLE2W (bstr_table), co_progress);
        if (!b_ret)
        {
            return E_FAIL;
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

}   //  ExportTable (...)

HRESULT CT_SqliteComWrapper::ImportTable (BSTR bstr_path, BSTR bstr_table)
{
    USES_CONVERSION;

    HRESULT h_r = S_OK;

    if (NULL == pco_Db)
    {
        ATLASSERT(0);
        ERROR_LOG (L"DB pointer is NULL.");
        return E_POINTER;
    }

    try
    {
        CT_StatusUpdate co_progress;
        co_progress.pco_Parent = this;
        bool b_ret = pco_Db->b_ImportTable (OLE2W (bstr_path), OLE2W (bstr_table), co_progress);
        if (!b_ret)
        {
            return E_FAIL;
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

}   //  ImportTable (...)