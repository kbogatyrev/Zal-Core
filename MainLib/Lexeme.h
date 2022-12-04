#ifndef C_LEXEME_H_INCLUDED
#define C_LEXEME_H_INCLUDED

#include "Enums.h"
#include "EString.h"

#include "ILexeme.h"
#include "LexemeProperties.h"

using namespace std;

namespace Hlib
{
    struct CWordForm;
    class CDictionary;
    class CSqlite;
    class CGramHasher;

//    static void operator++ (ET_MainSymbol& eMs)
//    {
//        eMs = (ET_MainSymbol)(eMs + 1);
//    }

    struct StIrregularForm
    {
        CWordForm * pWordForm;
        bool bIsOptional;

        StIrregularForm(CWordForm * pWf, bool bOpt) : pWordForm(pWf), bIsOptional(bOpt)
        {}
    };

    class CLexeme : public ILexeme
    {

    public:
        CLexeme() = delete;
        CLexeme(CDictionary *);
        CLexeme(const CLexeme&);
        ~CLexeme();

        virtual void SetDictionary(CDictionary * pDict)
        {
            m_pDictionary = pDict;
        }

        virtual const StLexemeProperties& stGetProperties()
        {
            return m_stProperties;
        }

        virtual StLexemeProperties& stGetPropertiesForWriteAccess()
        {
            return m_stProperties;
        }

        virtual int64_t llLexemeId()
        {
            return m_stProperties.llDescriptorId;
        }

        virtual int64_t llHeadwordId()
        {
            return m_stProperties.llHeadwordId;
        }

        virtual ET_Gender eInflectionTypeToGender()
        {
            return m_mapMainSymbolToGender[m_stProperties.sInflectionType];
        }

        virtual ET_Gender eMainSymbolToGender()
        {
            return m_mapMainSymbolToGender[m_stProperties.sMainSymbol];
        }

        virtual ET_Animacy eInflectionTypeToAnimacy(bool bAltMainSymbol = false)
        {
            auto sType = bAltMainSymbol ? m_stProperties.sAltMainSymbol : m_stProperties.sInflectionType;
            return m_mapMainSymbolToAnimacy[sType];
        }

        virtual ET_MainSymbol eMainSymbolToEnum(const CEString& sMainSym)
        {
            return m_mapMainSymbol[sMainSym];
        }

        virtual ET_Animacy eMainSymbolToAnimacy()
        {
            return m_mapMainSymbolToAnimacy[m_stProperties.sMainSymbol];
        }

        virtual CEString sGraphicStem()
        {
            return m_stProperties.sGraphicStem;
        }

        virtual void SetGraphicStem(const CEString& sValue)
        {
            m_stProperties.sGraphicStem = sValue;
        }

        virtual long long llSecondPartId()
        {
            return m_stProperties.llSecondPartId;
        }

        virtual void SetSecondPartId(long long llValue)
        {
            m_stProperties.llSecondPartId = llValue;
        }

        virtual bool bHasIrregularForms()
        {
            return m_stProperties.bHasIrregularForms;
        }

        virtual void SetHasIrregularForms(bool bValue)
        {
            m_stProperties.bHasIrregularForms = bValue;
        }
        
        virtual bool bHasSecondaryStress()
        {
            return !m_stProperties.vecSecondaryStressPos.empty();
        }
        
        virtual bool bHasFleetingVowel()
        {
            return m_stProperties.bFleetingVowel;
        }

        virtual void SetHasFleetingVowel(bool bValue)
        {
            m_stProperties.bFleetingVowel = bValue;
        }
        
        virtual bool bHasYoAlternation()
        {
            return m_stProperties.bYoAlternation;
        }

        virtual void SetHasYoAlternation(bool bValue)
        {
            m_stProperties.bYoAlternation = bValue;
        }
        
        virtual bool bHasOAlternation()
        {
            return m_stProperties.bOAlternation;
        }

        virtual void SetHasOAlternation(bool bValue)
        {
            m_stProperties.bOAlternation = bValue;
        }
        
