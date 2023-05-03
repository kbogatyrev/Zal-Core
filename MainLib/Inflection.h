#ifndef C_INFLECTION_H_INCLUDED
#define C_INFLECTION_H_INCLUDED

#include "Enums.h"
#include "EString.h"

#include "Lexeme.h"

//#include "IInflection.h"
#include "InflectionProperties.h"

using namespace std;

namespace Hlib
{
    class CDictionary;
    class CSqlite;
    class CGramHasher;
    class CWordForm;

//    static void operator++ (ET_MainSymbol& eMs)
//    {
//        eMs = (ET_MainSymbol)(eMs + 1);
//    }

    class CInflection : public enable_shared_from_this<CInflection>
    {
    public:
        CInflection(shared_ptr<CLexeme>);
        CInflection() = delete;
        CInflection(const CInflection&);
        ~CInflection();

//        CInflection* pGetInflectionPtr()
//        {
//            return this;
//        }

        shared_ptr<CLexeme> spGetLexeme()
        {
            return m_spLexeme;
        }

        void SetLexeme(shared_ptr<CLexeme> spLexeme)
        {
            if (nullptr == spLexeme)
            {
                throw CException(H_EXCEPTION, L"Lexeme pointer is NULL.");
            }
            m_spLexeme = spLexeme;
        }

        long long llDescriptorId()
        {
            return m_spLexeme->llLexemeId();
        }

        ET_ReturnCode eGetLexeme(shared_ptr<CLexeme>& spLexeme)
        {
            spLexeme = m_spLexeme;
            return H_NO_ERROR;
        }

        shared_ptr<CLexeme> spLexeme()
        {
            return m_spLexeme;
        }

        int nWordForms()
        {
            return (int)m_mmWordForms.size();
        }

//        ET_ReturnCode eGetWordForm(int iAt, shared_ptr<CInflection>& spInflection)
//        {
//            spInflection = m_mmWordForms&&&&
//            return H_NO_ERROR;
//        }

        const StInflectionProperties& stGetProperties()
        {
            return m_stProperties;
        }

        StInflectionProperties& stGetPropertiesForWriteAccess()
        {
            return m_stProperties;
        }

        // From inflection table:
        long long llInflectionId()
        {
            return m_stProperties.llInflectionId;
        }

        void SetInflectionId(int iValue)
        {
            m_stProperties.llInflectionId = iValue;
        }

        bool bHasFleetingVowel()
        {
            return m_stProperties.bFleetingVowel;
        }

        void SetHasFleetingVowel(bool bValue)
        {
            m_stProperties.bFleetingVowel = bValue;
        }

        bool bPrimaryInflectionGroup()
        {
            return m_stProperties.bPrimaryInflectionGroup;
        }

        void SetPrimaryInflectionGroup(bool bValue)
        {
            m_stProperties.bPrimaryInflectionGroup = bValue;
        }

        int iType()
        {
            return m_stProperties.iType;
        }

        void SetType(int iValue)
        {
            m_stProperties.iType = iValue;
        }

        ET_AccentType eAccentType1()
        {
            return m_stProperties.eAccentType1;
        }

        void SetAccentType1(ET_AccentType eValue)
        {
            m_stProperties.eAccentType1 = eValue;
        }

        ET_AccentType eAccentType2()
        {
            return m_stProperties.eAccentType2;
        }

        void SetAccentType2(ET_AccentType eValue)
        {
            m_stProperties.eAccentType2 = eValue;
        }

        bool bShortFormsRestricted()
        {
            return m_stProperties.bShortFormsRestricted;
        }

        void SetShortFormsRestricted(bool bValue)
        {
            m_stProperties.bShortFormsRestricted = bValue;
        }

        bool bPastParticipleRestricted()
        {
            return m_stProperties.bPastParticipleRestricted;
        }

        void SetPastParticipleRestricted(bool bValue)
        {
            m_stProperties.bPastParticipleRestricted = bValue;
        }

        //        virtual bool bNoLongForms()
        //        {
        //            return m_stProperties.bNoLongForms;
        //        }

        //        virtual void SetNoLongForms(bool bValue)
        //        {
        //            m_stProperties.bNoLongForms = bValue;
        //        }

        bool bShortFormsIncomplete()
        {
            return m_stProperties.bShortFormsIncomplete;
        }

