#include "StdAfx.h"
#include "WordForm.h"
#include "Lexeme.h"

using namespace Hlib;

ILexeme * CWordForm::pLexeme()
{
    return m_pLexeme;
}

ET_ReturnCode CWordForm::SaveTestData()
{
    ET_ReturnCode rc = H_NO_ERROR;

    CSqlite& db = *m_pLexeme->pGetDb();

    int iHash = iGramHash();
    try
    {
        db.PrepareForInsert (L"test_data", 3);
//        db.v_Bind (1, pco_Lexeme->i_DbKey);  // descriptor id
//        db.v_Bind (1, (__int64)pco_Lexeme->ui_Hash());    // lexeme hash
        db.Bind (1, m_pLexeme->sHash());    // lexeme hash
        db.Bind (2, iHash);
        db.Bind (3, m_sWordForm);
        db.InsertRow();
        db.Finalize();
        
        __int64 llWordFormId = db.llGetLastKey();

        map<int, ET_StressType>::iterator itStress = m_mapStress.begin();
        for (; itStress != m_mapStress.end(); ++itStress)
        {
            db.PrepareForInsert (L"test_data_stress", 3);
            db.Bind (1, llWordFormId);
            db.Bind (2, (*itStress).first);
            ET_StressType eType = (*itStress).second ? STRESS_PRIMARY : STRESS_SECONDARY;
            db.Bind (3, eType);
            db.InsertRow();
            db.Finalize();
        }
    }
    catch (CException& exc)
    {
        db.RollbackTransaction();
        CEString sMsg (exc.szGetDescription());
        CEString sError;
        try
        {
            db.GetLastError (sError);
            sMsg += L", error: ";
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }
    
        sMsg += CEString::sToString (db.iGetLastError());
        ERROR_LOG (sMsg);

        return ET_ReturnCode (H_ERROR_DB);
    }

    return H_NO_ERROR;

}   //  SaveTestData()

/////////////////////////////////////////////////////////////////

int CWordForm::iGramHash()
{
    CHasher h_ (const_cast<CWordForm&>(*this));
    return h_.iGramHash();
}

ET_ReturnCode CWordForm::eInitFromHash (int iHash)
{
    try
    {
        CHasher hasher (*this);
        ET_ReturnCode rc = hasher.eDecodeHash(iHash);
        this->m_eAnimacy = hasher.m_eAnimacy;
        this->m_eAspect = hasher.m_eAspect;
        this->m_eCase = hasher.m_eCase;
        this->m_eGender = hasher.m_eGender;
        this->m_eNumber = hasher.m_eNumber;
        this->m_ePerson = hasher.m_ePerson;
        this->m_ePos = hasher.m_ePos;
        this->m_eReflexive = hasher.m_eReflexive;
        this->m_eSubparadigm = hasher.m_eSubparadigm;
        
        return rc;
    }
    catch (CException ex)
    {
        return H_EXCEPTION;  // logging should be always done by callee
    }

}   //  eInitFromHash (...)
