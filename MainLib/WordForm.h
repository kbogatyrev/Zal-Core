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
        CWordForm(int iHash);
        CWordForm(const CWordForm&);

        virtual ET_ReturnCode eInitFromHash(int iHash);
        virtual ILexeme * pLexeme();

        virtual CEString sWordForm()
        {
            return m_sWordForm;
        }

        virtual CEString sLemma()
        {
            return m_sLemma;
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

        virtual ET_Case eCase()
        {
            return m_eCase;
        }

        virtual ET_Subparadigm eSubparadigm()
        {
            return m_eSubparadigm;
        }

        virtual ET_Number eNumber()
        {
            return m_eNumber;
        }

        virtual ET_Gender eGender()
        {
            return m_eGender;
        }

        virtual ET_Person ePerson()
        {
            return m_ePerson;
        }

        virtual ET_Animacy eAnimacy()
        {
            return m_eAnimacy;
        }

        virtual ET_Reflexive eReflexive()
        {
            return m_eReflexive;
        }
        
        virtual ET_Aspect eAspect()
        {
            return m_eAspect;
        }

        virtual ET_Status eStatus()
        {
            return m_eStatus;
        }

        virtual bool bIrregular()       // came from the DB as opposed to being generated by the app
        {
            return m_bIrregular;
        }

        virtual int iGramHash();

        virtual ET_ReturnCode eGetFirstStressPos(int& iPos, ET_StressType& eType);
        virtual ET_ReturnCode eGetNextStressPos(int& iPos, ET_StressType& eType);

        // Not exposed thru itf??? 
        bool bSaveStemToDb();
        bool bSaveToDb();
        ET_ReturnCode eSaveTestData();

        //
        // Member variables; public to facilitate access from internal consumers
        //
        CLexeme * m_pLexeme;
        unsigned int m_uiDbInsertHandle;
        CEString m_sWordForm;
        CEString m_sLemma;
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

        ET_ReturnCode eFromHash (int iHash, CWordForm& wf)
        {
            ET_ReturnCode rc = eDecodeHash (iHash);
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