        void SetShortFormsIncomplete(bool bValue)
        {
            m_stProperties.bShortFormsIncomplete = bValue;
        }

        bool bNoPassivePastParticiple()
        {
            return m_stProperties.bNoPassivePastParticiple;
        }

        void SetNoPassivePastParticiple(bool bValue)
        {
            m_stProperties.bNoPassivePastParticiple = bValue;
        }

        bool bFleetingVowel()
        {
            return m_stProperties.bFleetingVowel;
        }

        void SetFleetingVowel(bool bValue)
        {
            m_stProperties.bFleetingVowel = bValue;
        }

        int iStemAugment()
        {
            return m_stProperties.iStemAugment;
        }

        void SetStemAugment(int iValue)
        {
            m_stProperties.iStemAugment = iValue;
        }

        void ClearCommonDeviations()
        {
            m_stProperties.mapCommonDeviations.clear();
        }

        bool bHasCommonDeviation(int iCd);

        bool bDeviationOptional(int iCd);

        ET_ReturnCode eAddCommonDeviation(int iValue, bool bIsOptional);

        bool bFindCommonDeviation(int iNum, bool& bIsOptional);

        ET_ReturnCode eFormExists(const CEString& hash);
        ET_ReturnCode eSetFormExists(const CEString& hash, bool bExists);

        ET_ReturnCode eGenerateParadigm();

        void AddDifficultForm(const CEString& sGramHash)
        {
            m_vecDifficultForms.push_back(sGramHash);
        }

        void AddMissingForm(const CEString& sGramHash)
        {
            m_vecMissingForms.push_back(sGramHash);
        }

        bool bHasIrregularForm(CEString sGramHash);
        bool bNoRegularForms(CEString sGramHash);

        ET_ReturnCode eGetFirstWordForm(shared_ptr<CWordForm>&);
        ET_ReturnCode eGetNextWordForm(shared_ptr<CWordForm>&);
//        ET_ReturnCode eGetFirstWordForm(CWordForm*&);               // .Net consumers only
//        ET_ReturnCode eGetNextWordForm(CWordForm*&);                //        --"--

        //        ET_ReturnCode eGetFirstIrregularForm(CEString sHash, IWordForm*&, bool& bIsOptional);
        ET_ReturnCode eGetFirstIrregularForm(shared_ptr<CWordForm>&, bool& bIsOptional);
        ET_ReturnCode eGetNextIrregularForm(shared_ptr<CWordForm>&, bool& bIsOptional);
        ET_ReturnCode eGetFirstIrregularForm(CWordForm*&, bool& bIsOptional);           // .Net consumers only
        ET_ReturnCode eGetNextIrregularForm(CWordForm*&, bool& bIsOptional);            //        --"--

        ET_ReturnCode eGetFirstIrregularForm(CEString sHash, shared_ptr<CWordForm>&, bool& bIsOptional);
        ET_ReturnCode eGetFirstIrregularForm(CEString sHash, CWordForm*&, bool& bIsOptional);
        //        ET_ReturnCode eGetFirstIrregularForm(shared_ptr<CWordForm>&, bool& bIsOptional);
        //        ET_ReturnCode eGetNextIrregularForm(shared_ptr<CWordForm>&, bool& bIsOptional);

        ET_ReturnCode eGetIrregularForms(CEString sHash, map<shared_ptr<CWordForm>, bool>& mapResult);
//        ET_ReturnCode eGetIrregularForms(CEString sHash, map<shared_ptr<CWordForm>, bool>& mapResult);
        void AssignPrimaryStress(shared_ptr<CWordForm>);
        void AssignSecondaryStress(shared_ptr<CWordForm> spWordForm);

        ET_ReturnCode eWordFormFromHash(CEString sHash, int iAt, shared_ptr<CWordForm>& spWf);
        ET_ReturnCode eCreateWordForm(shared_ptr<CWordForm>&);
        ET_ReturnCode eRemoveWordForm(CEString sHash, int iAt);
        ET_ReturnCode eRemoveWordForms(CEString sHash);

        //ET_ReturnCode eGetFirstIrregularForm(CEString sHash, CWordForm *&, bool& bIsOptional);
        //ET_ReturnCode eGetFirstIrregularForm(CWordForm *&, bool& bIsOptional);
        //ET_ReturnCode eGetNextIrregularForm(CWordForm *&, bool& bIsOptional);

        int iFormCount(CEString sHash);

