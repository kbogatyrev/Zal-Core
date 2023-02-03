#include "WordForm.h"
#include "Lexeme.h"
#include "FormBuilderBaseDecl.h"

using namespace Hlib;

ET_ReturnCode CFormBuilderDecl::eFleetingVowelCheck (ET_Number eNumber,               // in
                                                     ET_Case eCase,                   // in
                                                     ET_Gender eGender,               // in
                                                     ET_StressLocation eStressType,   // in
                                                     ET_Subparadigm eSubparadigm,     // in
                                                     CEString& sEnding,               // in/out
                                                     CEString& sStem)                 // out
{
	assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    if (!m_bFleetingVowel)
    {
        return H_NO_ERROR;
    }

    //
    // Determine if the fleeting vowel is in initial form GDRL, p. 29-30
    //

    // Fl vowel is in the source form: m. (except for the likes of мужчина), f. type 8*,
    // and pronominal decl.

    m_bFleetingVowelPresent = true;

    if (POS_NOUN == m_spLexeme->ePartOfSpeech())
    {
        if (GENDER_M == eGender || (GENDER_F == eGender && 8 == m_spInflection->iType()))
        {
            if (L"мн." != m_spLexeme->sMainSymbol() && !m_spLexeme->bIsPluralOf())
            {
                rc = eFleetingVowelRemove (eGender, eSubparadigm, sEnding, sStem);
                return rc;
            }
        }
    }

    if (m_spLexeme->ePartOfSpeech() == POS_PRONOUN_ADJ || m_spLexeme->ePartOfSpeech() == POS_NUM_ADJ || 
        ((m_spLexeme->ePartOfSpeech() == POS_ADJ || m_spLexeme->ePartOfSpeech() == POS_NOUN) && L"мс" == m_spLexeme->sInflectionType()))
    {
        rc = eFleetingVowelRemove (eGender, eSubparadigm, sEnding, sStem);
        return rc;
    }

    if (2 == m_spInflection->iType() && AT_A == m_spInflection->eAccentType1())
    {
        if (POS_NOUN == m_spLexeme->ePartOfSpeech() && 
            GENDER_F == eGender && 
            NUM_PL == eNumber && 
            CASE_GEN == eCase)
        {                               // башен
//            if (L"ь" != sEnding)
//            {
//                assert(0);
//                CEString sMsg (L"Unexpected ending.");
//                ERROR_LOG (sMsg);
//                throw CException (H_ERROR_UNEXPECTED, sMsg);
//            }
            if (m_spLexeme->sSourceForm().bEndsWith (L"ня"))
            {
                if (L"ь" == sEnding)
                {
                    sEnding.Erase();
                }
            }
        }
        if (POS_ADJ == m_spLexeme->ePartOfSpeech() &&
            SUBPARADIGM_SHORT_ADJ == eSubparadigm &&
            NUM_SG == eNumber &&
            GENDER_M == eGender)
        {
            if (L"ь" != sEnding)
            {
				assert(0);
                CEString sMsg (L"Unexpected ending.");
                ERROR_LOG (sMsg);
                throw CException (H_ERROR_UNEXPECTED, sMsg);
            }
            sEnding.Erase();
        }
    }

    if (SUBPARADIGM_SHORT_ADJ == eSubparadigm && m_spLexeme->bNoLongForms())
    {
        rc = eFleetingVowelRemove (eGender, eSubparadigm, sEnding, sStem);
        return rc;
    }

    // Fl vowel is NOT in the source form: f. other than 8*, n., adj. declension,
    // nouns of the мн. type

    m_bFleetingVowelPresent = false;

    if (!(0 == sEnding.uiLength() || L"й" == sEnding || L"ь" == sEnding || L"ью" == sEnding))
    {
        return rc;
    }

    if (POS_NOUN == m_spLexeme->ePartOfSpeech())
    {
        if ((GENDER_F == eGender && 8 != m_spInflection->iType()) || GENDER_N == eGender)
        {
            if (GENDER_F == eGender && 2 == m_spInflection->iType())
            {
                if (STRESS_LOCATION_ENDING == eStressType || L"ей" == sEnding)
                {
                    return H_NO_ERROR;        // пятерня (why are those listed with "*"); сходня
                }
            }
            if (eGender == GENDER_N && m_spInflection->bHasCommonDeviation(2))
            {
                if (!m_spInflection->bDeviationOptional(2))
                {
                    return H_NO_ERROR;
                }
                else if (!sEnding.bIsEmpty())
                {
                    return H_NO_ERROR;
                }
            }
            rc = eFleetingVowelAdd (eNumber, eCase, eGender, eStressType, eSubparadigm, sStem);
            return rc;
        }

        if (L"мн." == m_spLexeme->sMainSymbol() || m_spLexeme->bIsPluralOf())
        {
            if (0 == sEnding.uiLength() || L"й" == sEnding || L"ь" == sEnding || L"ью" == sEnding)
            {
                rc = eFleetingVowelAdd(eNumber, eCase, eGender, eStressType, eSubparadigm, sStem);
                return rc;
            }
        }
    }
    if ((SUBPARADIGM_SHORT_ADJ == eSubparadigm && !m_spLexeme->bNoLongForms()) || 
        SUBPARADIGM_PART_PRES_PASS_SHORT == m_eSubparadigm ||
        SUBPARADIGM_PART_PAST_PASS_SHORT == m_eSubparadigm)
    {
        rc = eFleetingVowelAdd (eNumber, eCase, eGender, eStressType, eSubparadigm, sStem);
        return rc;
    }        

	assert(0);
    ERROR_LOG (L"Unable to determine fleeting vowel type.");
    return H_ERROR_GENERAL;

}   //  eFleetingVowelCheck (...)

