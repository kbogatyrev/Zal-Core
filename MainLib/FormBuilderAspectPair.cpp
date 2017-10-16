#include "EString.h"
#include "Lexeme.h"
#include "WordForm.h"
#include "FormBuilderAspectPair.h"

using namespace Hlib;

ET_ReturnCode CFormBuilderAspectPair::eBuild()
{
	StLexemeProperties stProperties = m_pLexeme->stGetProperties();
	if (!stProperties.bHasAspectPair)
	{
		return H_FALSE;
	}

    if (stProperties.sAspectPairData.bStartsWith(L"-") && stProperties.sAspectPairData.bEndsWith(L"-"))
    {
        m_eExtraData = ET_ExtraData::Vowel;
    }
    else if (stProperties.sAspectPairData.bStartsWith(L"-"))
    {
        m_eExtraData = ET_ExtraData::Suffix;
    }
    else
    {
        m_eExtraData = ET_ExtraData::WholeWord;
    }

	ET_ReturnCode eRet = H_NO_ERROR;
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }
    
    m_sSource = m_pLexeme->sSourceForm();
    if (m_sSource.bEndsWith(L"ся") || m_sSource.bEndsWith(L"сь"))
    {
        m_sReflexiveSuffix = m_sSource.sSubstr(m_sSource.uiLength() - 3);
        m_sSource = m_sSource.sRemoveCharsFromEnd(2);
    }

    eRet = m_pLexeme->eGenerateParadigm();

    switch (stProperties.iAspectPairType)
	{
		case -1:
		{
			eRet = eSvToNsvTypeI();
            if (eRet != H_NO_ERROR)
            { 
                return eRet;
            }

            eRet = eFindStressPositionI();
            if (eRet != H_NO_ERROR)
            {
                return eRet;
            }
            
            if (ET_ExtraData::Vowel == m_eExtraData)
            {
                eRet = eApplyVowelModification();
            }

            break;
		}
        case -2:
        {
            eRet = eSvToNsvTypeII();
            eRet = eFindStressPositionI();
            if (eRet != H_NO_ERROR)
            {
                return eRet;
            }

            break;
        }

		default:
		{}
	}

    m_sAspectPairSource += m_sReflexiveSuffix;

	return eRet;

}       //  eBuild()

ET_ReturnCode CFormBuilderAspectPair::eSvToNsvTypeI()
{
    ET_ReturnCode eRet = H_NO_ERROR;

    if (m_sSource.bEndsWith(L"ать") || m_sSource.bEndsWith(L"оть"))
    {
        CEString sStem = m_sSource.sRemoveCharsFromEnd(3);
        eRet = eBuildTypeIa(sStem);
        return eRet;
    }
    else if (m_sSource.bEndsWith(L"нуть") && !m_sSource.bEndsWith(L"ьнуть"))
    {
        CEString sStem = m_sSource.sRemoveCharsFromEnd(4);
        eRet = eBuildTypeIa(sStem);
        return eRet;
    }

    if (m_sSource.bEndsWith(L"ять"))
    {
        CEString sStem = m_sSource.sRemoveCharsFromEnd(3);
        eRet = eBuildTypeIb(sStem);
        return eRet;
    }
    else if (m_sSource.bEndsWith(L"ьнуть"))
    {
        CEString sStem = m_sSource.sRemoveCharsFromEnd(5);
        eRet = eBuildTypeIb(sStem);
        return eRet;
    }

    // All other stem types:
    eRet = eBuildTypeIc();

    return eRet;

}       // eSvToNsvTypeI()

ET_ReturnCode CFormBuilderAspectPair::eBuildTypeIa(const CEString& sStem)
{
    if (CEString::bIn(sStem[sStem.uiLength() - 1], CEString(g_szHushers) + CEString(L"кгх")))
    {
        m_sAspectPairSource = sStem + L"ивать";
    }
    else
    {
        m_sAspectPairSource = sStem + L"ывать";
    }

    return H_NO_ERROR;
}

ET_ReturnCode CFormBuilderAspectPair::eBuildTypeIb(const CEString& sStem)
{
    m_sAspectPairSource = sStem + L"ивать";
    return H_NO_ERROR;
}

ET_ReturnCode CFormBuilderAspectPair::eBuildTypeIc()
{
    ET_ReturnCode eRet = eGet1PersonWordForm();
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }
    
    if (NULL == m_p1stPersonWordForm)
    {
        return H_ERROR_POINTER;
    }

    if (L"у" == m_p1stPersonWordForm->sEnding())
    {
        if (m_p1stPersonWordForm->sLemma().bEndsWith(CEString(g_szHushers) + CEString(L"кгх")))
        {
            m_sAspectPairSource = m_p1stPersonWordForm->sLemma() + L"ивать";
        }
        else
        {
            m_sAspectPairSource = m_p1stPersonWordForm->sLemma() + L"ывать";
        }
    }

    return H_NO_ERROR;

}       //  eBuildTypeIc()

