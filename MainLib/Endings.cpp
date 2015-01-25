#include "StdAfx.h"
#include "Lexeme.h"
#include "SqliteWrapper.h"
#include "Endings.h"

using namespace Hlib;

static CEString sSelectBase
    (L"SELECT DISTINCT d.id, e.ending_string, number, case_value, stress FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");

ET_ReturnCode CEndings::eGetEnding(int iSeqNum, CEString& sEnding, unsigned __int64& llEndingKey)
{
    if ((int)m_vecEndings.size() <= iSeqNum || iSeqNum < 0)
    {
        CEString sMsg(L"Unexpected ending number.");
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

    pair<unsigned __int64, CEString> pairEndingData = m_vecEndings[iSeqNum];
    llEndingKey = pairEndingData.first;
    sEnding = pairEndingData.second;

    return H_NO_ERROR;
}

void CEndings::ReportDbError()
{
    if (!m_pLexeme)
    {
        ERROR_LOG(L"No lexeme handle.");
    }

    CSqlite * pDb = NULL;
    CEString sMsg;
    try
    {
        pDb = m_pLexeme->pGetDb();
        CEString sError;
        pDb->GetLastError(sError);
        sMsg = L"DB error: ";
        sMsg += sError;
        sMsg += L"; code = ";
        sMsg += CEString::sToString(pDb->iGetLastError());
    }
    catch (...)
    {
        sMsg = L"Apparent DB error ";
    }

    sMsg += CEString::sToString(pDb->iGetLastError());
    ERROR_LOG(sMsg);

}   //  ReportDbError()

CNounEndings::CNounEndings(CLexeme * pLexeme) : CEndings(pLexeme)
{}

ET_ReturnCode CNounEndings::eSelect(ET_Number eNumber, ET_Case eCase, ET_StressLocation eStressPos)                                                         // nouns
{
    m_eGender = GENDER_UNDEFINED;
    m_eNumber = eNumber;
    m_eCase = eCase;
    m_ePerson = PERSON_UNDEFINED;
    m_eAnimacy = ANIM_UNDEFINED;
    m_eStressPos = eStressPos;
    m_eStemAuslaut = STEM_AUSLAUT_UNDEFINED;
    m_iInflectionType = -1;

    m_vecEndings.clear();

    CEString sSelectBase(L"SELECT DISTINCT ed.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS ed ON (e.id = ed.ending_id) ");

    sSelectBase += L" WHERE inflection_class = ";
    sSelectBase += CEString::sToString(ENDING_CLASS_NOUN);
    sSelectBase += L" AND inflection_type = ";
    sSelectBase += CEString::sToString(m_pLexeme->iType());
    sSelectBase += L" AND (gender = ";
    sSelectBase += CEString::sToString(m_pLexeme->eGender());
    sSelectBase += L" OR gender = ";
    sSelectBase += CEString::sToString(GENDER_UNDEFINED) + L")";
    sSelectBase += L" AND (animacy = ";
    sSelectBase += CEString::sToString(m_pLexeme->eAnimacy());
    sSelectBase += L" OR animacy = ";
    sSelectBase += CEString::sToString(ANIM_UNDEFINED) + L")";
    sSelectBase += L" AND stem_augment = ";
    sSelectBase += CEString::sToString(m_pLexeme->iStemAugment());
    sSelectBase += L" AND case_value = ";
    sSelectBase += CEString::sToString(eCase);
    sSelectBase += L" AND number = ";
    sSelectBase += CEString::sToString(eNumber);

    CSqlite * pDb = NULL;
    try
    {
        pDb = m_pLexeme->pGetDb();

        CEString sSelect(sSelectBase);
        sSelect += L" AND common_deviation = 0;";
        pDb->PrepareForSelect(sSelect);
        while (pDb->bGetRow())
        {
            pDb->GetData(0, (unsigned __int64&)m_ullDbKey);
            pDb->GetData(1, m_sEnding);

            // Skip if non-optional common deviation
            bool bIsOptional = false;
            if (m_pLexeme->bFindCommonDeviation(1, bIsOptional))
            {
                if (NUM_PL == m_eNumber && CASE_NOM == m_eCase && GENDER_F != m_pLexeme->eGender())
                {
                    if (!bIsOptional)
                    {
                        continue;
                    }
                }
            }

            if (m_pLexeme->bFindCommonDeviation(2, bIsOptional))
            {
                if (NUM_PL == m_eNumber && CASE_GEN == m_eCase)
                {
                    if (!bIsOptional)    // optional?
                    {
                        continue;
                    }
                }
            }

            m_vecEndings.push_back(pair<unsigned __int64, CEString>(m_ullDbKey, m_sEnding));

        }   //  while (pDb->bGetRow())

        pDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_ERROR_GENERAL;
    }

    for (int iCd = 1; iCd <= 2; ++iCd)
    {
        if (!m_pLexeme->bHasCommonDeviation(iCd))
        {
            continue;
        }

        CEString sSelectCD(sSelectBase);
        sSelectCD += L" AND common_deviation = ";
        sSelectCD += CEString::sToString(iCd);
        sSelectCD += L";";

        try
        {
            pDb->PrepareForSelect(sSelectCD);
            while (pDb->bGetRow())
            {
                pDb->GetData(0, (unsigned __int64&)m_ullDbKey);
                pDb->GetData(1, m_sEnding);

                m_vecEndings.push_back(pair<unsigned __int64, CEString>(m_ullDbKey, m_sEnding));

            }
            pDb->Finalize();
        }
        catch (...)
        {
            ReportDbError();
            return H_ERROR_DB;
        }

    }   // for ...

    return H_NO_ERROR;

}       // select (nouns)

/*
ET_ReturnCode CNounEndings::eLoad()
{
    ET_ReturnCode rc = H_NO_ERROR;

    CEString sSelect(sSelectBase);
    sSelect += L" WHERE inflection_class = ";
    sSelect += CEString::sToString (ENDING_CLASS_NOUN);
    sSelect += L" AND inflection_type = ";
    sSelect += CEString::sToString (m_pLexeme->iType());
    sSelect += L" AND (gender = ";
    sSelect += CEString::sToString (m_pLexeme->eGender());
    sSelect += L" OR gender = ";
    sSelect += CEString::sToString (GENDER_UNDEFINED) + L")";
    sSelect += L" AND (animacy = ";
    sSelect += CEString::sToString (m_pLexeme->eAnimacy());
    sSelect += L" OR animacy = ";
    sSelect += CEString::sToString (ANIM_UNDEFINED) + L")";
    sSelect += L" AND stem_augment = ";
    sSelect += CEString::sToString (m_pLexeme->iStemAugment());

    CSqlite * pDb = NULL;
    try
    {
        pDb = m_pLexeme->pGetDb();

        CEString sSelectBase (sSelect);
        sSelectBase += L" AND common_deviation = 0;";

        pDb->PrepareForSelect (sSelectBase);
        while (pDb->bGetRow())
        {
            pDb->GetData (0, (unsigned __int64&)m_ullDbKey);
            pDb->GetData (1, m_sEnding);
            pDb->GetData (2, (int&)m_eNumber);
            pDb->GetData (3, (int&)m_eCase);
            pDb->GetData (4, (int&)m_eStressPos);

            bool bIsOptional = false;
            if (m_pLexeme->bFindCommonDeviation(1, bIsOptional))
            {
                if (NUM_PL == m_eNumber && CASE_NOM == m_eCase && GENDER_F != m_pLexeme->eGender())
                {
                    if (!bIsOptional)
                    {
                        continue;
                    }
                }
            }

            if (m_pLexeme->bFindCommonDeviation(2, bIsOptional))
            {
                if (NUM_PL == m_eNumber && CASE_GEN == m_eCase)
                {
                    if (!bIsOptional)    // optional?
                    {
                        continue;
                    }
                }
            }
//            rc = eAddEnding();
            if (rc != H_NO_ERROR)
            {
                return rc;
            }

        }   //  while (pDb->bGetRow())

        pDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_ERROR_GENERAL;
    }

    for (int iCd = 1; iCd <= 2; ++iCd)
    {
        if (!m_pLexeme->bHasCommonDeviation(iCd))
        {
            continue;
        }

        CEString sSelectCD (sSelect);
        sSelectCD += L" AND common_deviation = ";
        sSelectCD += CEString::sToString (iCd);
        sSelectCD += L";";

        try
        {
            pDb->PrepareForSelect (sSelectCD);
            while (pDb->bGetRow())
            {
                pDb->GetData (0, (unsigned __int64&)m_ullDbKey);
                pDb->GetData (1, m_sEnding);
                pDb->GetData (2, (int&)m_eNumber);
                pDb->GetData (3, (int&)m_eCase);
                pDb->GetData (4, (int&)m_eStressPos);
//                rc = eAddEnding();
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
            }
            pDb->Finalize();
        }
        catch (...)
        {
            ReportDbError();
            return H_ERROR_DB;
        }

    }   // for ...

    return H_NO_ERROR;

}   //  eLoad()
*/

/*
ET_ReturnCode CNounEndings::eAddEnding()
{
    std::vector<ET_Number> vecNumber;
    if (NUM_UNDEFINED == m_eNumber)
    {
        for (int iNum = (int)NUM_UNDEFINED; iNum < (int)NUM_COUNT; ++iNum)
        {
            vecNumber.push_back ((ET_Number)iNum);
        }
    }
    else
    {
        vecNumber.push_back (m_eNumber);
    }

    //
    // Case must be defined for nouns
    //
    if (CASE_UNDEFINED == m_eCase)
    {
        ERROR_LOG (L"Undefined case in noun ending.");
        return H_ERROR_INVALID_ARG;
    }

    std::vector<ET_StressLocation> vecStress;
    if (STRESS_LOCATION_UNDEFINED == m_eStressPos)
    {
        for (int iStress = STRESS_LOCATION_UNDEFINED; 
             iStress < (int)STRESS_LOCATION_COUNT; 
             ++iStress)
        {
            vecStress.push_back ((ET_StressLocation)iStress);
        }
    }
    else
    {
        vecStress.push_back (m_eStressPos);
    }

    for (int iN = 0; iN < (int)vecNumber.size(); ++iN)
        for (int iS = 0; iS < (int)vecStress.size(); ++iS)
        {
            m_eNumber = vecNumber[iN];
            m_eStressPos = vecStress[iS];
            std::pair<CEString, unsigned __int64> pairEndingData(m_sEnding, m_ullDbKey);
            m_mmEndings.insert(std::pair<__int64, std::pair<CEString, __int64> >(ullHash(), pairEndingData));
        }

    return H_NO_ERROR;

}   // CNounEndings::eAddEnding (...)
*/

/*
unsigned __int64 CNounEndings::ullHash() const
{
    unsigned int uiLoDWord = m_eNumber *               CASE_COUNT * STRESS_LOCATION_COUNT +
                             m_eCase *                              STRESS_LOCATION_COUNT +
                             m_eStressPos;

    unsigned __int64 ullHash = ENDING_CLASS_NOUN;
    ullHash = ullHash << 32;
    ullHash |= uiLoDWord;

    return ullHash;
}
*/

////////////////////////////////////////////////////////////////////////////////////

CAdjLongEndings::CAdjLongEndings(CLexeme * pLexeme, ET_Subparadigm eSubparadigm)
    : CEndings(pLexeme), m_eSubparadigm(eSubparadigm)
{}

ET_ReturnCode CAdjLongEndings::eSelect(ET_Subparadigm eSubparadigm, ET_Gender eGender, ET_Number eNumber, ET_Case eCase, ET_Animacy eAnimacy)                  // long adjectives, participles & pronouns
{
    m_eGender = eGender;
    m_eNumber = eNumber;
    m_eCase = eCase;
    m_ePerson = PERSON_UNDEFINED;
    m_eAnimacy = eAnimacy;
    m_eStressPos = STRESS_LOCATION_UNDEFINED;
    m_eStemAuslaut = STEM_AUSLAUT_UNDEFINED;
    m_iInflectionType = -1;

    m_vecEndings.clear();

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = ";

    if (L"мс" == m_pLexeme->sInflectionType())
    {
        sSelect += CEString::sToString(ENDING_CLASS_PRONOUN);
    }
    else
    {
        sSelect += CEString::sToString(ENDING_CLASS_LONG_ADJECTIVE);
    }

    sSelect += L" AND d.inflection_type = ";
    if (SUBPARADIGM_PART_PRES_ACT == eSubparadigm || SUBPARADIGM_PART_PAST_ACT == eSubparadigm)
    {
        sSelect += L"4";
    }
    else if (SUBPARADIGM_PART_PRES_PASS_LONG == eSubparadigm || SUBPARADIGM_PART_PAST_PASS_LONG == eSubparadigm)
    {
        sSelect += L"1";
    }
    else
    {
        sSelect += CEString::sToString(m_pLexeme->iType());
    }

    // Stress type:
    //
    // Type 1: relevant for N-Sg-m and A-Sg-m-Inanimate
    // Type 2: always irrelevant
    // Type 3: relevant for N-Sg-m and A-Sg-m-Inanimate
    // Type 4: NOT relevant for I-Sg-m, N-Sg-f, A-Sg-f, Pl

    bool bIsStressRelevant = false;

    if (m_pLexeme->iType() == 1 || m_pLexeme->iType() == 3)
    {
        if (GENDER_M == eGender && NUM_SG == eNumber)
        {
            if (CASE_NOM == eCase || (CASE_ACC == eCase && ANIM_NO == eAnimacy))
            {
                bIsStressRelevant = true;
            }
        }
    }
    else if (m_pLexeme->iType() == 4)
    {
        if (NUM_SG == eNumber)
        {
            if (((GENDER_M == eGender || GENDER_N == eGender) && CASE_INST == eCase) ||
                (GENDER_F == eGender && (CASE_NOM == eCase || CASE_ACC == eCase)))
            {
                bIsStressRelevant = false;
            }
            else
            {
                bIsStressRelevant = true;
            }
        }
    }

    if (bIsStressRelevant)
    {
        if (AT_A == m_pLexeme->eAccentType1())
        {
            m_eStressPos = STRESS_LOCATION_STEM;
        }
        else
        {
            if (AT_B == m_pLexeme->eAccentType1())
            {
                m_eStressPos = STRESS_LOCATION_ENDING;
            }
            else
            {
                ERROR_LOG(L"Unknown stress type.");
                return H_ERROR_UNEXPECTED;
            }
        }
    }
    else
    {
        m_eStressPos = STRESS_LOCATION_UNDEFINED;
    }

    ET_Reflexive eRefl = REFL_UNDEFINED;
    if (POS_VERB == m_pLexeme->ePartOfSpeech())
    {
        eRefl = m_pLexeme->eIsReflexive();
    }
    else if (SUBPARADIGM_LONG_ADJ == eSubparadigm)
    {
        if (m_pLexeme->sSourceForm().bEndsWith(L"ся"))
        {
            eRefl = REFL_YES;
        }
    }

    sSelect += L" AND d.gender = ";
    sSelect += CEString::sToString(eGender);
    sSelect += L" AND d.case_value = ";
    sSelect += CEString::sToString(eCase);
    sSelect += L" AND d.number = ";
    sSelect += CEString::sToString(eNumber);
    sSelect += L" AND d.stress = ";
    sSelect += CEString::sToString(m_eStressPos);
    sSelect += L" AND d.animacy = ";
    sSelect += CEString::sToString(eAnimacy);
    sSelect += L" AND d.is_reflexive = ";
    sSelect += (REFL_YES == eRefl) ? L"1" : L"0";

    CSqlite * pDb = NULL;

    try
    {
        pDb = m_pLexeme->pGetDb();
        pDb->PrepareForSelect(sSelect);
        while (pDb->bGetRow())
        {
            pDb->GetData(0, (__int64&)m_ullDbKey);
            pDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<unsigned __int64, CEString>(m_ullDbKey, m_sEnding));
        }
        pDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_ERROR_DB;
    }

    return H_NO_ERROR;

}       //  CAdjLongEndings::eSelect()