ET_ReturnCode CFormBuilderDecl::eFleetingVowelAdd (ET_Number eNumber,                   // in
                                                   ET_Case eCase,                       // in
                                                   ET_Gender eGender,                   // in
                                                   ET_StressLocation eStressType,       // in
                                                   ET_Subparadigm eSubparadigm,         // in
                                                   CEString& sStem)                     // out
{
	assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    const CEString& sInflection = m_spLexeme->sInflectionType();
//    int iType = m_pInflection->iType();
    CEString sGrStem;
    if (SUBPARADIGM_PART_PRES_PASS_SHORT == eSubparadigm ||
        SUBPARADIGM_PART_PAST_PASS_SHORT == eSubparadigm)
    {
        sGrStem = sStem;
    }
    else
    {
        sGrStem = m_spLexeme->sGraphicStem();
    }

    m_bFleetingVowelPresent = false;

    // Fem nouns except i-stems: G Pl (and A PL if animate);
    // adjectives (non-pronominal paradigm) short form m;
    // nouns with main symbol "mn."

    if (POS_NOUN == m_spLexeme->ePartOfSpeech())
    {
        if (NUM_PL != eNumber)
        {
            return H_NO_ERROR;
        }

        //if (eGender == GENDER_N && m_pLexeme->bHasCommonDeviation(2))
        //{
        //    if (!m_pLexeme->bDeviationOptional(2))
        //    {
        //        return H_NO_ERROR;
        //    }
        //}

        if (eCase == CASE_ACC)
        {
            rc = static_pointer_cast<CNounEndings>(m_spEndings)->eSelect(NUM_PL, CASE_ACC, eStressType);
            if (m_spEndings->iCount() != 1)
            {
                CEString sMsg (L"Wrong number of endings.");
                ERROR_LOG (sMsg);
                throw CException (H_ERROR_UNEXPECTED, sMsg);
            }

            CEString sEndingPlAcc;
            int64_t llEndingKey = -1;
            rc = static_pointer_cast<CNounEndings>(m_spEndings)->eSelect(NUM_PL, eCase, eStressType);
            rc = m_spEndings->eGetEnding(sEndingPlAcc, llEndingKey);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }

            CEString sEndingPlGen;
            rc = static_pointer_cast<CNounEndings>(m_spEndings)->eSelect(NUM_PL, CASE_GEN, eStressType);
            rc = m_spEndings->eGetEnding(sEndingPlGen, llEndingKey);
            if (rc != H_NO_ERROR)
            {
                return rc;
            }
            if (sEndingPlAcc != sEndingPlGen)
            {
                return H_NO_ERROR;
            }
        }
        else
        {
            if (eCase != CASE_GEN)
            {
                return H_NO_ERROR;
            }
        }
    }       // if (POS_NOUN == m_pLexeme->ePartOfSpeech())

    if (SUBPARADIGM_SHORT_ADJ == eSubparadigm || 
        SUBPARADIGM_PART_PRES_PASS_SHORT == eSubparadigm ||
        SUBPARADIGM_PART_PAST_PASS_SHORT == eSubparadigm)
    {
        if (GENDER_M != eGender)
        {
            return H_NO_ERROR;
        }
    }

    m_bFleetingVowelPresent = true;

    if (((GENDER_F == eGender) || (GENDER_N == eGender)) && (6 == m_spInflection->iType()))
    {
        if (STRESS_LOCATION_ENDING == eStressType)
        {
            sStem[sStem.uiLength()-1] = L'е';   // статей, питей
        }
        else
        {
            sStem[sStem.uiLength()-1] = L'и';   // гостий, ущелий
        }
        return H_NO_ERROR;
    }

    unsigned int uiLastConsonant = sGrStem.uiFindLastOf (CEString::g_szRusConsonants);
    if (ecNotFound == uiLastConsonant)
    {
		assert(0);
        ERROR_LOG (L"Can't find last consonant.");
        return H_ERROR_UNEXPECTED;
    }

    if ((L'ь' == sGrStem[uiLastConsonant-1]) || 
        (L'й' == sGrStem[uiLastConsonant-1]))
    {
        if (L'ц' == sGrStem[uiLastConsonant])
        {
            sStem[uiLastConsonant-1] = L'е';    // колец
            return H_NO_ERROR;
        }

        if (STRESS_LOCATION_ENDING == eStressType)
        {
            sStem[uiLastConsonant-1] = L'ё';     // серёг, каём
            return H_NO_ERROR;
        }
        else
        {
            sStem[uiLastConsonant-1] = L'е';       // шпилек, чаек, писем
            return H_NO_ERROR;
        }
    }
    if (L'к' == sGrStem[uiLastConsonant-1] || 
        L'г' == sGrStem[uiLastConsonant-1] ||
        L'х' == sGrStem[uiLastConsonant-1])
    {
        sStem.sInsert (uiLastConsonant, L'о');  // кукол, окон, мягок
        return H_NO_ERROR;
    }
    if (L'к' == sGrStem[uiLastConsonant] ||
        L'г' == sGrStem[uiLastConsonant] ||
        L'х' == sGrStem[uiLastConsonant])
    {
        if (3 != m_spInflection->iType())
        {
			assert(0);
            ERROR_LOG (L"Unexpected type.");
            return H_ERROR_UNEXPECTED;
        }

        if (!CEString::bIn (sGrStem[uiLastConsonant-1], L"шжчщц"))
        {
            sStem.sInsert (uiLastConsonant, L'о');  // сказок, ведерок, краток, долог
            return H_NO_ERROR;
        }
    }
    if (L'ц' == sGrStem[uiLastConsonant])
    {
        sStem.sInsert (uiLastConsonant, L'е');      // овец
        return H_NO_ERROR;
    }

    if (STRESS_LOCATION_ENDING == eStressType)
    {
        if (CEString::bIn (sGrStem[uiLastConsonant-1], L"шжчщ"))   // + ц?
        {
            sStem.sInsert (uiLastConsonant, L'о');    // кишок
            return H_NO_ERROR;
        }
        else
        {
            sStem.sInsert (uiLastConsonant, L'ё');    // сестёр, хитёр
            return H_NO_ERROR;
        }
    }
    else
    {
        sStem.sInsert (uiLastConsonant, L'е');       // сосен, чисел, ножен, верен
    }

    return H_NO_ERROR;

}   //  h_FleetingVowelAdd (...)

