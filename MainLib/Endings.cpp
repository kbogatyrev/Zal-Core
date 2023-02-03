#include "Logging.h"

#include "Lexeme.h"
#include "Inflection.h"
#include "SqliteWrapper.h"
#include "Endings.h"

// Тест типа

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

    pair<uint64_t, CEString> pairEndingData = m_vecEndings[iSeqNum];
    llEndingKey = pairEndingData.first;
    sEnding = pairEndingData.second;

    return H_NO_ERROR;
}

void CEndings::ReportDbError()
{
    if (!m_spLexeme)
    {
        ERROR_LOG(L"No lexeme handle.");
    }

    shared_ptr<CSqlite> spDb;
    CEString sMsg;
    try
    {
        spDb = m_spLexeme->spGetDb();
        CEString sError;
        spDb->GetLastError(sError);
        sMsg = L"DB error: ";
        sMsg += sError;
        sMsg += L"; code = ";
        sMsg += CEString::sToString(spDb->iGetLastError());
    }
    catch (...)
    {
        sMsg = L"Apparent DB error ";
    }

    sMsg += CEString::sToString(spDb->iGetLastError());
    ERROR_LOG(sMsg);

}   //  ReportDbError()

CNounEndings::CNounEndings(shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection) 
    : CEndings(spLexeme, spInflection)
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
    sSelectBase += CEString::sToString(m_spInflection->iType());
    sSelectBase += L" AND (gender = ";
    sSelectBase += CEString::sToString(m_spLexeme->eInflectionTypeToGender());
    sSelectBase += L" OR gender = ";
    sSelectBase += CEString::sToString(GENDER_UNDEFINED) + L")";
    sSelectBase += L" AND (animacy = ";
    sSelectBase += CEString::sToString(m_spLexeme->eInflectionTypeToAnimacy());
    sSelectBase += L" OR animacy = ";
    sSelectBase += CEString::sToString(ANIM_UNDEFINED) + L")";
    sSelectBase += L" AND stem_augment = ";
    auto iStemAugment = m_spInflection->iStemAugment();
    if (0 == iStemAugment)
    {
        iStemAugment = -1;   // Kludge: values 0 & -1 both signify no stem augment 
                             // but only -1 is used is the ending_data table
    }
//    sSelectBase += CEString::sToString(m_pLexeme->iStemAugment());
    sSelectBase += CEString::sToString(iStemAugment);
    sSelectBase += L" AND case_value = ";
    sSelectBase += CEString::sToString(eCase);
    sSelectBase += L" AND number = ";
    sSelectBase += CEString::sToString(eNumber);
    sSelectBase += L" AND (stress = 0 OR stress = ";
    sSelectBase += CEString::sToString(eStressPos);
    sSelectBase += L")";

    shared_ptr<CSqlite> spDb;
    try
    {
        spDb = m_spLexeme->spGetDb();

        CEString sSelect(sSelectBase);
        sSelect += L" AND common_deviation = 0;";
        spDb->PrepareForSelect(sSelect);
        while (spDb->bGetRow())
        {
            m_sEnding.Erase();
            spDb->GetData(0, (uint64_t&)m_ullDbKey);
            spDb->GetData(1, m_sEnding);

            // Skip if non-optional common deviation
            bool bIsOptional = false;
            if (m_spInflection->bFindCommonDeviation(1, bIsOptional))
            {
                if (NUM_PL == m_eNumber && CASE_NOM == m_eCase && GENDER_F != m_spLexeme->eInflectionTypeToGender())
                {
                    if (!bIsOptional)
                    {
                        continue;
                    }
                }
            }

            if (m_spInflection->bFindCommonDeviation(2, bIsOptional))
            {
                if (NUM_PL == m_eNumber && CASE_GEN == m_eCase)
                {
                    if (!bIsOptional)    // optional?
                    {
                        continue;
                    }
                }
            }

            m_vecEndings.push_back(pair<uint64_t, CEString>(m_ullDbKey, m_sEnding));

        }   //  while (pDb->bGetRow())

        spDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_ERROR_GENERAL;
    }

    for (int iCd = 1; iCd <= 2; ++iCd)
    {
        if (!m_spInflection->bHasCommonDeviation(iCd))
        {
            continue;
        }

        CEString sSelectCD(sSelectBase);
        sSelectCD += L" AND common_deviation = ";
        sSelectCD += CEString::sToString(iCd);
        sSelectCD += L";";

        try
        {
            spDb->PrepareForSelect(sSelectCD);
            while (spDb->bGetRow())
            {
                spDb->GetData(0, (uint64_t&)m_ullDbKey);
                spDb->GetData(1, m_sEnding);

                m_vecEndings.push_back(pair<uint64_t, CEString>(m_ullDbKey, m_sEnding));

            }
            spDb->Finalize();
        }
        catch (...)
        {
            ReportDbError();
            return H_ERROR_DB;
        }

    }   // for ...

    return H_NO_ERROR;

}       // select (nouns)

