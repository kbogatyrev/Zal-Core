#ifndef LEXEME_H_INCLUDED
#define LEXEME_H_INCLUDED

#include "LexemeProperties.h"
#include "WordForm.h"
//#include "FormDescriptor.h"

using namespace std;
using namespace std::tr1;

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
    CWordForm * pWordForm;
    bool bIsOptional;

    StIrregularForm(CWordForm * pWf, bool bOpt) : pWordForm(pWf), bIsOptional(bOpt)
    {}
};

class CLexeme
{

public:
    CLexeme();
    ~CLexeme();

    const StLexemeProperties& stGetProperties()
    {
        return m_stProperties;
    }

    StLexemeProperties& stGetPropertiesForWriteAccess()
    {
        return m_stProperties;
    }

    ET_Gender eGender()
    {
        return m_mapMainSymbolToGender[m_stProperties.sInflectionType];
    }

    ET_Animacy eAnimacy()
    {
        return m_mapMainSymbolToAnimacy[m_stProperties.sInflectionType];
    }

    CEString sGraphicStem()
    {
        return m_stProperties.sGraphicStem;
    }

    bool bHasIrregularForms()
    {
        return m_stProperties.bHasIrregularForms;
    }

    bool bHasSecondaryStress()
    {
        return !m_stProperties.vecSecondaryStressPos.empty();
    }

    bool bHasFleetingVowel()
    {
        return m_stProperties.bFleetingVowel;
    }

    bool bHasYoAlternation()
    {
        return m_stProperties.bYoAlternation;
    }

    bool bHasOAlternation()
    {
        return m_stProperties.bOAlternation;
    }

    CEString sSourceForm()
    {
        return m_stProperties.sSourceForm;
    }

    CSqlite * pGetDb()
    {
        return m_pDb;
    }

    CEString sHeadwordComment()
    {
        return m_stProperties.sHeadwordComment;
    }

    bool bIsUnstressed()
    {
        return m_stProperties.bIsUnstressed;
    }

    bool IsVariant()
    {
        return m_stProperties.bIsVariant;
    }

    CEString sMainSymbol()
    {
        return m_stProperties.sMainSymbol;
    }

    bool bIsPluralOf()
    {
        return m_stProperties.bIsPluralOf;
    }

    bool bTransitive()
    {
        return m_stProperties.bTransitive;
    }

    ET_Reflexive eIsReflexive()
    {
        return m_stProperties.eReflexive;
    }

    CEString sMainSymbolPluralOf()
    {
        return m_stProperties.sMainSymbolPluralOf;
    }

    CEString sAltMainSymbol()
    {
        return m_stProperties.sAltMainSymbol;
    }

    ET_Aspect eAspect()
    {
        return m_stProperties.eAspect;
    }

    CEString sInflectionType()
    {
        return m_stProperties.sInflectionType;
    }

    ET_PartOfSpeech ePartOfSpeech()
    {
        return m_stProperties.ePartOfSpeech;
    }

    CEString sComment()
    {
        return m_stProperties.sComment;
    }

    CEString sAltMainSymbolComment()
    {
        return m_stProperties.sAltMainSymbolComment;
    }

    CEString sAltInflectionComment()
    {
        return m_stProperties.sAltInflectionComment;
    }

    CEString sVerbStemAlternation()
    {
        return m_stProperties.sVerbStemAlternation;
    }

    bool bPartPastPassZhd()
    {
        return m_stProperties.bPartPastPassZhd;
    }

    int iSection()
    {
        return m_stProperties.iSection;
    }

    bool bNoComparative()
    {
        return m_stProperties.bNoComparative;
    }

    bool bAssumedForms()
    {
        return m_stProperties.bAssumedForms;
    }

    bool bYoAlternation()
    {
        return m_stProperties.bYoAlternation;
    }

    bool bOAlternation()
    {
        return m_stProperties.bOAlternation;
    }

    bool bSecondGenitive()
    {
        return m_stProperties.bSecondGenitive;
    }

    bool bSecondLocative()
    {
        return m_stProperties.bSecondLocative;
    }

    bool bSecondLocativeOptional()
    {
        return m_stProperties.bSecondLocativeOptional;
    }

    CEString sLoc2Preposition()
    {
        return m_stProperties.sLoc2Preposition;
    }

    bool bHasAspectPair()
    {
        return m_stProperties.bHasAspectPair;
    }

    int iAspectPairType()
    {
        return m_stProperties.iAspectPairType;
    }

    CEString sAspectPairComment()
    {
        return m_stProperties.sAspectPairComment;
    }

    CEString sQuestionableForms()
    {
        return m_stProperties.sQuestionableForms;
    }

    bool bHasIrregularVariants()
    {
        return m_stProperties.bHasIrregularVariants;    // true if any of irreg. forms alternates with regular
    }

    bool bHasDeficiencies()
    {
        return m_stProperties.bHasDeficiencies;
    }

    CEString sRestrictedFroms()
    {
        return m_stProperties.sRestrictedFroms;
    }

