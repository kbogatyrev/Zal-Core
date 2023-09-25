#ifndef C_WORDFORM_H_INCLUDED
#define C_WORDFORM_H_INCLUDED

#include "Enums.h"
#include "EString.h"

#include "SqliteWrapper.h"
#include "Lexeme.h"
#include "Inflection.h"
#include "GramHasher.h"

using namespace std;

namespace Hlib
{
    class CHasher;
    class CLexeme;
    class CInflection;

    class CWordForm
    {
        friend class CHasher;
        friend class CInflection;

    public:
        CWordForm();
        CWordForm(shared_ptr<CInflection>);
        CWordForm(const CEString& sHash, shared_ptr<CInflection>);
        CWordForm(const CWordForm&);

        const CWordForm& operator= (const CWordForm&);

        ET_ReturnCode eInitFromHash(const CEString& sHash);

        // Partial copy
        // TODO: Can we use copy ctor instead?
        ET_ReturnCode eCloneFrom(const shared_ptr<CWordForm>);

        shared_ptr<CLexeme> spLexeme()
        {
            return m_spInflection->spLexeme();
        }

        shared_ptr<CInflection> spInflection()
        {
            return m_spInflection;
        }

        void SetInflection(shared_ptr<CInflection>);

        CEString sWordForm()
        {
            return m_sWordForm;
        }

        void SetWordForm(const CEString& sWf)
        {
            m_sWordForm = sWf;
        }

        long long llDbId()
        {
            return m_llDbKey;
        }

        CEString sStem()
        {
            return m_sStem;
        }

        void SetStem(const CEString& sStem)
        {
            m_sStem = sStem;
        }

        long long llDbKey()
        {
            return m_llDbKey;
        }

        void SetDbKey(long long llDbKey)
        {
            m_llDbKey = llDbKey;
        }

        long long llStemId()
        {
            return m_llStemId;
        }

        void SetStemId(long long llStemId)
        {
            m_llStemId = llStemId;
        }

        long long llStemDataId()
        {
            return m_llStemDataId;
        }

        void SetStemDataId(long long llStemDataId)
        {
            m_llStemDataId = llStemDataId;
        }

        long long llIrregularFormId()
        {
            return m_llIrregularFormId;
        }

        void SetIrregularFormId(long long llIrregularFormId)
        {
            m_llIrregularFormId = llIrregularFormId;
        }

        CEString sEnding()
        {
            return m_sEnding;
        }

        void SetEnding(const CEString& sEnding)
        {
            m_sEnding = sEnding;
        }

        long long llEndingDataId()
        {
            return m_llEndingDataId;
        }

        void SetEndingDataId(long long llEndingDataId)
        {
            m_llEndingDataId = llEndingDataId;
        }

        long long llLexemeId()
        {
            return m_llLexemeId;
        }

        void SetLexemeId(long long llLexemeId)
        {
            m_llLexemeId = llLexemeId;
        }

        long long llInflectionId()
        {
            return m_llInflectionId;
        }

        void SetInflectionId(long long llInflectionId)
        {
            m_llInflectionId = llInflectionId;
        }

        ET_PartOfSpeech ePos()
        {
            return m_ePos;
        }

        void SetPos(ET_PartOfSpeech ePos)
        {
            m_ePos = ePos;
        }

        ET_Case eCase()
        {
            return m_eCase;
        }

        void SetCase(ET_Case eCase)
        {
            m_eCase = eCase;
        }

        ET_Subparadigm eSubparadigm()
        {
            return m_eSubparadigm;
        }

        void SetSubparadigm(ET_Subparadigm eSubparadigm)
        {
            m_eSubparadigm = eSubparadigm;
        }

        ET_Number eNumber()
        {
            return m_eNumber;
        }

        void SetNumber(ET_Number eNumber)
        {
            m_eNumber = eNumber;
        }

        ET_Gender eGender()
        {
            return m_eGender;
        }

        void SetGender(ET_Gender eGender)
        {
            m_eGender = eGender;
        }

        ET_Person ePerson()
        {
            return m_ePerson;
        }

        void SetPerson(ET_Person ePerson)
        {
            m_ePerson = ePerson;
        }

        ET_Animacy eAnimacy()
        {
            return m_eAnimacy;
        }

        void SetAnimacy(ET_Animacy eAnimacy)
        {
            m_eAnimacy = eAnimacy;
        }

        ET_Reflexivity eReflexive()
        {
            return m_eReflexivity;
        }

        void SetReflexivity(ET_Reflexivity eReflexivity)
        {
            m_eReflexivity = eReflexivity;
        }

        ET_Aspect eAspect()
        {
            return m_eAspect;
        }

        void SetAspect(ET_Aspect eAspect)
        {
            m_eAspect = eAspect;
        }

        ET_Status eStatus()
        {
            return m_eStatus;
        }

        void SetStatus(ET_Status eStatus)
        {
            m_eStatus = eStatus;
        }

