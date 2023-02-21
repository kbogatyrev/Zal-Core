#ifndef C_LEXEME_H_INCLUDED
#define C_LEXEME_H_INCLUDED

#include "Enums.h"
#include "EString.h"

//#include "ILexeme.h"
#include "LexemeProperties.h"

using namespace std;

namespace Hlib
{
    class CWordForm;
    class CDictionary;
    class CInflection;
    class CSqlite;
    class CGramHasher;

//    static void operator++ (ET_MainSymbol& eMs)
//    {
//        eMs = (ET_MainSymbol)(eMs + 1);
//    }

    struct StIrregularForm
    {
        shared_ptr<CWordForm> spWordForm;
        bool bIsOptional;

        StIrregularForm(shared_ptr<CWordForm> spWf, bool bOpt) : spWordForm(spWf), bIsOptional(bOpt)
        {}
    };

    class CLexeme;

    class CInflectionEnumerator
    {
    public:
        CInflectionEnumerator() = delete;
        CInflectionEnumerator(shared_ptr<CLexeme> spLexeme) : m_spLexeme(spLexeme) {}

        ET_ReturnCode eReset();

        ET_ReturnCode eGetFirstInflection(shared_ptr<CInflection>& pInflectionClass);
        ET_ReturnCode eGetNextInflection(shared_ptr<CInflection>& pInflectionClass);

    private:
        vector<shared_ptr<CInflection>>::iterator m_itCurrentInflection;
        shared_ptr<CLexeme> m_spLexeme;
    };

    class CLexeme : enable_shared_from_this<CLexeme>
    {
        friend class CInflectionEnumerator;

    public:
        CLexeme() = delete;
        CLexeme(shared_ptr<CDictionary>);
        CLexeme(const CLexeme&);
        ~CLexeme();

        CLexeme* pGetLexemePtr()
        {
            return this;
        }

        ET_ReturnCode eCreateInflectionEnumerator(shared_ptr<CInflectionEnumerator>&);

        void DeleteInflectionEnumerator(shared_ptr<CInflectionEnumerator>);

        void SetDictionary(shared_ptr<CDictionary> spDict);

        const StLexemeProperties& stGetProperties()
        {
            return m_stProperties;
        }

        StLexemeProperties& stGetPropertiesForWriteAccess()
        {
            return m_stProperties;
        }

        int64_t llLexemeId()
        {
            return m_stProperties.llDescriptorId;
        }

        int64_t llHeadwordId()
        {
            return m_stProperties.llHeadwordId;
        }

        ET_Gender eInflectionTypeToGender()
        {
            return m_mapMainSymbolToGender[m_stProperties.sInflectionType];
        }

        ET_Gender eMainSymbolToGender()
        {
            return m_mapMainSymbolToGender[m_stProperties.sMainSymbol];
        }

        ET_Animacy eInflectionTypeToAnimacy(bool bAltMainSymbol = false)
        {
            auto sType = bAltMainSymbol ? m_stProperties.sAltMainSymbol : m_stProperties.sInflectionType;
            return m_mapMainSymbolToAnimacy[sType];
        }

        ET_MainSymbol eMainSymbolToEnum(const CEString& sMainSym)
        {
            return m_mapMainSymbol[sMainSym];
        }

        ET_Animacy eMainSymbolToAnimacy()
        {
            return m_mapMainSymbolToAnimacy[m_stProperties.sMainSymbol];
        }

        CEString sGraphicStem()
        {
            return m_stProperties.sGraphicStem;
        }

        void SetGraphicStem(const CEString& sValue)
        {
            m_stProperties.sGraphicStem = sValue;
        }

        long long llSecondPartId()
        {
            return m_stProperties.llSecondPartId;
        }

        void SetSecondPartId(long long llValue)
        {
            m_stProperties.llSecondPartId = llValue;
        }