////////////////////////////////////////////////////////////////////////////////////

CAdjShortEndings::CAdjShortEndings(CLexeme * pLexeme) : CEndings(pLexeme)
{}

ET_ReturnCode CAdjShortEndings::eSelect(ET_Gender eGender, ET_Number eNumber, ET_StressLocation eStressPos)                                                     // short adjectives
{
    m_eGender = eGender;
    m_eNumber = eNumber;
    m_eCase = CASE_UNDEFINED;
    m_ePerson = PERSON_UNDEFINED;
    m_eAnimacy = ANIM_UNDEFINED;
    m_eStressPos = eStressPos;
    m_eStemAuslaut = STEM_AUSLAUT_UNDEFINED;
    m_iInflectionType = -1;

    m_vecEndings.clear();

    // Stress type:
    //
    // Type 1: Irrelevant
    // Type 2: Relevant for n Sg
    // Type 3: Irrelevant
    // Type 4: Relevant for n Sg
    // Type 5: Relevant for n Sg

    bool bIsStressRelevant = false;

    if (m_pLexeme->iType() == 2 || m_pLexeme->iType() == 4 || m_pLexeme->iType() == 5)
    {
        if (GENDER_N == eGender && NUM_SG == eNumber)
        {
            bIsStressRelevant = true;
        }
    }

    if (!bIsStressRelevant)
    {
        m_eStressPos = STRESS_LOCATION_UNDEFINED;
    }

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE inflection_class = ";
    sSelect += CEString::sToString(ENDING_CLASS_SHORT_ADJECTIVE);
    sSelect += L" AND inflection_type = ";
    sSelect += CEString::sToString(m_pLexeme->iType());
    sSelect += L" AND gender = ";
    sSelect += CEString::sToString(eGender);
    sSelect += L" AND number = ";
    sSelect += CEString::sToString(eNumber);
    sSelect += L" AND d.stress = ";
    sSelect += CEString::sToString(m_eStressPos);

    CSqlite * pDb = NULL;

    try
    {
        pDb = m_pLexeme->pGetDb();
        pDb->PrepareForSelect(sSelect);
        while (pDb->bGetRow())
        {
            pDb->GetData(0, (__int64&)m_ullDbKey);
            pDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<unsigned __int64, CEString>(m_ullDbKey, m_sEnding));
        }
        pDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_ERROR_DB;
    }

    return H_NO_ERROR;

}       //  CAdjShortEndings::eSelect()

