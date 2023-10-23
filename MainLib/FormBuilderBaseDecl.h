#ifndef FORMBUILDERBASEDECL_H_INCLUDED
#define FORMBUILDERBASEDECL_H_INCLUDED

#include "Enums.h"
#include "EString.h"

#include "FormBuilderBase.h"

using namespace std;

namespace Hlib
{

class CFormBuilderDecl : public CFormBuilder
{
public:

    //
    // Use with adjectives
    //
    CFormBuilderDecl (CLexeme* pLexeme, CInflection* pInflection, ET_Subparadigm eSubparadigm) : 
        CFormBuilder (pLexeme, pInflection, eSubparadigm),
        m_bFleetingVowel (pInflection->bHasFleetingVowel()),
        m_bFleetingVowelPresent (false),
        m_bYoAlternation (pLexeme->bHasYoAlternation()),
        m_bOAlternation (pLexeme->bHasOAlternation()),
        m_bIrregularSourceForm (false)
    {}

    //
    // Use with participles
    //
    CFormBuilderDecl (CLexeme* pLexeme,
                      CInflection* pInflection,
                      ET_Subparadigm eSubparadigm, 
                      bool bFleetingVowel, 
                      bool bYoAlternation) : 
    CFormBuilder (pLexeme, pInflection, eSubparadigm),
    m_bFleetingVowel (bFleetingVowel),
    m_bFleetingVowelPresent (false),
    m_bYoAlternation (bYoAlternation),
    m_bOAlternation (pLexeme->bHasOAlternation()),
    m_bIrregularSourceForm(false)
    {}

protected:
    ET_ReturnCode eFleetingVowelCheck (ET_Number eNumber,              // in
                                       ET_Case eCase,                  // in
                                       ET_Gender eGender,              // in
                                       ET_StressLocation eStressType,  // in
                                       ET_Subparadigm eSubparadigm,    // in
                                       CEString& sEnding,              // in/out (can be modified in forms
                                                                       // башен/древен, GDRL p. 30
                                       CEString& sStem);               // out

    ET_ReturnCode eFleetingVowelAdd (ET_Number eNumber,               // in
                                     ET_Case eCase,                   // in
                                     ET_Gender eGender,               // in
                                     ET_StressLocation eStressType,   // in
                                     ET_Subparadigm eSubparadigm,     // in
                                     CEString& sStem);                // out

    ET_ReturnCode eFleetingVowelRemove (ET_Gender eGender,             // in
                                        ET_Subparadigm eSubparadigm,   // in
                                        const CEString& sEnding,       // in
                                        CEString& sStem);              // out

    ET_ReturnCode eGetStemStressPositions (const CEString& sStem, 
                                           ET_Subparadigm eSubParadigm,
                                           ET_Number eNumber,
                                           vector<int>& vecPositions);

    ET_ReturnCode eGetEndingStressPosition (const CEString& sStem, 
                                            const CEString& sEnding,
                                            int& iPosition);

    ET_ReturnCode eHandleYoAlternation (ET_StressLocation eStressType, 
                                        int iStressPos,
                                        CEString& sStem,
                                        const CEString& sEnding);

protected:
    bool m_bFleetingVowel;
    bool m_bFleetingVowelPresent;
    bool m_bYoAlternation;
    bool m_bOAlternation;
    bool m_bIrregularSourceForm;
};

}   // namespace Hlib

#endif     //  FORMBUILDERBASEDECL_H_INCLUDED
