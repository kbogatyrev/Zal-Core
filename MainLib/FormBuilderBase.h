#pragma once
#include "Lexeme.h"
#include "WordForm.h"
#include "SqliteWrapper.h"
#include "Endings.h"

using namespace std;
using namespace std::tr1;

class CFormBuilderBase
{
public:
    CFormBuilderBase (CLexeme * pLexeme, ET_Subparadigm eSubparadigm) : 
       m_pLexeme (pLexeme), 
       m_eSubparadigm (eSubparadigm), 
       m_pEndings (NULL), 
       m_eStatus (STATUS_COMMON)
    {
        m_pDb = pLexeme->m_pDb;
    }

    virtual ~CFormBuilderBase()
    {
        //
        // Delete a member that may have been created by a subclass
        //
        if (m_pEndings)
        {
            delete m_pEndings;
            m_pEndings = NULL;
        }
    }

    void SetUsageStatus (ET_Status eStatus)
    {
        m_eStatus = eStatus;
    }

protected:
    void ReportDbError();
//    void AssignSecondaryStress (CComObject<CWordForm> * pWf);
    void AssignSecondaryStress (CWordForm * pWf);
    void AssignSecondaryStress (StIrregularForm& stIf);
    bool bMultiStress (const CEString& sLemma, vector<int>& vecStressPos);

//    HRESULT hCloneWordForm (const CComObject<CWordForm> * pSource, CComObject<CWordForm> *& pClone);
//    HRESULT hCloneWordForm (const  CWordForm * source, CComObject<CWordForm> *& pClone);

    // Non-COM version of the above:
    void CloneWordForm (const CWordForm * pSource, CWordForm *& pClone);

//    HRESULT hWordFormFromHash (int iHash, unsigned int uiAt, CWordForm *& pWf);
//    HRESULT hHandleIrregularForms (CComObject<CWordForm> * pWordForm, bool& bIsVariant);
//    HRESULT hHandleIrregularForms (CWordForm& wf, bool& bIsVariant);
    bool bNoRegularForms (int iGramHash);   // returns true if only irregular form(s) exist  
                                            // for this hash, so don't generate regular form

//    unsigned int uiFormCount (int iHash);
//    bool bHasCommonDeviation (int iCd);
//    bool bDeviationOptional (int iCd);
//    HRESULT hRetractStressToPreverb (CWordForm& wf, bool bIsOptional);

protected:
    CLexeme * m_pLexeme;
    ET_Subparadigm m_eSubparadigm;
    CSqlite * m_pDb;
    CEndings * m_pEndings;
    ET_Status m_eStatus;

private:
    CFormBuilderBase()
    {
        m_pEndings = NULL;
    }

};