        virtual CEString sSourceForm()
        {
            return m_stProperties.sSourceForm;
        }

        virtual void SetSourceForm(const CEString& sValue)
        {
            m_stProperties.sSourceForm = sValue;
        }
        
        virtual CEString sHeadwordComment()
        {
            return m_stProperties.sHeadwordComment;
        }

        virtual void SetHeadwordComment(const CEString& sValue)
        {
            m_stProperties.sHeadwordComment = sValue;
        }
        
        virtual CEString sHeadwordVariant()
        {
            return m_stProperties.sHeadwordVariant;
        }

        virtual void SetHeadwordVariant(const CEString& sValue)
        {
            m_stProperties.sHeadwordVariant = sValue;
        }
        
        virtual CEString sHeadwordVariantComment()
        {
            return m_stProperties.sHeadwordVariantComment;
        }

        virtual void SetHeadwordVariantComment(const CEString& sValue)
        {
            m_stProperties.sHeadwordVariantComment = sValue;
        }
        
        virtual CEString sPluralOf()
        {
            return m_stProperties.sPluralOf;
        }

        virtual void SetPluralOf(const CEString& sValue)
        {
            m_stProperties.sPluralOf = sValue;
        }
        
        virtual CEString sUsage()
        {
            return m_stProperties.sUsage;
        }
        
        virtual void SetUsage(const CEString& sValue)
        {
            m_stProperties.sUsage = sValue;
        }
        
        virtual CEString sSeeRef()
        {
            return m_stProperties.sSeeRef;
        }
        
        virtual void SetSeeRef(const CEString& sValue)
        {
            m_stProperties.sSeeRef = sValue;
        }
        
        virtual CEString sBackRef()
        {
            return m_stProperties.sBackRef;
        }

        virtual void SetBackRef(const CEString& sValue)
        {
            m_stProperties.sBackRef = sValue;
        }
        
        virtual bool bIsUnstressed()
        {
            return m_stProperties.bIsUnstressed;
        }

        virtual void SetIsUnstressed(bool bValue)
        {
            m_stProperties.bIsUnstressed = bValue;
        }
        
        virtual bool bIsVariant()
        {
            return m_stProperties.bIsVariant;
        }

        virtual void SetIsVariant(bool bValue)
        {
            m_stProperties.bIsVariant = bValue;
        }
        
        virtual CEString sMainSymbol()
        {
            return m_stProperties.sMainSymbol;
        }

        virtual void SetMainSymbol(const CEString& sValue)
        {
            m_stProperties.sMainSymbol = sValue;
        }
        
        virtual bool bIsPluralOf()
        {
            return m_stProperties.bIsPluralOf;
        }

        virtual void SetIsPluralOf(bool bValue)
        {
            m_stProperties.bIsPluralOf = bValue;
        }
        
        virtual bool bTransitive()
        {
            return m_stProperties.bTransitive;
        }

        virtual void SetTransitive(bool bValue)
        {
            m_stProperties.bTransitive = bValue;
        }
        
        virtual ET_Reflexivity eIsReflexive()
        {
            return m_stProperties.eReflexive;
        }

        virtual void SetIsReflexive(ET_Reflexivity eValue)
        {
            m_stProperties.eReflexive = eValue;
        }
        
        virtual CEString sMainSymbolPluralOf()
        {
            return m_stProperties.sMainSymbolPluralOf;
        }

        virtual void SetMainSymbolPluralOf(const CEString& sValue)
        {
            m_stProperties.sMainSymbolPluralOf = sValue;
        }
        
        virtual CEString sAltMainSymbol()
        {
            return m_stProperties.sAltMainSymbol;
        }

        virtual void SetAltMainSymbol(const CEString& sValue)
        {
            m_stProperties.sAltMainSymbol = sValue;
        }
        
        virtual ET_Aspect eAspect()
        {
            return m_stProperties.eAspect;
        }

        virtual void SetAspect(ET_Aspect eValue)
        {
            m_stProperties.eAspect = eValue;
        }
        
