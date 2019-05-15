#ifndef C_PARSER_H_INCLUDED
#define C_PARSER_H_INCLUDED

#include "IParser.h"

namespace Hlib
{

class CSqlite;
class CParsingTree;
struct CWordForm;

class CParser : public IParser
{

public:
    CParser(CSqlite *);
    ~CParser();

public:
    virtual ET_ReturnCode eParseWord(const CEString& sWord);
    virtual ET_ReturnCode eGetFirstWordForm(IWordForm *& pWordForm);
    virtual ET_ReturnCode eGetNextWordForm(IWordForm *& pWordForm);
    virtual void ClearResults();

    void SetDb(CSqlite * pDb);

private:
    CParser();  // no use

    CSqlite * m_pDb;
    CParsingTree * m_pEndingsTree;
    vector<CWordForm *> m_vecWordForms;
    vector<CWordForm *>::iterator m_itCurrentWordForm;


    ET_ReturnCode eIrregularFormLookup(const CEString&);
    ET_ReturnCode eWholeWordLookup(const CEString&);
    ET_ReturnCode eFormLookup(const CEString&);
    ET_ReturnCode eQueryDb(const CEString& sSelect, uint64_t& uiQueryHandle);
    void HandleDbException(CException& ex);
};

}   // namespace Hlib

#endif // C_PARSER_H_INCLUDED