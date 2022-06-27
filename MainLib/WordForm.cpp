#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "WordForm.h"
#include "Lexeme.h"

using namespace Hlib;

CWordForm::CWordForm() : m_pLexeme(0),
                         m_ullDbInsertHandle(0),
                         m_llDbKey(-1),
                         m_sWordForm(L""),
                         m_sStem(L""),
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
                         m_eReflexivity(REFL_UNDEFINED),
                         m_eAspect(ASPECT_UNDEFINED),
                         m_eStatus(STATUS_COMMON),
                         m_bIrregular(false),
                         m_bIsDifficult(false),
                         m_eStressType(WORD_STRESS_TYPE_UNDEFINED),
                         m_bIsEdited(false)
{
    m_sWordForm.SetVowels(CEString::g_szRusVowels);
    m_sStem.SetVowels(CEString::g_szRusVowels);
    m_sEnding.SetVowels(CEString::g_szRusVowels);
}

CWordForm::CWordForm(const CEString& sHash) : m_pLexeme(0),
                                              m_ullDbInsertHandle(0),
                                              m_llDbKey(-1),
                                              m_sWordForm(L""),
                                              m_sStem(L""),
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
                                              m_eReflexivity(REFL_UNDEFINED),
                                              m_eAspect(ASPECT_UNDEFINED),
                                              m_eStatus(STATUS_COMMON),
                                              m_bIrregular(false),
                                              m_bIsDifficult(false),
                                              m_eStressType(WORD_STRESS_TYPE_UNDEFINED),
                                              m_bIsEdited(false)
{
    ET_ReturnCode rc = eInitFromHash(sHash);
    if (rc != H_NO_ERROR)
    {
//        assert(0);
        CEString sMsg(L"eInitFromHash() failed, hash value =");
        sMsg += sHash + L".";
        ERROR_LOG(sMsg);
        throw CException(H_ERROR_INVALID_ARG, sMsg);
    }

    m_sWordForm.SetVowels(CEString::g_szRusVowels);
    m_sStem.SetVowels(CEString::g_szRusVowels);
    m_sEnding.SetVowels(CEString::g_szRusVowels);
}

CWordForm::CWordForm(const CWordForm * pSource)
{
    Copy(*pSource);
}

CWordForm::CWordForm(const CWordForm& source)
{
    Copy(source);
}

const CWordForm& CWordForm::operator=(const CWordForm& rhs)
{
    Copy(rhs);
    return *this;
}

ILexeme * CWordForm::pLexeme()
{
    return m_pLexeme;
}

void CWordForm::SetLexeme(ILexeme * pLexeme)
{
    m_pLexeme = dynamic_cast<CLexeme *>(pLexeme);
}

CEString CWordForm::sGramHash()
{
    CHasher h_ (const_cast<CWordForm&>(*this));
    return h_.sGramHash();
}

ET_ReturnCode CWordForm::eInitFromHash (const CEString& sHash)
{
    try
    {
        CHasher hasher (*this);
        ET_ReturnCode rc = hasher.eDecodeHash(sHash);
        this->m_eAnimacy = hasher.m_eAnimacy;
        this->m_eAspect = hasher.m_eAspect;
        this->m_eCase = hasher.m_eCase;
        this->m_eGender = hasher.m_eGender;
        this->m_eNumber = hasher.m_eNumber;
        this->m_ePerson = hasher.m_ePerson;
        this->m_ePos = hasher.m_ePos;
        this->m_eReflexivity = hasher.m_eReflexivity;
        this->m_eSubparadigm = hasher.m_eSubparadigm;
        
        return rc;
    }
    catch (CException& ex)
    {
        return H_EXCEPTION;  // logging should be always done by callee
    }

}   //  eInitFromHash (...)

ET_ReturnCode CWordForm::eClone(IWordForm *& pClonedObject)
{
    pClonedObject = new CWordForm(const_cast<const CWordForm&>(*this));

    return H_NO_ERROR;
}

