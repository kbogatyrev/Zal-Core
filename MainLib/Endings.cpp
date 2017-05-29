#include "StdAfx.h"
#include "Lexeme.h"
#include "SqliteWrapper.h"
#include "Endings.h"

using namespace Hlib;

static CEString sSelectBase
    (L"SELECT DISTINCT d.id, e.ending_string, number, case_value, stress FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");

ET_ReturnCode CEndings::eGetEnding(int iSeqNum, CEString& sEnding, int64_t& llEndingKey)
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
    sSelectBase += CEString::sToString(m_pLexeme->eInflectionTypeToGender());
    sSelectBase += L" OR gender = ";
    sSelectBase += CEString::sToString(GENDER_UNDEFINED) + L")";
    sSelectBase += L" AND (animacy = ";
    sSelectBase += CEString::sToString(m_pLexeme->eInflectionTypeToAnimacy());
    sSelectBase += L" OR animacy = ";
    sSelectBase += CEString::sToString(ANIM_UNDEFINED) + L")";
    sSelectBase += L" AND stem_augment = ";
    sSelectBase += CEString::sToString(m_pLexeme->iStemAugment());
    sSelectBase += L" AND case_value = ";
    sSelectBase += CEString::sToString(eCase);
    sSelectBase += L" AND number = ";
    sSelectBase += CEString::sToString(eNumber);
    sSelectBase += L" AND (stress = 0 OR stress = ";
    sSelectBase += CEString::sToString(eStressPos);
    sSelectBase += L")";

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
                if (NUM_PL == m_eNumber && CASE_NOM == m_eCase && GENDER_F != m_pLexeme->eInflectionTypeToGender())
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

    if (SUBPARADIGM_PART_PRES_ACT == eSubparadigm || SUBPARADIGM_PART_PAST_ACT == eSubparadigm)
    {
        m_iInflectionType = 4;
    }
    else if (SUBPARADIGM_PART_PRES_PASS_LONG == eSubparadigm || SUBPARADIGM_PART_PAST_PASS_LONG == eSubparadigm)
    {
        m_iInflectionType = 1;
    }
    else
    {
        m_iInflectionType = m_pLexeme->iType();
    }

    m_vecEndings.clear();

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = ";

    if (L"мс-п" == m_pLexeme->sInflectionType())
    {
        sSelect += CEString::sToString(ENDING_CLASS_PRONOUN);
    }
    else
    {
        sSelect += CEString::sToString(ENDING_CLASS_LONG_ADJECTIVE);
    }

    sSelect += L" AND d.inflection_type = ";
    sSelect += CEString::sToString(m_iInflectionType);

    // Stress type:
    //
    // Type 1: relevant for N-Sg-m and A-Sg-m-Inanimate
    // Type 2: always irrelevant
    // Type 3: relevant for N-Sg-m and A-Sg-m-Inanimate
    // Type 4: NOT relevant for I-Sg-m, N-Sg-f, A-Sg-f, Pl

    bool bIsStressRelevant = false;

    if (1 == m_iInflectionType || 3 == m_iInflectionType)
    {
        if (GENDER_M == eGender && NUM_SG == eNumber)
        {
            if (CASE_NOM == eCase || (CASE_ACC == eCase && ANIM_NO == eAnimacy))
            {
                bIsStressRelevant = true;
            }
        }
    }
    else if (4 == m_iInflectionType)
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
        if (SUBPARADIGM_LONG_ADJ == eSubparadigm)
        {
            if (AT_A == m_pLexeme->eAccentType1() || AT_A1 == m_pLexeme->eAccentType1())
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
            m_eStressPos = STRESS_LOCATION_STEM;
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

CAdjShortEndings::CAdjShortEndings(CLexeme * pLexeme, ET_Subparadigm eSubparadigm) 
    : CEndings(pLexeme), m_eSubparadigm(eSubparadigm)
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

    if (SUBPARADIGM_PART_PAST_PASS_SHORT == m_eSubparadigm)
    {
        m_iInflectionType = 1;
    }
    else if (SUBPARADIGM_SHORT_ADJ)
    {
        m_iInflectionType = m_pLexeme->iType();
    }
    else
    {
        CEString sMsg(L"Unexpected subparadigm: must be short adj. or short part. past. pass.");
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

    if (2 == m_iInflectionType|| 4 == m_iInflectionType|| 5 == m_iInflectionType)
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
//    sSelect += CEString::sToString(m_pLexeme->iType());
    sSelect += CEString::sToString(m_iInflectionType);
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

CAdjPronounEndings::CAdjPronounEndings(CLexeme * pLexeme) : CEndings(pLexeme)
{}

ET_ReturnCode CAdjPronounEndings::eSelect(ET_Gender eGender, ET_Number eNumber, ET_Case eCase, ET_Animacy eAnimacy)
{
    m_eGender = eGender;
    m_eNumber = eNumber;
    m_eCase = eCase;
    m_ePerson = PERSON_UNDEFINED;
    m_eAnimacy = eAnimacy;
    m_eStressPos = STRESS_LOCATION_UNDEFINED;
    m_eStemAuslaut = STEM_AUSLAUT_UNDEFINED;
    
    m_iInflectionType = m_pLexeme->iType();

    m_vecEndings.clear();

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = ";
    sSelect += CEString::sToString(ENDING_CLASS_PRONOUN);
    sSelect += L" AND d.inflection_type = ";
    sSelect += CEString::sToString(m_iInflectionType);

    /*    
        1a triangle этот
        1b triangle тот
        1*b triangle один
        1f triangle сам
        2b triangle сей
        2*b triangle весь
        4a наш ваш
        6a кой никой
        6b мой твой свой
        6*b чей ничей

        Stress:
        
        type 2 (and 6): N Sg n: е/ё
                        L Sg m,n: ем/ём
		
    */
    bool bIsStressRelevant = false;

    if (2 == m_iInflectionType || 6 == m_iInflectionType)
    {
        if ((GENDER_M == eGender || GENDER_N == eGender) && NUM_SG == eNumber)
        {
            if (CASE_PREP == eCase)
            {
                bIsStressRelevant = true;
            }
        }
        if (GENDER_N == eGender && NUM_SG == eNumber)
        {
            if (CASE_NOM == eCase || CASE_ACC == eCase)
            {
                bIsStressRelevant = true;
            }
        }
    }
    else if (4 == m_iInflectionType)
    {
        if (NUM_SG == eNumber)
        {
            if ((GENDER_M == eGender && (CASE_NOM == eCase || (CASE_ACC == eCase && ANIM_NO == eAnimacy))) ||
                ((GENDER_M == eGender || GENDER_N == eGender) && CASE_INST == eCase) ||
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
            else if (AT_F == m_pLexeme->eAccentType1())      // only сам
            {
                if (CASE_NOM == eCase && NUM_PL == eNumber)
                {
                    m_eStressPos = STRESS_LOCATION_STEM;
                }
                else
                {
                    m_eStressPos = STRESS_LOCATION_ENDING;
                }
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
    sSelect += L" AND d.is_reflexive = 0";

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

}       //  CAdjPronounEndings::eSelect()

CPersonalEndings::CPersonalEndings(CLexeme * pLexeme) : CEndings(pLexeme)
{}

ET_ReturnCode CPersonalEndings::eSelect(ET_Person ePerson, ET_Number eNumber, ET_StressLocation eStressPos, ET_StemAuslaut eStemAuslaut) 
{
    m_eGender = GENDER_UNDEFINED;
    m_eNumber = eNumber;
    m_eCase = CASE_UNDEFINED;
    m_ePerson = ePerson;
    m_eAnimacy = ANIM_UNDEFINED;
    m_eStressPos = eStressPos;
    m_eStemAuslaut = eStemAuslaut;

    m_vecEndings.clear();

    CEString sSelect(L"SELECT DISTINCT ed.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS ed ON (e.id = ed.ending_id) ");
    sSelect += L"WHERE ed.inflection_class = 5";
    sSelect += L" AND ed.number = " + CEString::sToString(eNumber);
    sSelect += L" AND ed.person = " + CEString::sToString(ePerson);
    sSelect += L" AND ed.inflection_type = " + CEString::sToString(m_pLexeme->iType());
    sSelect += L" AND ed.is_reflexive = ";
    sSelect += ((REFL_YES == m_pLexeme->eIsReflexive()) ? L"1" : L"0");

    if (m_pLexeme->iType() != 4 && m_pLexeme->iType() != 5)
    {
        if ((PERSON_1 != ePerson || NUM_SG != eNumber) &&
            (PERSON_3 != ePerson || NUM_PL != eNumber))
        {
            sSelect += L" AND ed.stress = " + CEString::sToString(eStressPos);
        }
    }

    if (m_pLexeme->iType() == 4 || m_pLexeme->iType() == 5 || m_pLexeme->iType() == 6)
    {
        if ((PERSON_1 == ePerson && NUM_SG == eNumber) ||
            (PERSON_3 == ePerson && NUM_PL == eNumber))
        {
            if (m_pLexeme->iType() == 6 && m_pLexeme->iStemAugment() == 1)
            {
                sSelect += L" AND ed.stem_augment = 1";
            }
            else
            {
                sSelect += L" AND ed.stem_auslaut = " + CEString::sToString(eStemAuslaut);
            }
        }
    }
    
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

//
// Infinitive
//
CInfinitiveEndings::CInfinitiveEndings(CLexeme * pLexeme) : CEndings(pLexeme)
{}

ET_ReturnCode CInfinitiveEndings::eSelect(int iInflectionType)                                                              // infinitive
{
    m_eGender = GENDER_UNDEFINED;
    m_eNumber = NUM_UNDEFINED;
    m_eCase = CASE_UNDEFINED;
    m_ePerson = PERSON_UNDEFINED;
    m_eAnimacy = ANIM_UNDEFINED;
    m_eStressPos = STRESS_LOCATION_UNDEFINED;
    m_eStemAuslaut = STEM_AUSLAUT_UNDEFINED;
    m_iInflectionType = iInflectionType;

    m_vecEndings.clear();

    if (POS_VERB != m_pLexeme->ePartOfSpeech())
    {
        ERROR_LOG(L"Bad part of speech value.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = " + CEString::sToString(ENDING_CLASS_INFINITIVE);
    sSelect += L" AND d.inflection_type = " + CEString::sToString(iInflectionType);
    sSelect += L" AND d.is_reflexive = ";
    sSelect += (REFL_YES == m_pLexeme->eIsReflexive()) ? L"1" : L"0";

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
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eSelect() -- infinitive

//
// Past tense
//
CPastTenseEndings::CPastTenseEndings(CLexeme * pLexeme) : CEndings(pLexeme)
{}

ET_ReturnCode CPastTenseEndings::eSelect(ET_Gender eGender, ET_Number eNumber, ET_StemAuslaut eAuslaut)                                                          // past tense
{
    m_eGender = eGender;
    m_eNumber = eNumber;
    m_eCase = CASE_UNDEFINED;
    m_ePerson = PERSON_UNDEFINED;
    m_eAnimacy = ANIM_UNDEFINED;
    m_eStressPos = STRESS_LOCATION_UNDEFINED;
    m_eStemAuslaut = eAuslaut;
    m_iInflectionType = -1;

    m_vecEndings.clear();

    if (POS_VERB != m_pLexeme->ePartOfSpeech())
    {
        ERROR_LOG(L"Bad part of speech value.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sSelect (L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = " + CEString::sToString(ENDING_CLASS_PAST_TENSE);
    sSelect += L" AND d.gender = " + CEString::sToString(eGender);
    sSelect += L" AND d.number = " + CEString::sToString(eNumber);
    sSelect += L" AND d.is_reflexive = ";
    sSelect += (REFL_YES == m_pLexeme->eIsReflexive()) ? L"1" : L"0";

    if (NUM_SG == eNumber && GENDER_M == eGender)
    {
        sSelect += L" AND d.stem_auslaut = " + CEString::sToString(eAuslaut);
    }

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
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eSelect() -- past tense

CImperativeEndings::CImperativeEndings(CLexeme * pLexeme) : CEndings(pLexeme)
{}

ET_ReturnCode CImperativeEndings::eSelect(ET_Number eNumber, int iType, bool bIsVariant)
{
    if (!bIsVariant)
    {
        m_vecEndings.clear();
    }

    if (POS_VERB != m_pLexeme->ePartOfSpeech())
    {
        ERROR_LOG(L"Bad part of speech value.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = " + CEString::sToString(ENDING_CLASS_IMPERATIVE);
    sSelect += L" AND d.number = " + CEString::sToString(eNumber);
    sSelect += L" AND d.inflection_type = " + CEString::sToString(iType);
    sSelect += L" AND d.is_reflexive = ";
    sSelect += (REFL_YES == m_pLexeme->eIsReflexive()) ? L"1" : L"0";

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
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eSelect -- imperative
