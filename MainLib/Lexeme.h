#pragma once
#include "resource.h"       // main symbols
#include "MainLib_i.h"

#include "VCUE_Collection.h"
#include "VCUE_Copy.h"

#include "WordForm.h"
#include "FormDescriptor.h"

using namespace std;
using namespace std::tr1;

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

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
static void operator++ (ET_MainSymbol& eMs) 
{
    eMs = (ET_MainSymbol)(eMs + 1);
}

struct StIrregularForm
{
    CEString sForm;
    map<int, ET_StressType> mapStress;
    bool bIsVariant;
//    bool b_Added;

    StIrregularForm() : sForm (L""), bIsVariant (false) //, b_Added (false)
    {};
    StIrregularForm (const StIrregularForm& stRhs) :
        sForm (stRhs.sForm), 
        mapStress (stRhs.mapStress), 
        bIsVariant (stRhs.bIsVariant)
    {}

};  //  StIrregularForm

namespace WordFormContainer
{
// *** This was taken from ATLCollections Sample
// I'm not using some of the typedefs listed below 
// as they only make this convoluted code even more obscure.
// However, the MSFT's typedef names appear in comments. 
//                                                  -- kb

// We always need to provide the following information
//typedef std::vector<CComVariant> ContainerType;
//typedef VARIANT	ExposedType;
//typedef IEnumVARIANT EnumeratorInterface;

// Typically the copy policy can be calculated from the typedefs defined above:
//typedef VCUE::GenericCopy<ExposedType, ContainerType::value_type> CopyType;
// However, we may want to use a different class, as in this case:
//typedef VCUE::MapCopy<ContainerType, ExposedType>				CopyType;
	// (The advantage of MapCopy is that we don't need to provide implementations 
	//  of GenericCopy for all the different pairs of key and value types)

// Now we have all the information we need to fill in the template arguments on the implementation classes

//
// Map enumerator
//
typedef CComEnumOnSTL <IEnumVARIANT,                                             // enumerator interface
                       &__uuidof(IEnumVARIANT), 
                       VARIANT,                                                  // exposed type
                       VCUE::MapCopy<std::multimap<int, CComVariant>, VARIANT>,  // copy type
                       std::multimap<int, CComVariant> > MapEnumeratorType;      // container type

//
// Map-based container
//
typedef VCUE::ICollectionOnSTLCopyImpl <ILexeme, 
                                        std::multimap<int, CComVariant>, 
                                        VARIANT,
                                        VCUE::MapCopy<std::multimap<int, CComVariant>, VARIANT>,
                                        MapEnumeratorType> MapCollection;

}   // namespace WordFormContainer

using namespace WordFormContainer;

class ATL_NO_VTABLE CLexeme :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLexeme, &CLSID_ZalLexeme>,
    public IDispatchImpl<MapCollection, &IID_ILexeme, &LIBID_MainLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
friend class CDictionary;
friend class CFormBuilderBase;
friend class CFormBuilderBaseDecl;
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
friend class CWordForm;
friend class CAnalyzer;

public:
	CLexeme()
	{
	}

    DECLARE_REGISTRY_RESOURCEID(IDR_LEXEME)
//    DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CLexeme)
	COM_INTERFACE_ENTRY(ILexeme)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
        Init();
		return S_OK;
	}

	void FinalRelease()
	{
        delete m_pDb;
	}

public:

// ILexeme
    STDMETHOD (GenerateWordForms) ();

    STDMETHOD (get_FormDescriptor) (IFormFinder **);

	STDMETHOD (get_LexemeId) (long *);
    STDMETHOD (get_GraphicStem) (BSTR *);
    STDMETHOD (get_InitialForm) (BSTR *);
    STDMETHOD (get_IsVariant) (BOOL *);
    STDMETHOD (get_MainSymbol) (BSTR *);
    STDMETHOD (get_IsPluralOf) (BOOL *);
    STDMETHOD (get_IsTransitive) (BOOL *);
    STDMETHOD (get_Aspect) (ET_Aspect *);
    STDMETHOD (get_IsReflexive) (ET_Reflexive *);
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
    STDMETHOD (get_HasIrregularForms) (BOOL *);

    STDMETHOD (get_IsUnstressed) (BOOL *);

    STDMETHOD (SaveTestData) ();


    //
    // Internal
    //
    ET_Gender eGetGender()
    {
        return m_mapMainSymbolToGender[m_sInflectionType];
    }

    ET_Animacy eGetAnimacy()
    {
        return m_mapMainSymbolToAnimacy[m_sInflectionType];
    }

