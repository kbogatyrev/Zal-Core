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

class ATL_NO_VTABLE CVerifier :
    public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CVerifier, &CLSID_ZalVerifier>,
//    public IVerifier,
    public IDispatchImpl<IVerifier, &IID_IVerifier, &LIBID_MainLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
friend class CTestData;

public:
    CVerifier()
	{
	}

//    DECLARE_NO_REGISTRY()
    DECLARE_REGISTRY_RESOURCEID(IDR_VERIFIER)

BEGIN_COM_MAP(CVerifier)
    COM_INTERFACE_ENTRY(IVerifier)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
        m_pDb = NULL;
        m_eResult = TEST_RESULT_UNDEFINED;
        
        return S_OK;
	}

	void FinalRelease()
    {}
//        delete m_pDb;
//	}

public:
// IVerifier
    STDMETHOD (put_DbPath) (BSTR);
    STDMETHOD (put_Headword) (BSTR);
    STDMETHOD (get_Headword) (BSTR *);
    STDMETHOD (put_LexemeId) (BSTR);
    STDMETHOD (get_LexemeId) (BSTR *);
    STDMETHOD (Verify) (BSTR bstrLexemeId, ET_TestResult * peResult);
    STDMETHOD (get_Result) (ET_TestResult *);

private:
    HRESULT hLoadStoredForms();

private:
//    ULONG m_ulLexemeHash;
    CEString m_sLexemeHash;
//    int i_DescriptorId;
    CEString m_sHeadword;
    CSqlite * m_pDb;
    CEString m_sDbPath;
    multimap<int, IWordForm *> m_mmapStoredForms;
    ET_TestResult m_eResult;
};

OBJECT_ENTRY_AUTO(__uuidof(ZalVerifier), CVerifier)
