#ifndef FORMBUILDERVERBIMPV_H_INCLUDED
#define FORMBUILDERVERBIMPV_H_INCLUDED

#include "Enums.h"
#include "EString.h"

#include "WordForm.h"
#include "SqliteWrapper.h"
#include "FormBuilderBaseConj.h"

using namespace std;
//using namespace std::tr1;

namespace Hlib
{

class CFormBuilderImperative : public CFormBuilderConj
{
public:
    CFormBuilderImperative (shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection) : 
        CFormBuilderConj (spLexeme, spInflection, SUBPARADIGM_IMPERATIVE), m_bIrregularFormsOptional(false)
    {}

public:
    ET_ReturnCode eGetStem(CEString&);
    ET_ReturnCode eGetEndingType (const CEString& sStem, int& iType);
                            // Ending type is an integer NOT defined in GD but rather assigned in the endings source file
                            // based on various in order to simplify DB structure. May need to be refactored.
    ET_ReturnCode eGetStressPositions (const CEString& sStem, 
                                       const CEString& sEnding,
                                       vector<int>& vecStressPositions);
    ET_ReturnCode eCreateFormTemplate (const CEString& sStem,
                                       CEString& sEnding,
                                       int64_t llEndingDataId,
                                       ET_Number eNumber,
                                       shared_ptr<CWordForm>& pWordForm);
    ET_ReturnCode eHandleCommonDeviations (const CEString& sStem, int& iVariantEndingType);
    ET_ReturnCode eCheckForIrregularForms (bool& bIsVariant); // check if irregular forms were built and
                                                        // regular processing must be skipped

    ET_ReturnCode eBuild();
    ET_ReturnCode eAssembleWordForm(ET_Number eNumber, const CEString& sStem, int iEndingType, CWordForm *& pWordForm);
    ET_ReturnCode eBuildIrregularForms();

protected:
    bool m_bIrregularFormsOptional;

};

}

#endif      //  FORMBUILDERVERBIMPV_H_INCLUDED
