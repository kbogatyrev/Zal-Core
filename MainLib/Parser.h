#ifndef C_PARSER_H_INCLUDED
#define C_PARSER_H_INCLUDED

#include <memory>
#include <vector>

#include "Dictionary.h"
#include "Enums.h"
#include "EString.h"

//#include "IParser.h"

namespace Hlib
{

class CSqlite;
class CParsingTree;
class CWordForm;
class CLexeme;

class CParser 
{

public:
    CParser() = delete;
    CParser(shared_ptr<CDictionary>);
    ~CParser();

public:
    ET_ReturnCode eParseWord(const CEString& sWord);
    ET_ReturnCode eGetFirstWordForm(shared_ptr<CWordForm>&);
    ET_ReturnCode eGetNextWordForm(shared_ptr<CWordForm>&);
    void ClearResults();

    void SetDb(shared_ptr<CSqlite> pDb);

private:
    shared_ptr<CDictionary> m_spDictionary;
    shared_ptr<CSqlite> m_spDb;
    shared_ptr<CParsingTree> m_spEndingsTree;
    vector<shared_ptr<CWordForm>> m_vecWordForms;
    vector<shared_ptr<CWordForm>>::iterator m_itCurrentWordForm;

    ET_ReturnCode eIrregularFormLookup(const CEString&, bool bIsSpryazgSm = false);
    ET_ReturnCode eWholeWordLookup(const CEString&);
    ET_ReturnCode eFormLookup(const CEString&);
    ET_ReturnCode eLexemeLookup(const CLexeme&);
    ET_ReturnCode eRemoveFalsePositives();
    ET_ReturnCode eQueryDb(const CEString& sSelect, uint64_t& uiQueryHandle);

    void HandleDbException(CException& ex);
};

}   // namespace Hlib

#endif // C_PARSER_H_INCLUDED