        bool bHasIrregularForms()
        {
            return m_stProperties.bHasIrregularForms;
        }

        void SetHasIrregularForms(bool bValue)
        {
            m_stProperties.bHasIrregularForms = bValue;
        }
        
        bool bHasSecondaryStress()
        {
            return !m_stProperties.vecSecondaryStressPos.empty();
        }
        
        bool bHasYoAlternation()
        {
            return m_stProperties.bYoAlternation;
        }

        void SetHasYoAlternation(bool bValue)
        {
            m_stProperties.bYoAlternation = bValue;
        }
        
        bool bHasOAlternation()
        {
            return m_stProperties.bOAlternation;
        }

        void SetHasOAlternation(bool bValue)
        {
            m_stProperties.bOAlternation = bValue;
        }
        
        CEString sSourceForm()
        {
            return m_stProperties.sSourceForm;
        }

        void SetSourceForm(const CEString& sValue)
        {
            m_stProperties.sSourceForm = sValue;
        }
        
        CEString sHeadwordComment()
        {
            return m_stProperties.sHeadwordComment;
        }

        void SetHeadwordComment(const CEString& sValue)
        {
            m_stProperties.sHeadwordComment = sValue;
        }
        
        CEString sHeadwordVariant()
        {
            return m_stProperties.sHeadwordVariant;
        }

        void SetHeadwordVariant(const CEString& sValue)
        {
            m_stProperties.sHeadwordVariant = sValue;
        }
        
        CEString sHeadwordVariantComment()
        {
            return m_stProperties.sHeadwordVariantComment;
        }

        void SetHeadwordVariantComment(const CEString& sValue)
        {
            m_stProperties.sHeadwordVariantComment = sValue;
        }
        
        CEString sPluralOf()
        {
            return m_stProperties.sPluralOf;
        }

        void SetPluralOf(const CEString& sValue)
        {
            m_stProperties.sPluralOf = sValue;
        }
        
        CEString sUsage()
        {
            return m_stProperties.sUsage;
        }
        
        void SetUsage(const CEString& sValue)
        {
            m_stProperties.sUsage = sValue;
        }
        
        CEString sSeeRef()
        {
            return m_stProperties.sSeeRef;
        }
        
        void SetSeeRef(const CEString& sValue)
        {
            m_stProperties.sSeeRef = sValue;
        }
        
        CEString sBackRef()
        {
            return m_stProperties.sBackRef;
        }

        void SetBackRef(const CEString& sValue)
        {
            m_stProperties.sBackRef = sValue;
        }
        
        bool bIsUnstressed()
        {
            return m_stProperties.bIsUnstressed;
        }

        void SetIsUnstressed(bool bValue)
        {
            m_stProperties.bIsUnstressed = bValue;
        }
        
        bool bIsVariant()
        {
            return m_stProperties.bIsVariant;
        }

        void SetIsVariant(bool bValue)
        {
            m_stProperties.bIsVariant = bValue;
        }
        
        CEString sMainSymbol()
        {
            return m_stProperties.sMainSymbol;
        }

        void SetMainSymbol(const CEString& sValue)
        {
            m_stProperties.sMainSymbol = sValue;
        }
        
        bool bIsPluralOf()
        {
            return m_stProperties.bIsPluralOf;
        }

        void SetIsPluralOf(bool bValue)
        {
            m_stProperties.bIsPluralOf = bValue;
        }
        
        bool bTransitive()
        {
            return m_stProperties.bTransitive;
        }

        void SetTransitive(bool bValue)
        {
            m_stProperties.bTransitive = bValue;
        }
        
        ET_Reflexivity eIsReflexive()
        {
            return m_stProperties.eReflexive;
        }

        void SetIsReflexive(ET_Reflexivity eValue)
        {
            m_stProperties.eReflexive = eValue;
        }
        
        CEString sMainSymbolPluralOf()
        {
            return m_stProperties.sMainSymbolPluralOf;
        }