////////////////////////////////////////////////////////////////////////////////////

CPersonalEndings::CPersonalEndings(CLexeme * pLexeme) : CEndings(pLexeme)
{}

ET_ReturnCode CPersonalEndings::Select(ET_Person ePerson, ET_Number eNumber, ET_StressLocation eStressPos, ET_StemAuslaut eStemAuslaut, int iInflectionType, ET_Reflexive eRefl)   // personal endings
{
    m_eGender = GENDER_UNDEFINED;
    m_eNumber = eNumber;
    m_eCase = CASE_UNDEFINED;
    m_ePerson = ePerson;
    m_eAnimacy = ANIM_UNDEFINED;
    m_eStressPos = eStressPos;
    m_eStemAuslaut = eStemAuslaut;
    m_iInflectionType = iInflectionType;

    CEString sSelect(L"SELECT DISTINCT ed.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS ed ON (e.id = ed.ending_id) ");
    sSelect += L"WHERE ed.number = " + CEString::sToString(eNumber) + L" AND ed.person = " + CEString::sToString(ePerson) + L" AND ed.stress = " +
        CEString::sToString(eStressPos) + L" AND ed.stem_auslaut = " + CEString::sToString(eStemAuslaut) + L" AND (ed.inflection_type = " + CEString::sToString(iInflectionType) +
        L" OR ed.inflection_type = -1) AND ed.is_reflexive = " + ((REFL_YES == eRefl) ? L"1" : L"0");

    CSqlite * pDb = NULL;
    try
    {
        pDb = m_pLexeme->pGetDb();

        pDb->PrepareForSelect(sSelect);
        while (pDb->bGetRow())
        {
            pDb->GetData(0, (unsigned __int64&)m_ullDbKey);
            pDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<unsigned __int64, CEString>(m_ullDbKey, m_sEnding));
        }

        pDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_ERROR_GENERAL;
    }

    return H_NO_ERROR;
}