        virtual CEString sInflectionType()
        {
            return m_stProperties.sInflectionType;
        }

        virtual void SetInflectionType(const CEString& sValue)
        {
            m_stProperties.sInflectionType = sValue;
        }
        
        virtual ET_PartOfSpeech ePartOfSpeech()
        {
            return m_stProperties.ePartOfSpeech;
        }

        virtual void SetPartOfSpeech(ET_PartOfSpeech eValue)
        {
            m_stProperties.ePartOfSpeech = eValue;
        }
        
        virtual CEString sComment()
        {
            return m_stProperties.sComment;
        }

        virtual void SetComment(const CEString& sValue)
        {
            m_stProperties.sComment = sValue;
        }
        
        virtual CEString sAltMainSymbolComment()
        {
            return m_stProperties.sAltMainSymbolComment;
        }

        virtual void SetAltMainSymbolComment(const CEString& sValue)
        {
            m_stProperties.sAltMainSymbolComment = sValue;
        }        

        virtual CEString sAltInflectionComment()
        {
            return m_stProperties.sAltInflectionComment;
        }

        virtual void SetAltInflectionComment(const CEString& sValue)
        {
            m_stProperties.sAltInflectionComment = sValue;
        }
        
        virtual CEString sVerbStemAlternation()
        {
            return m_stProperties.sVerbStemAlternation;
        }

        virtual void SetVerbStemAlternation(const CEString& sValue)
        {
            m_stProperties.sVerbStemAlternation = sValue;
        }
        
        virtual bool bPartPastPassZhd()
        {
            return m_stProperties.bPartPastPassZhd;
        }

        virtual void SetPartPastPassZhd(bool bValue)
        {
            m_stProperties.bPartPastPassZhd = bValue;
        }
        
        virtual int iSection()
        {
            return m_stProperties.iSection;
        }

        virtual void SetSection(int iValue)
        {
            m_stProperties.iSection = iValue;
        }
        
        virtual bool bNoComparative()
        {
            return m_stProperties.bNoComparative;
        }

        virtual void SetNoComparative(bool bValue)
        {
            m_stProperties.bNoComparative = bValue;
        }
        
        virtual bool bAssumedForms()
        {
            return m_stProperties.bAssumedForms;
        }

        virtual void SetAssumedForms(bool bValue)
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
        
        virtual bool bSecondGenitive()
        {
            return m_stProperties.bSecondGenitive;
        }

        virtual void SetSecondGenitive(bool bValue)
        {
            m_stProperties.bSecondGenitive = bValue;
        }
        
        virtual bool bSecondPrepositional()
        {
            return m_stProperties.bSecondPrepositional;
        }

        virtual void SetSecondPrepositional(bool bValue)
        {
            m_stProperties.bSecondPrepositional = bValue;
        }
        
        virtual bool bSecondPrepositionalOptional()
        {
            return m_stProperties.bSecondPrepositionalOptional;
        }

        virtual void SetSecondPrepositionalOptional(bool bValue)
        {
            m_stProperties.bSecondPrepositionalOptional = bValue;
        }
        
        virtual CEString sP2Preposition()
        {
            return m_stProperties.sP2Preposition;
        }

        virtual void SetP2Preposition(const CEString& sValue)
        {
            m_stProperties.sP2Preposition = sValue;
        }
        
        virtual bool bImpersonal()
        {
            return m_stProperties.bIsImpersonal;
        }

        virtual void SetImpersonal(bool bValue)
        {
            m_stProperties.bIsImpersonal = bValue;
        }

        virtual bool bIterative()
        {
            return m_stProperties.bIsIterative;
        }

        virtual void SetIterative(bool bValue)
        {
            m_stProperties.bIsIterative = bValue;
        }

        virtual bool bHasAspectPair()
        {
            return m_stProperties.bHasAspectPair && !m_stProperties.bSpryazhSmNoAspectPair;
        }

        virtual void SetHasAspectPair(bool bValue)
        {
            m_stProperties.bHasAspectPair = bValue;
        }