        void SetMainSymbolPluralOf(const CEString& sValue)
        {
            m_stProperties.sMainSymbolPluralOf = sValue;
        }
        
        CEString sAltMainSymbol()
        {
            return m_stProperties.sAltMainSymbol;
        }

        void SetAltMainSymbol(const CEString& sValue)
        {
            m_stProperties.sAltMainSymbol = sValue;
        }
        
        ET_Aspect eAspect()
        {
            return m_stProperties.eAspect;
        }

        void SetAspect(ET_Aspect eValue)
        {
            m_stProperties.eAspect = eValue;
        }
        
        CEString sInflectionType()
        {
            return m_stProperties.sInflectionType;
        }

        void SetInflectionType(const CEString& sValue)
        {
            m_stProperties.sInflectionType = sValue;
        }
        
        ET_PartOfSpeech ePartOfSpeech()
        {
            return m_stProperties.ePartOfSpeech;
        }

        void SetPartOfSpeech(ET_PartOfSpeech eValue)
        {
            m_stProperties.ePartOfSpeech = eValue;
        }
        
        CEString sComment()
        {
            return m_stProperties.sComment;
        }

        void SetComment(const CEString& sValue)
        {
            m_stProperties.sComment = sValue;
        }
        
        CEString sAltMainSymbolComment()
        {
            return m_stProperties.sAltMainSymbolComment;
        }

        void SetAltMainSymbolComment(const CEString& sValue)
        {
            m_stProperties.sAltMainSymbolComment = sValue;
        }        

        CEString sAltInflectionComment()
        {
            return m_stProperties.sAltInflectionComment;
        }

        void SetAltInflectionComment(const CEString& sValue)
        {
            m_stProperties.sAltInflectionComment = sValue;
        }
        
        CEString sVerbStemAlternation()
        {
            return m_stProperties.sVerbStemAlternation;
        }

        void SetVerbStemAlternation(const CEString& sValue)
        {
            m_stProperties.sVerbStemAlternation = sValue;
        }
        
        bool bPartPastPassZhd()
        {
            return m_stProperties.bPartPastPassZhd;
        }

        void SetPartPastPassZhd(bool bValue)
        {
            m_stProperties.bPartPastPassZhd = bValue;
        }
        
        int iSection()
        {
            return m_stProperties.iSection;
        }

        void SetSection(int iValue)
        {
            m_stProperties.iSection = iValue;
        }
        
        bool bNoComparative()
        {
            return m_stProperties.bNoComparative;
        }

        void SetNoComparative(bool bValue)
        {
            m_stProperties.bNoComparative = bValue;
        }
        
        bool bAssumedForms()
        {
            return m_stProperties.bAssumedForms;
        }

        void SetAssumedForms(bool bValue)
        {
            m_stProperties.bAssumedForms = bValue;
        }
        
//        virtual bool bYoAlternation()
//        {
//            return m_stProperties.bYoAlternation;
//        }

//        virtual void SetYoAlternation(bool bValue)
//        {
//            m_stProperties.bYoAlternation = bValue;
//        }
        
//        virtual bool bOAlternation()
//        {
//            return m_stProperties.bOAlternation;
//        }

//        virtual void SetOAlternation(bool bValue)
//        {
//            m_stProperties.bOAlternation = bValue;
//        }
        
        bool bSecondGenitive()
        {
            return m_stProperties.bSecondGenitive;
        }

        void SetSecondGenitive(bool bValue)
        {
            m_stProperties.bSecondGenitive = bValue;
        }
        
        bool bSecondPrepositional()
        {
            return m_stProperties.bSecondPrepositional;
        }

        void SetSecondPrepositional(bool bValue)
        {
            m_stProperties.bSecondPrepositional = bValue;
        }
        
        bool bSecondPrepositionalOptional()
        {
            return m_stProperties.bSecondPrepositionalOptional;
        }

