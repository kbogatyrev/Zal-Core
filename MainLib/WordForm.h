#pragma once
#include "resource.h"       // main symbols
#include "MainLib_i.h"
#include "SqliteWrapper.h"

using namespace std;
using namespace std::tr1;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

/*
class CT_WordForm
{
public:
    wstring str_wordform;
    wstring str_lemma;
    __int64 ll_lexeme_id;
    ET_PartOfSpeech eo_POS;
    ET_Case eo_Case;
    ET_Number eo_Number;
    ET_AdjForm eo_AdjForm;
    ET_Gender eo_Gender;
    ET_Tense eo_Tense;
    ET_Person eo_Person;
    ET_Animacy eo_Animacy;
    ET_Reflexive eo_Reflexive;
    ET_Voice eo_Voice;
    ET_SpecialForm eo_Special;
    
    CT_WordForm()
    {
        eo_POS = POS_UNDEFINED;
        eo_Case = CASE_UNDEFINED;
        eo_Number = NUM_UNDEFINED;
        eo_AdjForm = FORM_UNDEFINED;
        eo_Gender = GENDER_UNDEFINED;
        eo_Tense = TENSE_UNDEFINED;
        eo_Person = PERSON_UNDEFINED;
        eo_Reflexive = REFL_UNDEFINED;
        eo_Voice = VOICE_UNDEFINED;
        eo_Animacy = ANIM_UNDEFINED;
        eo_Special = SPECIAL_UNDEFINED;
    }

    CT_WordForm (wstring str_wf)
    {
        eo_POS = POS_UNDEFINED;
        eo_Case = CASE_UNDEFINED;
        eo_Number = NUM_UNDEFINED;
        eo_AdjForm = FORM_UNDEFINED;
        eo_Gender = GENDER_UNDEFINED;
        eo_Tense = TENSE_UNDEFINED;
        eo_Person = PERSON_UNDEFINED;
        eo_Reflexive = REFL_UNDEFINED;
        eo_Voice = VOICE_UNDEFINED;
        eo_Animacy = ANIM_UNDEFINED;
        eo_Special = SPECIAL_UNDEFINED;
        str_wordform = str_wf;
    }

    CT_WordForm (wstring str_wf, __int64 ll_id)
    {
        eo_POS = POS_UNDEFINED;
        eo_Case = CASE_UNDEFINED;
        eo_Number = NUM_UNDEFINED;
        eo_AdjForm = FORM_UNDEFINED;
        eo_Gender = GENDER_UNDEFINED;
        eo_Tense = TENSE_UNDEFINED;
        eo_Person = PERSON_UNDEFINED;
        eo_Reflexive = REFL_UNDEFINED;
        eo_Voice = VOICE_UNDEFINED;
        eo_Animacy = ANIM_UNDEFINED;
        eo_Special = SPECIAL_UNDEFINED;
        str_wordform = str_wf;
        ll_lexeme_id = ll_id;
    }

    ~CT_WordForm() {}

    void GramClear()
    {
        eo_Case = CASE_UNDEFINED;
        eo_Number = NUM_UNDEFINED;
        eo_AdjForm = FORM_UNDEFINED;
        eo_Gender = GENDER_UNDEFINED;
        eo_Tense = TENSE_UNDEFINED;
        eo_Person = PERSON_UNDEFINED;
        eo_Reflexive = REFL_UNDEFINED;
        eo_Voice = VOICE_UNDEFINED;
        eo_Animacy = ANIM_UNDEFINED;
        eo_Special = SPECIAL_UNDEFINED;

        return;
    }

    void Clear()
    {
        GramClear();
        eo_POS = POS_UNDEFINED;
        str_wordform = L"";
        str_lemma = L"";
        ll_lexeme_id = -1;

        return;
    }

    int i_GramNumber();
    int i_DecodeNumber (int i_gram);
    int i_DecodeString (wstring str_gram);
*/

class ATL_NO_VTABLE CT_WordForm :
	public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CT_WordForm, &CLSID_ZalWordForm>,
    public IWordForm
{
friend class CT_Lexeme;

public:

    friend class CT_Dictionary;

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
		return S_OK;
	}

	void FinalRelease()
	{}

public:
    STDMETHOD (get_Wordform) (BSTR *);
    STDMETHOD (get_Lemma) (BSTR *);
    STDMETHOD (get_LexemeId) (LONG *);
    STDMETHOD (get_PartOfSpeech) (ET_PartOfSpeech *);
    STDMETHOD (get_Case) (ET_Case *);
    STDMETHOD (get_Number) (ET_Number *);
    STDMETHOD (get_IsShortForm) (BOOL *);
    STDMETHOD (get_Gender) (ET_Gender *);
    STDMETHOD (get_Tense) (ET_Tense *);
    STDMETHOD (get_Person) (ET_Person *);
    STDMETHOD (get_IsReflexive) (BOOL *);
    STDMETHOD (get_Voice) (ET_Voice *);
    STDMETHOD (get_SpecialForm) (ET_SpecialForm *);

private:

    wstring str_WordForm;
    wstring str_Lemma;
    __int64 ll_Lexeme_id;
    int i_Stress;
    ET_PartOfSpeech eo_POS;
    bool b_ShortForm;
    ET_Case eo_Case;
    ET_Number eo_Number;
    ET_AdjForm eo_AdjForm;
    ET_Gender eo_Gender;
    ET_Tense eo_Tense;
    ET_Person eo_Person;
    bool b_Reflexive;
    ET_Animacy eo_Animacy;
    ET_Reflexive eo_Reflexive;
    ET_Voice eo_Voice;
    ET_SpecialForm eo_Special;

    void GramClear()
    {
        eo_Case = CASE_UNDEFINED;
        eo_Number = NUM_UNDEFINED;
        eo_AdjForm = ADJ_FORM_UNDEFINED;
        eo_Gender = GENDER_UNDEFINED;
        eo_Tense = TENSE_UNDEFINED;
        eo_Person = PERSON_UNDEFINED;
        eo_Reflexive = REFL_UNDEFINED;
        eo_Voice = VOICE_UNDEFINED;
        eo_Animacy = ANIM_UNDEFINED;
        eo_Special = SPECIAL_UNDEFINED;

        return;
    }

    void Clear()
    {
        GramClear();
        eo_POS = POS_UNDEFINED;
        str_WordForm = L"";
        str_Lemma = L"";
        ll_Lexeme_id = -1;

        return;
    }

    int i_GramNumber();
    int i_DecodeNumber (int i_gram);
    int i_DecodeString (wstring str_gram);

};

OBJECT_ENTRY_AUTO(__uuidof(ZalWordForm), CT_WordForm)