/*
ET_ReturnCode CPersonalEndings::eLoad()
{
    ET_ReturnCode rc = H_NO_ERROR;

    if (POS_VERB != m_pLexeme->ePartOfSpeech())
    {
        ERROR_LOG (L"Bad part of speech value.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sSelect 
        (L"SELECT DISTINCT d.id, e.ending_string, d.person, d.number, d.stress, d.stem_auslaut, d.inflection_type FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE inflection_class = " + CEString::sToString (ENDING_CLASS_PRESENT_TENSE);

    CSqlite * pDb = NULL;

    try
    {
        pDb = m_pLexeme->pGetDb();
        pDb->PrepareForSelect (sSelect);
        while (pDb->bGetRow())
        {
            pDb->GetData (0, (__int64&)m_ullDbKey);
            pDb->GetData (1, m_sEnding);
            pDb->GetData (2, (int&)m_ePerson);
            pDb->GetData (3, (int&)m_eNumber);
            pDb->GetData (4, (int&)m_eStressPos);
            pDb->GetData (5, (int&)m_eStemAuslaut);
            pDb->GetData (6, m_iInflectionType);
//            rc = eAddEnding();
        }
        pDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();    
        rc = H_EXCEPTION;
    }

    return rc;

}   //  eLoad()
*/