        void SetSecondPrepositionalOptional(bool bValue)
        {
            m_stProperties.bSecondPrepositionalOptional = bValue;
        }
        
        CEString sP2Preposition()
        {
            return m_stProperties.sP2Preposition;
        }

        void SetP2Preposition(const CEString& sValue)
        {
            m_stProperties.sP2Preposition = sValue;
        }
        
        bool bImpersonal()
        {
            return m_stProperties.bIsImpersonal;
        }

        void SetImpersonal(bool bValue)
        {
            m_stProperties.bIsImpersonal = bValue;
        }

        bool bIterative()
        {
            return m_stProperties.bIsIterative;
        }

        void SetIterative(bool bValue)
        {
            m_stProperties.bIsIterative = bValue;
        }

        bool bHasAspectPair()
        {
            return m_stProperties.bHasAspectPair && !m_stProperties.bSpryazhSmNoAspectPair;
        }

        void SetHasAspectPair(bool bValue)
        {
            m_stProperties.bHasAspectPair = bValue;
        }

        bool bHasAltAspectPair()
        {
            if (!m_stProperties.bHasAspectPair)
            {
                return false;
            }
            return m_stProperties.iAltAspectPairType != 0;
        }
        
        int iAspectPairType()
        {
            return m_stProperties.iAspectPairType;
        }

        void SetAspectPairType(int iValue)
        {
            m_stProperties.iAspectPairType = iValue;
        }

        int iAltAspectPairType()
        {
            return m_stProperties.iAltAspectPairType;
        }
        
        void SetAltAspectPairType(int iValue)
        {
            m_stProperties.iAltAspectPairType = iValue;
        }

        CEString sAspectPairData()
        {
            return m_stProperties.sAspectPairData;
        }

        void SetAspectPairData(const CEString& sValue)
        {
            m_stProperties.sAspectPairData = sValue;
        }

        CEString sAltAspectPairData()
        {
            return m_stProperties.sAltAspectPairData;
        }

        void SetAltAspectPairData(const CEString& sValue)
        {
            m_stProperties.sAltAspectPairData = sValue;
        }

        CEString sAltAspectPairComment()
        {
            return m_stProperties.sAltAspectPairComment;
        }

        void SetAltAspectPairComment(const CEString& sValue)
        {
            m_stProperties.sAltAspectPairComment = sValue;
        }
        
        bool bHasIrregularVariants()
        {
            return m_stProperties.bHasIrregularVariants;    // true if any of irreg. forms alternates with regular
        }

        void SetHasIrregularVariants(bool bValue)
        {
            m_stProperties.bHasIrregularVariants = bValue;
        }
        
        bool bHasDifficultForms()
        {
            return m_stProperties.bHasDifficultForms;
        }

        void SetHasDifficultForms(bool bValue)
        {
            m_stProperties.bHasDifficultForms = bValue;
        }

        bool bHasMissingForms()
        {
            return m_stProperties.bHasMissingForms;
        }

        void SetHasMissingForms(bool bValue)
        {
            m_stProperties.bHasMissingForms = bValue;
        }

        //virtual void AddDifficultForm(const CEString& sGramHash)
        //{
        //    m_vecDifficultForms.push_back(sGramHash);
        //}

        //virtual void AddMissingForm(const CEString& sGramHash)
        //{
        //    m_vecMissingForms.push_back(sGramHash);
        //}

        CEString sRestrictedContexts()
        {
            return m_stProperties.sRestrictedContexts;
        }

        void SetRestrictedContexts(const CEString& sValue)
        {
            m_stProperties.sRestrictedContexts = sValue;
        }
        
        CEString sContexts()
        {
            return m_stProperties.sContexts;
        }

        void SetContexts(const CEString& sValue)
        {
            m_stProperties.sContexts = sValue;
        }
        
        CEString sTrailingComment()
        {
            return m_stProperties.sTrailingComment;
        }