private:
    map<CEString, ET_MainSymbol> m_mapMainSymbol;
    map<CEString, ET_Gender> m_mapMainSymbolToGender;
    map<CEString, ET_Animacy> m_mapMainSymbolToAnimacy;
    map<CEString, CEString> m_mapStandardAlternations;
    multimap<int, int> m_mmEndingsHash;
    vector<CEString> m_vecAlternatingPreverbs, m_vecAlternatingPreverbsWithVoicing;
    CEString sDbPath;
    CSqlite * m_pDb;

    //
    // Stem data as acquired from the DB
    //
    int m_iDbKey;
    CEString m_sSourceForm;
    CEString m_sHeadwordComment;
    vector<int> m_vecSourceStressPos, m_vecSecondaryStressPos;
    bool m_bIsUnstressed;
    vector<int> m_vecHomonyms;
    CEString m_sGraphicStem;
    bool m_bIsVariant;
    CEString m_sMainSymbol;
    bool m_bIsPluralOf;
    bool m_bTransitive;
    ET_Reflexive m_eReflexive;
    CEString m_sMainSymbolPluralOf;
    CEString m_sAltMainSymbol;
    ET_Aspect m_eAspect;
    CEString m_sInflectionType;
    ET_PartOfSpeech m_ePartOfSpeech;
    CEString m_sComment;
    CEString m_sAltMainSymbolComment;
    CEString m_sAltInflectionComment;
    CEString m_sVerbStemAlternation;
    bool m_bPartPastPassZhd;
    int m_iSection;
    bool m_bNoComparative;
    bool m_bAssumedForms;
    bool m_bYoAlternation;
    bool m_bOAlternation;
    bool m_bSecondGenitive;
    bool m_bHasAspectPair;
    int m_i_AspectPairType;
    CEString m_sAspectPairComment;
    CEString m_sQuestionableForms;
    bool m_bHasIrregularForms;
    bool m_bHasIrregularVariants;    // true if any of irreg. forms alternates with regular
    bool m_bHasDeficiencies;
    CEString m_sRestrictedFroms;
    CEString m_sContexts;
    CEString m_sTrailingComment;

    // From inflection table:
    bool m_bPrimaryInflectionGroup;
    int m_iType;
    ET_AccentType m_eAccentType1;
    ET_AccentType m_eAccentType2;
    bool m_bShortFormsRestricted;
    bool m_bPastParticipleRestricted;
    bool m_bNoLongForms;
    bool m_bNoShortForms;
    bool m_bNoPastParticiple;
    bool m_bFleetingVowel;
    int m_iStemAugment;

    map<int, bool> m_mapCommonDeviations;

    //
    // Irregular forms
    //
    multimap<int, StIrregularForm>m_mmapIrregularForms; 
    typedef pair<multimap<int, StIrregularForm>::iterator, 
                 multimap<int, StIrregularForm>::iterator> Pair_itIrregularFormRange;
    Pair_itIrregularFormRange pair_ItIfRange;
    multimap<int, StIrregularForm>::iterator m_itCurrentIrregularForm;

    //
    // Missing forms
    //
    vector<int> m_vecMissingForms;

    //
    // Verb stems:
    //
    CEString m_sInfStem;
    CEString m_sInfinitive;
    CEString m_s1SgStem;
    CEString m_s3SgStem;

    //
    // Helpers:
    //
    void Init();
    void SetDb (const CEString& str_dbPath);
    HRESULT hAddWordForm (CComObject<CWordForm> * pWordForm);
    HRESULT hGetIrregularForms();
    HRESULT hGetMissingForms();
    HRESULT hGenerateParadigm();
    CEString sHash();

    HRESULT hAddIrregularForm (int iHash, const StIrregularForm& stIf);
    HRESULT hGetFirstIrregularForm (int iHash, StIrregularForm& stIf);
    HRESULT hGetNextIrregularForm (int iHash, StIrregularForm& stIf);
    HRESULT hFormExists (CGramHasher& hasher);

};  //  CLexeme

OBJECT_ENTRY_AUTO(__uuidof(ZalLexeme), CLexeme)
