#include "WordForm.h"
#include "Lexeme.h"
#include "FormBuilderBase.h"

using namespace Hlib;

CFormBuilder::CFormBuilder (CLexeme * pLexeme, ET_Subparadigm eSubparadigm) : 
    m_pLexeme (pLexeme), 
    m_eSubparadigm (eSubparadigm), 
    m_pEndings (NULL), 
    m_eStatus (STATUS_COMMON)
{
    if (NULL == pLexeme)
    {
        assert(0);
        CEString sMsg(L"CLexeme ptr is NULL.");
        ERROR_LOG (sMsg);
        throw CException (H_ERROR_UNEXPECTED, sMsg);
    }   
}

void CFormBuilder::ReportDbError()
{
    if (!m_pLexeme)
    {
        ERROR_LOG (L"No lexeme handle.");
    }

    CSqlite * pDb = NULL;
    CEString sMsg;
    try
    {
        pDb = m_pLexeme->pGetDb();
        CEString sError;
        pDb->GetLastError (sError);
        sMsg = L"DB error: ";
        sMsg += sError;
        sMsg += L"; code = ";
        sMsg += CEString::sToString (pDb->iGetLastError());
    }
    catch (...)
    {
        sMsg = L"Apparent DB error ";
    }

    sMsg += CEString::sToString (pDb->iGetLastError());
    ERROR_LOG (sMsg);
}

void CFormBuilder::CloneWordForm (const  CWordForm * pSource, CWordForm *& pClone)
{
    pClone = new CWordForm;
    if (NULL == pClone)
    {
        CEString sMsg (L"Unable to create instance of CWordForm.");
        assert(0);
        throw CException (H_ERROR_POINTER, sMsg);   
    }

    pClone->m_pLexeme = pSource->m_pLexeme;
    pClone->m_sWordForm = pSource->m_sWordForm;
    pClone->m_sStem = pSource->m_sStem;
    pClone->m_llLexemeId = pSource->m_llLexemeId;
    pClone->m_mapStress = pSource->m_mapStress;
    pClone->m_ePos = pSource->m_ePos;
    pClone->m_eCase = pSource->m_eCase;
    pClone->m_eNumber = pSource->m_eNumber;
    pClone->m_eSubparadigm = pSource->m_eSubparadigm;
    pClone->m_eGender = pSource->m_eGender;
//    pClone->m_eTense = pSource->m_eTense;
    pClone->m_ePerson = pSource->m_ePerson;
    pClone->m_eAnimacy = pSource->m_eAnimacy;
    pClone->m_eReflexivity = pSource->m_eReflexivity;
//    pClone->m_eVoice = pSource->m_eVoice;
//    pClone->m_eSpecialForm = pSource->m_eSpecialForm;
//    pClone->m_eAspect = pSource->m_eAspect;
    pClone->m_eStatus = pSource->m_eStatus;

}  // CloneWordForm (...)