        void SetTrailingComment(const CEString& sValue)
        {
            m_stProperties.sTrailingComment = sValue;
        }
        
        bool bNoLongForms()
        {
            return m_stProperties.bNoLongForms;
        }

        void SetNoLongForms(bool bValue)
        {
            m_stProperties.bNoLongForms = bValue;
        }
        
        CEString s1SgStem()
        {
            return m_stProperties.s1SgStem;
        }

        void Set1SgStem(const CEString& sValue)
        {
            m_stProperties.s1SgStem = sValue;
        }
        
        CEString s3SgStem()
        {
            return m_stProperties.s3SgStem;
        }

        void Set3SgStem(const CEString& sValue)
        {
            m_stProperties.s3SgStem = sValue;
        }
        
        CEString sInfinitive()
        {
            return m_stProperties.sInfinitive;
        }

        void SetInfinitive(const CEString& sValue)
        {
            m_stProperties.sInfinitive = sValue;
        }
        
        CEString sInfStem()
        {
            return m_stProperties.sInfStem;
        }

        void SetInfStem(const CEString& sValue)
        {
            m_stProperties.sInfStem = sValue;
        }
        
//        virtual ET_ReturnCode eAddCommonDeviation(int iValue, bool bIsOptional);

//        virtual void ClearCommonDeviations()
//        {
//            m_stProperties.mapCommonDeviations.clear();
//        }
        
        //        virtual int iInflectedParts()
//        {
//            return m_stProperties.iInflectedParts;
//        }

//        virtual void SetInflectedParts(int iValue)
//        {
//            m_stProperties.iInflectedParts = iValue;
//        }

        bool bHasPresPassParticiple()
        {
            return m_stProperties.bHasPresPassParticiple;
        }

        void SetHasPresPassParticiple(bool bValue)
        {
            m_stProperties.bHasPresPassParticiple = bValue;
        }

        bool bSpryazhSm()
        {
            return m_stProperties.bSpryazhSm;
        }

        CEString sSpryazhSmRefSource()
        {
            return m_stProperties.sSpryazhSmRefSource;
        }

        ET_Subparadigm eSubparadigm()
        {
            return m_stProperties.eSubparadigm;
        }

        void SetSubparadigm(ET_Subparadigm eSp)    // currently only used with numerals
        {
            m_stProperties.eSubparadigm = eSp;
        }

        bool bIsSecondPart()
        {
            return m_stProperties.bSecondPart;
        }

        void SetIsSecondPart(bool bIsSecondPart)
        {
            m_stProperties.bSecondPart = bIsSecondPart;
        }

//        virtual bool bFindCommonDeviation(int iNum, bool& bIsOptional);
        bool bFindStandardAlternation(const CEString& sKey, CEString& sValue);

        ET_ReturnCode eGetSourceFormWithStress(CEString& sSourceForm, bool bIsVariant=false);
        ET_ReturnCode eGetSourceFormWithDiacritics(CEString& sSourceForm, bool bIsVariant = false);
        ET_ReturnCode eGetFirstStemStressPos(int& iPos);
        ET_ReturnCode eGetNextStemStressPos(int& iPos);
        ET_ReturnCode eGetFirstSecondaryStemStressPos(int& iPos);
        ET_ReturnCode eGetNextSecondaryStemStressPos(int& iPos);

//        virtual ET_ReturnCode eGetAlternatingPreverb(const CEString& sVerbForm, CEString& sPreverb, bool& bVoicing);

//        virtual CEString sHash();
//        virtual CEString sParadigmHash();
//        virtual ET_ReturnCode eWordFormFromHash(CEString sHash, int iAt, IWordForm *& pWf);
//        virtual ET_ReturnCode eCreateWordForm(IWordForm *&);
//        virtual ET_ReturnCode eRemoveWordForm(CEString sHash, int iAt);
//        virtual ET_ReturnCode eRemoveWordForms(CEString sHash);
//        virtual void AddWordForm(IWordForm* pWordForm);         // ITF version, cf internal version below

//        virtual bool bHasIrregularForm(CEString sGramHash);
//        virtual bool bNoRegularForms(CEString sGramHash);

//        ET_ReturnCode eGetFirstWordForm(IWordForm *& pWf);
//        ET_ReturnCode eGetNextWordForm(IWordForm *& pWf);

