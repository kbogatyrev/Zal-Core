#pragma once
#include "resource.h"       // main symbols
#include "MainLib_i.h"
#include "SqliteWrapper.h"
#include "GramHasher.h"

using namespace std;
using namespace std::tr1;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

class CHasher;

class ATL_NO_VTABLE CWordForm :
	public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CWordForm, &CLSID_ZalWordForm>,
//    public IWordForm,
    public IDispatchImpl<IWordForm, &IID_IWordForm, &LIBID_MainLib, /*wMajor =*/ 1, /*wMinor =*/ 0>

{
friend class CLexeme;
friend class CFormBuilderBase;
friend class CFormBuilderBaseConj;
friend class CFormBuilderNouns;
friend class CFormBuilderLongAdj;
friend class CFormBuilderShortAdj;
friend class CFormBuilderPronounAdj;
friend class CFormBuilderPersonal;
friend class CFormBuilderPast;
friend class CFormBuilderImperative;
friend class CFormBuilderNonFinite;
friend class CFormBuilderComparative;
friend class CDictionary;
friend class CAnalyzer;
friend class CHasher;
friend class CVerifier;

public:
    CWordForm()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_WORDFORM)
//    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CWordForm)
        COM_INTERFACE_ENTRY(IWordForm)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
        Init();
		return S_OK;
	}

	void FinalRelease()
	{}

public:
// IWordForm
    STDMETHOD (get_Wordform) (BSTR *);
    STDMETHOD (get_Lemma) (BSTR *);
    STDMETHOD (get_LexemeId) (LONG *);
    STDMETHOD (get_PartOfSpeech) (ET_PartOfSpeech *);
    STDMETHOD (get_Subparadigm) (ET_Subparadigm *);
    STDMETHOD (get_Case) (ET_Case *);
    STDMETHOD (get_Number) (ET_Number *);
    STDMETHOD (get_Gender) (ET_Gender *);
    STDMETHOD (get_Person) (ET_Person *);
    STDMETHOD (get_IsReflexive) (ET_Reflexive *);
    STDMETHOD (get_Animacy) (ET_Animacy *);
    STDMETHOD (get_Aspect) (ET_Aspect *);
    STDMETHOD (get_Stress) (LONG * pl_stressPos);
    STDMETHOD (GetStressPos) (LONG l_at, LONG * pl_stressPosition, BOOL * pb_isPrimary);
    STDMETHOD (get_StressCount) (LONG * p_lVal);
    STDMETHOD (get_IsPrimaryStress) (LONG l_pos, BOOL * pb_type);

    STDMETHOD (get_Status) (ET_Status *);

    STDMETHOD (get_Hash) (LONG * pVal);

private:
    CLexeme * m_pLexeme;
    CEString m_sWordForm;
    CEString m_sLemma;
    __int64 m_llLexemeId;
    map<int, ET_StressType> m_mapStress; // <stressed syll, primary/secondary>
    ET_PartOfSpeech m_ePos;
    ET_Case m_eCase;
    ET_Number m_eNumber;
    ET_Subparadigm m_eSubparadigm;
    ET_Gender m_eGender;
    ET_Person m_ePerson;
    ET_Animacy m_eAnimacy;
    ET_Reflexive m_eReflexive;
    ET_Aspect m_eAspect;
    ET_Status m_eStatus;

    /*
    static const long MIN_NOUN = 0;
    static const long MIN_ADJ = 1000;
    static const long MIN_NUM = 2000;
    static const long MIN_PRONOUN = 3000;
    static const long MIN_VERB = 4000;
    static const long MIN_UNCHANGEABLE = 20000;
    */

    void Init()
    {
        m_sWordForm = L"";
        m_sLemma = L"";
        m_llLexemeId = -1;
        m_ePos = POS_UNDEFINED;
        m_eCase = CASE_UNDEFINED;
        m_eNumber = NUM_UNDEFINED;
        m_eSubparadigm = SUBPARADIGM_UNDEFINED;
        m_eGender = GENDER_UNDEFINED;
        m_ePerson = PERSON_UNDEFINED;
        m_eAnimacy = ANIM_UNDEFINED;
        m_eReflexive = REFL_UNDEFINED;
        m_eAspect = ASPECT_UNDEFINED;
        m_eStatus = STATUS_COMMON;

        return;
    }

    int iGramHash();
    HRESULT hDecodeHash (int iHash);    
    HRESULT SaveTestData();

};

class CHasher : public CGramHasher
{
    friend class CWordForm;
    friend class CAnalyzer;

public:
    CHasher() {}
    CHasher (const CWordForm& wf) : CGramHasher (wf.m_ePos, 
                                                 wf.m_eSubparadigm, 
                                                 wf.m_eCase, 
                                                 wf.m_eNumber, 
                                                 wf.m_eGender,  
                                                 wf.m_ePerson, 
                                                 wf.m_eAnimacy,
                                                 wf.m_eAspect,
                                                 wf.m_eReflexive)
    {}

    void FromHash (int iHash, CWordForm& wf)
    {
        HRESULT hr = hDecodeHash (iHash);
        if (S_OK != hr)
        {
            ATLASSERT(0);
            CString csMsg;
            csMsg.Format (L"hDecodeHash() failed, error %x.", hr);
            ERROR_LOG ((LPCTSTR)csMsg);
            throw CException (hr, (LPCTSTR)csMsg);
        }
        
        wf.m_ePos = m_ePos;
        wf.m_eSubparadigm = m_eSubparadigm;
        wf.m_eCase = m_eCase;
        wf.m_eNumber = m_eNumber;
        wf.m_eGender = m_eGender;
        wf.m_ePerson = m_ePerson;
        wf.m_eAnimacy = m_eAnimacy;
        wf.m_eReflexive = m_eReflexive;
        wf.m_eAspect = m_eAspect;
    }

private:
    __int64 m_llLexemeId;

};

OBJECT_ENTRY_AUTO(__uuidof(ZalWordForm), CWordForm)
