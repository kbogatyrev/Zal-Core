#include "StdAfx.h"
#include "FormBuilderBaseDecl.h"

HRESULT CFormBuilderBaseDecl::hFleetingVowelCheck (ET_Number eNumber,               // in
                                                   ET_Case eCase,                   // in
                                                   ET_Gender eGender,               // in
                                                   ET_StressLocation eStressType,   // in
                                                   ET_Subparadigm eSubparadigm,     // in
                                                   CEString& sEnding,               // in/out
                                                   CEString& sLemma)                // out
{
    HRESULT hr = S_OK;

    if (!m_bFleetingVowel)
    {
        return S_OK;
    }

    int iType = m_pLexeme->m_iType;
    CEString& sInflection = m_pLexeme->m_sInflectionType;

    //
    // Determine if the fleeting vowel is in initial form GDRL, p. 29-30
    //

    // Fl vowel is in the source form: m. (except for the likes of мужчина), f. type 8*,
    // and pronominal decl.

    m_bFleetingVowelPresent = true;

    if (POS_NOUN == m_pLexeme->m_ePartOfSpeech)
    {
        if (GENDER_M == eGender || (GENDER_F == eGender && 8 == m_pLexeme->m_iType))
        {
            if (L"мн." != m_pLexeme->m_sMainSymbol)
            {
                hFleetingVowelRemove (eGender, eSubparadigm, sEnding, sLemma);
            }
            return S_OK;
        }
    }
    if (m_pLexeme->m_ePartOfSpeech == POS_PRONOUN_ADJ)
    {
        hFleetingVowelRemove (eGender, eSubparadigm, sEnding, sLemma);
        return S_OK;
    }

    if (2 == m_pLexeme->m_iType && AT_A == m_pLexeme->m_eAccentType1)
    {
        if (POS_NOUN == m_pLexeme->m_ePartOfSpeech && 
            GENDER_F == eGender && 
            NUM_PL == eNumber && 
            CASE_GEN == eCase)
        {                               // башен
//            if (L"ь" != sEnding)
//            {
//                ATLASSERT(0);
//                CEString sMsg (L"Unexpected ending.");
//                ERROR_LOG (sMsg);
//                throw CException (E_UNEXPECTED, sMsg);
//            }
            if (m_pLexeme->m_sSourceForm.bEndsWith (L"ня"))
            {
                if (L"ь" == sEnding)
                {
                    sEnding.Erase();
                }
                else
                {
                    return S_OK;    // сходней
                }
            }
        }
        if (POS_ADJ == m_pLexeme->m_ePartOfSpeech &&
            SUBPARADIGM_SHORT_ADJ == eSubparadigm &&
            NUM_SG == eNumber &&
            GENDER_M == eGender)
        {
            if (L"ь" != sEnding)
            {
                ATLASSERT(0);
                CEString sMsg (L"Unexpected ending.");
                ERROR_LOG (sMsg);
                throw CException (E_UNEXPECTED, sMsg);
            }
            sEnding.Erase();
        }
    }

    if (SUBPARADIGM_SHORT_ADJ == eSubparadigm && m_pLexeme->m_bNoLongForms)
    {
        hr = hFleetingVowelRemove (eGender, eSubparadigm, sEnding, sLemma);
        return hr;
    }

    // Fl vowel is NOT in the source form: f. other than 8*, n., adj. declension,
    // nouns of the мн. type

    m_bFleetingVowelPresent = false;

    if (POS_NOUN == m_pLexeme->m_ePartOfSpeech)
    {
        if ((GENDER_F == eGender && 8 != iType) || GENDER_N == eGender)
        {
            if (GENDER_F == eGender && 2 == iType && STRESS_LOCATION_ENDING == eStressType)
            {
                return S_OK;        // пятерня; why are those listed with "*"?
            }
            hFleetingVowelAdd (eNumber, eCase, eGender, eStressType, eSubparadigm, sLemma);
            return S_OK;
        }
        if (L"мн." == m_pLexeme->m_sMainSymbol)
        {
            hFleetingVowelAdd (eNumber, eCase, eGender, eStressType, eSubparadigm, sLemma);
            return S_OK;
        }
    }
    if ((SUBPARADIGM_SHORT_ADJ == eSubparadigm && !m_pLexeme->m_bNoLongForms) || 
        SUBPARADIGM_PART_PRES_PASS_SHORT == m_eSubparadigm ||
        SUBPARADIGM_PART_PAST_PASS_SHORT == m_eSubparadigm)
    {
        hFleetingVowelAdd (eNumber, eCase, eGender, eStressType, eSubparadigm, sLemma);
        return S_OK;
    }        

    ATLASSERT(0);
    CEString sMsg (L"Unable to determine fleeting vowel type.");
    ERROR_LOG (sMsg);
    throw CException (E_FAIL, sMsg);

}   //  hFleetingVowelCheck (...)