/*
ET_ReturnCode CPersonalEndings::eAddEnding()
{
    //
    // Person
    //
    std::vector<ET_Person> vecPerson;
    if (PERSON_UNDEFINED == m_ePerson)
    {
        ERROR_LOG (L"Undefined person.");
        return H_ERROR_INVALID_ARG;
    }
    else
    {
        vecPerson.push_back (m_ePerson);
    }

    //
    // Number
    //
    if (NUM_UNDEFINED == m_eNumber)
    {
        ERROR_LOG (L"Undefined number in personal ending.");
        return H_ERROR_INVALID_ARG;
    }

    //
    // Ending stressed/unstressed
    //
    std::vector<ET_StressLocation> vecStress;
    if (STRESS_LOCATION_UNDEFINED == m_eStressPos)
    {
        for (int iStress = STRESS_LOCATION_UNDEFINED; 
             iStress < (int)STRESS_LOCATION_COUNT; 
             ++iStress)
        {
            vecStress.push_back ((ET_StressLocation)iStress);
        }
    }
    else
    {
        vecStress.push_back (m_eStressPos);
    }

    //
    // Ending after sh/not after sh
    //
    std::vector<ET_StemAuslaut> vecStemAuslaut;
    if (STEM_AUSLAUT_UNDEFINED == m_eStemAuslaut)
    {
        for (int iStemAuslaut = STEM_AUSLAUT_UNDEFINED; 
             iStemAuslaut < (int)STEM_AUSLAUT_COUNT; 
             ++iStemAuslaut)
        {
            vecStemAuslaut.push_back ((ET_StemAuslaut)iStemAuslaut);
        }
    }
    else
    {
        vecStemAuslaut.push_back (m_eStemAuslaut);
    }

    for (int iS = 0; iS < (int)vecStress.size(); ++iS)
        for (int iSa = 0; iSa < (int)vecStemAuslaut.size(); ++iSa)
        {
            m_eStressPos = vecStress[iS];
            m_eStemAuslaut = vecStemAuslaut[iSa];
            __int64 llKey = ullHash();
            std::pair<CEString, __int64> pairEndingData(m_sEnding, m_ullDbKey);
            m_mmEndings.insert(std::pair<unsigned __int64, std::pair<CEString, unsigned __int64> >(ullHash(), pairEndingData));
        }

    return H_NO_ERROR;

}   //  CPersonalEndings::eAddEnding (...)

unsigned __int64 CPersonalEndings::ullHash() const
{
    unsigned int uiLoDWord = m_ePerson * NUM_COUNT * STRESS_LOCATION_COUNT * STEM_AUSLAUT_COUNT + INFLECTION_TYPE_COUNT +
                             m_eNumber *             STRESS_LOCATION_COUNT * STEM_AUSLAUT_COUNT + INFLECTION_TYPE_COUNT +
                             m_eStressPos *                                  STEM_AUSLAUT_COUNT + INFLECTION_TYPE_COUNT +
                             m_iInflectionType;

    unsigned __int64 ullHash = ENDING_CLASS_PRESENT_TENSE;
    ullHash = ullHash << 32;
    ullHash |= uiLoDWord;

    return ullHash;
}
*/