CAdjLongEndings::CAdjLongEndings(shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection, ET_Subparadigm eSubparadigm)
    : CEndings(spLexeme, spInflection), m_eSubparadigm(eSubparadigm)
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
        m_iInflectionType = m_spInflection->iType();
    }

    m_vecEndings.clear();

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = ";

    if (L"мс-п" == m_spLexeme->sInflectionType())
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
            if (AT_A == m_spInflection->eAccentType1() || AT_A1 == m_spInflection->eAccentType1())
            {
                m_eStressPos = STRESS_LOCATION_STEM;
            }
            else
            {
                if (AT_B == m_spInflection->eAccentType1())
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

    ET_Reflexivity eRefl = REFL_UNDEFINED;
    if (POS_VERB == m_spLexeme->ePartOfSpeech())
    {
        eRefl = m_spLexeme->eIsReflexive();
    }
    else if (SUBPARADIGM_LONG_ADJ == eSubparadigm)
    {
        if (m_spLexeme->sSourceForm().bEndsWith(L"ся"))
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

    shared_ptr<CSqlite> spDb = NULL;

    try
    {
        spDb = m_spLexeme->spGetDb();
        spDb->PrepareForSelect(sSelect);
        while (spDb->bGetRow())
        {
            spDb->GetData(0, (int64_t&)m_ullDbKey);
            spDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<uint64_t, CEString>(m_ullDbKey, m_sEnding));
        }
        spDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_ERROR_DB;
    }

    return H_NO_ERROR;

}       //  CAdjLongEndings::eSelect()

CAdjShortEndings::CAdjShortEndings(shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection, ET_Subparadigm eSubparadigm) 
    : CEndings(spLexeme, spInflection), m_eSubparadigm(eSubparadigm)
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

    if (SUBPARADIGM_PART_PAST_PASS_SHORT == m_eSubparadigm || SUBPARADIGM_PART_PRES_PASS_SHORT == m_eSubparadigm)
    {
        m_iInflectionType = 1;
    }
    else if (SUBPARADIGM_SHORT_ADJ == m_eSubparadigm )
    {
        m_iInflectionType = m_spInflection->iType();
    }
    else
    {
//        CEString sMsg(L"Unexpected subparadigm: must be short adj. or short part. past. pass.");
        ERROR_LOG(L"Unexpected subparadigm: must be short adj. or short part. past. pass.");
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

    shared_ptr<CSqlite> spDb;

    try
    {
        spDb = m_spLexeme->spGetDb();
        spDb->PrepareForSelect(sSelect);
        while (spDb->bGetRow())
        {
            spDb->GetData(0, (int64_t&)m_ullDbKey);
            spDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<uint64_t, CEString>(m_ullDbKey, m_sEnding));
        }
        spDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_ERROR_DB;
    }

    return H_NO_ERROR;

}       //  CAdjShortEndings::eSelect()

