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
        CWordForm() :
        m_pLexeme(0),
        m_sWordForm(L""),
        m_sLemma(L""),
        m_sEnding(L""),
        m_llEndingDataId(-1),
        m_llLexemeId(-1),
        m_ePos(POS_UNDEFINED),
        m_eCase(CASE_UNDEFINED),
        m_eNumber(NUM_UNDEFINED),
        m_eSubparadigm(SUBPARADIGM_UNDEFINED),
        m_eGender(GENDER_UNDEFINED),
        m_ePerson(PERSON_UNDEFINED),
        m_eAnimacy(ANIM_UNDEFINED),
        m_eReflexive(REFL_UNDEFINED),
        m_eAspect(ASPECT_UNDEFINED),
        m_eStatus(STATUS_COMMON),
        m_bIrregular(false)
        {}

        CWordForm(int iHash) :
            m_pLexeme(0),
            m_sWordForm(L""),
            m_sLemma(L""),
            m_sEnding(L""),
            m_llEndingDataId(-1),
            m_llLexemeId(-1),
            m_ePos(POS_UNDEFINED),
            m_eCase(CASE_UNDEFINED),
            m_eNumber(NUM_UNDEFINED),
            m_eSubparadigm(SUBPARADIGM_UNDEFINED),
            m_eGender(GENDER_UNDEFINED),
            m_ePerson(PERSON_UNDEFINED),
            m_eAnimacy(ANIM_UNDEFINED),
            m_eReflexive(REFL_UNDEFINED),
            m_eAspect(ASPECT_UNDEFINED),
            m_eStatus(STATUS_COMMON),
            m_bIrregular(false)
        {
            ET_ReturnCode rc = eInitFromHash(iHash);
            if (rc != H_NO_ERROR)
            {
                ASSERT(0);
                CEString sMsg(L"eInitFromHash() failed, hash value =");
                sMsg += CEString::sToString(iHash) + L".";
                ERROR_LOG(sMsg);
                throw CException(H_ERROR_INVALID_ARG, sMsg);
            }
        }

        virtual ET_ReturnCode eInitFromHash(int iHash);

        CLexeme * m_pLexeme;
        virtual ILexeme * pLexeme();

        CEString m_sWordForm;
        virtual CEString sWordForm()
        {
            return m_sWordForm;
        }

        CEString m_sLemma;
        virtual CEString sLemma()
        {
            return m_sLemma;
        }

        CEString m_sEnding;
        virtual CEString sEnding()
        {
            return m_sEnding;
        }

        __int64 m_llEndingDataId;       // key into ending_data table
        virtual __int64 llEndingDataId()
        {
            return m_llEndingDataId;
        }

        __int64 m_llLexemeId;
        virtual __int64 llLexemeId()
        {
            return m_llLexemeId;
        }

        map<int, ET_StressType> m_mapStress; // <stressed syll, primary/secondary>
        ET_ReturnCode eGetFirstStressPos(int& iPos, ET_StressType& eType)
        {
            m_itStressPos = m_mapStress.begin();
            if (m_mapStress.end() == m_itStressPos)
            {
                iPos = -1;
                eType = STRESS_TYPE_UNDEFINED;
                return H_FALSE;
            }
            iPos = m_itStressPos->first;
            eType = m_itStressPos->second;

            return H_NO_ERROR;
        }

        ET_ReturnCode eGetNextStressPos(int& iPos, ET_StressType& eType)
        {
            if (m_itStressPos != m_mapStress.end())
            {
                ++m_itStressPos;
            }
            if (m_mapStress.end() == m_itStressPos)
            {
                return H_NO_MORE;
            }
            iPos = m_itStressPos->first;
            eType = m_itStressPos->second;

            return H_NO_ERROR;
        }

        ET_PartOfSpeech m_ePos;
        virtual ET_PartOfSpeech ePos()
        {
            return m_ePos;
        }

        ET_Case m_eCase;
        virtual ET_Case eCase()
        {
            return m_eCase;
        }

        ET_Number m_eNumber;
        virtual ET_Number eNumber()
        {
            return m_eNumber;
        }

        ET_Subparadigm m_eSubparadigm;
        virtual ET_Subparadigm eSubparadigm()
        {
            return m_eSubparadigm;
        }

        ET_Gender m_eGender;
        virtual ET_Gender eGender()
        {
            return m_eGender;
        }

        ET_Person m_ePerson;
        virtual ET_Person ePerson()
        {
            return m_ePerson;
        }

        ET_Animacy m_eAnimacy;
        virtual ET_Animacy eAnimacy()
        {
            return m_eAnimacy;
        }

        ET_Reflexive m_eReflexive;
        virtual ET_Reflexive eReflexive()
        {
            return m_eReflexive;
        }

        ET_Aspect m_eAspect;
        virtual ET_Aspect eAspect()
        {
            return m_eAspect;
        }

        ET_Status m_eStatus;
        virtual ET_Status eStatus()
        {
            return m_eStatus;
        }

        bool m_bIrregular;              // came from the DB as opposed to being generated by the app
        virtual bool bIrregular()       // came from the DB as opposed to being generated by the app
        {
            return m_bIrregular;
        }

        virtual int iGramHash();

        // Not exposed thru itf??? 
        bool bSaveToDb();

    private:
        map<int, ET_StressType>::iterator m_itStressPos;
        ET_ReturnCode SaveTestData();

    };      //  struct CWordForm : public IWordForm

    class CHasher : public CGramHasher
    {
    //    friend struct CWordForm;
    //    friend class CAnalyzer;

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
        }

    private:
        __int64 m_llLexemeId;

    };      //  class CHasher : public CGramHasher

}       //  namespace Hlib

#endif      // C_WORDFORM_H_INCLUDED