        //virtual ET_ReturnCode eGetFirstIrregularForm(CEString sHash, IWordForm *&, bool& bIsOptional);
        //virtual ET_ReturnCode eGetFirstIrregularForm(IWordForm *&, bool& bIsOptional);
        //virtual ET_ReturnCode eGetNextIrregularForm(IWordForm *&, bool& bIsOptional);

        //ET_ReturnCode eGetFirstIrregularForm(CEString sHash, CWordForm *&, bool& bIsOptional);
        //ET_ReturnCode eGetFirstIrregularForm(CWordForm *&, bool& bIsOptional);
        //ET_ReturnCode eGetNextIrregularForm(CWordForm *&, bool& bIsOptional);

        ET_ReturnCode eGetAspectPair(CEString&, int&);
        ET_ReturnCode eGetAltAspectPair(CEString&, int&);
//        virtual int iFormCount(CEString sHash);
//        virtual bool bHasCommonDeviation(int iCd);
//        virtual bool bDeviationOptional(int iCd);
//        virtual ET_ReturnCode eFormExists(const CEString& hash);
//        virtual ET_ReturnCode eSetFormExists(const CEString& hash, bool bExists);
        //virtual ET_ReturnCode eIsFormDifficult(const CEString& sGramHash);
        //virtual ET_ReturnCode eSetFormDifficult(const CEString& hash, bool bIsDifficult);
        //virtual ET_ReturnCode eDifficultFormsHashes(vector<CEString>&);
        //virtual ET_ReturnCode eIsFormAssumed(const CEString& sGramHash);
        //virtual ET_ReturnCode eSetHasAssumedForms(bool bIsAssumed);
        //virtual bool bIsMultistressedCompound();
        ET_ReturnCode eSetDb(const CEString& sDbPath);
//        virtual ET_ReturnCode eGenerateParadigm();
//        virtual ET_ReturnCode eSaveTestData();
//        virtual ET_ReturnCode eDeleteIrregularForm(const CEString& sFormHash);
//        virtual ET_ReturnCode eSaveIrregularForm(const CEString& sFormHash, IWordForm *&);
//        virtual ET_ReturnCode eSaveIrregularForms(const CEString& sGramHash);

//        virtual ET_ReturnCode eMakeGraphicStem();
//        virtual ET_ReturnCode eMakeGraphicStem(const CEString& sSource, CEString& sGraphicStem);

//        ET_ReturnCode eClone(shared_ptr<CLexeme>&);
        ET_ReturnCode eCheckLexemeProperties();
        ET_ReturnCode eGetErrorMsg(CEString& sErrorMsg);

        // Non-virtual public functions (not available to external consumers)
    public:
        shared_ptr<CSqlite> spGetDb();
        StLexemeProperties& stGetSecondPartProperties();
        shared_ptr<CLexeme> spGetSecondPart()
        {
            return m_spSecondPart;
        }

        void AddInflection(shared_ptr<CInflection>);

//        ET_ReturnCode eWordFormFromHash(CEString sHash, int iAt, CWordForm *& pWf);
        ET_ReturnCode eGetStemStressPositions(const CEString& sStem, vector<int>& vecPosition);
        ET_ReturnCode eGetIrregularForms(CEString sHash, map<shared_ptr<CWordForm>, bool>& mapResult);
        void AssignPrimaryStress(shared_ptr<CWordForm>);
        void AssignSecondaryStress(shared_ptr<CWordForm>);
//        void AddWordForm(CWordForm* pWordForm);
//        void SetHypotheticalForm(CEString& sGramHash);
//        bool bIsHypotheticalForm(CEString& sGramHash);
//        uint64_t uiTotalWordForms();