//
// Infinitive
//
CInfinitiveEndings::CInfinitiveEndings(CLexeme * pLexeme) : CEndings(pLexeme)
{}

void CInfinitiveEndings::Select(int iInflectionType)                                                                                                    // infinitive
{
    m_eGender = GENDER_UNDEFINED;
    m_eNumber = NUM_UNDEFINED;
    m_eCase = CASE_UNDEFINED;
    m_ePerson = PERSON_UNDEFINED;
    m_eAnimacy = ANIM_UNDEFINED;
    m_eStressPos = STRESS_LOCATION_UNDEFINED;
    m_eStemAuslaut = STEM_AUSLAUT_UNDEFINED;
    m_iInflectionType = -1;
}


/*
ET_ReturnCode CInfinitiveEndings::eLoad()
{
    ET_ReturnCode rc = H_NO_ERROR;

    if (POS_VERB != m_pLexeme->ePartOfSpeech())
    {
        ERROR_LOG(L"Bad part of speech value.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sSelect
        (L"SELECT ending_data.id, ending_data.conjugation, endings.ending_string FROM endings INNER JOIN ending_data ON(endings.id = ending_data.ending_id)");
        sSelect += L" WHERE inflection_class = " + CEString::sToString(ENDING_CLASS_INFINITIVE);

    CSqlite * pDb = NULL;

    try
    {
        pDb = m_pLexeme->pGetDb();
        pDb->PrepareForSelect(sSelect);
        while (pDb->bGetRow())
        {
            pDb->GetData(0, (__int64&)m_ullDbKey);
            pDb->GetData(1, (int&)m_iInflectionType);
            pDb->GetData(2, m_sEnding);
//            rc = eAddEnding();
        }
        pDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        rc = H_EXCEPTION;
    }

    return rc;

}   //  eLoad()
*/

