#ifndef C_VERIFIER_H_INCLUDED
#define C_VERIFIER_H_INCLUDED

#include "Enums.h"
#include "EString.h"

#include "Verifier.h"
#include "Lexeme.h"
#include "WordForm.h"
#include "Dictionary.h"

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

    class CVerifier
    {
    public:
        CVerifier(shared_ptr<CDictionary>);
        ~CVerifier();

    public:
        ET_ReturnCode eVerify(const CEString& sLexemeHash);
        ET_TestResult eResult();
        int iCount();

        ET_ReturnCode eLoadStoredLexemes();
        ET_ReturnCode eDeleteStoredLexeme(const CEString& sLexeme);

        ET_ReturnCode eGetFirstLexemeData(CEString& sLexemeHash, CEString& sHeadword);
        ET_ReturnCode eGetNextLexemeData(CEString& sLexemeHash, CEString& sHeadword);

        ET_ReturnCode eGetFirstWordForm(shared_ptr<CWordForm>&);
        ET_ReturnCode eGetNextWordForm(shared_ptr<CWordForm>&);

    private:
        CVerifier();        // no use
        ET_ReturnCode eLoadStoredForms(const CEString& sLexemeHash);
        ET_ReturnCode eCheckParadigm(shared_ptr<CInflection>&, const CEString& sLexemeHash, bool& bCheckedOut);
        bool bWordFormsMatch(shared_ptr<CWordForm> spLhs, shared_ptr<CWordForm> spRhs);
        ET_ReturnCode eGetStoredLexemeData(const CEString& sSelect);

    private:
        shared_ptr<CDictionary> m_spDictionary;
        vector<StStoredLexeme> m_vecStoredLexemes;
        shared_ptr<CSqlite> m_spDb;
        multimap<CEString, shared_ptr<CWordForm>> m_mmapStoredForms;
        multimap<CEString, shared_ptr<CWordForm>>::iterator m_itCurrentForm;
        ET_TestResult m_eResult;
        vector<StStoredLexeme>::iterator m_itCurrentLexeme;
    };

}           // namespace

#endif  //  C_VERIFIER_H_INCLUDED