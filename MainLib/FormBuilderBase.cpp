#include "WordForm.h"
#include "Lexeme.h"
#include "FormBuilderBase.h"

using namespace Hlib;

CFormBuilder::CFormBuilder (shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection, ET_Subparadigm eSubparadigm) : 
    m_spLexeme (spLexeme), 
    m_spInflection (spInflection),
    m_eSubparadigm (eSubparadigm), 
    m_spEndings (NULL), 
    m_eStatus (STATUS_COMMON)
{
    if (NULL == spLexeme)
    {
        assert(0);
        CEString sMsg(L"CLexeme ptr is NULL.");
        ERROR_LOG (sMsg);
        throw CException (H_ERROR_UNEXPECTED, sMsg);
    }   
}

void CFormBuilder::ReportDbError()
{
    if (!m_spLexeme)
    {
        ERROR_LOG (L"No lexeme handle.");
    }

    if (!m_spInflection)
    {
        ERROR_LOG(L"No lexeme handle.");
    }

    shared_ptr<CSqlite> spDb;
    CEString sMsg;
    try
    {
        spDb = m_spLexeme->spGetDb();
        CEString sError;
        spDb->GetLastError (sError);
        sMsg = L"DB error: ";
        sMsg += sError;
        sMsg += L"; code = ";
        sMsg += CEString::sToString (spDb->iGetLastError());
    }
    catch (...)
    {
        sMsg = L"Apparent DB error ";
    }

    sMsg += CEString::sToString (spDb->iGetLastError());
    ERROR_LOG (sMsg);
}

void CFormBuilder::CloneWordForm(const  shared_ptr<CWordForm> spSource, shared_ptr<CWordForm>& spClone)
{
    spClone = make_shared<CWordForm>();
    if (nullptr == spClone)
    {
        CEString sMsg (L"Unable to create instance of CWordForm.");
        assert(0);
        throw CException (H_ERROR_POINTER, sMsg);   
    }

    spClone->m_spLexeme = spSource->m_spLexeme;
    spClone->m_sWordForm = spSource->m_sWordForm;
    spClone->m_sStem = spSource->m_sStem;
    spClone->m_llLexemeId = spSource->m_llLexemeId;
    spClone->m_mapStress = spSource->m_mapStress;
    spClone->m_ePos = spSource->m_ePos;
    spClone->m_eCase = spSource->m_eCase;
    spClone->m_eNumber = spSource->m_eNumber;
    spClone->m_eSubparadigm = spSource->m_eSubparadigm;
    spClone->m_eGender = spSource->m_eGender;
//    pClone->m_eTense = pSource->m_eTense;
    spClone->m_ePerson = spSource->m_ePerson;
    spClone->m_eAnimacy = spSource->m_eAnimacy;
    spClone->m_eReflexivity = spSource->m_eReflexivity;
//    pClone->m_eVoice = pSource->m_eVoice;
//    pClone->m_eSpecialForm = pSource->m_eSpecialForm;
//    pClone->m_eAspect = pSource->m_eAspect;
    spClone->m_eStatus = spSource->m_eStatus;

}  // CloneWordForm (...)
