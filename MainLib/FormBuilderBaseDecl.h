#pragma once
#include "FormBuilderBase.h"

using namespace std;
using namespace std::tr1;

class CFormBuilderBaseDecl : public CFormBuilderBase
{
public:

    //
    // Use with adjectives
    //
    CFormBuilderBaseDecl (CLexeme * pLexeme, ET_Subparadigm eSubparadigm) : 
        CFormBuilderBase (pLexeme, eSubparadigm),
        m_bFleetingVowel (pLexeme->m_bFleetingVowel),
        m_bYoAlternation (pLexeme->m_bYoAlternation),
        m_bOAlternation (pLexeme->m_bOAlternation),
        m_bFleetingVowelPresent (false)
    {}

    //
    // Use with participles
    //
    CFormBuilderBaseDecl (CLexeme * pLexeme, 
                          ET_Subparadigm eSubparadigm, 
                          bool bFleetingVowel, 
                          bool bYoAlternation) : 
    CFormBuilderBase (pLexeme, eSubparadigm),
    m_bFleetingVowel (bFleetingVowel),
    m_bYoAlternation (bYoAlternation),
    m_bOAlternation (pLexeme->m_bOAlternation),
    m_bFleetingVowelPresent (false)
    {}

protected:
    HRESULT hFleetingVowelCheck (ET_Number eNumber,              // in
                                 ET_Case eCase,                  // in
                                 ET_Gender eGender,              // in
                                 ET_StressLocation eStressType,  // in
                                 ET_Subparadigm eSubparadigm,    // in
                                 CEString& sEnding,              // in/out (can be modified in forms
                                                                 // башен/древен, GDRL p. 30
                                 CEString& sLemma);              // out

    HRESULT hFleetingVowelAdd (ET_Number eNumber,               // in
                               ET_Case eCase,                   // in
                               ET_Gender eGender,               // in
                               ET_StressLocation eStressType,   // in
                               ET_Subparadigm eSubparadigm,     // in
                               CEString& sLemma);               // out

    HRESULT hFleetingVowelRemove (ET_Gender eGender,             // in
                                  ET_Subparadigm eSubparadigm,   // in
                                  const CEString& sEnding,       // in
                                  CEString& sLemma);             // out

    HRESULT hGetStemStressPositions (const CEString& sLemma, 
                                     ET_Subparadigm eSubParadigm,
                                     vector<int>& vecPositions);

    HRESULT hGetEndingStressPosition (const CEString& sLemma, 
                                      const CEString& sEnding,
                                      int& iPosition);

    HRESULT hHandleYoAlternation (ET_StressLocation eStressType, 
                                  int iStressPos,
                                  CEString& sLemma,
                                  const CEString& sEnding);

protected:
    bool m_bFleetingVowel;
    bool m_bFleetingVowelPresent;
    bool m_bYoAlternation;
    bool m_bOAlternation;
};
