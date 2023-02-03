#include "WordForm.h"
#include "Lexeme.h"
#include "FormBuilderBaseConj.h"

using namespace Hlib;

ET_ReturnCode CFormBuilderConj::eBuildVerbStems()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    StLexemeProperties& stProperties = m_spLexeme->stGetPropertiesForWriteAccess();

    CEString& sInfinitive = stProperties.sSourceForm;
    CEString sGraphicStem = stProperties.sGraphicStem;                  // immutable
    CEString& s1SgStem = stProperties.s1SgStem;
    CEString& sInfStem = stProperties.sInfStem;
    CEString& s3SgStem = stProperties.s3SgStem;
    CEString sVerbStemAlternation = stProperties.sVerbStemAlternation;  // immutable

    sInfStem = stProperties.sGraphicStem;

    switch (m_spInflection->iType())
    {
        case 1:         // делать, читать(ся), верстать, терять, жалеть, читать, стараться
        {
            s1SgStem = s3SgStem = sGraphicStem;
            break;
        }

        case 2:         // требовать, рисовать(ся), тушевать, малевать, ковать, жевать, клевать, радоваться
        {
            s1SgStem = s3SgStem = CEString(sGraphicStem).sErase (sInfStem.uiLength()-3, 3);
            if (sInfinitive.bEndsWith (L"овать") || sInfinitive.bEndsWith (L"оваться"))
            {
                s1SgStem += L"у";
                s3SgStem += L"у";
            }
            else if (sInfinitive.bEndsWith (L"евать") || sInfinitive.bEndsWith (L"еваться"))
            {
                if (s1SgStem.bEndsWithOneOf (L"шжчщц"))
                {
                    s1SgStem += L"у";
                    s3SgStem += L"у";
                }
                else
                {
                    s1SgStem += L"ю";
                    s3SgStem += L"ю";
                }
            }
            break;
        }

        case 3:         // Without circle: (вы)тянуть, тронуть, стукнуть, гнуть, повернуть, тянуть
        {               // With circle: вымокнуть, (за)вянуть, (по)гибнуть, стынуть, достигнуть 
                        //              (достичь), свыкнуться
            s1SgStem = s3SgStem = CEString(sGraphicStem).sErase (sInfStem.uiLength()-1, 1);
            break;
        }

        case 4:         // строить(ся), множить, грабить, тратить, смолить, крушить, томить, щадить, хоронить, 
                        // точить, ловить, просить, молиться

        case 5:         // (вы)стоять, выглядеть, слышать, видеть, кричать, бежать, звенеть, шуметь, висеть
        {
            rc = eStandardAlternation (s1SgStem);
            if (H_NO_ERROR != rc)
            {
                ERROR_LOG (L"hStandardAlternation() failed.");
                return rc;
            }
            s3SgStem = CEString(sGraphicStem).sErase (sInfStem.uiLength()-1, 1);
            break;
        }

        case 6:         // Without circle: сеять(ся), глаголать, колебать, сыпать, прятать, смеяться, ржать, 
                        //                 слать, стлать, трепать, скакать, казаться
                        // With circle: (вы)сосать(ся), жаждать, рвать(ся), ткать, лгать, брать, звать, стонать
        {
            if (1 != m_spInflection->iStemAugment())
            {
                rc = eStandardAlternation (s1SgStem);
                if (H_NO_ERROR != rc)
                {
                    ERROR_LOG (L"eStandardAlternation() failed.");
                    return rc;
                }
            }
            else
            {
                s1SgStem = sGraphicStem;
                s1SgStem.sErase (s1SgStem.uiLength() - 1);
            }

            s3SgStem = s1SgStem;
            break;
        }

        case 7:         // (вы)грызть, лезть, сесть, красть(ся), ползти, везти, пасти, нести, блюсти, вести, 
                        // мести(сь), (у)честь, расти, грести
        {
            if (sInfStem.bEndsWith (L"с"))
            {
                if (m_spLexeme->sVerbStemAlternation().bIsEmpty())
                {
                    assert(0);
                    ERROR_LOG (L"Missing stem alternation for type 7");
                    return H_ERROR_GENERAL;
                }

                s1SgStem = sInfStem.sSubstr (0, sInfStem.uiLength()-1) + sVerbStemAlternation;
            }
            else
            {
                assert(sInfStem.bEndsWith(L"з"));
                s1SgStem = sInfStem;
            }

            s3SgStem = s1SgStem;
            break;
        }

        case 8:         // (вы)стричь, лечь, запрячь, беречь(ся), волочь, печь(ся), жечь, толочь, мочь
        {
            if (sVerbStemAlternation.bIsEmpty())
            {
                assert(0);
                ERROR_LOG (L"Missing stem alternation for type 8");
                return H_ERROR_GENERAL;
            }

            s1SgStem = sInfStem += sVerbStemAlternation;
            if (L"к" == sVerbStemAlternation)
            {
                s3SgStem = sInfStem;
                s3SgStem.sErase (s3SgStem.uiLength() - 1);
                s3SgStem += L"ч";
            }
            else
            {
                if (L"г" == sVerbStemAlternation)
                {
                    s3SgStem = sInfStem;
                    s3SgStem.sErase (s3SgStem.uiLength() - 1);
                    s3SgStem += L"ж";
                }
                else
                {
                    assert(0);
                    ERROR_LOG (L"Bad stem finale");
                    return H_ERROR_GENERAL;
                }
            }
            break;
        }

        case 9:             // (вы-, с)тереть(ся), запереть, умереть, отпереть
        {
            s1SgStem = s3SgStem = sInfStem.sSubstr (0, sInfStem.uiLength()-3) + L"р";
            break;
        }
        case 10:            // (вы)пороть, (у)колоть(ся), молоть, бороться
        {
            s1SgStem = s3SgStem = CEString(sGraphicStem).sErase (sInfStem.uiLength()-1, 1);
            break;
        }
        case 11:            // шить(ся), (вы-, до)бить(ся), лить(ся), пить(ся)
        {
            s1SgStem = s3SgStem = CEString(sGraphicStem).sErase (sInfStem.uiLength()-1, 1) + L"ь";
            break;
        }
        case 12:            // выть, крыть, мыть, ныть,...
        {
            if (sInfStem.bEndsWith (L"ы"))
            {
                s1SgStem = s3SgStem = CEString(sInfStem).sReplace(sInfStem.uiLength() - 1, L'о');
            }
            else
            {
                s1SgStem = s3SgStem = sInfStem;
            }
            break;
        }
        case 13:            // давать(ся), оставаться
        {
            s1SgStem = s3SgStem = CEString(sInfStem).sErase(sInfStem.uiLength() - 2, 2);
            break;
        }
        case 14:            // мять(ся), жать(ся), сжать, отнять, занять, ...
        {
            if (sVerbStemAlternation.bIsEmpty())
            {
                if (17 == m_spLexeme->iSection())
                {
                    return H_NO_MORE; // внять, объять и т.п.
                }
                else
                {

                    assert(0);
                    ERROR_LOG (L"Missing stem alternation for type 14");
                    return H_ERROR_GENERAL;
                }
            }

            if (L"н" != sVerbStemAlternation && L"м" != sVerbStemAlternation && L"им" != sVerbStemAlternation)
            {
                s1SgStem = s3SgStem = sVerbStemAlternation;
            }
            else
            {
                s1SgStem = s3SgStem = CEString(sGraphicStem).sErase (sInfStem.uiLength()-1, 1) + sVerbStemAlternation;
            }
            break;
        }
        case 15:            // одеть, достать, остаться
        {
            s1SgStem = s3SgStem = sInfStem + L"н";
            break;
        }
        case 16:            // жить(ся), плыть, слыть
        {
            s1SgStem = s3SgStem = sInfStem + L"в";
            break;
        }
        default:
        {
//            assert(0);
//            ERROR_LOG (L"Unrecognized conjugation type.");
//            return H_ERROR_GENERAL;
            return H_NO_MORE;     // this is possible for some irregular verbs
        }
    }

    return rc;

}    //  eBuildVerbStems()

