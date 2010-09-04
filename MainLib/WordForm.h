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

class CT_Hasher;

class ATL_NO_VTABLE CT_WordForm :
	public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CT_WordForm, &CLSID_ZalWordForm>,
//    public IWordForm,
    public IDispatchImpl<IWordForm, &IID_IWordForm, &LIBID_MainLib, /*wMajor =*/ 1, /*wMinor =*/ 0>

{
friend class CT_Lexeme;
friend class CT_FormBuilderBase;
friend class CT_FormBuilderBaseConj;
friend class CT_FormBuilderNouns;
friend class CT_FormBuilderLongAdj;
friend class CT_FormBuilderShortAdj;
friend class CT_FormBuilderPersonal;
friend class CT_FormBuilderPast;
friend class CT_FormBuilderImperative;
friend class CT_FormBuilderNonFinite;
friend class CT_FormBuilderComparative;
friend class CT_Dictionary;
friend class CT_Analyzer;
friend class CT_Hasher;
friend class CT_Verifier;

public:
    CT_WordForm()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_WORDFORM)
//    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CT_WordForm)
        COM_INTERFACE_ENTRY(IWordForm)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
        v_Init();
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
    STDMETHOD (get_Tense) (ET_Tense *);
    STDMETHOD (get_Person) (ET_Person *);
    STDMETHOD (get_IsReflexive) (ET_Reflexive *);
    STDMETHOD (get_Animacy) (ET_Animacy *);
    STDMETHOD (get_Voice) (ET_Voice *);
    STDMETHOD (get_SpecialForm) (ET_SpecialForm *);
    STDMETHOD (get_Aspect) (ET_Aspect *);
    STDMETHOD (get_Stress) (LONG * pl_stressPos);
    STDMETHOD (GetStressPos) (LONG l_at, LONG * pl_stressPosition, BOOL * pb_isPrimary);
    STDMETHOD (get_StressCount) (LONG * p_lVal);
    STDMETHOD (get_IsPrimaryStress) (LONG l_pos, BOOL * pb_type);

    STDMETHOD (get_Status) (ET_Status *);

    STDMETHOD (get_Hash) (LONG * pVal);

private:
    CT_Lexeme * pco_Lexeme;
    wstring str_WordForm;
    wstring str_Lemma;
    __int64 ll_Lexeme_id;
//    map<int, bool> map_Stress;  // <position, is_primary>
    map<int, ET_StressType> map_Stress;
    ET_PartOfSpeech eo_POS;
    ET_Case eo_Case;
    ET_Number eo_Number;
    ET_Subparadigm eo_Subparadigm;
    ET_Gender eo_Gender;
    ET_Tense eo_Tense;
    ET_Person eo_Person;
    ET_Animacy eo_Animacy;
    ET_Reflexive eo_Reflexive;
    ET_Voice eo_Voice;
    ET_SpecialForm eo_SpecialForm;
    ET_Aspect eo_Aspect;
    ET_Status eo_Status;

    static const long MIN_NOUN = 0;
    static const long MIN_ADJ = 1000;
    static const long MIN_NUM = 2000;
    static const long MIN_PRONOUN = 3000;
    static const long MIN_VERB = 4000;
    static const long MIN_UNCHANGEABLE = 20000;

    void v_Init()
    {
        str_WordForm = L"";
        str_Lemma = L"";
        ll_Lexeme_id = -1;
        eo_POS = POS_UNDEFINED;
        eo_Case = CASE_UNDEFINED;
        eo_Number = NUM_UNDEFINED;
        eo_Subparadigm = SUBPARADIGM_UNDEFINED;
        eo_Gender = GENDER_UNDEFINED;
        eo_Tense = TENSE_UNDEFINED;
        eo_Person = PERSON_UNDEFINED;
        eo_Animacy = ANIM_UNDEFINED;
        eo_Reflexive = REFL_UNDEFINED;
        eo_Voice = VOICE_UNDEFINED;
        eo_SpecialForm = SPECIAL_UNDEFINED;
        eo_Aspect = ASPECT_UNDEFINED;
        eo_Status = STATUS_COMMON;

        return;
    }

    int i_GramHash();
    HRESULT h_DecodeHash (int i_hash);    
    HRESULT SaveTestData();

};

class CT_Hasher : public CT_GramHasher
{
    friend class CT_WordForm;

public:
    CT_Hasher() {}
    CT_Hasher (const CT_WordForm& co_wf) : CT_GramHasher (co_wf.eo_POS, 
                                                          co_wf.eo_Subparadigm, 
                                                          co_wf.eo_Case, 
                                                          co_wf.eo_Number, 
                                                          co_wf.eo_Gender, 
                                                          co_wf.eo_Tense, 
                                                          co_wf.eo_Person, 
                                                          co_wf.eo_Animacy,
                                                          co_wf.eo_Reflexive,
                                                          co_wf.eo_Voice,
                                                          co_wf.eo_Aspect)
    {}

    void v_FromHash (int i_hash, CT_WordForm& co_wf)
    {
        HRESULT h_r = h_DecodeHash (i_hash);
        if (S_OK != h_r)
        {
            ATLASSERT(0);
            CString cs_msg;
            cs_msg.Format (L"h_DecodeHash() failed, error %x.", h_r);
            ERROR_LOG ((LPCTSTR)cs_msg);
            throw CT_Exception (h_r, (LPCTSTR)cs_msg);
        }
        
        co_wf.eo_POS = eo_POS;
        co_wf.eo_Subparadigm = eo_Subparadigm;
        co_wf.eo_Case = eo_Case;
        co_wf.eo_Number = eo_Number;
        co_wf.eo_Gender = eo_Gender;
        co_wf.eo_Tense = eo_Tense;
        co_wf.eo_Person = eo_Person;
        co_wf.eo_Animacy = eo_Animacy;
        co_wf.eo_Reflexive = eo_Reflexive;
        co_wf.eo_Voice = eo_Voice;
        co_wf.eo_SpecialForm = eo_SpecialForm;
        co_wf.eo_Aspect = eo_Aspect;
    }

};

OBJECT_ENTRY_AUTO(__uuidof(ZalWordForm), CT_WordForm)