    CEString sContexts()
    {
        return m_stProperties.sContexts;
    }

    CEString sTrailingComment()
    {
        return m_stProperties.sTrailingComment;
    }

    // From inflection table:
    int iInflectionId()
    {
        return m_stProperties.iInflectionId;
    }

    bool bPrimaryInflectionGroup()
    {
        return m_stProperties.bPrimaryInflectionGroup;
    }

    int iType()
    {
        return m_stProperties.iType;
    }

    ET_AccentType eAccentType1()
    {
        return m_stProperties.eAccentType1;
    }

    ET_AccentType eAccentType2()
    {
        return m_stProperties.eAccentType2;
    }

    bool bShortFormsRestricted()
    {
        return m_stProperties.bShortFormsRestricted;
    }

    bool bPastParticipleRestricted()
    {
        return m_stProperties.bPastParticipleRestricted;
    }

    bool bNoLongForms()
    {
        return m_stProperties.bNoLongForms;
    }

    bool bNoShortForms()
    {
        return m_stProperties.bNoShortForms;
    }

    bool bNoPastParticiple()
    {
        return m_stProperties.bNoPastParticiple;
    }

    bool bFleetingVowel()
    {
        return m_stProperties.bFleetingVowel;
    }

    int iStemAugment()
    {
        return m_stProperties.iStemAugment;
    }

    CEString s1SgStem()
    {
        return m_stProperties.s1SgStem;
    }

    CEString s3SgStem()
    {
        return m_stProperties.s3SgStem;
    }

    CEString sInfinitive()
    {
        return m_stProperties.sInfinitive;
    }

    CEString sInfStem()
    {
        return m_stProperties.sInfStem;
    }

    bool bFindCommonDeviation (int iNum, bool& bIsOptional);
    bool bFindStandardAlternation (const CEString& sKey, CEString& sValue);
    ET_ReturnCode eGetStemStressPositions (CEString& sLemma, vector<int>& vecPosition);
    ET_ReturnCode eGetAlternatingPreverb (const CEString& sVerbForm, CEString& sPreverb, bool& bVoicing);

    void WordFormFromHash (int iHash, unsigned int uiAt, CWordForm *& pWf);
    bool bHasIrregularForm (int iGramHash);
    bool bNoRegularForms (int iGramHash);
    ET_ReturnCode eGetIrregularForms (int iHash, map<CWordForm *, bool>& mapResult);
    unsigned int uiFormCount (int iHash);
    bool bHasCommonDeviation (int iCd);
    bool bDeviationOptional (int iCd);
    bool bIsFormMissing (int iGramHash);
    bool bIsMultistressedCompound();

    ET_ReturnCode eGetFirstStemStressPos (int& iPos);
    ET_ReturnCode eGetNextStemStressPos (int& iPos);

    void AssignPrimaryStress (CWordForm * pWordForm);
    void AssignSecondaryStress (CWordForm * pWordForm);
    void AddWordForm (CWordForm * pWordForm);

    ET_ReturnCode SetDb (const CEString& sDbPath);
    ET_ReturnCode eGenerateParadigm();

private:
    void Init();

private:
    StLexemeProperties m_stProperties;

    multimap<int, CWordForm *> m_mmWordForms;
    map<CEString, ET_MainSymbol> m_mapMainSymbol;
    map<CEString, ET_Gender> m_mapMainSymbolToGender;
    map<CEString, ET_Animacy> m_mapMainSymbolToAnimacy;
    map<CEString, CEString> m_mapStandardAlternations;
    multimap<int, int> m_mmEndingsHash;
    vector<CEString> m_vecAlternatingPreverbs, m_vecAlternatingPreverbsWithVoicing;
    CEString sDbPath;
    CSqlite * m_pDb;

    //
    // Word forms
    //
    multimap<int, CWordForm *> m_mmapWordForms;

    //
    // Irregular forms
    //
    multimap<int, StIrregularForm> m_mmapIrregularForms;    // hash to irregular form struct
    typedef pair<multimap<int, StIrregularForm>::iterator, 
                 multimap<int, StIrregularForm>::iterator> Pair_itIrregularFormRange;
    Pair_itIrregularFormRange m_pairItIfRange;
    multimap<int, StIrregularForm>::iterator m_itCurrentIrregularForm;

    //
    // Missing forms
    //
    vector<int> m_vecMissingForms;

    //
    // Helpers:
    //
    int iGramHashNSgMLong();

    ET_ReturnCode eLoadIrregularForms();

    ET_ReturnCode eLoadMissingForms();
    CEString sHash();

    ET_ReturnCode eFormExists (CGramHasher& hasher);

    //    ET_ReturnCode hAddIrregularForm (int iHash, const StIrregularForm& stIf);
    ET_ReturnCode eGetFirstIrregularForm (int iHash, CWordForm *&, bool& bIsOptional);
    ET_ReturnCode eGetNextIrregularForm (CWordForm *&, bool& bIsOptional);

    vector<int>::iterator m_itCurrentStressPos;

};  //  CLexeme

#endif  //  LEXEME_H_INCLUDED