ET_ReturnCode CFormBuilderConj::eStandardAlternation (CEString& sPresentStem)
{
	assert(m_spLexeme);   // we assume base class ctor took care of this

    const CEString& sVerbStemAlternation = m_spLexeme->sVerbStemAlternation();

    sPresentStem = m_spLexeme->sGraphicStem();
    sPresentStem.sErase (sPresentStem.uiLength()-1);

    CEString sFinale;
    if (sPresentStem.bEndsWith (L"ск"))
    {
        sFinale = L"ск";
    }
    else
    {
        if (sPresentStem.bEndsWith (L"ст"))
        {
            sFinale = L"ст";
        }
        else
        {
            sFinale = sPresentStem[sPresentStem.uiLength()-1];
        }
    }

    CEString sAlternant;
    bool bHasStandardAlternation = m_spLexeme->bFindStandardAlternation(sFinale, sAlternant);
    if (bHasStandardAlternation)
    {
        if (L"щ" == sVerbStemAlternation)
        {
            if (L"т" != sFinale)
            {
				assert(0);
                CEString sMsg(L"Unexpected verb stem alternation: ");
                sMsg += sFinale + L".";
                ERROR_LOG (sMsg);
                return H_ERROR_UNEXPECTED;
            }

            sPresentStem.sReplace (sPresentStem.uiLength()-1, 1, sVerbStemAlternation);
        }
        else
        {
            sPresentStem.sErase (sPresentStem.uiLength()- sFinale.uiLength());
            sPresentStem += sAlternant;
        }
    }

    return H_NO_ERROR;

}   //  eStandardAlternation()