HRESULT CFormBuilderBaseDecl::hFleetingVowelAdd (ET_Number eNumber,                   // in
                                                 ET_Case eCase,                       // in
                                                 ET_Gender eGender,                   // in
                                                 ET_StressLocation eStressType,       // in
                                                 ET_Subparadigm eSubparadigm,         // in
                                                 CEString& sLemma)                    // out
{
    HRESULT hr = S_OK;

    CEString& sInflection = m_pLexeme->m_sInflectionType;
    int iType = m_pLexeme->m_iType;
    CEString sGrStem;
    if (SUBPARADIGM_PART_PRES_PASS_SHORT == eSubparadigm ||
        SUBPARADIGM_PART_PAST_PASS_SHORT == eSubparadigm)
    {
        sGrStem = sLemma;
    }
    else
    {
        sGrStem = m_pLexeme->m_sGraphicStem;
    }

    m_bFleetingVowelPresent = false;

    // Fem nouns except i-stems: G Pl (and A PL if animate);
    // adjectives (non-pronominal paradigm) short form m;
    // nouns with main symbol "mn."

    if (POS_NOUN == m_pLexeme->m_ePartOfSpeech)
    {
        if (NUM_PL != eNumber)
        {
            return S_OK;
        }

        if (eGender == GENDER_N && bHasCommonDeviation (2))
        {
            return S_OK;
        }

        if (eCase == CASE_ACC)
        {

            if (m_pEndings->iCount (StEndingDescriptor (GENDER_UNDEFINED, 
                                                        NUM_PL, 
                                                        CASE_ACC, 
                                                        ANIM_UNDEFINED, 
                                                        eStressType)) != 1)
            {
                CEString sMsg (L"Wrong number of endings.");
                ERROR_LOG (sMsg);
                throw CException (E_UNEXPECTED, sMsg);
            }

            CEString sEndingPlAcc;
            hr = m_pEndings->hGetEnding (StEndingDescriptor (GENDER_UNDEFINED, 
                                                             NUM_PL, 
                                                             eCase, 
                                                             ANIM_UNDEFINED, 
                                                             eStressType), 
                                                             sEndingPlAcc);

            CEString sEndingPlGen;
            hr = m_pEndings->hGetEnding (StEndingDescriptor (GENDER_UNDEFINED, 
                                                             NUM_PL, 
                                                             CASE_GEN, 
                                                             ANIM_UNDEFINED, 
                                                             eStressType),
                                                             sEndingPlGen);
            if (sEndingPlAcc != sEndingPlGen)
            {
                return S_OK;
            }
        }
        else
        {
            if (eCase != CASE_GEN)
            {
                return S_OK;
            }
        }
    }       // if (POS_NOUN == m_pLexeme->m_ePartOfSpeech)

    if (SUBPARADIGM_SHORT_ADJ == eSubparadigm || 
        SUBPARADIGM_PART_PRES_PASS_SHORT == eSubparadigm ||
        SUBPARADIGM_PART_PAST_PASS_SHORT == eSubparadigm)
    {
        if (GENDER_M != eGender)
        {
            return S_OK;
        }
    }

    m_bFleetingVowelPresent = true;

    if (((GENDER_F == eGender) || (GENDER_N == eGender)) && (6 == iType))
    {
        if (STRESS_LOCATION_ENDING == eStressType)
        {
            sLemma[sLemma.uiLength()-1] = L'е';   // статей, питей
        }
        else
        {
            sLemma[sLemma.uiLength()-1] = L'и';   // гостий, ущелий
        }
        return S_OK;
    }

    unsigned int uiLastConsonant = sGrStem.uiFindLastOf (g_szConsonants);
    if (ecNotFound == uiLastConsonant)
    {
        ATLASSERT(0);
        CEString sMsg (L"Can't find last consonant.");
        ERROR_LOG (sMsg);
        throw CException (E_UNEXPECTED, sMsg);
    }

    if ((L'ь' == sGrStem[uiLastConsonant-1]) || 
        (L'й' == sGrStem[uiLastConsonant-1]))
    {
        if (L'ц' == sGrStem[uiLastConsonant])
        {
            sLemma[uiLastConsonant-1] = L'е';    // колец
            return S_OK;
        }

        if (STRESS_LOCATION_ENDING == eStressType)
        {
            sLemma[uiLastConsonant-1] = L'ё';     // серёг, каём
            return S_OK;
        }
        else
        {
            sLemma[uiLastConsonant-1] = L'е';       // шпилек, чаек, писем
            return S_OK;
        }
    }
    if (L'к' == sGrStem[uiLastConsonant-1] || 
        L'г' == sGrStem[uiLastConsonant-1] ||
        L'х' == sGrStem[uiLastConsonant-1])
    {
        sLemma.sInsert (uiLastConsonant, L'о');  // кукол, окон, мягок
        return S_OK;
    }
    if (L'к' == sGrStem[uiLastConsonant] ||
        L'г' == sGrStem[uiLastConsonant] ||
        L'х' == sGrStem[uiLastConsonant])
    {
        if (3 != iType)
        {
            ATLASSERT(0);
            CEString sMsg (L"Unexpected type.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        if (!CEString::bIn (sGrStem[uiLastConsonant-1], L"шжчщц"))
        {
            sLemma.sInsert (uiLastConsonant, L'о');  // сказок, ведерок, краток, долог
            return S_OK;
        }
    }
    if (L'ц' == sGrStem[uiLastConsonant])
    {
        sLemma.sInsert (uiLastConsonant, L'е');      // овец
        return S_OK;
    }

    if (STRESS_LOCATION_ENDING == eStressType)
    {
        if (CEString::bIn (sGrStem[uiLastConsonant-1], L"шжчщ"))   // + ц?
        {
            sLemma.sInsert (uiLastConsonant, L'о');    // кишок
            return S_OK;
        }
        else
        {
            sLemma.sInsert (uiLastConsonant, L'ё');    // сестёр, хитёр
            return S_OK;
        }
    }
    else
    {
        sLemma.sInsert (uiLastConsonant, L'е');       // сосен, чисел, ножен, верен
    }

    return S_OK;

}   //  h_FleetingVowelAdd (...)

HRESULT CFormBuilderBaseDecl::hFleetingVowelRemove (ET_Gender eGender,               // in
                                                    ET_Subparadigm eSubparadigm,     // in
                                                    const CEString& sEnding,         // in
                                                    CEString& sLemma)                // out
{
    HRESULT hr = S_OK;

    int iType = m_pLexeme->m_iType;
    CEString& sGrStem = m_pLexeme->m_sGraphicStem;

    m_bFleetingVowelPresent = true;

    // Forms that have the fleeting vowel (all from the initial form except -ью):
    // m.: NULL (сон, конец)
    // m.: -ь (кашель)
    // f.: -ь, -ью (любовь)
    // pron.: NULL, -ь, -й (весь, волчий)

    // ASSERT: last stem vowel is { о, е, ё, и } [also а, я "in several anomalous words"]

    //
    // No vowel in ending: keep the fleeting vowel
    //
    if (sEnding.bIsEmpty())
    {
        if (GENDER_M != eGender)
        {
            ATLASSERT(0);
            CEString sMsg (L"Unexpected gender for null ending.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }
        return S_OK;
    }

    if (L"ь" == sEnding)
    {
    //    if (L"мс" != str_inflection && !(GENDER_F == eGender && 8 == iType))
    //    {
    //        ATLASSERT(0);
    //        wstring sMsg (L"Unexpected type for ending 'ь'."); .. NO!! cf кашель
    //        ERROR_LOG (sMsg);
    //        throw CT_Exception (E_UNEXPECTED, sMsg);
    //    }

        return S_OK;
    }

    if (L"ью" == sEnding)
    {
        if (GENDER_F != eGender || 8 != iType)
        {
            ATLASSERT(0);
            CEString sMsg (L"Unexpected type for ending 'ью'.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }
        return S_OK;
    }

    if (L"й" == sEnding)
    {
/*
        if (L"мс-п" != m_pLexeme->m_sInflectionType)
        {
            ATLASSERT(0);
            CEString sMsg (L"Unexpected type for ending 'й'.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }
*/
        return S_OK;
    }

    unsigned int uiLastVowel = sGrStem.uiFindLastOf (g_szVowels);

    if (ecNotFound == uiLastVowel)
    {
        sLemma.Erase();
        ATLASSERT(0);
        CEString sMsg (L"Vowel expected.");
        ERROR_LOG (sMsg);
        throw CException (E_UNEXPECTED, sMsg);
    }

    if (sGrStem[uiLastVowel] == L'о')   // сон, любовь
    {
        m_bFleetingVowelPresent = false;
        sLemma.sErase (uiLastVowel, 1);
        return S_OK;
    }

    if (sGrStem[uiLastVowel] == L'и' ||
        (sGrStem[uiLastVowel] == L'е' && POS_PRONOUN_ADJ == m_pLexeme->m_ePartOfSpeech))   // чей, волчий
    {
        if ((L"мс-п" != m_pLexeme->m_sInflectionType) || (1 != iType && 2 != iType && 6 != iType))
        {
            ATLASSERT(0);
            CEString sMsg (L"Unexpected inflection type.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }

        m_bFleetingVowelPresent = false;

        if (1 == iType)        // один
        {
            sLemma.sErase (uiLastVowel, 1);
        }
        else if (2 == iType)   // весь
        {
            sLemma.sErase (uiLastVowel, 1);
            // Palatalize the ending
            if (L"о" == sEnding)
            {
                //sEnding = L"ё";
            }
            if (L"а" == sEnding)
            {
                //sEnding = L"я";
            }
        }
        else if (6 == iType)   // чей, волчий
        {
            sLemma[uiLastVowel] = L'ь';
        }
        return S_OK;
    }

    if ((sGrStem[uiLastVowel] == L'е') || (sGrStem[uiLastVowel] == L'ё'))
    {
        if (uiLastVowel > 0)
        {
            //if (ecNotFound != CEString (szVowels).uiFind (sGrStem[uiLastVowel-1]))
            if (CEString::bIn (sGrStem[uiLastVowel-1], g_szVowels))
            {
                sLemma[uiLastVowel] = L'й';   // боец, паек
                return S_OK;
            }
        }
        if (GENDER_M == eGender)
        {
            if (6 == iType)                    // улей
            {
                sLemma = sGrStem;
                sLemma[uiLastVowel] = L'ь';
                return S_OK;
            }
            if (3 == iType)
            {
                if ((uiLastVowel > 0) && 
                    (!CEString::bIn (sGrStem[uiLastVowel-1], L"шжчщц")))
                {
                    sLemma[uiLastVowel] = L'ь';   // зверек
                    return S_OK;
                }
            }
            if ((uiLastVowel > 0) && (L'л' == sGrStem[uiLastVowel-1]))
            {
                sLemma[uiLastVowel] = L'ь';       // лед, палец
                return S_OK;
            }
        }
        
        // Special case == #11; 
        if (m_pLexeme->m_bNoLongForms)
        {
            if (sGrStem[uiLastVowel] == L'е' && 
                SUBPARADIGM_SHORT_ADJ == eSubparadigm && 
                GENDER_M != eGender)
            {
                sLemma[uiLastVowel] = L'ь';       // беленек, GDRL, pp 75 - 76
            }
            return S_OK;
        }

        sLemma.sErase (uiLastVowel, 1);   // default -- just remove the vowel

        return S_OK;
    
    }   // е or ё

    ATLASSERT(0);
    CEString sMsg (L"Function logic error.");
    ERROR_LOG (sMsg);
    throw CException (E_UNEXPECTED, sMsg);

}   //  h_FleetingVowelRemove (...)

HRESULT CFormBuilderBaseDecl::hGetStemStressPositions (const CEString& sLemma, 
                                                       ET_Subparadigm eSubParadigm,
                                                       vector<int>& vecPositions)
{
    HRESULT hr = S_OK;

    int iSyllablesInStem = sLemma.uiNSyllables();

    vector<int>::iterator itPos = m_pLexeme->m_vecSourceStressPos.begin();
    for (; AT_D != m_pLexeme->m_eAccentType1 &&
           itPos != m_pLexeme->m_vecSourceStressPos.end() && 
           *itPos >= 0 &&          // that should be always true? 
           *itPos < iSyllablesInStem; 
         ++itPos)
    {
        vecPositions.push_back (*itPos);
    }

    if (m_pLexeme->m_vecSourceStressPos.end() == itPos)
    {
        return S_OK;
    }

    //
    // Source stress is on ending:
    //
	int iNSyll = sLemma.uiGetNumOfSyllables();
    if (iNSyll < 1)
    {
        ATLASSERT(0);
        CEString sMsg (L"i_NSyllables() failed.");
        ERROR_LOG (sMsg);
        throw CException (E_FAIL, sMsg);
    }

    //
    // Trivial
    //
    if (1 == iNSyll)
    {
        vecPositions.push_back (0);
        return S_OK;
    }

    //
    // 2 or more syllables
    //
    ET_AccentType eAccentType = (SUBPARADIGM_SHORT_ADJ == eSubParadigm ||
                                 SUBPARADIGM_PART_PRES_PASS_SHORT == eSubParadigm ||
                                 SUBPARADIGM_PART_PAST_PASS_SHORT == eSubParadigm)
                                    ? m_pLexeme->m_eAccentType2 
                                    : m_pLexeme->m_eAccentType1;
    switch (eAccentType)
    {
        case AT_A:
        case AT_A1:
        case AT_B:
        {
            if (AT_A == eAccentType && 1 == m_pLexeme->m_iStemAugment)  // армянин
            {
                vecPositions.push_back (iNSyll - 1);
                return S_OK;
            }

            ATLASSERT(0);
            CEString sMsg (L"Unexpected accent type.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }
        case AT_B1:
        {
            if (m_bFleetingVowelPresent)
            {
                vecPositions.push_back (iNSyll - 2);
            }
            else
            {
                vecPositions.push_back (iNSyll - 1);
            }
            return S_OK;
        }
        case AT_C:
        case AT_C1:
        {
            if (AT_C == eAccentType && 1 == m_pLexeme->m_iStemAugment)  // армянин
            {
                vecPositions.push_back (iNSyll - 1);
                return S_OK;
            }
            ATLASSERT(0);
            CEString sMsg (L"Unexpected accent type.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }
        case AT_D:
        case AT_D1:
        {
            if (m_bFleetingVowelPresent)
            {
                vecPositions.push_back (iNSyll - 2);
            }
            else
            {
                vecPositions.push_back (iNSyll - 1);
            }
            return S_OK;
        }
        case AT_E:
        {
            ATLASSERT(0);
            CEString sMsg (L"Unexpected accent type.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }
        case AT_F:
        case AT_F1:
        case AT_F2:
        {
            vecPositions.push_back (0);
            return S_OK;
        }
        default:
        {
            ATLASSERT(0);
            CEString sMsg (L"Unexpected accent type.");
            ERROR_LOG (sMsg);
            throw CException (E_UNEXPECTED, sMsg);
        }
    }

    return S_OK;

}   //  hGetStemStressPositions (...)

HRESULT CFormBuilderBaseDecl::hGetEndingStressPosition (const CEString& sLemma, 
                                                        const CEString& sEnding,
                                                        int& iPosition)
{
    if (sEnding.uiNSyllables() > 0)
    {
        iPosition = sLemma.uiNSyllables();
    }
    else
    {
        iPosition = sLemma.uiNSyllables() - 1;
        if (iPosition < 0)
        {
            ATLASSERT(0);
            CEString sMsg (L"Warning: can't find stressed vowel.");
            ERROR_LOG (sMsg);
            throw CException (E_FAIL, sMsg);
        }
    }

    return S_OK;

}   //  hGetEndingStressPosition (...)

HRESULT CFormBuilderBaseDecl::hHandleYoAlternation (ET_StressLocation eStressType, 
                                                    int iStressedSyll,
                                                    CEString& sLemma)
{
    HRESULT hr = S_OK;

    if (!m_bYoAlternation)
    {
        return S_OK;
    }

    if (STRESS_LOCATION_UNDEFINED == eStressType)
    {
        ATLASSERT(0);
        CEString sMsg (L"Undefined stress type.");
        ERROR_LOG (sMsg);
        throw CException (E_INVALIDARG, sMsg);
    }

    CEString sGraphicStem = (POS_VERB == m_pLexeme->m_ePartOfSpeech) 
        ? sLemma // for participles, graphic stem is participial stem
        : m_pLexeme->m_sGraphicStem;

    int iYoOffset = sGraphicStem.uiFind (_T("ё"));
    if (ecNotFound != iYoOffset)
    {
        // Graphic stem contains yo: replace with e if stress is on ending
        if (STRESS_LOCATION_ENDING == eStressType)
        {
            if (iStressedSyll >= sLemma.uiNSyllables())
            {
                sLemma[iYoOffset] = L'е';        //  yo --> e: ежа, щелочей, тяжела
            }
        }
    }
    else
    {
        // Graphic stem has no yo (must have at least on e)
        if (STRESS_LOCATION_STEM == eStressType)
        {
            int iEOffset = sGraphicStem.uiRFind (_T("е"));    // last "e" in graphic stem (?)
            if (ecNotFound == iEOffset)
            {
                if (POS_VERB != m_pLexeme->m_ePartOfSpeech) 
                {
                    ATLASSERT(0);
                    CEString sMsg (L"Unstressed stem with yo alternation has no e.");
                    ERROR_LOG (sMsg);
                    throw CException (E_FAIL, sMsg);
                }
                else
                {
                    return S_OK;
                }
            }

            int iStressPos = sLemma.uiGetVowelPos (iStressedSyll);
            if (iStressPos < 0 || iStressPos > (int)sLemma.uiLength())
            {
                ATLASSERT(0);
                CEString sMsg (L"Bad stress position.");
                ERROR_LOG (sMsg);
                throw CException (E_FAIL, sMsg);
            }
            // Replace stressed e with yo:
            if (iEOffset == iStressPos)
            {
                sLemma[iEOffset] = L'ё';
            }
        }
    }

    return hr;

}   //  hHandleYoAlternation (...)
