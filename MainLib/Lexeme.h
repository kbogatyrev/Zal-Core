#pragma once
#include "resource.h"       // main symbols
#include "MainLib_i.h"
#include "SqliteWrapper.h"
#include "WordForm.h"
#include "Endings.h"

#include "VCUE_Collection.h"
#include "VCUE_Copy.h"

using namespace std;
using namespace std::tr1;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

static wstring str_CyrillicLowercase = L"-абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
static wstring str_Vowels = L"аеёиоуыэюя";
static wstring str_Consonants = L"бвгджзйклмнпрстфхцчшщ";
static wstring str_ShSounds = L"шжчщц";

enum ET_MainSymbol
{
    MS_START,
    MS_M = MS_START,
    MS_MO,
    MS_ZH,
    MS_ZHO,
    MS_S,
    MS_SO,
    MS_MO_ZHO,
    MS_MN,
    MS_MN_NEOD,
    MS_MN_ODUSH,
    MS_MN_OT,
    MS_P,
    MS_MS,
    MS_MS_P,
    MS_CHISL,
    MS_CHISL_P,
    MS_SV,
    MS_NSV,
    MS_SV_NSV,
    MS_N,
    MS_PREDL,
    MS_SOJUZ,
    MS_PREDIK,
    MS_VVODN,
    MS_SRAVN,
    MS_CHAST,
    MS_MEZHD,
    MS_END,
    MS_UNDEFINED
};
static void operator++ (ET_MainSymbol& eo_ms) 
{
    eo_ms = (ET_MainSymbol)(eo_ms + 1);
}

static void operator++ (ET_Case& eo_c) 
{
    eo_c = (ET_Case)(eo_c + 1);
}

static void operator++ (ET_Number& eo_n) 
{
    eo_n = (ET_Number)(eo_n + 1);
}

static void operator++ (ET_Gender& eo_g) 
{
	eo_g = (ET_Gender)(eo_g + 1);
}

static void operator++ (ET_Animacy& eo_a) 
{
    eo_a = (ET_Animacy)(eo_a + 1);
}

static void operator++ (ET_Person& eo_p) 
{
    eo_p = (ET_Person)(eo_p + 1);
}

// We always need to provide the following information
//typedef std::vector<CComVariant> ContainerType;
//typedef VARIANT	ExposedType;
//typedef IEnumVARIANT EnumeratorInterface;

// Typically the copy policy can be calculated from the typedefs defined above:
//typedef VCUE::GenericCopy<ExposedType, ContainerType::value_type> CopyType;

// However, we may want to use a different class, as in this case:
// typedef VCUE::MapCopy<ContainerType, ExposedType>				CopyType;
	// (The advantage of MapCopy is that we don't need to provide implementations 
	//  of GenericCopy for all the different pairs of key and value types)

// Now we have all the information we need to fill in the template arguments on the implementation classes
typedef CComEnumOnSTL <IEnumVARIANT,                            // enumerator interface
                       &__uuidof(IEnumVARIANT), 
                       VARIANT,                                 // exposed type
                       VCUE::GenericCopy<VARIANT,               // copy type
                                         std::vector<CComVariant>::value_type>,
                       vector<CComVariant> > EnumeratorType;    // container type

typedef VCUE::ICollectionOnSTLCopyImpl <ILexeme, 
                                        std::vector<CComVariant>, 
                                        VARIANT,
                                        VCUE::GenericCopy<VARIANT, std::vector<CComVariant>::value_type>,
                                        EnumeratorType> WordFormCollection;

class ATL_NO_VTABLE CT_Lexeme :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CT_Lexeme, &CLSID_ZalLexeme>,
    public IDispatchImpl<WordFormCollection, &IID_ILexeme, &LIBID_MainLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
friend class CT_Dictionary;

public:
	CT_Lexeme()
	{
	}

//DECLARE_REGISTRY_RESOURCEID(IDR_LEXEME)
    DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CT_Lexeme)
	COM_INTERFACE_ENTRY(ILexeme)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
        v_Init();
		return S_OK;
	}

	void FinalRelease()
	{
        delete pco_Endings;
        delete pco_Db;
	}

public:
    STDMETHOD (GenerateWordForms) ();

	STDMETHOD (get_LexemeId) (long *);
    STDMETHOD (get_GraphicStem) (BSTR *);
    STDMETHOD (get_InitialForm) (BSTR *);
    STDMETHOD (get_IsVariant) (BOOL *);
    STDMETHOD (get_MainSymbol) (BSTR *);
    STDMETHOD (get_IsPluralOf) (BOOL *);
    STDMETHOD (get_IsTransitive) (BOOL *);
    STDMETHOD (get_MainSymbolPluralOf) (BSTR *);
    STDMETHOD (get_AltMainSymbol) (BSTR *);
    STDMETHOD (get_InflectionType) (BSTR *);
    STDMETHOD (get_PartOfSpeech) (ET_PartOfSpeech *);
    STDMETHOD (get_Comment) (BSTR *);
    STDMETHOD (get_AltMainSymbolComment) (BSTR *);
    STDMETHOD (get_AltInflectionComment) (BSTR *);
    STDMETHOD (get_VerbStemAlternation) (BSTR *);
    STDMETHOD (get_Section) (LONG *);
    STDMETHOD (get_NoComparative) (BOOL *);
    STDMETHOD (get_HasAssumedForms) (BOOL *);
    STDMETHOD (get_HasYoAlternation) (BOOL *);
    STDMETHOD (get_HasOAlternation) (BOOL *);
    STDMETHOD (get_HasSecondGenitive) (BOOL *);
    STDMETHOD (get_QuestionableForms) (BSTR *);
    STDMETHOD (get_IrregularForms) (BSTR *);
    STDMETHOD (get_RestrictedForms) (BSTR *);
    STDMETHOD (get_Contexts) (BSTR *);
    STDMETHOD (get_TrailingComment) (BSTR *);