ET_ReturnCode CFormBuilderConj::eGetPastTenseStressTypes (ET_AccentType eAccentType,
                                                          ET_Number eNumber, 
                                                          ET_Gender eGender,
                                                          vector<ET_StressLocation>& vecStressType)
{
	assert(m_spLexeme);   // we assume base class ctor took care of this

    if (POS_VERB != m_spLexeme->ePartOfSpeech())
    {
		assert(0);
        CEString sMsg (L"Unexpected part of speech value.");
        ERROR_LOG (sMsg);
        return H_ERROR_GENERAL;
    }

    vector<ET_StressLocation> vecStress;
    switch (eAccentType)
    {
        case AT_A:
        {
            vecStressType.push_back (STRESS_LOCATION_STEM);
            break;
        }
        case AT_B:
        {
            vecStressType.push_back (STRESS_LOCATION_ENDING);
            break;
        }
        case AT_C:
        {
            if (NUM_SG == eNumber && GENDER_N == eGender)
            {
                vecStressType.push_back (STRESS_LOCATION_STEM);
            }
            else if (NUM_PL == eNumber)
            {
                vecStressType.push_back (STRESS_LOCATION_STEM);
                break;
            }
            else
            {
                vecStressType.push_back (STRESS_LOCATION_ENDING);
            }
            break;
        }
        case AT_C1:     // дать & взять only
        {
            if (NUM_SG == eNumber && GENDER_N == eGender)
            {
                vecStressType.push_back(STRESS_LOCATION_STEM);
                vecStressType.push_back(STRESS_LOCATION_ENDING);
            }
            else if (NUM_SG == eNumber && GENDER_F == eGender)
            {
                vecStressType.push_back(STRESS_LOCATION_ENDING);
                break;
            }
            else
            {
                vecStressType.push_back(STRESS_LOCATION_STEM);
            }
            break;
        }
        case AT_C2:
        {
            if (REFL_NO == m_spLexeme->eIsReflexive())
            {
                assert(0);
                CEString sMsg (L"Unexpected part of speech value.");
                ERROR_LOG (sMsg);
                return H_ERROR_GENERAL;
            }
            else if (!(GENDER_F == eGender && NUM_SG == eNumber))
            {
                vecStressType.push_back (STRESS_LOCATION_STEM);
                vecStressType.push_back (STRESS_LOCATION_ENDING);
            }
            else
            {
                vecStressType.push_back (STRESS_LOCATION_ENDING);
            }
            break;
        }
        default:
        {
            assert(0);
            ERROR_LOG (L"Unexpected past tense accent type.");
            return H_ERROR_GENERAL;
        }

    }   // switch (eAccentType)

    return H_NO_ERROR;

}   //  eGetPastTenseStressType (...)