CAdjPronounEndings::CAdjPronounEndings(shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection) 
    : CEndings(spLexeme, spInflection)
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
    
    m_iInflectionType = m_spInflection->iType();
    m_vecEndings.clear();
    bool bIsStressRelevant = false;

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = ";
    sSelect += CEString::sToString(ENDING_CLASS_PRONOUN);
    sSelect += L" AND d.inflection_type = ";
    sSelect += CEString::sToString(m_iInflectionType);

    int iSubType = 0;

    if ((m_spLexeme->ePartOfSpeech() == POS_ADJ 
        || m_spLexeme->ePartOfSpeech() == POS_NOUN) && m_spLexeme->sInflectionType() == L"мс")     // only п <мс 1...> and п <мс 2...>
    {
        if (m_spLexeme->sGraphicStem().bEndsWithNoCase(L"ин"))
        {
            if (m_spInflection->eAccentType1() == ET_AccentType::AT_A)
            {
//                sSelect += L" AND d.adjective_pronominal_decl_subtype = 1";
                iSubType = 1;
            }
            else if (m_spInflection->eAccentType1() == ET_AccentType::AT_B)
            {
                iSubType = 2;
//                sSelect += L" AND d.adjective_pronominal_decl_subtype = 2";
                if ((GENDER_M == eGender || GENDER_N == eGender) && NUM_SG == eNumber)
                {
                    if (CASE_PREP == eCase)
                    {
                        bIsStressRelevant = true;
                    }
                }
            }
            else
            {
                ERROR_LOG(L"Unexpected accent paradigm of type 1 adjective with pronominal declension.");
                return H_ERROR_UNEXPECTED;
            }
        }
        else if (m_spLexeme->sGraphicStem().bEndsWithNoCase(L"ов") ||
            m_spLexeme->sGraphicStem().bEndsWithNoCase(L"ев") ||
            m_spLexeme->sGraphicStem().bEndsWithNoCase(L"ёв"))
        {
            if ((GENDER_M == eGender || GENDER_N == eGender) && NUM_SG == eNumber)
            {
                if (CASE_PREP == eCase)
                {
                    bIsStressRelevant = true;
                }
            }
            iSubType = 2;
//            sSelect += L" AND d.adjective_pronominal_decl_subtype = 2";
        }
        else if (m_spInflection->iType() == 2)       // господень
        {
            iSubType = 3;
//            sSelect += L" AND d.adjective_pronominal_decl_subtype = 3";
        }    
    }       //  п <мс 1...> and п <мс 2...>
    else
    {
        iSubType = 0;
    }

    sSelect += L" AND d.adjective_pronominal_decl_subtype = ";
    sSelect += CEString::sToString(iSubType);

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
        if (AT_A == m_spInflection->eAccentType1())
        {
            m_eStressPos = STRESS_LOCATION_STEM;
        }
        else
        {
            if (AT_B == m_spInflection->eAccentType1())
            {
                m_eStressPos = STRESS_LOCATION_ENDING;
            }
            else if (AT_F == m_spInflection->eAccentType1())      // only сам
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
    
    shared_ptr<CSqlite> spDb;

    try
    {
        spDb = m_spLexeme->spGetDb();
        spDb->PrepareForSelect(sSelect);
        while (spDb->bGetRow())
        {
            spDb->GetData(0, (int64_t&)m_ullDbKey);
            spDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<uint64_t, CEString>(m_ullDbKey, m_sEnding));
        }
        spDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_ERROR_DB;
    }

    return H_NO_ERROR;

}       //  CAdjPronounEndings::eSelect()

