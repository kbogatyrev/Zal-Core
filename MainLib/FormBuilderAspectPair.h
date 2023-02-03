#ifndef ASPECTPAIRBUILDER_H_INCLUDED
#define ASPECTPAIRBUILDER_H_INCLUDED

#include "Enums.h"
#include "EString.h"

namespace Hlib
{
    class CLexeme;
    class CWordForm;

	class CFormBuilderAspectPair
	{
    protected:
        enum class ET_ExtraData
        {
            None,
            Vowel,
            Suffix,
            WholeWord
        };

        typedef map<int, ET_StressType> ET_StressMap;

    protected:
        shared_ptr<CLexeme> m_spLexeme;
        shared_ptr<CInflection> m_spInflection;
        shared_ptr<CWordForm> m_sp1stPersonWordForm;
        shared_ptr<CWordForm> m_spInfWordForm;
        ET_ExtraData m_eExtraData;
        ET_ExtraData m_eAltExtraData;
        CEString m_sComment;
        CEString m_sSource;
        CEString m_sReflexiveSuffix;
        CEString m_sAspectPairSource;
        CEString m_sAltAspectPairSource;
        int m_iStressPos;
        int m_iAltStressPos;
        bool m_bBuilt;
        bool m_bError;

	public:
        CFormBuilderAspectPair(shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection) : m_spLexeme(spLexeme), 
                               m_spInflection(spInflection), m_sp1stPersonWordForm(nullptr), m_spInfWordForm(nullptr),
            m_eExtraData(ET_ExtraData::None), m_bBuilt(false), m_bError(false)
		{}

        ET_ReturnCode eBuild();
        ET_ReturnCode eGetAspectPair(CEString&, int&);
        ET_ReturnCode eGetAltAspectPair(CEString&, int&);

    protected:      // helpers
        ET_ReturnCode eFindStressPositionI(bool bIsVariant = false);
        ET_ReturnCode eExtractStressMark(bool bIsVariant);
        ET_ReturnCode eGet1PersonWordForm();
        ET_ReturnCode eGetInfinitiveWordForm();
        ET_ReturnCode eGetInfinitiveStressPos(int&);
        ET_ReturnCode eGetNsvDerivationType(shared_ptr<CWordForm> spInfinitive, int& iType);

	protected:      // build
		ET_ReturnCode eSvToNsvTypeI(bool bIsVariant = false);
        ET_ReturnCode eBuildTypeIa(const CEString& sStem, CEString& sOutput);
        ET_ReturnCode eBuildTypeIb(const CEString& sStem, CEString& sOutput);
        ET_ReturnCode eBuildTypeIc(CEString& sOutput);
        ET_ReturnCode eSvToNsvTypeII(bool bIsVariant = false);
        ET_ReturnCode eSvToNsvTypeIII(bool bIsVariant = false);
        ET_ReturnCode eNsvToSvTypes1_5_6(bool bIsVariant = false);
        ET_ReturnCode eNsvToSvTypes2(bool bIsVariant = false);
        ET_ReturnCode eNsvToSvTypes3(bool bIsVariant = false);
        ET_ReturnCode eNsvToSvTypes4(bool bIsVariant = false);
        ET_ReturnCode eNsvToSvTypes7(bool bIsVariant = false);
        ET_ReturnCode eNsvToSvTypes8(bool bIsVariant = false);
        ET_ReturnCode eNsvToSvTypes9(bool bIsVariant = false);
        ET_ReturnCode eNsvToSvTypes10(bool bIsVariant = false);
        ET_ReturnCode eNsvToSvTypes14(bool bIsVariant = false);
        ET_ReturnCode eNsvToSvTypes11_12_13_15_16(bool bIsVariant = false);
        ET_ReturnCode eApplyNsvVowelModification(bool bIsVariant = false);
        ET_ReturnCode eApplySvVowelModification(bool bIsVariant = false);
        ET_ReturnCode eApplySvSuffix(bool bIsVariant = false);
        ET_ReturnCode eApplySvWholeWord(bool bIsVariant = false);
    };
}

#endif		// ASPECTPAIRBUILDER_H_INCLUDED