        // Populate DB:
//        ET_ReturnCode eSaveStemsToDb();
//        ET_ReturnCode eAssignStemIds();
//        ET_ReturnCode ePrepareDataInsert(uint64_t& uiStmtHandle);
//        ET_ReturnCode eSaveWordFormsToDb();
//        ET_ReturnCode eSaveIrregularFormsToDb();  // for spryazh sm entries

        // Helpers:
//        ET_ReturnCode eLoadDifficultAndMissingForms();
//        ET_ReturnCode eLoadIrregularForms();
        ET_ReturnCode eHandleSpryazhSmEntry();
//        ET_ReturnCode eSaveIrregularForms(long long llDescriptorDbKey);
//        ET_ReturnCode eLoadDifficultForms();
//
        CEString sGramHashNSgMLong();
//        ET_ReturnCode eDifficultFormLabelToGramHash(const CEString sLabel);
        void SetSecondPart(shared_ptr<CLexeme>);

        // Support for manual editing
        ET_ReturnCode eUpdateDescriptorInfo(shared_ptr<CLexeme>);
//        ET_ReturnCode eGetWordForm(unsigned int uiAt, CWordForm *&);
        ET_ReturnCode eExtractStressSymbols();
        ET_ReturnCode eInitializeFromProperties();

    private:
        void Init();
//        ET_ReturnCode eAlignInflectedParts();
//        ET_ReturnCode eConcatenateInflectedParts(CWordForm& left, CWordForm& right);

    private:
//        CLexeme() {}

        StLexemeProperties m_stProperties, m_stProperties2ndPart;
        vector<shared_ptr<CInflection>> m_vecInflection;

//        bool m_bFormsGenerated;
//        multimap<CEString, CWordForm *> m_mmWordForms;        // gramm. hash --> wordform struct
        map<CEString, ET_MainSymbol> m_mapMainSymbol;
        map<CEString, ET_Gender> m_mapMainSymbolToGender;
        map<CEString, ET_Animacy> m_mapMainSymbolToAnimacy;
        map<CEString, CEString> m_mapStandardAlternations;
        multimap<int, int> m_mmEndingsHash;
//        vector<CEString> m_vecAlternatingPreverbs, m_vecAlternatingPreverbsWithVoicing;

        shared_ptr<CDictionary> m_spDictionary;
        shared_ptr<CLexeme> m_spSecondPart;
/*
        vector<CEString> m_vecHypotheticalForms;     //  gram hashes of missing forms that were generated anyway 
                                                     //  to be used in generation of other forms

        // Word forms
        multimap<CEString, CWordForm *>::iterator m_itCurrentWordForm;

        // Irregular forms
        multimap<CEString, StIrregularForm> m_mmapIrregularForms;    // hash to irregular form struct
        typedef pair<multimap<CEString, StIrregularForm>::iterator,
            multimap<CEString, StIrregularForm>::iterator> Pair_itIrregularFormRange;
        Pair_itIrregularFormRange m_pairItIfRange;
        multimap<CEString, StIrregularForm>::iterator m_itCurrentIrregularForm;

        // Difficult and missing forms
//        vector<CEString> m_vecDifficultForms;
//        vector<CEString> m_vecMissingForms;
*/

        vector<int>::iterator m_itCurrentStressPos;

        // Aspect pairs
        CEString m_sAspectPair;
        int m_iAspectPairStressPos;
        CEString m_sAltAspectPair;
        int m_iAltAspectPairStressPos;

        CEString m_sLastErrorMsg;

    };  //  CLexeme

}   //  namespace Hlib

#endif  //  C_LEXEME_H_INCLUDED