CPersonalEndings::CPersonalEndings(shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection) 
    : CEndings(spLexeme, spInflection)
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
    sSelect += L" AND ed.inflection_type = " + CEString::sToString(m_spInflection->iType());
    sSelect += L" AND ed.is_reflexive = ";
    sSelect += ((REFL_YES == m_spLexeme->eIsReflexive()) ? L"1" : L"0");

    if (m_spInflection->iType() != 4 && m_spInflection->iType() != 5)
    {
        if ((PERSON_1 != ePerson || NUM_SG != eNumber) &&
            (PERSON_3 != ePerson || NUM_PL != eNumber))
        {
            sSelect += L" AND ed.stress = " + CEString::sToString(eStressPos);
        }
    }

    if (m_spInflection->iType() == 4 || m_spInflection->iType() == 5 || m_spInflection->iType() == 6)
    {
        if ((PERSON_1 == ePerson && NUM_SG == eNumber) ||
            (PERSON_3 == ePerson && NUM_PL == eNumber))
        {
            if (m_spInflection->iType() == 6 && m_spInflection->iStemAugment() == 1)
            {
                sSelect += L" AND ed.stem_augment = 1";
            }
            else
            {
                sSelect += L" AND ed.stem_auslaut = " + CEString::sToString(eStemAuslaut);
            }
        }
    }
    
    shared_ptr<CSqlite> spDb;
    try
    {
        spDb = m_spLexeme->spGetDb();

        spDb->PrepareForSelect(sSelect);
        while (spDb->bGetRow())
        {
            spDb->GetData(0, (uint64_t&)m_ullDbKey);
            spDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<uint64_t, CEString>(m_ullDbKey, m_sEnding));
        }

        spDb->Finalize();
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
CInfinitiveEndings::CInfinitiveEndings(shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection) 
    : CEndings(spLexeme, spInflection)
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

    if (POS_VERB != m_spLexeme->ePartOfSpeech())
    {
        ERROR_LOG(L"Bad part of speech value.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = " + CEString::sToString(ENDING_CLASS_INFINITIVE);
    sSelect += L" AND d.inflection_type = " + CEString::sToString(iInflectionType);
    sSelect += L" AND d.is_reflexive = ";
    sSelect += (REFL_YES == m_spLexeme->eIsReflexive()) ? L"1" : L"0";

    shared_ptr<CSqlite> spDb;

    try
    {
        spDb = m_spLexeme->spGetDb();

        spDb->PrepareForSelect(sSelect);
        while (spDb->bGetRow())
        {
            spDb->GetData(0, (uint64_t&)m_ullDbKey);
            spDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<uint64_t, CEString>(m_ullDbKey, m_sEnding));
        }

        spDb->Finalize();
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
CPastTenseEndings::CPastTenseEndings(shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection) 
    : CEndings(spLexeme, spInflection)
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

    if (POS_VERB != m_spLexeme->ePartOfSpeech())
    {
        ERROR_LOG(L"Bad part of speech value.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sSelect (L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = " + CEString::sToString(ENDING_CLASS_PAST_TENSE);
    sSelect += L" AND d.gender = " + CEString::sToString(eGender);
    sSelect += L" AND d.number = " + CEString::sToString(eNumber);
    sSelect += L" AND d.is_reflexive = ";
    sSelect += (REFL_YES == m_spLexeme->eIsReflexive()) ? L"1" : L"0";

    if (NUM_SG == eNumber && GENDER_M == eGender)
    {
        sSelect += L" AND d.stem_auslaut = " + CEString::sToString(eAuslaut);
    }

    shared_ptr<CSqlite> spDb;

    try
    {
        spDb = m_spLexeme->spGetDb();

        spDb->PrepareForSelect(sSelect);
        while (spDb->bGetRow())
        {
            spDb->GetData(0, (uint64_t&)m_ullDbKey);
            spDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<uint64_t, CEString>(m_ullDbKey, m_sEnding));
        }

        spDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eSelect() -- past tense

CImperativeEndings::CImperativeEndings(shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection) 
    : CEndings(spLexeme, spInflection)
{}

ET_ReturnCode CImperativeEndings::eSelect(ET_Number eNumber, int iType, bool bIsVariant)
{
    if (!bIsVariant)
    {
        m_vecEndings.clear();
    }

    if (POS_VERB != m_spLexeme->ePartOfSpeech())
    {
        ERROR_LOG(L"Bad part of speech value.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = " + CEString::sToString(ENDING_CLASS_IMPERATIVE);
    sSelect += L" AND d.number = " + CEString::sToString(eNumber);
    sSelect += L" AND d.inflection_type = " + CEString::sToString(iType);
    sSelect += L" AND d.is_reflexive = ";
    sSelect += (REFL_YES == m_spLexeme->eIsReflexive()) ? L"1" : L"0";

    shared_ptr<CSqlite> spDb;

    try
    {
        spDb = m_spLexeme->spGetDb();

        spDb->PrepareForSelect(sSelect);
        while (spDb->bGetRow())
        {
            spDb->GetData(0, (uint64_t&)m_ullDbKey);
            spDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<uint64_t, CEString>(m_ullDbKey, m_sEnding));
        }

        spDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  eSelect -- imperative

CAdverbialEndings::CAdverbialEndings(shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection) 
    : CEndings(spLexeme, spInflection)
{}

/*
CREATE TABLE ending_data (id INTEGER PRIMARY KEY ASC, ending_id INTEGER, inflection_class INTEGER, inflection_type INTEGER, 
case_value INTEGER, person INTEGER, gender INTEGER, animacy INTEGER, stress INTEGER, number INTEGER, stem_augment BOOLEAN DEFAULT (0), 
common_deviation INTEGER, conjugation INTEGER, tense INTEGER, is_reflexive BOOLEAN DEFAULT(0), stem_auslaut INTEGER, 
FOREIGN KEY (ending_id) REFERENCES endings (id));
*/

ET_ReturnCode CAdverbialEndings::eSelect(ET_Subparadigm eSubparadigm, bool bHusherStem, bool bVStem, bool bIsVariant)
{
    if (!bIsVariant)
    {
        m_vecEndings.clear();
    }

    if (POS_VERB != m_spLexeme->ePartOfSpeech())
    {
        ERROR_LOG(L"Bad part of speech value.");
        return H_ERROR_UNEXPECTED;
    }

    ET_Tense eTense = ET_Tense::TENSE_UNDEFINED;
    if (ET_Subparadigm::SUBPARADIGM_ADVERBIAL_PRESENT == eSubparadigm)
    { 
        eTense = ET_Tense::TENSE_PRESENT;
    }
    else if (ET_Subparadigm::SUBPARADIGM_ADVERBIAL_PAST == eSubparadigm)
    {
        eTense = ET_Tense::TENSE_PAST;
    }
    else
    {
        ERROR_LOG(L"Bad subparadigm value.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = ";
    sSelect += CEString::sToString(ENDING_CLASS_ADVERBIAL);

    sSelect += L" AND d.tense = ";
    sSelect += CEString::sToString(eTense);

    if (ET_Tense::TENSE_PRESENT == eTense)
    {
        sSelect += L" AND d.stem_auslaut = ";
        sSelect += bHusherStem ? CEString::sToString(STEM_AUSLAUT_SH) : CEString::sToString(STEM_AUSLAUT_NOT_SH);
    }

    if (ET_Tense::TENSE_PAST == eTense)
    {
//        sSelect += L" AND d.stem_augment = ";
//        sSelect += CEString::sToString(m_pLexeme->iStemAugment());

        sSelect += L" AND d.stem_auslaut = ";
        if (bVStem)
        {
            sSelect += CEString::sToString(STEM_AUSLAUT_V);
        }
        else
        {
            sSelect += CEString::sToString(STEM_AUSLAUT_NOT_V);
        }
    }

    sSelect += L" AND d.is_reflexive = ";
    sSelect += (REFL_YES == m_spLexeme->eIsReflexive()) ? L"1" : L"0";

    sSelect += L" AND d.inflection_type = -1";

    shared_ptr<CSqlite> spDb;
    try
    {
        spDb = m_spLexeme->spGetDb();

        spDb->PrepareForSelect(sSelect);
        while (spDb->bGetRow())
        {
            spDb->GetData(0, (uint64_t&)m_ullDbKey);
            spDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<uint64_t, CEString>(m_ullDbKey, m_sEnding));
        }

        spDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  CAdverbialEndings::eSelect()

ET_ReturnCode CAdverbialEndings::eSelectPastAdvAugmentedEndings(ET_Subparadigm eSubparadigm, bool bIsVariant)
{
    if (!bIsVariant)
    {
        m_vecEndings.clear();
    }

    if (POS_VERB != m_spLexeme->ePartOfSpeech())
    {
        ERROR_LOG(L"Bad part of speech value.");
        return H_ERROR_UNEXPECTED;
    }

    if (eSubparadigm != ET_Subparadigm::SUBPARADIGM_ADVERBIAL_PAST)
    {
        ERROR_LOG(L"Bad subparadigm value.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = ";
    sSelect += CEString::sToString(ENDING_CLASS_ADVERBIAL);

    sSelect += L" AND d.tense = ";
    sSelect += CEString::sToString(ET_Tense::TENSE_PAST);

    sSelect += L" AND d.is_reflexive = 0";

    if (9 == m_spInflection->iType() || 3 == m_spInflection->iType())
    {
        sSelect += L" AND d.inflection_type = ";
        sSelect += CEString::sToString(m_spInflection->iType());
    }

    if (3 == m_spInflection->iType())
    {
        if (m_spInflection->iStemAugment() != 1)
        {
            CEString sMsg(L"Stem augment expected for type 3 verb ");
            sMsg += m_spLexeme->sInfinitive();
            sMsg += L".";
            wchar_t * szMsg = sMsg;
            ERROR_LOG(szMsg);
            return H_ERROR_UNEXPECTED;
        }

        sSelect += L" AND d.stem_augment = 1";
    }

    shared_ptr<CSqlite> spDb;
    try
    {
        spDb = m_spLexeme->spGetDb();

        spDb->PrepareForSelect(sSelect);
        while (spDb->bGetRow())
        {
            spDb->GetData(0, (uint64_t&)m_ullDbKey);
            spDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<uint64_t, CEString>(m_ullDbKey, m_sEnding));
        }

        spDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  CAdverbialEndings::eSelect()

CComparativeEndings::CComparativeEndings(shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection)
    : CEndings(spLexeme, spInflection)
{}

/*
CREATE TABLE ending_data (id INTEGER PRIMARY KEY ASC, ending_id INTEGER, inflection_class INTEGER, inflection_type INTEGER,
case_value INTEGER, person INTEGER, gender INTEGER, animacy INTEGER, stress INTEGER, number INTEGER, stem_augment BOOLEAN DEFAULT (0),
common_deviation INTEGER, conjugation INTEGER, tense INTEGER, is_reflexive BOOLEAN DEFAULT(0), stem_auslaut INTEGER,
FOREIGN KEY (ending_id) REFERENCES endings (id));
*/

ET_ReturnCode CComparativeEndings::eSelect(bool bVelarStem, bool bIsVariant)
{
    m_eStemAuslaut = bVelarStem ? STEM_AUSLAUT_VELAR : STEM_AUSLAUT_NOT_VELAR;

    if (!bIsVariant)
    {
        m_vecEndings.clear();
    }

    if (POS_ADJ != m_spLexeme->ePartOfSpeech())
    {
        ERROR_LOG(L"Bad part of speech value.");
        return H_ERROR_UNEXPECTED;
    }

    CEString sSelect(L"SELECT DISTINCT d.id, e.ending_string FROM endings AS e INNER JOIN ending_data AS d ON (e.id = d.ending_id)");
    sSelect += L" WHERE d.inflection_class = ";
    sSelect += CEString::sToString(ENDING_CLASS_COMPARATIVE);
    sSelect += L" AND d.inflection_type = -1";
    sSelect += L" AND stem_auslaut = ";
    sSelect += CEString::sToString(m_eStemAuslaut);

    shared_ptr<CSqlite> spDb;
    try
    {
        spDb = m_spLexeme->spGetDb();

        spDb->PrepareForSelect(sSelect);
        while (spDb->bGetRow())
        {
            spDb->GetData(0, (uint64_t&)m_ullDbKey);
            spDb->GetData(1, m_sEnding);

            m_vecEndings.push_back(pair<uint64_t, CEString>(m_ullDbKey, m_sEnding));
        }

        spDb->Finalize();
    }
    catch (...)
    {
        ReportDbError();
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}   //  CComparativeEndings::eSelect()