/*
ET_ReturnCode CFormBuilderConj::eGetStemStressPositions (CEString& sStem, vector<int>& vecPosition)
{
    //
    // Find the sequence # of the stressed vowel in infinitive
    //
    vector<int>& vecInfStress = m_spLexeme->m_vecSourceStressPos; // alias for readability
    vector<int>::iterator itInfStresPos = vecInfStress.begin();
    for (; itInfStresPos != vecInfStress.end(); ++itInfStresPos)
    {
        sStem.SetVowels (g_szVowels);
        int iStemSyllables = sStem.uiNSyllables();
        if (iStemSyllables < 1)
        {
            assert(0);
            ERROR_LOG (L"No vowels in verb stem.");
            return H_ERROR_GENERAL;
        }

        if (*itInfStresPos >= iStemSyllables)
        {
            vecPosition.push_back (iStemSyllables - 1);  // last stem syllable
        }
        else
        {
            vecPosition.push_back (*itInfStresPos);    // same syllable as in infinitive
        }

    }   //  for (; it_sourceStressPos != vec_SourceStressPos.end(); ... )

    return H_NO_ERROR;

}   //  hGetStemStressPositions (...)
*/

ET_ReturnCode CFormBuilderConj::eGetEndingStressPosition (const CEString& sStemSource, const CEString& sEndingSource, int& iPosition)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;

    if (POS_VERB != m_spLexeme->ePartOfSpeech())
    {
        assert(0);
        ERROR_LOG (L"Unexpected part of speech.");
        return H_ERROR_UNEXPECTED;
    }

    bool bRetract = false;

    CEString sEnding(sEndingSource);        // Can't be const
    sEnding.SetVowels (CEString::g_szRusVowels);
    if (0 == sEnding.uiNSyllables())
    {
        if (!(sEnding.uiLength() == 0 || L"ь" == sEnding || L"й" == sEnding || L"л" == sEnding))
        {
            ASSERT(0);
            CEString sMsg (L"Unexpected non-syllabic ending.");
            ERROR_LOG (sMsg);
            throw CException (H_ERROR_GENERAL, sMsg);
        }
        bRetract = true;
    }
    else
    {
        if (L"те" == sEnding || L"ся" == sEnding || L"тесь" == sEnding)
        {
            bRetract = true;
        }
        else if (sEnding.bEndsWith (L"те") || sEnding.bEndsWith (L"ся") || sEnding.bEndsWith (L"тесь"))
        {
            if (L'ь' == sEnding[0] || L'й' == sEnding[0])
            {
                bRetract = true;
            }
        }
    }

    CEString sStem(sStemSource);
    sStem.SetVowels (CEString::g_szRusVowels);
    if (!bRetract)
    {
        iPosition = sStem.uiNSyllables();
    }
    else
    {
        iPosition = sStem.uiNSyllables() - 1;
    }

    return rc;

}   //  eGetEndingStressPosition (...)

ET_ReturnCode CFormBuilderConj::eFleetingVowelCheck (CEString& sVerbForm)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    if (!m_spInflection->bFleetingVowel())
    {
        return H_NO_ERROR;
    }

// types 5, 6, 7, 8, 9, 11, 14

/*
    CEString sPreverb;
    bool bPreverb = false;
    bool bVoicing = false;

    vector<CEString>::iterator itP = m_spLexeme->m_vecAlternatingPreverbs.begin();
    for (; itP != m_spLexeme->m_vecAlternatingPreverbs.end()&&!bPreverb; ++itP)
    {
        if (sVerbForm.bStartsWith (*itP))
        {
            sPreverb = *itP;
            bPreverb = true;
        }
    }

    if (!bPreverb)
    {
        itP = m_spLexeme->m_vecAlternatingPreverbsWithVoicing.begin();
        for (; itP != m_spLexeme->m_vecAlternatingPreverbsWithVoicing.end()&&!bPreverb; ++itP)
        {
            if (sVerbForm.bStartsWith (*itP))
            {
                sPreverb = *itP;    
                bPreverb = true;
                bVoicing = true;
            }
        }
    }

    if (!bPreverb)
    {
        return H_NO_ERROR;
    }
*/
    ET_ReturnCode rc = H_NO_ERROR;

    CEString sPreverb;
    bool bVoicing = false;