        virtual bool bHasAltAspectPair()
        {
            if (!m_stProperties.bHasAspectPair)
            {
                return false;
            }

            return m_stProperties.iAltAspectPairType != 0;
        }
        
        virtual int iAspectPairType()
        {
            return m_stProperties.iAspectPairType;
        }

        virtual void SetAspectPairType(int iValue)
        {
            m_stProperties.iAspectPairType = iValue;
        }

        virtual int iAltAspectPairType()
        {
            return m_stProperties.iAltAspectPairType;
        }
        
        virtual void SetAltAspectPairType(int iValue)
        {
            m_stProperties.iAltAspectPairType = iValue;
        }

        virtual CEString sAspectPairData()
        {
            return m_stProperties.sAspectPairData;
        }

        virtual void SetAspectPairData(const CEString& sValue)
        {
            m_stProperties.sAspectPairData = sValue;
        }

        virtual CEString sAltAspectPairData()
        {
            return m_stProperties.sAltAspectPairData;
        }

        virtual void SetAltAspectPairData(const CEString& sValue)
        {
            m_stProperties.sAltAspectPairData = sValue;
        }

        virtual CEString sAltAspectPairComment()
        {
            return m_stProperties.sAltAspectPairComment;
        }

        virtual void SetAltAspectPairComment(const CEString& sValue)
        {
            m_stProperties.sAltAspectPairComment = sValue;
        }
        
        virtual bool bHasIrregularVariants()
        {
            return m_stProperties.bHasIrregularVariants;    // true if any of irreg. forms alternates with regular
        }

        virtual void SetHasIrregularVariants(bool bValue)
        {
            m_stProperties.bHasIrregularVariants = bValue;
        }
        
        virtual bool bHasDifficultForms()
        {
            return m_stProperties.bHasDifficultForms;
        }

        virtual void SetHasDifficultForms(bool bValue)
        {
            m_stProperties.bHasDifficultForms = bValue;
        }

        virtual bool bHasMissingForms()
        {
            return m_stProperties.bHasMissingForms;
        }

        virtual void SetHasMissingForms(bool bValue)
        {
            m_stProperties.bHasMissingForms = bValue;
        }

        virtual void AddDifficultForm(const CEString& sGramHash)
        {
            m_vecDifficultForms.push_back(sGramHash);
        }

        virtual void AddMissingForm(const CEString& sGramHash)
        {
            m_vecMissingForms.push_back(sGramHash);
        }

        virtual CEString sRestrictedContexts()
        {
            return m_stProperties.sRestrictedContexts;
        }

        virtual void SetRestrictedContexts(const CEString& sValue)
        {
            m_stProperties.sRestrictedContexts = sValue;
        }
        
        virtual CEString sContexts()
        {
            return m_stProperties.sContexts;
        }

        virtual void SetContexts(const CEString& sValue)
        {
            m_stProperties.sContexts = sValue;
        }
        
        virtual CEString sTrailingComment()
        {
            return m_stProperties.sTrailingComment;
        }

        virtual void SetTrailingComment(const CEString& sValue)
        {
            m_stProperties.sTrailingComment = sValue;
        }
        
        // From inflection table:
        virtual long long llInflectionId()
        {
            return m_stProperties.llInflectionId;
        }

        virtual void SetInflectionId(int iValue)
        {
            m_stProperties.llInflectionId = iValue;
        }
        
        virtual bool bPrimaryInflectionGroup()
        {
            return m_stProperties.bPrimaryInflectionGroup;
        }

        virtual void SetPrimaryInflectionGroup(bool bValue)
        {
            m_stProperties.bPrimaryInflectionGroup = bValue;
        }
        
        virtual int iType()
        {
            return m_stProperties.iType;
        }

        virtual void SetType(int iValue)
        {
            m_stProperties.iType = iValue;
        }
        
        virtual ET_AccentType eAccentType1()
        {
            return m_stProperties.eAccentType1;
        }

