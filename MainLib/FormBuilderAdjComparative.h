#ifndef FORMBUILDERADJCOMPARATIVE_H_INCLUDED
#define FORMBUILDERADJCOMPARATIVE_H_INCLUDED

#include "FormBuilderBaseDecl.h"

using namespace std;
using namespace std::tr1;

class CLexeme;

class CFormBuilderComparative : public CFormBuilderBaseDecl
{
public:
    CFormBuilderComparative (CLexeme * pLexeme) 
        : CFormBuilderBaseDecl (pLexeme, SUBPARADIGM_COMPARATIVE)
    {}

public:
    ET_ReturnCode eCreateFormTemplate (const CEString& sLemma, CWordForm *& pWordForm);
    ET_ReturnCode eHandleStressAndAdd (CWordForm * pWordForm, vector<int>& vecIStress);
    ET_ReturnCode eBuild();

};

#endif