ET_ReturnCode CFormBuilderDecl::eFleetingVowelRemove (ET_Gender eGender,               // in
                                                      ET_Subparadigm eSubparadigm,     // in
                                                      const CEString& sEnding,         // in
                                                      CEString& sStem)                 // out
{
	assert(m_spLexeme);   // we assume base class ctor took care of this

//    int iType = m_pInflection->iType();
//    CEString& sGrStem = m_pLexeme->m_sGraphicStem;

    m_bFleetingVowelPresent = true;

    // Forms that have the fleeting vowel (all from the initial form except -ью):
    // m.: NULL (сон, конец)
    // m.: -ь (кашель)
    // f.: -ь, -ью (любовь)
    // pron.: NULL, -ь, -й (весь, волчий)

    // assert: last stem vowel is { о, е, ё, и } [also а, я "in several anomalous words"]

    //
    // No vowel in ending: keep the fleeting vowel
    //
    if (sEnding.bIsEmpty())
    {
        if (GENDER_M != eGender)
        {
			assert(0);
            ERROR_LOG (L"Unexpected gender for null ending.");
            return H_ERROR_UNEXPECTED;
        }
        return H_NO_ERROR;
    }

    if (L"ь" == sEnding)
    {
    //    if (L"мс" != str_inflection && !(GENDER_F == eGender && 8 == iType))
    //    {
    //        assert(0);
    //        wstring sMsg (L"Unexpected type for ending 'ь'."); .. NO!! cf кашель
    //        ERROR_LOG (sMsg);
    //        throw CT_Exception (H_ERROR_UNEXPECTED, sMsg);
    //    }

        return H_NO_ERROR;
    }

    if (L"ью" == sEnding)
    {
        if (GENDER_F != eGender || 8 != m_spInflection->iType())
        {
			assert(0);
            ERROR_LOG (L"Unexpected type for ending 'ью'.");
            return H_ERROR_UNEXPECTED;
        }
        return H_NO_ERROR;
    }

    if (L"й" == sEnding)
    {
/*
        if (L"мс-п" != m_pLexeme->m_sInflectionType)
        {
            assert(0);
            CEString sMsg (L"Unexpected type for ending 'й'.");
            ERROR_LOG (sMsg);
            throw CException (H_ERROR_UNEXPECTED, sMsg);
        }
*/
        return H_NO_ERROR;
    }

    unsigned int uiLastVowel = m_spLexeme->sGraphicStem().uiFindLastOf (CEString::g_szRusVowels);

    if (ecNotFound == uiLastVowel)
    {
        sStem.Erase();
		assert(0);
        ERROR_LOG (L"Vowel expected.");
        return H_ERROR_UNEXPECTED;
    }

    if (m_spLexeme->sGraphicStem()[uiLastVowel] == L'о')   // сон, любовь
    {
        m_bFleetingVowelPresent = false;
        sStem.sErase (uiLastVowel, 1);
        return H_NO_ERROR;
    }

    if ((m_spLexeme->sGraphicStem()[uiLastVowel] == L'и') ||
        (m_spLexeme->sGraphicStem()[uiLastVowel] == L'е' && 
            (POS_PRONOUN_ADJ == m_spLexeme->ePartOfSpeech() ||  // чей, волчий
                (POS_NOUN == m_spLexeme->ePartOfSpeech() && SUBPARADIGM_PRONOUN_ADJ == eSubparadigm))))              // ничья
    {
        if (L"мс-п" != m_spLexeme->sInflectionType() && 
            !((m_spLexeme->ePartOfSpeech() == POS_ADJ || m_spLexeme->ePartOfSpeech() == POS_NUM_ADJ || m_spLexeme->ePartOfSpeech() == POS_NOUN) && 
                ((L"мс" == m_spLexeme->sInflectionType()) ||
                (1 != m_spInflection->iType() && 2 != m_spInflection->iType() && 6 != m_spInflection->iType()))))
        {
//			assert(0);
            CEString sMsg(L"Unexpected inflection type: ");
            sMsg += m_spLexeme->sSourceForm();
            ERROR_LOG (sMsg);
            return H_ERROR_UNEXPECTED;
        }

        m_bFleetingVowelPresent = false;

        if (1 == m_spInflection->iType())        // один
        {
            sStem.sErase (uiLastVowel, 1);
        }
        else if (2 == m_spInflection->iType())   // весь
        {
            sStem.sErase (uiLastVowel, 1);
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
        else if (6 == m_spInflection->iType())   // чей, волчий
        {
            sStem[uiLastVowel] = L'ь';
        }
        return H_NO_ERROR;
    }

    if ((m_spLexeme->sGraphicStem()[uiLastVowel] == L'е') || (m_spLexeme->sGraphicStem()[uiLastVowel] == L'ё'))
    {
        if (uiLastVowel > 0)
        {
            //if (ecNotFound != CEString (szVowels).uiFind (sGrStem[uiLastVowel-1]))
            if (CEString::bIn (m_spLexeme->sGraphicStem()[uiLastVowel-1], CEString::g_szRusVowels))
            {
                sStem[uiLastVowel] = L'й';   // боец, паек
                return H_NO_ERROR;
            }
        }
        if (GENDER_M == eGender)
        {
            if (6 == m_spInflection->iType())                    // улей
            {
                sStem = m_spLexeme->sGraphicStem();
                sStem[uiLastVowel] = L'ь';
                return H_NO_ERROR;
            }
            if (3 == m_spInflection->iType())
            {
                if ((uiLastVowel > 0) && 
                    (!CEString::bIn (m_spLexeme->sGraphicStem()[uiLastVowel-1], L"шжчщц")))
                {
                    sStem[uiLastVowel] = L'ь';   // зверек
                    return H_NO_ERROR;
                }
            }
            if ((uiLastVowel > 0) && (L'л' == m_spLexeme->sGraphicStem()[uiLastVowel-1]))
            {
                sStem[uiLastVowel] = L'ь';       // лед, палец
                return H_NO_ERROR;
            }
        }
        
        // Special case == #11; 
        if (m_spLexeme->bNoLongForms())
        {
            if (m_spLexeme->sGraphicStem()[uiLastVowel] == L'е' && 
                SUBPARADIGM_SHORT_ADJ == eSubparadigm && 
                GENDER_M != eGender)
            {
                sStem[uiLastVowel] = L'ь';       // беленек, GDRL, pp 75 - 76
            }
            return H_NO_ERROR;
        }

        sStem.sErase (uiLastVowel, 1);   // default -- just remove the vowel

        return H_NO_ERROR;
    
    }   // е or ё

//	assert(0);
    CEString sMsg(L"Function logic error, lexeme = ");
    sMsg += m_spLexeme->sSourceForm();
    ERROR_LOG (sMsg);
    return H_ERROR_UNEXPECTED;

}   //  eFleetingVowelRemove (...)

ET_ReturnCode CFormBuilderDecl::eGetStemStressPositions (const CEString& sStem, 
                                                         ET_Subparadigm eSubParadigm,
                                                         ET_Number eNumber,
                                                         vector<int>& vecPositions)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    // vved_prav. p. 34

    ET_ReturnCode rc = H_NO_ERROR;

    int iSyllablesInStem = sStem.uiNSyllables();

    int iPos = -1;
    rc = m_spLexeme->eGetFirstStemStressPos(iPos);
    while (AT_D != m_spInflection->eAccentType1() && H_NO_ERROR == rc && iPos < iSyllablesInStem)
    {
		assert(iPos >= 0);
        vecPositions.push_back (iPos);
        rc = m_spLexeme->eGetNextStemStressPos(iPos);
    }

    if (m_spInflection->iType() == 0)
    {
        return H_NO_ERROR;
    }

    if (AT_D != m_spInflection->eAccentType1() && 
        !m_bIrregularSourceForm && 
        (H_NO_ERROR == rc || H_NO_MORE == rc) && 
        (iPos < iSyllablesInStem))
    {
        return H_NO_ERROR;
    }

    //
    // Source stress is on ending:
    //
    int iNSyll = sStem.uiGetNumOfSyllables();
    if (iNSyll < 1)
    {
//        assert(0);
        CEString sMsg(L"uiGetNumOfSyllables() failed, lexeme = ");
        sMsg += m_spLexeme->sSourceForm();
        ERROR_LOG (sMsg);
        return H_ERROR_GENERAL;
    }

    //
    // Trivial
    //
    if (1 == iNSyll)
    {
        vecPositions.push_back (0);
        return H_NO_ERROR;
    }

    //
    // 2 or more syllables
    //
    ET_AccentType eAccentType = (SUBPARADIGM_SHORT_ADJ == eSubParadigm ||
                                 SUBPARADIGM_PART_PRES_PASS_SHORT == eSubParadigm ||
                                 SUBPARADIGM_PART_PAST_PASS_SHORT == eSubParadigm)
                                    ? m_spInflection->eAccentType2()
                                    : m_spInflection->eAccentType1();
    switch (eAccentType)
    {
        case AT_A:
        case AT_A1:
        case AT_B:
        {
            if (AT_A == eAccentType && SUBPARADIGM_NOUN == eSubParadigm && 1 == m_spInflection->iStemAugment())  // армянин, stem = 'армян'
            {
                if (NUM_PL == eNumber)
                {
                    vecPositions.push_back(iNSyll - 1);
                }
                else
                {
                    vecPositions.push_back(iNSyll);
                }
                return H_NO_ERROR;
            }

            vecPositions.push_back(iNSyll - 1);
            return H_NO_ERROR;
            
//            assert(0);
//            CEString sMsg(L"Unexpected accent type; lexeme = ");
//            sMsg += m_pLexeme->sSourceForm();
//            ERROR_LOG (sMsg);
//            return H_ERROR_UNEXPECTED;
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
            return H_NO_ERROR;
        }
        case AT_C:
        case AT_C1:
        {
            if (AT_C == eAccentType && 1 == m_spInflection->iStemAugment())
            {
                if (NUM_PL == eNumber)
                {
                    vecPositions.push_back(iNSyll - 1);
                }
                else
                {
                    vecPositions.push_back(iNSyll);
                }
                return H_NO_ERROR;
            }
                
            vecPositions.push_back(iNSyll - 1);
            return H_NO_ERROR;
  
//            assert(0);
//            CEString sMsg(L"Unexpected accent type; lexeme = ");
//            sMsg += m_pLexeme->sSourceForm();
//            ERROR_LOG(sMsg);
//            return H_ERROR_UNEXPECTED;
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
            return H_NO_ERROR;
        }
        case AT_E:
        {
            assert(0);
            ERROR_LOG (L"Unexpected accent type.");
            return H_ERROR_UNEXPECTED;
        }
        case AT_F:
        case AT_F1:
        case AT_F2:
        {
            vecPositions.push_back (0);
            return H_NO_ERROR;
        }
        default:
        {
            assert(0);
            ERROR_LOG (L"Unexpected accent type.");
            return H_ERROR_UNEXPECTED;
        }
    }

    return H_ERROR_UNEXPECTED;

}   //  eGetStemStressPositions (...)