//    rc = m_spLexeme->eGetAlternatingPreverb (sVerbForm, sPreverb, bVoicing);
    rc = eGetAlternatingPreverb(sVerbForm, sPreverb, bVoicing);
    if (H_FALSE == rc)
    {
        return H_NO_ERROR;
    }

    if (H_NO_ERROR != rc)
    {
        return rc;
    }

    if (sVerbForm.uiLength() < sPreverb.uiLength() + 1)
    {
        assert(0);
        ERROR_LOG (L"Stem too short.");
        return H_ERROR_INVALID_ARG;
    }

    // o after prefix?
    if (sVerbForm[sPreverb.uiLength()-1] == L'о')
    {
        // Remove o if followed by CV
        if (CEString::bIn (sVerbForm[sPreverb.uiLength()], CEString::g_szRusConsonants) && 
            CEString::bIn (sVerbForm[sPreverb.uiLength()+1], CEString::g_szRusVowels))
        {
            sVerbForm.sErase (sPreverb.uiLength()-1);
            if (bVoicing)
            {
                wchar_t chrStemAnlaut = sVerbForm[sPreverb.uiLength()];
                if (CEString::bIn (chrStemAnlaut, CEString::g_szRusVoicelessConsonants))
                {
                    sVerbForm[sPreverb.uiLength()] = L'з';
                }
                else
                {
                    sVerbForm[sPreverb.uiLength()] = L'с';
                }
            }
        }
    }
    else
    {
        //  p. 79: Беглое о присутствует в конце приставки в тех формах, где за приставкой следует две согласных или согласная + ь; 
        //  в прочих формах оно отсутствует (во всех таких формах за приставкой следует согласная + гласная).
        if (sVerbForm.uiLength() >= sPreverb.uiLength()+2 &&  
            CEString::bIn(sVerbForm[sPreverb.uiLength()], CEString::g_szRusConsonants) &&
            (L'ь' == sVerbForm[sPreverb.uiLength()+1] ||  CEString::bIn(sVerbForm[sPreverb.uiLength()+1], CEString::g_szRusConsonants)))
        {
            sVerbForm.sInsert (sPreverb.uiLength(), L'о');
            if (bVoicing)
            {
                sVerbForm[sPreverb.uiLength()-1] = L'з';  // force voicing
            }
        }
    }

    return H_NO_ERROR;

}   //  hFleetingVowelCheck (...)

ET_ReturnCode CFormBuilderConj::eBuildPastTenseStem (CEString& sStem)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode hr = H_NO_ERROR;

    int iType = m_spInflection->iType();
    if (7 == iType || 8 == iType)
    {
        sStem = m_spLexeme->s1SgStem();
        if (sStem.bEndsWithOneOf (L"тд"))
        {
            sStem = sStem.sErase (sStem.uiLength()-1);
        }
    }
    else
    {
        sStem = m_spLexeme->sInfStem();
    }
    if (9 == iType)
    {
        assert(sStem.bEndsWith(L"е"));
        sStem.sErase (sStem.uiLength()-1);
    }
    if (3 == iType && 1 == m_spInflection->iStemAugment())
    {
        assert(sStem.bEndsWith(L"ну"));
        sStem.sErase (sStem.uiLength()-2);
    }

    return hr;

}   //  eBuildPastTenseStem (...)

