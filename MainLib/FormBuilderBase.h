#ifndef FORMBUILDERBASE_H_INCLUDED
#define FORMBUILDERBASE_H_INCLUDED

#include "Enums.h"
#include "EString.h"

#include "Lexeme.h"
#include "Inflection.h"
#include "WordForm.h"
#include "SqliteWrapper.h"
#include "Endings.h"

using namespace std;
//using namespace std::tr1;

namespace Hlib
{

class CFormBuilder
{
public:
    CFormBuilder (CLexeme* pLexeme, CInflection* pInflection, ET_Subparadigm eSubparadigm);

    virtual ~CFormBuilder()
    {
        //
        // Delete a member that may have been created by a subclass
        //
//        if (m_pEndings)
//        {
//            delete m_pEndings;
//        }
    }

    void SetUsageStatus (ET_Status eStatus)
    {
        m_eStatus = eStatus;
    }

protected:
    void ReportDbError();
//    void CloneWordForm (const shared_ptr<CWordForm> spSource, shared_ptr<CWordForm>& spClone);

protected:
    CLexeme* m_pLexeme;
    CInflection* m_pInflection;
    ET_Subparadigm m_eSubparadigm;
    shared_ptr<CEndings> m_spEndings;
    ET_Status m_eStatus;

private:
    CFormBuilder() {}

};

}   // namespace Hlib

#endif      //   FORMBUILDERBASE_H_INCLUDED