        virtual void SetAccentType1(ET_AccentType eValue)
        {
            m_stProperties.eAccentType1 = eValue;
        }
        
        virtual ET_AccentType eAccentType2()
        {
            return m_stProperties.eAccentType2;
        }

        virtual void SetAccentType2(ET_AccentType eValue)
        {
            m_stProperties.eAccentType2 = eValue;
        }
        
        virtual bool bShortFormsRestricted()
        {
            return m_stProperties.bShortFormsRestricted;
        }

        virtual void SetShortFormsRestricted(bool bValue)
        {
            m_stProperties.bShortFormsRestricted = bValue;
        }
        
        virtual bool bPastParticipleRestricted()
        {
            return m_stProperties.bPastParticipleRestricted;
        }

        virtual void SetPastParticipleRestricted(bool bValue)
        {
            m_stProperties.bPastParticipleRestricted = bValue;
        }
        
        virtual bool bNoLongForms()
        {
            return m_stProperties.bNoLongForms;
        }

        virtual void SetNoLongForms(bool bValue)
        {
            m_stProperties.bNoLongForms = bValue;
        }
        
        virtual bool bShortFormsIncomplete()
        {
            return m_stProperties.bShortFormsIncomplete;
        }

        virtual void SetShortFormsIncomplete(bool bValue)
        {
            m_stProperties.bShortFormsIncomplete = bValue;
        }
        
        virtual bool bNoPassivePastParticiple()
        {
            return m_stProperties.bNoPassivePastParticiple;
        }

        virtual void SetNoPassivePastParticiple(bool bValue)
        {
            m_stProperties.bNoPassivePastParticiple = bValue;
        }
        
        virtual bool bFleetingVowel()
        {
            return m_stProperties.bFleetingVowel;
        }

        virtual void SetFleetingVowel(bool bValue)
        {
            m_stProperties.bFleetingVowel = bValue;
        }
        
        virtual int iStemAugment()
        {
            return m_stProperties.iStemAugment;
        }

        virtual void SetStemAugment(int iValue)
        {
            m_stProperties.iStemAugment = iValue;
        }
        
        virtual CEString s1SgStem()
        {
            return m_stProperties.s1SgStem;
        }

        virtual void Set1SgStem(const CEString& sValue)
        {
            m_stProperties.s1SgStem = sValue;
        }
        
        virtual CEString s3SgStem()
        {
            return m_stProperties.s3SgStem;
        }

        virtual void Set3SgStem(const CEString& sValue)
        {
            m_stProperties.s3SgStem = sValue;
        }
        
        virtual CEString sInfinitive()
        {
            return m_stProperties.sInfinitive;
        }

        virtual void SetInfinitive(const CEString& sValue)
        {
            m_stProperties.sInfinitive = sValue;
        }
        
        virtual CEString sInfStem()
        {
            return m_stProperties.sInfStem;
        }

        virtual void SetInfStem(const CEString& sValue)
        {
            m_stProperties.sInfStem = sValue;
        }
        
        virtual ET_ReturnCode eAddCommonDeviation(int iValue, bool bIsOptional);

        virtual void ClearCommonDeviations()
        {
            m_stProperties.mapCommonDeviations.clear();
        }
        
        //        virtual int iInflectedParts()
//        {
//            return m_stProperties.iInflectedParts;
//        }

//        virtual void SetInflectedParts(int iValue)
//        {
//            m_stProperties.iInflectedParts = iValue;
//        }

        virtual bool bHasPresPassParticiple()
        {
            return m_stProperties.bHasPresPassParticiple;
        }

        virtual void SetHasPresPassParticiple(bool bValue)
        {
            m_stProperties.bHasPresPassParticiple = bValue;
        }

        virtual bool bSpryazhSm()
        {
            return m_stProperties.bSpryazhSm;
        }

        virtual CEString sSpryazhSmRefSource()
        {
            return m_stProperties.sSpryazhSmRefSource;
        }

        virtual ET_Subparadigm eSubparadigm()
        {
            return m_stProperties.eSubparadigm;
        }