// From Inflection table:
    STDMETHOD (get_IsPrimaryInflectionGroup) (BOOL *);
    STDMETHOD (get_Type) (LONG *);
    STDMETHOD (get_AccentType1) (ET_AccentType *);
    STDMETHOD (get_AccentType2) (ET_AccentType *);
    STDMETHOD (get_ShortFormsAreRestricted) (BOOL *);
    STDMETHOD (get_PastParticipleIsRestricted) (BOOL *);
    STDMETHOD (get_NoShortForms) (BOOL *);
    STDMETHOD (get_NoPastParticiple) (BOOL *);
    STDMETHOD (get_HasFleetingVowel) (BOOL *);
    STDMETHOD (get_HasStemAugment) (BOOL *);

private:
    map<wstring, ET_MainSymbol> map_MainSymbol;
    map<wstring, ET_Gender> map_MainSymbolToGender;
    map<wstring, ET_Animacy> map_MainSymbolToAnimacy;
    map<wstring, wstring> map_StandardAlternations;
    multimap<int, int> mmap_EndingsHash;
    vector<wstring> vec_AlternatingPreverbs, vec_AlternatingPreverbsWithVoicing;
    wstring str_DbPath;
    CT_Sqlite * pco_Db;
    CT_Endings * pco_Endings;

    //
    // Stem data as acquired from the DB
    //

    // From descriptor table:
    int i_DbKey;
    wstring str_SourceForm;
    int i_SourceStressPos;
    int i_SecondarySourceStressPos;
    wstring str_GraphicStem;
    bool b_IsVariant;
    wstring str_MainSymbol;
    bool b_IsPluralOf;
    bool b_Transitive;
    wstring str_MainSymbolPluralOf;
    wstring str_AltMainSymbol;
    wstring str_InflectionType;
    ET_PartOfSpeech eo_PartOfSpeech;
    wstring str_Comment;
    wstring str_AltMainSymbolComment;
    wstring str_AltInflectionComment;
    wstring str_VerbStemAlternation;
    int i_Section;
    bool b_NoComparative;
    bool b_AssumedForms;
    bool b_YoAlternation;
    bool b_OAlternation;
    bool b_SecondGenitive;
    wstring str_QuestionableForms;
    wstring str_IrregularForms;
    wstring str_RestrictedFroms;
    wstring str_Contexts;
    wstring str_TrailingComment;

    // From inflection table:
    bool b_PrimaryInflectionGroup;
    int i_Type;
    ET_AccentType eo_AccentType1;
    ET_AccentType eo_AccentType2;
    bool b_ShortFormsRestricted;
    bool b_PastParticipleRestricted;
    bool b_NoShortForms;
    bool b_NoPastParticiple;
    bool b_FleetingVowel;
    bool b_StemAugment;
    int i_CommonDeviation;

    //
    // Verb stems:
    //
    CT_ExtString xstr_InfStem;
    CT_ExtString xstr_Infinitive;
    CT_ExtString xstr_1SgStem;
    CT_ExtString xstr_3SgStem;
    CT_ExtString xstr_1SgEnding;

    //
    // Helpers:
    //
    void v_Init();
    void v_SetDb (const wstring& str_dbPath);
    HRESULT h_GetNounEndings();
	HRESULT h_GetLongFormEndings();
    HRESULT h_GetShortFormEndings();
    HRESULT h_GetPersonalEndings();
    HRESULT h_GenerateParadigm();
    HRESULT h_BuildNounForms();
    HRESULT h_BuildAdjForms();
    HRESULT h_BuildVerbStems();
    HRESULT h_BuildPersonalForms();
    HRESULT h_BuildLongForms();
    HRESULT h_BuildShortForms();
    HRESULT h_BuildComparative();
    HRESULT h_StressOnNounEnding (ET_Number, ET_Case);
    HRESULT h_StressOnShortFormEnding (ET_Number eo_number, ET_Gender eo_gender);

    // Overload for nouns/adjectives:
    HRESULT h_HandleFleetingVowel (ET_Number eo_number,        // in
                                   ET_Case eo_case,            // in
                                   ET_Gender eo_gender,        // in, adj only
                                   ET_AdjForm eo_longOrShort,  // in, adj only
                                   const wstring& str_ending,  // in
                                   wstring& str_lemma);        // out  

    // Overload for verbs:
    HRESULT h_HandleFleetingVowel (CT_ExtString& xstr_stem); 

    HRESULT h_HandleYoAlternation (ET_EndingStressType eo_type,  // in
                                   bool b_shortForm,             // in
                                   wstring& str_lemma);          // in/out
    HRESULT h_GetStressPosition (const wstring& str_lemma, 
                                 const wstring& str_ending, 
                                 ET_EndingStressType eo_type, 
                                 int& i_position,
                                 bool b_shortForm = false);
    HRESULT h_GetDeclStemStressPosition (const wstring& str_lemma, 
                                         int& i_position, 
                                         bool b_shortForm = false);
    HRESULT h_GetVerbStemStressPosition (const wstring& str_lemma, 
                                         int& i_position);
    HRESULT h_StandardAlternation (CT_ExtString& xstr_presentStem);

};

OBJECT_ENTRY_AUTO(__uuidof(ZalLexeme), CT_Lexeme)