/*
ET_ReturnCode CInfinitiveEndings::eAddEnding()
{
    std::pair<CEString, __int64> pairEndingData(m_sEnding, m_ullDbKey);
    m_mmEndings.insert(std::pair<__int64, std::pair<CEString, __int64> >(ullHash(), pairEndingData));

    return H_NO_ERROR;

}   //  CInfinitiveEndings::eAddEnding (...)
*/

/*
unsigned __int64 CInfinitiveEndings::ullHash() const
{
    unsigned int uiLoDWord = m_iInflectionType;

    unsigned __int64 ullHash = ENDING_CLASS_INFINITIVE;
    ullHash = ullHash << 32;
    ullHash |= uiLoDWord;

    return ullHash;
}
*/

//
// Past tense
//
CPastTenseEndings::CPastTenseEndings(CLexeme * pLexeme) : CEndings(pLexeme)
{}

void CPastTenseEndings::Select(ET_Gender eGender, ET_Number eNumber, ET_StemAuslaut eAuslaut)                                                          // past tense
{
    m_eGender = eGender;
    m_eNumber = eNumber;
    m_eCase = CASE_UNDEFINED;
    m_ePerson = PERSON_UNDEFINED;
    m_eAnimacy = ANIM_UNDEFINED;
    m_eStressPos = STRESS_LOCATION_UNDEFINED;
    m_eStemAuslaut = eAuslaut;
    m_iInflectionType = -1;
}

/*
ET_ReturnCode CPastTenseEndings::eLoad()
{
    ET_ReturnCode rc = H_NO_ERROR;

    if (POS_VERB != m_pLexeme->ePartOfSpeech())
    {
        ERROR_LOG(L"Bad part of speech value.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sSelect
        (L"SELECT DISTINCT d.id, e.ending_string, d.gender, d.number, d.stem_auslaut FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE inflection_class = " + CEString::sToString(ENDING_CLASS_PAST_TENSE);

    CSqlite * pDb = NULL;

    try
    {
        pDb = m_pLexeme->pGetDb();
        pDb->PrepareForSelect(sSelect);
        while (pDb->bGetRow())
        {
            pDb->GetData(0, (__int64&)m_ullDbKey);
            pDb->GetData(1, m_sEnding);
            pDb->GetData(2, (int&)m_eGender);
            pDb->GetData(3, (int&)m_eNumber);
            pDb->GetData(4, (int&)m_eStemAuslaut);
//            rc = eAddEnding();
        }
        pDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        rc = H_EXCEPTION;
    }

    return rc;

}   //  eLoad()
*/

/*
ET_ReturnCode CPastTenseEndings::eAddEnding()
{
    std::pair<CEString, __int64> pairEndingData(m_sEnding, m_ullDbKey);
    m_mmEndings.insert(std::pair<__int64, std::pair<CEString, __int64> >(ullHash(), pairEndingData));

    return H_NO_ERROR;

}   //  CPastTenseEndings::eAddEnding (...)
*/

/*
unsigned __int64 CPastTenseEndings::ullHash() const
{
    unsigned int uiLoDWord = m_eGender * NUM_COUNT * STEM_AUSLAUT_COUNT +
                             m_eNumber *             STEM_AUSLAUT_COUNT +
                             m_eStemAuslaut;
    
    unsigned __int64 ullHash = ENDING_CLASS_PAST_TENSE;
    ullHash = ullHash << 32;
    ullHash |= uiLoDWord;

    return ullHash;
}
*/