ET_ReturnCode CFormBuilderDecl::eGetEndingStressPosition (const CEString& sStem, 
                                                          const CEString& sEnding,
                                                          int& iPosition)
{
    const_cast<CEString&>(sStem).SetVowels(CEString::g_szRusVowels);
    const_cast<CEString&>(sEnding).SetVowels(CEString::g_szRusVowels);

    if (sEnding.uiNSyllables() > 0)
    {
        iPosition = sStem.uiNSyllables();
    }
    else
    {
        iPosition = sStem.uiNSyllables() - 1;
        if (iPosition < 0)
        {
			assert(0);
            ERROR_LOG (L"Warning: can't find stressed vowel.");
            return H_ERROR_UNEXPECTED;
        }
    }

    return H_NO_ERROR;

}   //  eGetEndingStressPosition (...)

ET_ReturnCode CFormBuilderDecl::eHandleYoAlternation (ET_StressLocation eStressType, 
                                                      int iStressedSyll,
                                                      CEString& sStem,
                                                      const CEString& sEnding)
{
	assert(m_spLexeme);   // we assume base class ctor took care of this

    if (!m_bYoAlternation)
    {
        return H_NO_ERROR;
    }

    if (STRESS_LOCATION_UNDEFINED == eStressType)
    {
		assert(0);
        ERROR_LOG (L"Undefined stress type.");
        return H_ERROR_INVALID_ARG;
    }

//    CEString sGraphicStem = (POS_VERB == m_pLexeme->ePartOfSpeech()) 
//        ? sStem // for participles, graphic stem is participial stem
//        : m_pLexeme->m_sGraphicStem;

    CEString sGraphicStem;
    if (POS_VERB == m_spLexeme->ePartOfSpeech()) 
    {
        sGraphicStem = sStem; // for participles, graphic stem is participial stem
    }
    else if (m_spInflection->iStemAugment() > 0)
    {
        sGraphicStem = sStem;
    }
    else
    {
        sGraphicStem = m_spLexeme->sGraphicStem();
    }

    int iYoOffset = sGraphicStem.uiFind (L"ё");
    if (ecNotFound != iYoOffset)
    {
        // Graphic stem contains yo: replace with e if stress is on ending
        if (STRESS_LOCATION_ENDING == eStressType)
        {
            if (iStressedSyll >= sStem.uiNSyllables())
            {
                sStem[iYoOffset] = L'е';        //  yo --> e: ежа, щелочей, тяжела
            }
        }
    }
    else
    {
        // Graphic stem has no yo (must have at least on e)
        if (STRESS_LOCATION_STEM == eStressType || sEnding.bIsEmpty())
        {
            int iEOffset = sGraphicStem.uiRFind (L"е");    // last "e" in graphic stem (?)
//            if (ecNotFound == iEOffset && !m_pLexeme->m_bHasIrregularForms)
//            {
//                if (POS_VERB != m_pLexeme->ePartOfSpeech()) 
//                {
//                    assert(0);
//                    CEString sMsg (L"Unstressed stem with yo alternation has no e.");
//                    ERROR_LOG (sMsg);
//                    throw CException (E_FAIL, sMsg);
//                }
//                else
//                {
//                    return H_NO_ERROR;
//                }
//            }

            if (POS_NOUN == m_spLexeme->ePartOfSpeech() && 
                GENDER_N == m_spLexeme->eInflectionTypeToGender() &&
                3 == m_spInflection->iStemAugment() &&
                0 == iStressedSyll)
            {
                return H_NO_ERROR;
            }

            sStem.SetVowels(CEString::g_szRusVowels);
            int iStressPos = sStem.uiGetVowelPos (iStressedSyll);
            if (iStressPos < 0 || iStressPos > (int)sStem.uiLength())
            {
				assert(0);
                ERROR_LOG (L"Bad stress position.");
                return H_ERROR_UNEXPECTED;
            }
            // Replace stressed e with yo:
            if (iEOffset == iStressPos)
            {
                sStem[iEOffset] = L'ё';
            }
        }
    }

    return H_NO_ERROR;

}   //  eHandleYoAlternation (...)
