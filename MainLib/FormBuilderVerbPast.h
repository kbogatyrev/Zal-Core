#ifndef FORMBUILDERVERBPAST_H_INCLUDED
#define FORMBUILDERVERBPAST_H_INCLUDED

#include "Enums.h"
#include "EString.h"

#include "FormBuilderBaseConj.h"

using namespace std;

namespace Hlib
{
    class CFormBuilderPast : public CFormBuilderConj
    {
        struct stStressData
        {
            ET_StressLocation eType;
            int iPosition;

            stStressData (ET_StressLocation eType, int iPos) : eType (eType), iPosition (iPos)
            {}
        };

    public:
        CFormBuilderPast (shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection) 
            : CFormBuilderConj (spLexeme, spInflection, SUBPARADIGM_PAST_TENSE)
        {}

    public:
//        ET_ReturnCode eGetEnding (CEString& sLemma, ET_Number eNumber, ET_Gender eGender, CEString& sEnding);

        ET_ReturnCode eGetStressPositions (const CEString& sLemma,
                                           const CEString& sEnding,
                                           ET_Number eNumber, 
                                           ET_Gender eGender, 
                                           vector<int>& vecStressPositions);

        ET_ReturnCode eCreateFormTemplate (const CEString& sLemma,
                                           const CEString& sEnding,
                                           ET_Number eNumber, 
                                           ET_Gender eGender, 
                                           int64_t llEndingKey,
                                           shared_ptr<CWordForm>& spWordForm);

        ET_ReturnCode eAssemble(shared_ptr<CWordForm> spWordForm, int iStressPos, const CEString& sStem, CEString& sEnding);

        ET_ReturnCode eBuild();

        ET_ReturnCode eBuildIrregular(bool&);

        ET_ReturnCode eRetractStressToPreverb(shared_ptr<CWordForm>, bool bIsOptional);

    };

}       //  namespace Hlib

#endif      //  FORMBUILDERVERBPAST_H_INCLUDED
