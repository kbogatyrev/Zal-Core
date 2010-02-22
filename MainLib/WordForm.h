#pragma once
#include "resource.h"       // main symbols
#include "MainLib_i.h"
#include "SqliteWrapper.h"

using namespace std;
using namespace std::tr1;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

class ATL_NO_VTABLE CT_WordForm :
	public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CT_WordForm, &CLSID_ZalWordForm>,
    public IWordForm
{
friend class CT_Lexeme;
friend class CT_FormBuilderBase;
friend class CT_FormBuilderNouns;
friend class CT_FormBuilderLongAdj;
friend class CT_FormBuilderShortAdj;
friend class CT_FormBuilderPersonal;
friend class CT_FormBuilderPast;
friend class CT_FormBuilderImperative;
friend class CT_FormBuilderNonFinite;
friend class CT_FormBuilderComparative;

public:

    friend class CT_Dictionary;
    friend class CT_Analyzer;

    CT_WordForm()
	{
	}

//DECLARE_REGISTRY_RESOURCEID(IDR_LEXEME)
    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CT_WordForm)
        COM_INTERFACE_ENTRY(IWordForm)
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
    STDMETHOD (get_StressPos) (long *);
    STDMETHOD (get_PolytonicStressPos) (long i_at, long * pl_pos);

    STDMETHOD (GramClear) ();
    STDMETHOD (Clear) ();
    STDMETHOD (get_GramHash) (long *);
    STDMETHOD (DecodeHash) (long);
    STDMETHOD (DecodeString) (BSTR);

private:
    wstring str_WordForm;
    wstring str_Lemma;
    __int64 ll_Lexeme_id;
    vector<int> vec_Stress;
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
//        i_Stress = -1;
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

        return;
    }

};

OBJECT_ENTRY_AUTO(__uuidof(ZalWordForm), CT_WordForm)