ET_ReturnCode CFormBuilderAspectPair::eFindStressPositionI()
{
    CWordForm * pInfinitive = NULL;
    CGramHasher hasherInf(POS_VERB, SUBPARADIGM_INFINITIVE, CASE_UNDEFINED, NUM_UNDEFINED,
        GENDER_UNDEFINED, PERSON_UNDEFINED, ANIM_UNDEFINED, m_pLexeme->eAspect(),
        m_pLexeme->eIsReflexive());

    CWordForm * pWordForm = NULL;
    ET_ReturnCode eRet = m_pLexeme->eWordFormFromHash(hasherInf.sGramHash(), 0, pWordForm);

    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (NULL == pWordForm)
    {
        return H_ERROR_POINTER;
    }

    int iStressPos = -1;
    ET_StressType eType;
    eRet = pWordForm->eGetFirstStressPos(iStressPos, eType);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    while (eType != ET_StressType::STRESS_PRIMARY && H_NO_ERROR == eRet)
    {
        eRet = pWordForm->eGetNextStressPos(iStressPos, eType);
    }

    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    try
    {
        int iStressedSyllable = pWordForm->sWordForm().uiGetSyllableFromVowelPos(iStressPos);
        if (pWordForm->sWordForm().bStartsWith(L"вы") ||
            pWordForm->sWordForm().uiNSyllables() - 1 == iStressedSyllable)
        {
            ASSERT(m_sAspectPairSource.bEndsWith(L"ивать") || m_sAspectPairSource.bEndsWith(L"ывать"));
            m_iStressPos = m_sAspectPairSource.uiGetVowelPos(m_sAspectPairSource.uiNSyllables() - 3);
        }
        else
        {
            m_iStressPos = iStressPos;
        }
    }
    catch (CException& ex)
    {
        return H_ERROR_UNEXPECTED;
    }

    return H_NO_ERROR;

}       //  eFindStressPositionI()

ET_ReturnCode CFormBuilderAspectPair::eApplyVowelModification()
{
    StLexemeProperties stProperties = m_pLexeme->stGetProperties();
    if (!stProperties.bHasAspectPair)
    {
        return H_ERROR_UNEXPECTED;
    }

    if (stProperties.sAltAspectPairData.bIsEmpty())
    {
        return H_ERROR_UNEXPECTED;
    }

    CEString sData = stProperties.sAltAspectPairData;
    if (!sData.bStartsWith(L"-") || !sData.bEndsWith(L"-") || sData.uiLength() > 4 || sData.uiLength() < 3)
    {
        return H_ERROR_UNEXPECTED;
    }

    sData = sData.sSubstr(1, sData.uiLength() - 2);

//    bool bStressed = false;
//    if (sData.bStartsWith(L"/"))
//    {
//        bStressed = true;
//        sData = sData.sSubstr(1);
//    }

    if (sData.uiLength() != 1)
    {
        return H_ERROR_UNEXPECTED;
    }

    if (sData != L'ё' && sData != L'о' && sData != L'а')
    {
        return H_ERROR_UNEXPECTED;
    }

    try
    {
        m_sAspectPairSource[m_iStressPos] = sData[0];
    }
    catch (...)
    {
        return H_ERROR_UNEXPECTED;
    }

    return H_NO_ERROR;

}       //  eApplyVowelModification()

ET_ReturnCode CFormBuilderAspectPair::eSvToNsvTypeII()
{
    ET_ReturnCode eRet = H_NO_ERROR;

    if (m_sSource.bEndsWith(L"ать"))
    {
        m_sAspectPairSource = m_sSource;
    }
    else if (m_sSource.bEndsWith(L"нуть"))
    {
        m_sAspectPairSource = m_sSource.sRemoveCharsFromEnd(3) + L"ать";
    }
    else
    {
        eRet = eGet1PersonWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }

        if (NULL == m_p1stPersonWordForm)
        {
            return H_ERROR_POINTER;
        }

        if (m_p1stPersonWordForm->sEnding().bEndsWith(L"у"))
        {
            m_sAspectPairSource = m_p1stPersonWordForm->sLemma() + L"ать";
        }
        else
        {
            ASSERT(m_p1stPersonWordForm->sEnding() == L"ю");
            m_sAspectPairSource = m_p1stPersonWordForm->sLemma() + L"ять";
        }
    }

    m_iStressPos = m_sAspectPairSource.uiGetVowelPos(m_sAspectPairSource.uiNSyllables() - 1);

    return eRet;

}       //  eSvToNsvTypeII()

ET_ReturnCode CFormBuilderAspectPair::eSvToNsvTypeIII()
{
    ET_ReturnCode eRet = H_NO_ERROR;

    if (m_sSource.bEndsWith(L"нуть"))
    {
        m_sAspectPairSource = m_sSource.sRemoveCharsFromEnd(3) + L"вать";
    }
    else
    {
        m_sAspectPairSource = m_sSource.sRemoveCharsFromEnd(2) + L"вать";
    }

    m_iStressPos = m_sAspectPairSource.uiGetVowelPos(m_sAspectPairSource.uiNSyllables() - 1);

    return H_NO_ERROR;
}

ET_ReturnCode CFormBuilderAspectPair::eGet1PersonWordForm()
{
    CGramHasher hasher1Sg(POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_SG,
        GENDER_UNDEFINED, PERSON_1, ANIM_UNDEFINED, m_pLexeme->eAspect(),
        m_pLexeme->eIsReflexive());

    CWordForm * pWordForm = NULL;
    ET_ReturnCode eRet = m_pLexeme->eWordFormFromHash(hasher1Sg.sGramHash(), 0, m_p1stPersonWordForm);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (NULL == pWordForm)
    {
        return H_ERROR_POINTER;
    }

    return H_NO_ERROR;
}