        void AddWordForm(shared_ptr<CWordForm>);
        void SetHypotheticalForm(CEString& sGramHash);
        bool bIsHypotheticalForm(CEString& sGramHash);
        uint64_t uiTotalWordForms();

        ET_ReturnCode eSaveTestData();

        CEString sHash();
        CEString sParadigmHash();

        ET_ReturnCode eIsFormDifficult(const CEString& sGramHash);
        ET_ReturnCode eSetFormDifficult(const CEString& hash, bool bIsDifficult);
        ET_ReturnCode eDifficultFormsHashes(vector<CEString>&);
        ET_ReturnCode eIsFormAssumed(const CEString& sGramHash);
        ET_ReturnCode eSetHasAssumedForms(bool bIsAssumed);

        ET_ReturnCode eDeleteIrregularForm(const CEString& sFormHash);
        ET_ReturnCode eSaveIrregularForm(const CEString& sFormHash, shared_ptr<CWordForm>&);
        ET_ReturnCode eSaveIrregularForms(const CEString& sGramHash);

        bool bIsMultistressedCompound();

        ET_ReturnCode eMakeGraphicStem();
        ET_ReturnCode eMakeGraphicStem(const CEString& sSource, CEString& sGraphicStem);

        ET_ReturnCode eGetAspectPair(CEString& sAspectPair, int& iStressPos);
        ET_ReturnCode eGetAltAspectPair(CEString& sAltAspectPair, int& iStressPos);

        // -----------------------------------------------------------------------------------

        ET_ReturnCode eAlignInflectedParts();
        ET_ReturnCode eConcatenateInflectedParts(CWordForm& left, CWordForm& right);

        ET_ReturnCode eLoadMissingForms();
        ET_ReturnCode eGetWordForm(unsigned int uiAt, shared_ptr<CWordForm>&);


        // Populate DB:
        ET_ReturnCode eSaveStemsToDb();
        ET_ReturnCode eAssignStemIds();
        //        ET_ReturnCode ePrepareDataInsert(uint64_t& uiStmtHandle);
        ET_ReturnCode eSaveWordFormsToDb();
        ET_ReturnCode eSaveIrregularFormsToDb();  // for spryazh sm entries
        ET_ReturnCode eLoadIrregularForms();
        ET_ReturnCode eSaveIrregularForms(long long llDescriptorDbKey);
        ET_ReturnCode eLoadDifficultForms();

    private:
        shared_ptr<CLexeme> m_spLexeme;
        StInflectionProperties m_stProperties;

        // Aspect pairs
        CEString m_sAspectPair;
        int m_iAspectPairStressPos;
        CEString m_sAltAspectPair;
        int m_iAltAspectPairStressPos;

        //        bool m_bFormsGenerated;
        multimap<CEString, shared_ptr<CWordForm>> m_mmWordForms;        // gramm. hash --> wordform struct
        map<CEString, ET_MainSymbol> m_mapMainSymbol;
        map<CEString, ET_Gender> m_mapMainSymbolToGender;
        map<CEString, ET_Animacy> m_mapMainSymbolToAnimacy;
        map<CEString, CEString> m_mapStandardAlternations;
        multimap<int, int> m_mmEndingsHash;
        //        vector<CEString> m_vecAlternatingPreverbs, m_vecAlternatingPreverbsWithVoicing;
//        CDictionary* m_pDictionary;
//        shared_ptr<CLexeme> m_spSecondPart;
        vector<CEString> m_vecHypotheticalForms;     //  gram hashes of missing forms that were generated anyway 
        //  to be used in generation of other forms

// Word forms
        multimap<CEString, shared_ptr<CWordForm>>::iterator m_itCurrentWordForm;

        // Irregular forms
        multimap<CEString, StIrregularForm> m_mmapIrregularForms;    // hash to irregular form struct
        typedef pair<multimap<CEString, StIrregularForm>::iterator,
            multimap<CEString, StIrregularForm>::iterator> Pair_itIrregularFormRange;
        Pair_itIrregularFormRange m_pairItIfRange;
        multimap<CEString, StIrregularForm>::iterator m_itCurrentIrregularForm;


        vector<CEString> m_vecDifficultForms;
        vector<CEString> m_vecMissingForms;

        bool m_bFormsGenerated { false };
    };

}   //  namespace Hlib

#endif  //  C_INFLECTION_H_INCLUDED