        virtual void SetSubparadigm(ET_Subparadigm eSp)    // currently only used with numerals
        {
            m_stProperties.eSubparadigm = eSp;
        }

        virtual bool bIsSecondPart()
        {
            return m_stProperties.bSecondPart;
        }

        virtual void SetIsSecondPart(bool bIsSecondPart)
        {
            m_stProperties.bSecondPart = bIsSecondPart;
        }

        virtual bool bFindCommonDeviation(int iNum, bool& bIsOptional);
        virtual bool bFindStandardAlternation(const CEString& sKey, CEString& sValue);

        virtual ET_ReturnCode eGetSourceFormWithStress(CEString& sSourceForm, bool bIsVariant=false);
        virtual ET_ReturnCode eGetSourceFormWithDiacritics(CEString& sSourceForm, bool bIsVariant = false);
        virtual ET_ReturnCode eGetFirstStemStressPos(int& iPos);
        virtual ET_ReturnCode eGetNextStemStressPos(int& iPos);
        virtual ET_ReturnCode eGetFirstSecondaryStemStressPos(int& iPos);
        virtual ET_ReturnCode eGetNextSecondaryStemStressPos(int& iPos);

        virtual ET_ReturnCode eGetAlternatingPreverb(const CEString& sVerbForm, CEString& sPreverb, bool& bVoicing);

        virtual CEString sHash();
        virtual CEString sParadigmHash();
        virtual ET_ReturnCode eWordFormFromHash(CEString sHash, int iAt, IWordForm *& pWf);
        virtual ET_ReturnCode eCreateWordForm(IWordForm *&);
        virtual ET_ReturnCode eRemoveWordForm(CEString sHash, int iAt);
        virtual ET_ReturnCode eRemoveWordForms(CEString sHash);
        virtual void AddWordForm(IWordForm* pWordForm);         // ITF version, cf internal version below

        virtual bool bHasIrregularForm(CEString sGramHash);
        virtual bool bNoRegularForms(CEString sGramHash);

        ET_ReturnCode eGetFirstWordForm(IWordForm *& pWf);
        ET_ReturnCode eGetNextWordForm(IWordForm *& pWf);

        virtual ET_ReturnCode eGetFirstIrregularForm(CEString sHash, IWordForm *&, bool& bIsOptional);
        virtual ET_ReturnCode eGetFirstIrregularForm(IWordForm *&, bool& bIsOptional);
        virtual ET_ReturnCode eGetNextIrregularForm(IWordForm *&, bool& bIsOptional);

        ET_ReturnCode eGetFirstIrregularForm(CEString sHash, CWordForm *&, bool& bIsOptional);
        ET_ReturnCode eGetFirstIrregularForm(CWordForm *&, bool& bIsOptional);
        ET_ReturnCode eGetNextIrregularForm(CWordForm *&, bool& bIsOptional);

        virtual ET_ReturnCode eGetAspectPair(CEString&, int&);
        virtual ET_ReturnCode eGetAltAspectPair(CEString&, int&);
        virtual int iFormCount(CEString sHash);
        virtual bool bHasCommonDeviation(int iCd);
        virtual bool bDeviationOptional(int iCd);
        virtual ET_ReturnCode eFormExists(const CEString& hash);
        virtual ET_ReturnCode eSetFormExists(const CEString& hash, bool bExists);
        virtual ET_ReturnCode eIsFormDifficult(const CEString& sGramHash);
        virtual ET_ReturnCode eSetFormDifficult(const CEString& hash, bool bIsDifficult);
        virtual ET_ReturnCode eDifficultFormsHashes(vector<CEString>&);
        virtual ET_ReturnCode eIsFormAssumed(const CEString& sGramHash);
        virtual ET_ReturnCode eSetHasAssumedForms(bool bIsAssumed);
        virtual bool bIsMultistressedCompound();
        virtual ET_ReturnCode eSetDb(const CEString& sDbPath);
        virtual ET_ReturnCode eGenerateParadigm();
        virtual ET_ReturnCode eSaveTestData();
        virtual ET_ReturnCode eDeleteIrregularForm(const CEString& sFormHash);
        virtual ET_ReturnCode eSaveIrregularForm(const CEString& sFormHash, IWordForm *&);
        virtual ET_ReturnCode eSaveIrregularForms(const CEString& sGramHash);

