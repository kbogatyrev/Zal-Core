#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "WordForm.h"
//#include "Lexeme.h"
#include "Inflection.h"

using namespace Hlib;

CWordForm::CWordForm() : 
    m_ullDbInsertHandle(0),
    m_llDbKey(0),
    m_sWordForm(L""),
    m_sStem(L""),
    m_llStemId(0),
    m_llStemDataId(0),
    m_llIrregularFormId(0),
    m_sEnding(L""),
    m_llEndingDataId(0),
    m_llLexemeId(0),
    m_llInflectionId(0),
    //                         m_llLexemeId(-1),
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

CWordForm::CWordForm(CInflection* pInflection) :  
    m_spInflection(pInflection),
    m_ullDbInsertHandle(0),
    m_llDbKey(-1),
    m_sWordForm(L""),
    m_sStem(L""),
    m_llStemId(0),
    m_sEnding(L""),
    m_llEndingDataId(-1),
    //                                              m_llLexemeId(-1),
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

CWordForm::CWordForm(const CEString& sHash, CInflection* spInflection) : 
    m_spInflection(spInflection),
    m_ullDbInsertHandle(0),
    m_llDbKey(-1),
    m_sWordForm(L""),
    m_sStem(L""),
    m_llStemId(0),
    m_sEnding(L""),
    m_llEndingDataId(-1),
//  m_llLexemeId(-1),
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

CWordForm::CWordForm(const CWordForm& source)
{
    Copy(source);
}

CWordForm::~CWordForm()
{}

const CWordForm& CWordForm::operator=(const CWordForm& rhs)
{
    Copy(rhs);
    return *this;
}

void CWordForm::SetInflection(CInflection* pInflection)
{
    m_spInflection = pInflection;
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
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;  // logging should be always done by callee
    }

}   //  eInitFromHash (...)

//ET_ReturnCode CWordForm::eClone(shared_ptr<CWordForm>& spClonedObject)
//{
//    spClonedObject = make_shared<CWordForm>(const_cast<const CWordForm&>(*this));
//
//    return H_NO_ERROR;
//}

// Partial copy
// TODO: can it be replaced with a copy ctor?
ET_ReturnCode CWordForm::eCloneFrom(const CWordForm* spSource)
{
    if (nullptr == spSource)
    {
        CEString sMsg(L"Copy instance is NULL.");
        assert(0);
        return H_ERROR_POINTER;
    }

    m_spInflection = spSource->m_spInflection;
    m_sWordForm = spSource->m_sWordForm;
    m_sStem = spSource->m_sStem;
    m_llStemId = spSource->m_llStemId;
    m_llStemDataId = spSource->m_llStemDataId;
    m_llIrregularFormId = spSource->m_llIrregularFormId;
    m_sEnding = spSource->m_sEnding;
    m_llEndingDataId = spSource->m_llEndingDataId;
    m_llLexemeId = spSource->m_llLexemeId;
    m_llInflectionId = spSource->m_llInflectionId;
    m_mapStress = spSource->m_mapStress;
    m_ePos = spSource->m_ePos;
    m_eCase = spSource->m_eCase;
    m_eNumber = spSource->m_eNumber;
    m_eSubparadigm = spSource->m_eSubparadigm;
    m_eGender = spSource->m_eGender;
    m_ePerson = spSource->m_ePerson;
    m_eAnimacy = spSource->m_eAnimacy;
    m_eReflexivity = spSource->m_eReflexivity;
    m_eAspect = spSource->m_eAspect;
    m_eStatus = spSource->m_eStatus;
    m_bIrregular = spSource->m_bIrregular;
    m_bIsDifficult = spSource->m_bIsDifficult;
    m_eStressType = spSource->m_eStressType;

    // Irregular forms only:
    m_sLeadComment = spSource->m_sLeadComment;
    m_sTrailingComment = spSource->m_sLeadComment;
//    m_bIsVariant = spSource->m_bIsVariant;        -- probably should be assigned each time
    m_bIsEdited = spSource->m_bIsEdited;
 
    return H_NO_ERROR;
}

void CWordForm::ClearStress()
{
    m_mapStress.clear();
}

void CWordForm::AssignStress(const map<int, ET_StressType>& mapStress)
{
    m_mapStress = mapStress;
}

int CWordForm::iStressPositions()
{
    return (int)m_mapStress.size();
}

