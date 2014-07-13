#include "StdAfx.h"
#include "WordForm.h"
#include "Lexeme.h"

using namespace Hlib;

CWordForm::CWordForm() : m_pLexeme(0),
                         m_sWordForm(L""),
                         m_sLemma(L""),
                         m_llStemId(0),
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

CWordForm::CWordForm(int iHash) : m_pLexeme(0),
                                   m_sWordForm(L""),
                                   m_sLemma(L""),
                                   m_llStemId(0),
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

ILexeme * CWordForm::pLexeme()
{
    return m_pLexeme;
}

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

ET_ReturnCode CWordForm::eGetFirstStressPos(int& iPos, ET_StressType& eType)
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

ET_ReturnCode CWordForm::eGetNextStressPos(int& iPos, ET_StressType& eType)
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

bool CWordForm::bSaveStemsToDb()
{
    CSqlite * pDbHandle = NULL;
    try
    {
        pDbHandle = m_pLexeme->pGetDb();
        if (NULL == pDbHandle)
        {
            ERROR_LOG(L"No database access.");
            return false;
        }

        if (m_sLemma.bIsEmpty())
        {
            ERROR_LOG(L"No lemma.");
            return false;
        }

        m_llStemId = -1;

        CEString sQuery(L"SELECT id FROM stems WHERE stem_string=\"");
        sQuery += m_sLemma;
        sQuery += L'\"';
        pDbHandle->PrepareForSelect(sQuery);
        if (pDbHandle->bGetRow())
        {
            pDbHandle->GetData(0, m_llStemId);
        }
        else
        {
            bool bIgnoreOnConflict = true;
            pDbHandle->PrepareForInsert(L"stems", 1, bIgnoreOnConflict);
            pDbHandle->Bind(1, m_sLemma);
            pDbHandle->InsertRow();
            pDbHandle->Finalize();
            m_llStemId = pDbHandle->llGetLastKey();
        }
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            pDbHandle->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg + CEString::sToString(pDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return false;
    }

    return true;

}       // bSaveStemsToDb()

bool CWordForm::bSaveToDb()
{
    __int64 llStemDataId = -1;
    CSqlite * pDbHandle = NULL;
    const StLexemeProperties& stLexemeProperties = m_pLexeme->stGetProperties();

    try
    {
        pDbHandle = m_pLexeme->pGetDb();
        if (NULL == pDbHandle)
        {
            ERROR_LOG(L"No database access.");
            return false;
        }

        if (m_sLemma.bIsEmpty())
        {
            ERROR_LOG(L"No lemma.");
            return false;
        }

        pDbHandle->PrepareForInsert(L"stem_data", 3);
        pDbHandle->Bind(1, m_llStemId);
        pDbHandle->Bind(2, iGramHash());
        pDbHandle->Bind(3, stLexemeProperties.iDbKey);
        pDbHandle->InsertRow();
        pDbHandle->Finalize();
        llStemDataId = pDbHandle->llGetLastKey();

        pDbHandle->PrepareForInsert(L"wordforms", 2);
        pDbHandle->Bind(1, llStemDataId);
        pDbHandle->Bind(2, m_llEndingDataId);
        pDbHandle->InsertRow();
        pDbHandle->Finalize();
        __int64 llWordFormDbKey = pDbHandle->llGetLastKey();
        
        map<int, ET_StressType>::iterator itStress = m_mapStress.begin();
        for (; itStress != m_mapStress.end(); ++itStress)
        {
            pDbHandle->PrepareForInsert(L"stress_data", 3);
            pDbHandle->Bind(1, llWordFormDbKey);
            pDbHandle->Bind(2, (*itStress).first);
            ET_StressType eType = (*itStress).second ? STRESS_PRIMARY : STRESS_SECONDARY;
            pDbHandle->Bind(3, eType);
            pDbHandle->InsertRow();
            pDbHandle->Finalize();
        }
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            pDbHandle->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg + CEString::sToString(pDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return false;
    }

    return true;

}   //  bool bSaveToDb (...)

ET_ReturnCode CWordForm::SaveTestData()
{
    ET_ReturnCode rc = H_NO_ERROR;

    CSqlite& db = *m_pLexeme->pGetDb();

    int iHash = iGramHash();
    try
    {
        db.PrepareForInsert(L"test_data", 3);
        db.Bind(1, m_pLexeme->sHash());    // lexeme hash
        db.Bind(2, iHash);
        db.Bind(3, m_sWordForm);
        db.InsertRow();
        db.Finalize();

        __int64 llWordFormId = db.llGetLastKey();

    }
    catch (CException& exc)
    {
        db.RollbackTransaction();
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            db.GetLastError(sError);
            sMsg += L", error: ";
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(db.iGetLastError());
        ERROR_LOG(sMsg);

        return ET_ReturnCode(H_ERROR_DB);
    }

    return H_NO_ERROR;

}   //  SaveTestData()
