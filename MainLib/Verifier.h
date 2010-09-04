#pragma once
#include "resource.h"       // main symbols
#include "MainLib_i.h"

#include "VCUE_Collection.h"
#include "VCUE_Copy.h"

using namespace std;
using namespace std::tr1;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

class ATL_NO_VTABLE CT_Verifier :
    public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CT_Verifier, &CLSID_ZalVerifier>,
//    public IVerifier,
    public IDispatchImpl<IVerifier, &IID_IVerifier, &LIBID_MainLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
friend class CT_TestData;

public:
    CT_Verifier()
	{
	}

//    DECLARE_NO_REGISTRY()
    DECLARE_REGISTRY_RESOURCEID(IDR_VERIFIER)

BEGIN_COM_MAP(CT_Verifier)
    COM_INTERFACE_ENTRY(IVerifier)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
        pco_Db = NULL;
        eo_Result = TEST_RESULT_UNDEFINED;
        
        return S_OK;
	}

	void FinalRelease()
    {}
//        delete pco_Db;
//	}

public:
// IVerifier
    STDMETHOD (put_DbPath) (BSTR);
    STDMETHOD (put_Headword) (BSTR);
    STDMETHOD (get_Headword) (BSTR *);
    STDMETHOD (put_LexemeId) (ULONG);
    STDMETHOD (get_LexemeId) (ULONG *);
    STDMETHOD (Verify) (ULONG ul_lexemeId, ET_TestResult * pe_result);
    STDMETHOD (get_Result) (ET_TestResult *);

private:
    HRESULT h_LoadStoredForms();

private:
    ULONG ul_LexemeHash;
//    int i_DescriptorId;
    wstring str_Headword;
    CT_Sqlite * pco_Db;
    wstring str_DbPath;
    multimap<int, IWordForm *> mmap_StoredForms;
    ET_TestResult eo_Result;
};

OBJECT_ENTRY_AUTO(__uuidof(ZalVerifier), CT_Verifier)
