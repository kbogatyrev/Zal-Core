#ifndef FORMBUILDERVERBPERS_H_INCLUDED
#define FORMBUILDERVERBPERS_H_INCLUDED

#include "Enums.h"
#include "EString.h"

#include "FormBuilderBaseConj.h"

using namespace std;

namespace Hlib
{

    class CFormBuilderPersonal : public CFormBuilderConj
    {
    public:
        CFormBuilderPersonal (CLexeme* pLexeme, CInflection* pInflection) : 
            CFormBuilderConj (pLexeme, pInflection, SUBPARADIGM_PRESENT_TENSE)
        {}

    public:
        ET_ReturnCode eGetStressType (ET_Number, ET_Person,  ET_StressLocation&);
        ET_ReturnCode eGetAuslautType (ET_Number, ET_Person, ET_StemAuslaut&);
        ET_ReturnCode eGetStem(ET_Number, ET_Person, CEString&);
        ET_ReturnCode eGetStressPositions (const CEString& sLemma,
                                           const CEString& sEnding,
                                           ET_StressLocation eStressType,
                                           vector<int>& vecStressPositions);
    //    ET_ReturnCode h_FleetingVowelCheck (wstring& str_verbForm);
        ET_ReturnCode eCreateFormTemplate (ET_Number, ET_Person, shared_ptr<CWordForm>&);
        ET_ReturnCode eBuild();
        ET_ReturnCode eHandleIrregularForms(); // get from the dictionary or generate if abbreviated paradigm was used
        ET_ReturnCode eBuildIrregularForms (ET_Number, ET_Person); // ... when the dictionary only provides base forms

    protected:
        void CreateIrregular1SgStem(const CEString& s1SgForm);
        void CreateIrregular3SgStem(const CEString& s3SgForm);

    };

}       //  namespace Hlib

#endif      //  FORMBUILDERVERBPERS_H_INCLUDED