ET_ReturnCode CWordForm::eGetFirstStressPos(int& iPos, ET_StressType& eType)
{
    m_sWordForm.SetVowels(CEString::g_szRusVowels);

    m_itStressPos = m_mapStress.begin();
    if (m_mapStress.end() == m_itStressPos)
    {
        iPos = -1;
        eType = STRESS_TYPE_UNDEFINED;
        return H_FALSE;
    }

    try
    {
        iPos = m_sWordForm.uiGetVowelPos(m_itStressPos->first);
        eType = m_itStressPos->second;
    }
    catch (CException& ex)
    {
        return H_EXCEPTION;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CWordForm::eGetNextStressPos(int& iPos, ET_StressType& eType)
{
    m_sWordForm.SetVowels(CEString::g_szRusVowels);

    if (m_itStressPos != m_mapStress.end())
    {
        ++m_itStressPos;
    }
    if (m_mapStress.end() == m_itStressPos)
    {
        return H_NO_MORE;
    }

    try
    {
        iPos = m_sWordForm.uiGetVowelPos(m_itStressPos->first);
        eType = m_itStressPos->second;
    }
    catch (CException& ex)
    {
        return H_EXCEPTION;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CWordForm::eSetStressPositions(map<int, ET_StressType> mapStress)
{
    m_mapStress = mapStress;

    return H_NO_ERROR;
}

//
// Used in manual editing
//
ET_ReturnCode CWordForm::eSaveIrregularForm()
{
    CSqlite* pDbHandle = m_pLexeme->pGetDb();
    if (NULL == pDbHandle)
    {
        ERROR_LOG(L"No database access.");
        return H_ERROR_DB;
    }

    const StLexemeProperties& stProps = m_pLexeme->stGetProperties();

    if (m_llDbKey >= 0)
    {
        try
        {
            CEString sDelQuery(L"DELETE FROM irregular_forms WHERE id = ");
            sDelQuery += CEString::sToString(m_llDbKey);
            pDbHandle->Delete(sDelQuery);

// TODO: verify that the loop was not needed
//            for (auto pairStress : m_mapStress)
//            {
                sDelQuery = L"DELETE FROM irregular_stress WHERE form_id = ";
                sDelQuery += CEString::sToString(m_llDbKey);
                pDbHandle->Delete(sDelQuery);
//            }
        }
        catch (CException & exc)
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

            sMsg += CEString::sToString(pDbHandle->iGetLastError());
            ERROR_LOG(sMsg);

            return H_ERROR_DB;
        }
    }

    if (!m_pLexeme->bHasIrregularForms())
    {
        m_pLexeme->SetHasIrregularForms(true);
        m_pLexeme->eUpdateDescriptorInfo(m_pLexeme);
    }

    try
    {
        pDbHandle->PrepareForInsert(L"irregular_forms", 7);
        pDbHandle->Bind(1, (int64_t)stProps.llDescriptorId);
        pDbHandle->Bind(2, sGramHash());
        pDbHandle->Bind(3, m_sWordForm);
        pDbHandle->Bind(4, false);
        pDbHandle->Bind(5, sLeadComment());            // TODO
        pDbHandle->Bind(6, sTrailingComment());        // TODO
        pDbHandle->Bind(7, true);                      // is_edited

        pDbHandle->InsertRow();
        pDbHandle->Finalize();

        m_llDbKey = pDbHandle->llGetLastKey();
        m_bIsEdited = true;
    }
    catch (CException & exc)
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

        sMsg += CEString::sToString(pDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return H_ERROR_DB;
    }

    try
    {
        for (auto pair : m_mapStress)
        {
            pDbHandle->PrepareForInsert(L"irregular_stress", 4);
            pDbHandle->Bind(1, (int64_t)m_llDbKey);
            int iCharPos = m_sWordForm.uiGetVowelPos(pair.first);
            pDbHandle->Bind(2, iCharPos);
            bool bIsPrimary = (ET_StressType::STRESS_PRIMARY == pair.second) ? true : false;
            pDbHandle->Bind(3, bIsPrimary);
            pDbHandle->Bind(4, true);       // is_edited

            pDbHandle->InsertRow();
            pDbHandle->Finalize();
        }

//        long long llFormKey = pDbHandle->llGetLastKey();
    }
    catch (CException & exc)
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

        sMsg += CEString::sToString(pDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return H_ERROR_DB;
    }

    return H_NO_ERROR;

}    //  eSaveIrregularForm()

bool CWordForm::bSaveStemToDb()
{
    CSqlite * pDbHandle = NULL;
    try
    {
        if (NULL == m_pLexeme)
        {
            ERROR_LOG(L"No lexeme.");
            return false;
        }

        pDbHandle = m_pLexeme->pGetDb();
        if (NULL == pDbHandle)
        {
            ERROR_LOG(L"No database access.");
            return false;
        }

        if (m_sStem.bIsEmpty())
        {
            ERROR_LOG(L"No stem.");
            return false;
        }

        bool bIgnoreOnConflict = true;
        pDbHandle->PrepareForInsert(L"stems", 1, bIgnoreOnConflict);
        pDbHandle->Bind(1, m_sStem);
        pDbHandle->InsertRow();
        pDbHandle->Finalize();
        if (m_llStemId < 1)
        {
            m_llStemId = -1;
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

        sMsg += CEString::sToString(pDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return false;
    }

    return true;

}       // bSaveStemsToDb()

bool CWordForm::bSaveToDb()
{
    long long llStemDataId = -1;
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

        if (m_sStem.bIsEmpty())
        {
            ERROR_LOG(L"No stem.");
            return false;
        }

        if (0 == m_ullDbInsertHandle)
        {
            sqlite3_stmt* pStmt = nullptr;
            pDbHandle->uiPrepareForInsert(L"stem_data", 3, pStmt, false);
            m_ullDbInsertHandle = (unsigned long long)pStmt;
        }
        pDbHandle->Bind(1, (int64_t)m_llStemId, m_ullDbInsertHandle);
        pDbHandle->Bind(2, sGramHash(), m_ullDbInsertHandle);
        pDbHandle->Bind(3, (int64_t)stLexemeProperties.llDescriptorId, m_ullDbInsertHandle);
        pDbHandle->InsertRow(m_ullDbInsertHandle);
        pDbHandle->Finalize(m_ullDbInsertHandle);
        llStemDataId = pDbHandle->llGetLastKey();

        pDbHandle->PrepareForInsert(L"wordforms", 2);
        pDbHandle->Bind(1, (int64_t)llStemDataId);
        pDbHandle->Bind(2, (int64_t)m_llEndingDataId);
        pDbHandle->InsertRow();
        pDbHandle->Finalize();
        long long llWordFormDbKey = pDbHandle->llGetLastKey();
        
        map<int, ET_StressType>::iterator itStress = m_mapStress.begin();
        for (; itStress != m_mapStress.end(); ++itStress)
        {
            pDbHandle->PrepareForInsert(L"stress_data", 4);
            pDbHandle->Bind(1, (int64_t)llWordFormDbKey);
            pDbHandle->Bind(2, (*itStress).first);
            bool bIsPrimary = ((*itStress).second == STRESS_PRIMARY) ? true : false;
            pDbHandle->Bind(3, bIsPrimary);
            pDbHandle->Bind(4, m_bIsVariant);
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

        sMsg += CEString::sToString(pDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return false;
    }

    return true;

}   //  bool bSaveToDb (...)

bool CWordForm::bSaveIrregularForm() // currently intended for spryazh. sm verbs only
{
//    long long llStemDataId = -1;
    CSqlite* pDbHandle = NULL;
    const StLexemeProperties& stLexemeProperties = m_pLexeme->stGetProperties();

    try
    {
        pDbHandle = m_pLexeme->pGetDb();
        if (NULL == pDbHandle)
        {
            ERROR_LOG(L"No database access.");
            return false;
        }

        sqlite3_stmt* pStmt = nullptr;
        pDbHandle->uiPrepareForInsert(L"irregular_forms_spryazh_sm", 7, pStmt, false);
        auto ullInsertHandle = (unsigned long long)pStmt;

        pDbHandle->Bind(1, (int64_t)stLexemeProperties.llDescriptorId, ullInsertHandle);
        pDbHandle->Bind(2, sGramHash(), ullInsertHandle);
        pDbHandle->Bind(3, m_sWordForm, ullInsertHandle);
        pDbHandle->Bind(4, false, ullInsertHandle);      // is_alternative
        pDbHandle->Bind(5, L"", ullInsertHandle);        // lead_comment
        pDbHandle->Bind(6, L"", ullInsertHandle);        // trailing_comment
        pDbHandle->Bind(7, false, ullInsertHandle);      // is_edited
        pDbHandle->InsertRow(ullInsertHandle);
        pDbHandle->Finalize(ullInsertHandle);
        long long llFormId = pDbHandle->llGetLastKey();

        for (auto&& pairStressPos : m_mapStress)
        {
            pDbHandle->uiPrepareForInsert(L"irregular_stress_spryazh_sm", 4, pStmt, false);
            auto ullStressInsertHandle = (unsigned long long)pStmt;

            pDbHandle->Bind(1, (int64_t)llFormId, ullStressInsertHandle);
            int iPos = m_sWordForm.uiGetVowelPos(pairStressPos.first);
            pDbHandle->Bind(2, iPos, ullStressInsertHandle);
            pDbHandle->Bind(3, pairStressPos.second, ullStressInsertHandle);
            pDbHandle->Bind(4, false, ullStressInsertHandle);      // is_edited
            pDbHandle->InsertRow(ullStressInsertHandle);
            pDbHandle->Finalize(ullStressInsertHandle);
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

        sMsg += CEString::sToString(pDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return false;
    }

    return true;

}   //  bool bSaveIrregularForm()

ET_ReturnCode CWordForm::eSaveTestData()
{
    if (NULL == m_pLexeme)
    {
        return H_ERROR_POINTER;
    }

    CSqlite& db = *m_pLexeme->pGetDb();
    CEString sHash = sGramHash();

    try
    {
        db.PrepareForInsert(L"test_data", 3);
        db.Bind(1, m_pLexeme->sHash());         // lexeme hash
        db.Bind(2, sHash);
        db.Bind(3, m_sWordForm);
        db.InsertRow();
        db.Finalize();

        auto llWordFormId = db.llGetLastKey();

        map<int, ET_StressType>::iterator itStress = m_mapStress.begin();
        for (; itStress != m_mapStress.end(); ++itStress)
        {
            db.PrepareForInsert(L"test_data_stress", 3);
            db.Bind(1, llWordFormId);
            db.Bind(2, (*itStress).first);
            ET_StressType eType = (*itStress).second ? STRESS_PRIMARY : STRESS_SECONDARY;
            db.Bind(3, eType);
            db.InsertRow();
            db.Finalize();
        }
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

}   //  eSaveTestData()

void CWordForm::Copy(const CWordForm& source)
{
    m_pLexeme = source.m_pLexeme;
    m_ullDbInsertHandle = source.m_ullDbInsertHandle;
    m_llDbKey = source.m_llDbKey;
    m_sWordForm = source.m_sWordForm;
    m_sStem = source.m_sStem;
    m_llStemId = source.m_llStemId;
    m_sEnding = source.m_sEnding;
    m_llEndingDataId = source.m_llEndingDataId;
    m_llLexemeId = source.m_llLexemeId;
    m_mapStress = source.m_mapStress;
    m_ePos = source.m_ePos;
    m_eCase = source.m_eCase;
    m_eNumber = source.m_eNumber;
    m_eSubparadigm = source.m_eSubparadigm;
    m_eGender = source.m_eGender;
    m_ePerson = source.m_ePerson;
    m_eAnimacy = source.m_eAnimacy;
    m_eReflexivity = source.m_eReflexivity;
    m_eAspect = source.m_eAspect;
    m_eStatus = source.m_eStatus;
    m_bIrregular = source.m_bIrregular;
    m_bIsEdited = source.m_bIsEdited;
    m_bIsDifficult = source.m_bIsDifficult;
    m_eStressType = source.m_eStressType;
    m_itStressPos = source.m_itStressPos;
    m_sLeadComment = source.m_sLeadComment;
    m_sTrailingComment = source.m_sTrailingComment;
}
