#ifndef C_VERIFIER_H_INCLUDED
#define C_VERIFIER_H_INCLUDED

#include "Enums.h"
#include "EString.h"

#include "IVerifier.h"
#include "ILexeme.h"
#include "IWordForm.h"
#include "Dictionary.h"     // we need internal methods as well

using namespace std;

namespace Hlib
{
    struct StStoredLexeme
    {
        CEString m_sLexemeHash;
        CEString m_sHeadword;

        StStoredLexeme(const CEString& sLexemeHash, const CEString& sHeadword) 
            : m_sLexemeHash(sLexemeHash), m_sHeadword(sHeadword)
        {}
    };

    class CVerifier : public IVerifier
    {
    public:
        CVerifier(CDictionary *);
        ~CVerifier();

    public:
        virtual ET_ReturnCode eVerify(const CEString& sLexemeHash);
        virtual ET_TestResult eResult();
        virtual int iCount();

        virtual ET_ReturnCode eLoadStoredLexemes();
        virtual ET_ReturnCode eDeleteStoredLexeme(const CEString& sLexeme);

        virtual ET_ReturnCode eGetFirstLexemeData(CEString& sLexemeHash, CEString& sHeadword);
        virtual ET_ReturnCode eGetNextLexemeData(CEString& sLexemeHash, CEString& sHeadword);

        virtual ET_ReturnCode eGetFirstWordForm(IWordForm *& pWordForm);
        virtual ET_ReturnCode eGetNextWordForm(IWordForm *& pWordForm);

    private:
        CVerifier();        // no use
        ET_ReturnCode eLoadStoredForms(const CEString& sLexemeHash);
        ET_ReturnCode eCheckLexeme(ILexeme&, const CEString& sLexemeHash, bool& bCheckedOut);
        bool bWordFormsMatch(IWordForm * pLhs, IWordForm * pRhs);
        ET_ReturnCode eGetStoredLexemeData(const CEString& sSelect);

    private:
        CDictionary * m_pDictionary;
        vector<StStoredLexeme> m_vecStoredLexemes;
        CSqlite * m_pDb;
        multimap<CEString, IWordForm *> m_mmapStoredForms;
        multimap<CEString, IWordForm *>::iterator m_itCurrentForm;
        ET_TestResult m_eResult;
        vector<StStoredLexeme>::iterator m_itCurrentLexeme;
    };

}           // namespace

#endif  //  C_VERIFIER_H_INCLUDED