ET_ReturnCode CFormBuilderConj::eHandleYoAlternation (int iStressSyll, ET_Subparadigm eoSubParadigm, CEString& sStem)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    if (!m_spLexeme->bHasYoAlternation())
    {
        return H_NO_ERROR;
    }

    if (SUBPARADIGM_PAST_TENSE != eoSubParadigm && 
        SUBPARADIGM_PART_PAST_ACT != eoSubParadigm &&
        SUBPARADIGM_PART_PAST_PASS_LONG != eoSubParadigm &&
        SUBPARADIGM_PART_PAST_PASS_SHORT != eoSubParadigm &&
        SUBPARADIGM_ADVERBIAL_PAST != eoSubParadigm &&
        SUBPARADIGM_PART_PRES_PASS_LONG != eoSubParadigm &&
        SUBPARADIGM_PART_PRES_PASS_SHORT != eoSubParadigm )
    {
        return H_NO_ERROR;
    }

    if (SUBPARADIGM_PART_PAST_ACT == eoSubParadigm)
    {
        assert(7 == m_spInflection->iType() &&                  // GDRL, p. 85
        m_spLexeme->sSourceForm().bEndsWith (L"сти") &&
        (L"т" == m_spLexeme->sVerbStemAlternation() || 
         L"д" == m_spLexeme->sVerbStemAlternation()));
    }

    sStem.SetVowels(CEString::g_szRusVowels);
    if (iStressSyll >= sStem.uiNSyllables())
    {
        return H_NO_MORE;
    }

    unsigned int uiEOffset = sStem.uiRFind (L"е");    // last "e" in graphic stem (?)
    if (ecNotFound == uiEOffset)
    {
        assert(0);
        ERROR_LOG (L"Unstressed stem with yo alternation has no e.");
        return H_ERROR_GENERAL;
    }

    unsigned int uiStressPos = sStem.uiGetVowelPos (iStressSyll);
    if (uiStressPos == uiEOffset)
    {
        sStem[uiStressPos] = L'ё';
    }

    return H_NO_ERROR;

}   //  eHandleYoAlternation (...)

ET_ReturnCode CFormBuilderConj::eGetAlternatingPreverb(const CEString& sVerbForm, CEString& sPreverb, bool& bVoicing)
{
    if (!m_spInflection->bFleetingVowel())
    {
        return H_NO_MORE;
    }

    // types 5, 6, 7, 8, 9, 11, 14
    bool bPreverb = false;

    auto itP = m_vecAlternatingPreverbsWithVoicing.begin();
    for (; itP != m_vecAlternatingPreverbsWithVoicing.end() && !bPreverb; ++itP)
    {
        if (sVerbForm.bStartsWith(*itP))
        {
            sPreverb = *itP;
            bPreverb = true;
            bVoicing = true;
        }
    }

    if (!bPreverb)
    {
        auto itP = m_vecAlternatingPreverbs.begin();
        for (; itP != m_vecAlternatingPreverbs.end() && !bPreverb; ++itP)
        {
            if (sVerbForm.bStartsWith(*itP))
            {
                sPreverb = *itP;
                bPreverb = true;
            }
        }
    }

    if (!bPreverb)
    {
        return H_FALSE;
    }

    if (sVerbForm.uiLength() < sPreverb.uiLength() + 1)
    {
        //        assert(0);
        CEString sMsg(L"Stem too short; lexeme = ");
        ERROR_LOG(sMsg + m_spLexeme->sSourceForm());
        return H_ERROR_INVALID_ARG;
    }

    return H_NO_ERROR;

}       //  eGetAlternatingPreverb(...)

bool CFormBuilderConj::bHasIrregularPresent()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    try
    {
        if (!m_spLexeme->bHasIrregularForms())
        {
            return false;
        }

        // Must have at least two forms: 1 sg and 3 sg (GDRL, p. 89)
        CGramHasher sg1Hash (SUBPARADIGM_PRESENT_TENSE, 
                             NUM_SG, 
                             GENDER_UNDEFINED, 
                             PERSON_1, 
                             ANIM_UNDEFINED, 
                             m_spLexeme->eAspect(),
                             CASE_UNDEFINED, 
                             m_spLexeme->eIsReflexive());
    
        if (!m_spInflection->bHasIrregularForm(sg1Hash.sGramHash()))
        {
            return false;
        }

        CGramHasher sg3Hash (SUBPARADIGM_PRESENT_TENSE, 
                             NUM_SG, 
                             GENDER_UNDEFINED, 
                             PERSON_3, 
                             ANIM_UNDEFINED,
                             m_spLexeme->eAspect(),
                            CASE_UNDEFINED, 
                            m_spLexeme->eIsReflexive());
    
        if (!m_spInflection->bHasIrregularForm(sg3Hash.sGramHash()))
        {
            return false;   // isolated forms are permitted
        }
    }
    catch (CException& ex)
    {
        assert(0);
        CEString sMsg (L"Error checking for irregular present forms of ");
        sMsg += m_spLexeme->sSourceForm();
        sMsg += L"; ";
        sMsg += ex.szGetDescription();
        ERROR_LOG (sMsg);
        throw (ex);
    }

    return true;

}   //  bHasIrregularPresent()