        virtual ET_ReturnCode eMakeGraphicStem();
        virtual ET_ReturnCode eMakeGraphicStem(const CEString& sSource, CEString& sGraphicStem);

        virtual ET_ReturnCode eClone(ILexeme *&);
        virtual ET_ReturnCode eCheckLexemeProperties();
        virtual ET_ReturnCode eGetErrorMsg(CEString& sErrorMsg);

        // Non-virtual public functions (not available to external consumers)
    public:
        CSqlite * pGetDb();
        StLexemeProperties& stGetSecondPartProperties();

        ET_ReturnCode eWordFormFromHash(CEString sHash, int iAt, CWordForm *& pWf);
        ET_ReturnCode eGetStemStressPositions(const CEString& sStem, vector<int>& vecPosition);
        ET_ReturnCode eGetIrregularForms(CEString sHash, map<CWordForm *, bool>& mapResult);
        void AssignPrimaryStress(CWordForm * pWordForm);
        void AssignSecondaryStress(CWordForm * pWordForm);
        void AddWordForm(CWordForm* pWordForm);
        void SetHypotheticalForm(CEString& sGramHash);
        bool bIsHypotheticalForm(CEString& sGramHash);
        uint64_t uiTotalWordForms();

        // Populate DB:
        ET_ReturnCode eSaveStemsToDb();
        ET_ReturnCode eAssignStemIds();
//        ET_ReturnCode ePrepareDataInsert(uint64_t& uiStmtHandle);
        ET_ReturnCode eSaveWordFormsToDb();
        ET_ReturnCode eSaveIrregularFormsToDb();  // for spryazh sm entries

        // Helpers:
//        ET_ReturnCode eLoadDifficultAndMissingForms();
        ET_ReturnCode eLoadIrregularForms();
        ET_ReturnCode eHandleSpryazhSmForms();
        ET_ReturnCode eSaveIrregularForms(long long llDescriptorDbKey);
        ET_ReturnCode eLoadDifficultForms();
        ET_ReturnCode eLoadMissingForms();
        CEString sGramHashNSgMLong();
//        ET_ReturnCode eDifficultFormLabelToGramHash(const CEString sLabel);
        void SetSecondPart(shared_ptr<CLexeme>&);

        // Support for manual editing
        ET_ReturnCode eUpdateDescriptorInfo(CLexeme *);
        ET_ReturnCode eGetWordForm(unsigned int uiAt, CWordForm *&);
        ET_ReturnCode eExtractStressSymbols();
        ET_ReturnCode eInitializeFromProperties();

    private:
        void Init();
        ET_ReturnCode eAlignInflectedParts();
        ET_ReturnCode eConcatenateInflectedParts(CWordForm& left, CWordForm& right);

    private:
//        CLexeme() {}

        //
        // Member variables
        //
        StLexemeProperties m_stProperties, m_stProperties2ndPart;
        bool m_bFormsGenerated;
        multimap<CEString, CWordForm *> m_mmWordForms;        // gramm. hash --> wordform struct
        map<CEString, ET_MainSymbol> m_mapMainSymbol;
        map<CEString, ET_Gender> m_mapMainSymbolToGender;
        map<CEString, ET_Animacy> m_mapMainSymbolToAnimacy;
        map<CEString, CEString> m_mapStandardAlternations;
        multimap<int, int> m_mmEndingsHash;
        vector<CEString> m_vecAlternatingPreverbs, m_vecAlternatingPreverbsWithVoicing;
        CDictionary * m_pDictionary;
        shared_ptr<CLexeme> m_spSecondPart;
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
        vector<CEString> m_vecDifficultForms;
        vector<CEString> m_vecMissingForms;

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