ET_ReturnCode CWordForm::eGetFirstStressPos(int& iPos, ET_StressType& eType)
{
    m_sWordForm.SetVowels(CEString::g_szRusVowels);

    m_itStressSyll = m_mapStress.begin();
    if (m_mapStress.end() == m_itStressSyll)
    {
        iPos = -1;
        eType = STRESS_TYPE_UNDEFINED;
        return H_FALSE;
    }

    try
    {
        iPos = m_sWordForm.uiGetVowelPos(m_itStressSyll->first);
        eType = m_itStressSyll->second;
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
        return H_EXCEPTION;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CWordForm::eGetNextStressPos(int& iPos, ET_StressType& eType)
{
    m_sWordForm.SetVowels(CEString::g_szRusVowels);

    if (m_itStressSyll != m_mapStress.end())
    {
        ++m_itStressSyll;
    }
    if (m_mapStress.end() == m_itStressSyll)
    {
        return H_NO_MORE;
    }

    try
    {
        iPos = m_sWordForm.uiGetVowelPos(m_itStressSyll->first);
        eType = m_itStressSyll->second;
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CWordForm::eFindStressPos(int iPos, ET_StressType& eType)
{
    auto itStress = m_mapStress.find(iPos);
    if (itStress != m_mapStress.end())
    {
        eType = itStress->second;
        return ET_ReturnCode::H_NO_ERROR;
    }
    eType = ET_StressType::STRESS_TYPE_UNDEFINED;
    return ET_ReturnCode::H_FALSE;
}

ET_ReturnCode CWordForm::eRemoveStressPos(int iPos)
{
    auto num_removed = m_mapStress.erase(iPos);
    if (num_removed != 1)
    {
        return H_ERROR_UNEXPECTED;
    }
    return H_NO_ERROR;
}

const map<int, ET_StressType>& CWordForm::mapGetStressPositions()
{
    return m_mapStress;
}

ET_ReturnCode CWordForm::eSetStressPositions(map<int, ET_StressType> mapStress)
{
    m_mapStress = mapStress;
    return H_NO_ERROR;
}

void CWordForm::SetStressPos(int iPos, ET_StressType eType)
{
    m_mapStress[iPos] = eType;
}

ET_ReturnCode CWordForm::eGetFirstStressSyll(int& iSyll, ET_StressType& eType)
{
    m_itStressSyll = m_mapStress.begin();
    if (m_mapStress.end() == m_itStressSyll)
    {
        iSyll = -1;
        eType = STRESS_TYPE_UNDEFINED;
        return H_FALSE;
    }

    iSyll = m_itStressSyll->first;
    eType = m_itStressSyll->second;

    return H_NO_ERROR;
}

ET_ReturnCode CWordForm::eGetNextStressSyll(int& iSyll, ET_StressType& eType)
{
    m_sWordForm.SetVowels(CEString::g_szRusVowels);

    if (m_itStressSyll != m_mapStress.end())
    {
        ++m_itStressSyll;
    }
    if (m_mapStress.end() == m_itStressSyll)
    {
        return H_NO_MORE;
    }

    iSyll = m_itStressSyll->first;
    eType = m_itStressSyll->second;

    return H_NO_ERROR;
}

//
// Used in manual editing
//
ET_ReturnCode CWordForm::eSaveIrregularForm()
{
    shared_ptr<CSqlite> spDbHandle = pLexeme()->spGetDb();
    if (nullptr == spDbHandle)
    {
        ERROR_LOG(L"No database access.");
        return H_ERROR_DB;
    }

    const StLexemeProperties& stProps = pLexeme()->stGetProperties();

    if (m_llDbKey >= 0)
    {
        try
        {
            CEString sDelQuery(L"DELETE FROM irregular_forms WHERE id = ");
            sDelQuery += CEString::sToString(m_llDbKey);
            spDbHandle->Delete(sDelQuery);

// TODO: verify that the loop was not needed
//            for (auto pairStress : m_mapStress)
//            {
                sDelQuery = L"DELETE FROM irregular_stress WHERE form_id = ";
                sDelQuery += CEString::sToString(m_llDbKey);
                spDbHandle->Delete(sDelQuery);
//            }
        }
        catch (CException& exc)
        {
            CEString sMsg(exc.szGetDescription());
            CEString sError;
            try
            {
                spDbHandle->GetLastError(sError);
                sMsg += CEString(L", error %d: ");
                sMsg += sError;
            }
            catch (...)
            {
                sMsg = L"Apparent DB error ";
            }

            sMsg += CEString::sToString(spDbHandle->iGetLastError());
            ERROR_LOG(sMsg);

            return H_ERROR_DB;
        }
    }

    if (!pLexeme()->bHasIrregularForms())
    {
        pLexeme()->SetHasIrregularForms(true);
        pLexeme()->eUpdateDescriptorInfo(pLexeme());
    }

    try
    {
        spDbHandle->PrepareForInsert(L"irregular_forms", 7);
        spDbHandle->Bind(1, (int64_t)m_llInflectionId);
        spDbHandle->Bind(2, sGramHash());
        spDbHandle->Bind(3, m_sWordForm);
        spDbHandle->Bind(4, false);
        spDbHandle->Bind(5, sLeadComment());            // TODO
        spDbHandle->Bind(6, sTrailingComment());        // TODO
        spDbHandle->Bind(7, true);                      // is_edited

        spDbHandle->InsertRow();
        spDbHandle->Finalize();

        m_llDbKey = spDbHandle->llGetLastKey();
        m_bIsEdited = true;
    }
    catch (CException & exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            spDbHandle->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return H_ERROR_DB;
    }

    try
    {
        for (auto pair : m_mapStress)
        {
            spDbHandle->PrepareForInsert(L"irregular_stress", 4);
            spDbHandle->Bind(1, (int64_t)m_llDbKey);
            int iCharPos = m_sWordForm.uiGetVowelPos(pair.first);
            spDbHandle->Bind(2, iCharPos);
            bool bIsPrimary = (ET_StressType::STRESS_PRIMARY == pair.second) ? true : false;
            spDbHandle->Bind(3, bIsPrimary);
            spDbHandle->Bind(4, true);       // is_edited

            spDbHandle->InsertRow();
            spDbHandle->Finalize();
        }

//        long long llFormKey = pDbHandle->llGetLastKey();
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            spDbHandle->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return H_ERROR_DB;
    }
    return H_NO_ERROR;

}    //  eSaveIrregularForm()

bool CWordForm::bSaveStemToDb()
{
    shared_ptr<CSqlite> spDbHandle;
    try
    {
        if (nullptr == pLexeme())
        {
            ERROR_LOG(L"No lexeme.");
            return false;
        }

        spDbHandle = pLexeme()->spGetDb();
        if (nullptr == spDbHandle)
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
        spDbHandle->PrepareForInsert(L"stems", 1, bIgnoreOnConflict);
        spDbHandle->Bind(1, m_sStem);
        spDbHandle->InsertRow();
        spDbHandle->Finalize();
        if (m_llStemId < 1)
        {
            m_llStemId = -1;
            m_llStemId = spDbHandle->llGetLastKey();
        }
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            spDbHandle->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return false;
    }

    return true;

}       // bSaveStemsToDb()

bool CWordForm::bSaveToDb()
{
    long long llStemDataId = -1;
    shared_ptr<CSqlite> spDbHandle;
    const StLexemeProperties& stLexemeProperties = pLexeme()->stGetProperties();

    try
    {
        spDbHandle = pLexeme()->spGetDb();
        if (nullptr == spDbHandle)
        {
            ERROR_LOG(L"No database access.");
            return false;
        }

        if (m_sStem.bIsEmpty())
        {
            ERROR_LOG(L"No stem.");
            return false;
        }

        sqlite3_stmt * pStmt = nullptr;     // TODO: smart ptr??
        spDbHandle->uiPrepareForInsert(L"stem_data", 4, pStmt, false);
        m_ullDbInsertHandle = (unsigned long long)pStmt;
        spDbHandle->Bind(1, (int64_t)m_llStemId, m_ullDbInsertHandle);
        spDbHandle->Bind(2, sGramHash(), m_ullDbInsertHandle);
        spDbHandle->Bind(3, (int64_t)stLexemeProperties.llDescriptorId, m_ullDbInsertHandle);
        spDbHandle->Bind(4, (int64_t)m_spInflection->llInflectionId(), m_ullDbInsertHandle);
        spDbHandle->InsertRow(m_ullDbInsertHandle);
        spDbHandle->Finalize(m_ullDbInsertHandle);
        llStemDataId = spDbHandle->llGetLastKey();

        spDbHandle->PrepareForInsert(L"wordforms", 2);
        spDbHandle->Bind(1, (int64_t)llStemDataId);
        spDbHandle->Bind(2, (int64_t)m_llEndingDataId);
        spDbHandle->InsertRow();
        spDbHandle->Finalize();
        long long llWordFormDbKey = spDbHandle->llGetLastKey();
        
        map<int, ET_StressType>::iterator itStress = m_mapStress.begin();
        for (; itStress != m_mapStress.end(); ++itStress)
        {
            spDbHandle->PrepareForInsert(L"stress_data", 4);
            spDbHandle->Bind(1, (int64_t)llWordFormDbKey);
            spDbHandle->Bind(2, (*itStress).first);
            bool bIsPrimary = ((*itStress).second == STRESS_PRIMARY) ? true : false;
            spDbHandle->Bind(3, bIsPrimary);
            spDbHandle->Bind(4, m_bIsVariant);
            spDbHandle->InsertRow();
            spDbHandle->Finalize();
        }
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            spDbHandle->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return false;
    }

    return true;

}   //  bool bSaveToDb (...)

bool CWordForm::bSaveIrregularForm() // currently intended for spryazh. sm verbs only
{
//    long long llStemDataId = -1;
    shared_ptr<CSqlite> spDbHandle;
    const StLexemeProperties& stLexemeProperties = pLexeme()->stGetProperties();

    try
    {
        spDbHandle = pLexeme()->spGetDb();
        if (nullptr == spDbHandle)
        {
            ERROR_LOG(L"No database access.");
            return false;
        }

        sqlite3_stmt* pStmt = nullptr;
        spDbHandle->uiPrepareForInsert(L"irregular_forms_spryazh_sm", 7, pStmt, false);
        auto ullInsertHandle = (unsigned long long)pStmt;

        spDbHandle->Bind(1, (int64_t)m_llInflectionId, ullInsertHandle);
        spDbHandle->Bind(2, sGramHash(), ullInsertHandle);
        spDbHandle->Bind(3, m_sWordForm, ullInsertHandle);
        spDbHandle->Bind(4, false, ullInsertHandle);      // is_alternative
        spDbHandle->Bind(5, L"", ullInsertHandle);        // lead_comment
        spDbHandle->Bind(6, L"", ullInsertHandle);        // trailing_comment
        spDbHandle->Bind(7, false, ullInsertHandle);      // is_edited
        spDbHandle->InsertRow(ullInsertHandle);
        spDbHandle->Finalize(ullInsertHandle);
        long long llFormId = spDbHandle->llGetLastKey();

        for (auto&& pairStressPos : m_mapStress)
        {
            spDbHandle->uiPrepareForInsert(L"irregular_stress_spryazh_sm", 4, pStmt, false);
            auto ullStressInsertHandle = (unsigned long long)pStmt;

            spDbHandle->Bind(1, (int64_t)llFormId, ullStressInsertHandle);
            int iPos = m_sWordForm.uiGetVowelPos(pairStressPos.first);
            spDbHandle->Bind(2, iPos, ullStressInsertHandle);
            spDbHandle->Bind(3, pairStressPos.second, ullStressInsertHandle);
            spDbHandle->Bind(4, false, ullStressInsertHandle);      // is_edited
            spDbHandle->InsertRow(ullStressInsertHandle);
            spDbHandle->Finalize(ullStressInsertHandle);
        }
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            spDbHandle->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return false;
    }

    return true;

}   //  bool bSaveIrregularForm()

ET_ReturnCode CWordForm::eSaveTestData()
{
    if (nullptr == pLexeme())
    {
        return H_ERROR_POINTER;
    }

    CSqlite& db = *pLexeme()->spGetDb();
    CEString sHash = sGramHash();

    try
    {
        db.PrepareForInsert(L"test_data", 3);
        db.Bind(1, m_spInflection->sHash());         // lexeme hash
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
//    m_pLexeme = source.m_pLexeme;
    m_spInflection = source.m_spInflection;
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
    m_itStressSyll = source.m_itStressSyll;
    m_sLeadComment = source.m_sLeadComment;
    m_sTrailingComment = source.m_sTrailingComment;
}
