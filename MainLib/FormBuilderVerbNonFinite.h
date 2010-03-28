#pragma once
#include "resource.h"       // main symbols
#include "MainLib_i.h"
#include "WordForm.h"
#include "SqliteWrapper.h"
#include "Endings.h"
#include "FormBuilderBaseConj.h"

using namespace std;
using namespace std::tr1;

class CT_FormBuilderNonFinite : public CT_FormBuilderBaseConj
{
public:
    CT_FormBuilderNonFinite (CT_Lexeme * pco_lexeme) : CT_FormBuilderBaseConj (pco_lexeme)
    {}

public:
    HRESULT h_Build();

private:
    HRESULT h_BuildInfinitive();
    HRESULT h_BuildPresentActiveParticiple();
    HRESULT h_BuildPresentAdverbial();
    HRESULT h_BuildPastActiveParticiple();
    HRESULT h_BuildPastAdverbial();
};