bool CFormBuilderConj::bHasIrregularPast()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    try
    {
        if (!m_spLexeme->bHasIrregularForms())
        {
            return false;
        }

        // Must have at least two forms: m and f (GDRL, p. 89)
        CGramHasher mSgHash (SUBPARADIGM_PAST_TENSE, 
                             NUM_SG, 
                             GENDER_M, 
                             PERSON_UNDEFINED, 
                             ANIM_UNDEFINED,
                             m_spLexeme->eAspect(),
                             CASE_UNDEFINED, 
                             m_spLexeme->eIsReflexive());

        if (!m_spInflection->bHasIrregularForm(mSgHash.sGramHash()))
        {
            return false;
        }

        CGramHasher fSgHash (SUBPARADIGM_PAST_TENSE, 
                             NUM_SG, 
                             GENDER_F, 
                             PERSON_UNDEFINED, 
                             ANIM_UNDEFINED,
                             m_spLexeme->eAspect(),
                             CASE_UNDEFINED, 
                             m_spLexeme->eIsReflexive());

        if (!m_spInflection->bHasIrregularForm(fSgHash.sGramHash()))
        {
            return false;   // isolated forms are permitted?
        }
    }
    catch (CException& ex)
    {
        assert(0);
        CEString sMsg (L"Error checking for irregular past forms of ");
        sMsg += m_spLexeme->sSourceForm();
        sMsg += L"; ";
        sMsg += ex.szGetDescription();
        ERROR_LOG (sMsg);
    }

    return true;

}   //  bHasIrregularPast()

bool CFormBuilderConj::bHasIrregularImperative()
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    try
    {
        if (!m_spLexeme->bHasIrregularForms())
        {
            return false;
        }

        // Must have at least sg form
        CGramHasher sg2Hash (SUBPARADIGM_IMPERATIVE, 
                             NUM_SG, 
                             GENDER_UNDEFINED, 
                             PERSON_2, 
                             ANIM_UNDEFINED, 
                             m_spLexeme->eAspect(),
                             CASE_UNDEFINED, 
                             m_spLexeme->eIsReflexive());
            
        return m_spInflection->bHasIrregularForm(sg2Hash.sGramHash());
    }
    catch (CException& ex)
    {
        assert(0);
        CEString sMsg (L"Error checking for irregular imperative forms of ");
        sMsg += m_spLexeme->sSourceForm();
        sMsg += L"; ";
        sMsg += ex.szGetDescription();
        ERROR_LOG (sMsg);
        throw CException (H_ERROR_GENERAL, sMsg);
    }

}   //  bHasIrregularImperative

//
// Get irregular form; fill stress pos if it is monosyllabic
//
ET_ReturnCode CFormBuilderConj::eGetIrregularForms (CEString sHash, vector<shared_ptr<CWordForm>>& vecForms)
{
    assert(m_spLexeme);   // we assume base class ctor took care of this

    ET_ReturnCode rc = H_NO_ERROR;
    
    map<shared_ptr<CWordForm>, bool> mapIrregularForms;
    rc = m_spInflection->eGetIrregularForms(sHash, mapIrregularForms);
    if (rc != H_NO_ERROR)
    {
        return rc;
    }

    auto it = mapIrregularForms.begin();
    for (; it != mapIrregularForms.end(); ++it)
    {
        auto spWf = it->first;
        if (spWf->m_mapStress.empty())
        {
            CEString sWf(spWf->m_sWordForm);
            sWf.SetVowels (CEString::g_szRusVowels);
            if (sWf.uiGetNumOfSyllables() != 1)
            {
                assert(0);
                CEString sMsg(L"No stress position given for a polysyllabic irregular form.");
                ERROR_LOG(sMsg);
                throw CException(H_ERROR_UNEXPECTED, sMsg);
            }
            int iStressPos = sWf.uiFindFirstOf(CEString::g_szRusVowels);
            spWf->m_mapStress[iStressPos] = STRESS_PRIMARY;
        }
        vecForms.push_back(spWf);
    }

    return H_NO_ERROR;

}   //  eGetIrregularForms (...)