        bool bIrregular()       // came from the DB as opposed to being generated by the app
        {
            return m_bIrregular;
        }

        void SetIrregular(bool bIrregular)
        {
            m_bIrregular = bIrregular;
        }

        CEString sLeadComment()
        {
            return m_sLeadComment;
        }

        void SetLeadComment(const CEString& sLeadComment)
        {
            m_sLeadComment = sLeadComment;
        }

        CEString sTrailingComment()
        {
            return m_sTrailingComment;
        }

        void SetTrailingComment(const CEString& sTrailingComment)
        {
            m_sTrailingComment = sTrailingComment;
        }

        bool bIsEdited()
        {
            return m_bIsEdited;
        }

        void SetIsEdited(bool bIsEdited)
        {
            m_bIsEdited = bIsEdited;
        }

        bool bIsVariant()
        {
            return m_bIsVariant;
        }

        void SetIsVariant(bool bIsVariant)
        {
            m_bIsVariant = bIsVariant;
        }

        bool bIsDifficult()
        {
            return m_bIsDifficult;
        }
        
        void SetIsDifficult(bool bIsDifficult)
        {
            m_bIsDifficult = bIsDifficult;
        }

        CEString sGramHash();

        void ClearStress();
        void AssignStress(const map<int, ET_StressType>&);
        int iStressPositions();
        ET_ReturnCode eGetFirstStressPos(int& iPos, ET_StressType&);
        ET_ReturnCode eGetNextStressPos(int& iPos, ET_StressType&);
        ET_ReturnCode eFindStressPos(int iPos, ET_StressType&);
        ET_ReturnCode eRemoveStressPos(int);
        ET_ReturnCode eGetFirstStressSyll(int& iSyll, ET_StressType&);
        ET_ReturnCode eGetNextStressSyll(int& iSyll, ET_StressType&);

        const map<int, ET_StressType>& mapGetStressPositions();
        ET_ReturnCode eSetStressPositions(map<int, ET_StressType> mapStress);
        void SetStressPos(int iPos, ET_StressType);

        ET_ReturnCode eSaveIrregularForm();

        // Should not be exposed thru itf??? 
        bool bSaveStemToDb();
        bool bSaveToDb();
        bool bSaveIrregularForm();
        ET_ReturnCode eSaveTestData();
        void Copy(const CWordForm&);

    private:
        shared_ptr<CInflection> m_spInflection;
        unsigned long long m_ullDbInsertHandle;
        long long m_llDbKey;
        CEString m_sWordForm;
        CEString m_sStem;
        long long m_llStemId;             // key into stems table
        long long m_llStemDataId;         // key into stem_data table
        long long m_llIrregularFormId;    // irreg. only
        CEString m_sEnding;
        long long m_llEndingDataId;       // key into ending_data table
        long long m_llLexemeId;
        long long m_llInflectionId;
        map<int, ET_StressType> m_mapStress; // <stressed syll, primary/secondary>
        ET_PartOfSpeech m_ePos;
        ET_Case m_eCase;
        ET_Number m_eNumber;
        ET_Subparadigm m_eSubparadigm;
        ET_Gender m_eGender;
        ET_Person m_ePerson;
        ET_Animacy m_eAnimacy;
        ET_Reflexivity m_eReflexivity;
        ET_Aspect m_eAspect;
        ET_Status m_eStatus;
        bool m_bIrregular;              // came from the DB as opposed to being generated by the app
        bool m_bIsDifficult;
        ET_WordStressType m_eStressType;

// Irregular forms only:
        CEString m_sLeadComment;
        CEString m_sTrailingComment;
        bool m_bIsVariant;
        bool m_bIsEdited;
//
        map<int, ET_StressType>::iterator m_itStressSyll;

    };      //  class CWordForm

    class CHasher : public CGramHasher
    {
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
                                                     wf.m_eReflexivity)
        {}

        ET_ReturnCode eFromHash (const CEString& sHash, CWordForm& wf)
        {
            ET_ReturnCode rc = eDecodeHash (sHash);
            if (H_NO_ERROR != rc)
            {
                assert(0);
                CEString sMsg(L"hDecodeHash() failed, error ");
                sMsg += CEString::sToString(rc);
                ERROR_LOG (sMsg);
                return rc;
            }
        
            wf.m_ePos = m_ePos;
            wf.m_eSubparadigm = m_eSubparadigm;
            wf.m_eCase = m_eCase;
            wf.m_eNumber = m_eNumber;
            wf.m_eGender = m_eGender;
            wf.m_ePerson = m_ePerson;
            wf.m_eAnimacy = m_eAnimacy;
            wf.m_eReflexivity = m_eReflexivity;
            wf.m_eAspect = m_eAspect;

            return H_NO_ERROR;
        }

    private:
        shared_ptr<CInflection> m_spInflection;
        long long m_llLexemeId;
        long long m_llInflectionId;

    };      //  class CHasher : public CGramHasher

}       //  namespace Hlib

#endif      // C_WORDFORM_H_INCLUDED
