#ifndef C_WORDFORM_H_INCLUDED
#define C_WORDFORM_H_INCLUDED

#include "IWordForm.h"
#include "SqliteWrapper.h"
#include "Lexeme.h"
#include "GramHasher.h"

using namespace std;
using namespace std::tr1;

namespace Hlib
{
    class CHasher;
    class CLexeme;

    struct CWordForm : public IWordForm
    {
        CWordForm();
        CWordForm(const CEString& sHash);
        CWordForm(const CWordForm&);
        CWordForm(const CWordForm*);

        virtual ET_ReturnCode eInitFromHash(const CEString& sHash);

        virtual ET_ReturnCode eClone(IWordForm *&);

        virtual ILexeme * pLexeme();

        virtual void SetLexeme(ILexeme * pLexeme);

        virtual CEString sWordForm()
        {
            return m_sWordForm;
        }

        virtual void SetWordForm(const CEString& sWf)
        {
            m_sWordForm = sWf;
        }

        virtual CEString sStem()
        {
            return m_sStem;
        }

        virtual void SetStem(const CEString& sStem)
        {
            m_sStem = sStem;
        }

        virtual __int64 llStemId()
        {
            return m_llStemId;
        }

        virtual CEString sEnding()
        {
            return m_sEnding;
        }

        virtual __int64 llEndingDataId()
        {
            return m_llEndingDataId;
        }

        virtual __int64 llLexemeId()
        {
            return m_llLexemeId;
        }

        virtual ET_PartOfSpeech ePos()
        {
            return m_ePos;
        }

        virtual void SetPos(ET_PartOfSpeech ePos)
        {
            m_ePos = ePos;
        }

        virtual ET_Case eCase()
        {
            return m_eCase;
        }

        virtual void SetCase(ET_Case eCase)
        {
            m_eCase = eCase;
        }

        virtual ET_Subparadigm eSubparadigm()
        {
            return m_eSubparadigm;
        }

        virtual void SetSubparadigm(ET_Subparadigm eSubparadigm)
        {
            m_eSubparadigm = eSubparadigm;
        }

        virtual ET_Number eNumber()
        {
            return m_eNumber;
        }

        virtual void SetNumber(ET_Number eNumber)
        {
            m_eNumber = eNumber;
        }

        virtual ET_Gender eGender()
        {
            return m_eGender;
        }

        virtual void SetGender(ET_Gender eGender)
        {
            m_eGender = eGender;
        }

        virtual ET_Person ePerson()
        {
            return m_ePerson;
        }

        virtual void SetPerson(ET_Person ePerson)
        {
            m_ePerson = ePerson;
        }

        virtual ET_Animacy eAnimacy()
        {
            return m_eAnimacy;
        }

        virtual void SetAnimacy(ET_Animacy eAnimacy)
        {
            m_eAnimacy = eAnimacy;
        }

        virtual ET_Reflexive eReflexive()
        {
            return m_eReflexive;
        }

        virtual void SetReflexive(ET_Reflexive eReflexive)
        {
            m_eReflexive = eReflexive;
        }

        virtual ET_Aspect eAspect()
        {
            return m_eAspect;
        }

        virtual void SetAspect(ET_Aspect eAspect)
        {
            m_eAspect = eAspect;
        }

        virtual ET_Status eStatus()
        {
            return m_eStatus;
        }

        virtual void SetStatus(ET_Status eStatus)
        {
            m_eStatus = eStatus;
        }

        virtual bool bIrregular()       // came from the DB as opposed to being generated by the app
        {
            return m_bIrregular;
        }

        virtual void SetIrregular(bool bIrregular)
        {
            m_bIrregular = bIrregular;
        }

        virtual CEString sGramHash();

        virtual ET_ReturnCode eGetFirstStressPos(int& iPos, ET_StressType& eType);
        virtual ET_ReturnCode eGetNextStressPos(int& iPos, ET_StressType& eType);

        virtual ET_ReturnCode eSaveIrregularForm();
        virtual ET_ReturnCode eSaveIrregularStress(map<int, ET_StressType>&);

        // Not exposed thru itf??? 
        bool bSaveStemToDb();
        bool bSaveToDb();
        ET_ReturnCode eSaveTestData();
        void Copy(const CWordForm&);

        //
        // Member variables; public to facilitate access from internal consumers
        //
        CLexeme * m_pLexeme;
        unsigned int m_uiDbInsertHandle;
        CEString m_sWordForm;
        CEString m_sStem;
        __int64 m_llStemId;             // key into stems table
        __int64 m_llStemDataId;         // key into stem_data table
        CEString m_sEnding;
        __int64 m_llEndingDataId;       // key into ending_data table
        __int64 m_llLexemeId;
        map<int, ET_StressType> m_mapStress; // <stressed syll, primary/secondary>
        ET_PartOfSpeech m_ePos;
        ET_Case m_eCase;
        ET_Number m_eNumber;
        ET_Subparadigm m_eSubparadigm;
        ET_Gender m_eGender;
        ET_Person m_ePerson;
        ET_Animacy m_eAnimacy;
        ET_Reflexive m_eReflexive;
        ET_Aspect m_eAspect;
        ET_Status m_eStatus;
        bool m_bIrregular;              // came from the DB as opposed to being generated by the app
        map<int, ET_StressType>::iterator m_itStressPos;

    };      //  struct CWordForm : public IWordForm

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
                                                     wf.m_eReflexive)
        {}

        ET_ReturnCode eFromHash (const CEString& sHash, CWordForm& wf)
        {
            ET_ReturnCode rc = eDecodeHash (sHash);
            if (H_NO_ERROR != rc)
            {
                ASSERT(0);
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
            wf.m_eReflexive = m_eReflexive;
            wf.m_eAspect = m_eAspect;

            return H_NO_ERROR;
        }

    private:
        __int64 m_llLexemeId;

    };      //  class CHasher : public CGramHasher

}       //  namespace Hlib

#endif      // C_WORDFORM_H_INCLUDED
