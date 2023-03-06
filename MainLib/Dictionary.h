#ifndef C_DICTIONARY_H_INCLUDED
#define C_DICTIONARY_H_INCLUDED

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <memory>

#include "Enums.h"
#include "EString.h"
#include "IDictionary.h"
#include "LexemeProperties.h"
#include "InflectionProperties.h"

namespace Hlib
{

class CDictionary;
class CLexeme;
class CInflection;
class CSqlite;
class CWordForm;
class CParser;
class CAnalytics;
class CVerifier;

typedef void(*PROGRESS_CALLBACK_CLR) (int iPercentDone, bool bOperationComplete);
typedef void(*PROGRESS_CALLBACK_PYTHON) (int iPercentDone, bool bOperationComplete, int iRecord, double dDuration);

class CLexemeEnumerator
{
public:
    CLexemeEnumerator() = delete;
    CLexemeEnumerator(shared_ptr<CDictionary> spDict) : m_spDictionary(spDict) {}

    ET_ReturnCode eReset();

    ET_ReturnCode eGetFirstLexeme(shared_ptr<CLexeme>&);
    ET_ReturnCode eGetNextLexeme(shared_ptr<CLexeme>&);

private:
    vector<shared_ptr<CLexeme>>::iterator m_itCurrentLexeme;
    shared_ptr<CDictionary> m_spDictionary;
};

class CDictionary : public enable_shared_from_this<CDictionary>
{
    friend class CLexemeEnumerator;

public:
    CDictionary();
    ~CDictionary();

public:
    ET_ReturnCode eSetDbPath(const CEString& sDbPath);
    CEString sGetDbPath();
    shared_ptr<CSqlite> spGetDb();
    ET_ReturnCode eCreateLexemeForEdit(shared_ptr<CLexeme>&);
    ET_ReturnCode eCopyEntryForEdit(const shared_ptr<CLexeme> spLSource, const shared_ptr<CInflection> spISource, 
                                    shared_ptr<CLexeme>& spLCopy, shared_ptr<CInflection>& spICopy);
    ET_ReturnCode eGetLexemeById(long long Id, shared_ptr<CLexeme>& spLexeme);
    ET_ReturnCode eGetLexemesByHash(const CEString& sMd5);
//    ET_ReturnCode eGetLexemesByGraphicStem(const CEString&);
    ET_ReturnCode eGetLexemesByInitialForm(const CEString&);
    ET_ReturnCode eGenerateAllForms();
//    ET_ReturnCode eGenerateFormsForSelectedLexemes();
    ET_ReturnCode eCountLexemes(int64_t& iLexemes);

//    ET_ReturnCode eGetFirstLexeme(shared_ptr<CLexeme>& pLexemeItf);
//    ET_ReturnCode eGetNextLexeme(shared_ptr<CLexeme>& pLexemeItf);

    int nLexemesFound();
//    ET_ReturnCode eGetFirstLexeme(shared_ptr<CLexeme>& pLexeme);
//    ET_ReturnCode eGetNextLexeme(shared_ptr<CLexeme>& pLexeme);

    void Clear();
    ET_ReturnCode Clear(shared_ptr<CLexeme>);

    ET_ReturnCode eCreateLexemeEnumerator(shared_ptr<CLexemeEnumerator>&);
    void DeleteLexemeEnumerator(shared_ptr<CLexemeEnumerator>);

    ET_ReturnCode eGetParser(shared_ptr<CParser>&);
    ET_ReturnCode eGetAnalytics(shared_ptr<CAnalytics>&);
    ET_ReturnCode eGetVerifier(shared_ptr<CVerifier>&);

#ifdef WIN32
    ET_ReturnCode eExportTestData(CEString& sPath, PROGRESS_CALLBACK_CLR);
    ET_ReturnCode eImportTestData(CEString& sPath, PROGRESS_CALLBACK_CLR);
#endif

    ET_ReturnCode eDeleteLexeme(shared_ptr<CLexeme>);
    ET_ReturnCode eUpdateHeadword(shared_ptr<CLexeme>);
    ET_ReturnCode eSaveNewHeadword(shared_ptr<CLexeme>);
    ET_ReturnCode eSaveHeadwordStress(shared_ptr<CLexeme>);
    ET_ReturnCode eSaveHomonyms(shared_ptr<CLexeme>);
    ET_ReturnCode eSaveAspectPairInfo(shared_ptr<CLexeme>);
    ET_ReturnCode eSaveP2Info(shared_ptr<CLexeme>);
    ET_ReturnCode eUpdateDescriptorInfo(shared_ptr<CLexeme>);
    ET_ReturnCode eSaveDescriptorInfo(shared_ptr<CLexeme>);
    ET_ReturnCode eSaveCommonDeviation(shared_ptr<CInflection>);
    ET_ReturnCode eSaveInflectionInfo(shared_ptr<CInflection>);
//    ET_ReturnCode eSaveMissingFormHashes(shared_ptr<CLexeme> pLexeme);
//    ET_ReturnCode eSaveLexeme(shared_ptr<CLexeme> pLexeme);

    ET_ReturnCode ePopulateHashToDescriptorTable(PROGRESS_CALLBACK_CLR, PROGRESS_CALLBACK_PYTHON=nullptr);

//    shared_ptr<CSqlite> spGetDbHandle();

//    ET_ReturnCode eGetSecondPart(long long Id, shared_ptr<CLexeme>& pLexeme);       // non-itf version used to get second part of a xurda-murda compound
    ET_ReturnCode eGetSecondPart(long long llId, shared_ptr<CLexeme>& pLexeme);


private:
    CEString m_sDbPath;
    shared_ptr<CSqlite> m_spDb;
    shared_ptr<CParser> m_spParser;
    shared_ptr<CAnalytics> m_spAnalytics;
    shared_ptr<CVerifier> m_spVerifier;
    vector<shared_ptr<CLexeme>> m_vecLexemes;
    shared_ptr<CLexemeEnumerator> m_spLexemeEnumerator;
    vector<shared_ptr<CLexeme>>::iterator m_itCurrentLexeme;
    vector<shared_ptr<CWordForm>>::iterator m_itCurrentWordForm;

    ET_ReturnCode eInit();

    // Populate DB tables
    ET_ReturnCode ePopulateStemsTable();
    ET_ReturnCode eUpdateStemIds();
    ET_ReturnCode ePopulateWordFormDataTables();

    // Execute DB query and retrieve results
    ET_ReturnCode eQueryDb(const CEString& sSelect, uint64_t& uiQuertHandle);
    ET_ReturnCode eReadDescriptorData(shared_ptr<CLexeme>, uint64_t uiQuertHandle, bool bIsSpryazhSm = false);
    ET_ReturnCode eReadInflectionData(shared_ptr<CLexeme>, uint64_t uiQueryHandle, bool bIsSpryazhSm);

    ET_ReturnCode eReadFromDb(int64_t llLexemeId);
    ET_ReturnCode eReadFromDb(const CEString& sStem);

    ET_ReturnCode eGetP2Data(int64_t llLexemeId, StLexemeProperties& properties);
    ET_ReturnCode eGetWordIdFromLemexeId(int64_t llLexemeId, int64_t& llWordId);

    ET_ReturnCode eMarkLexemeAsEdited(shared_ptr<CLexeme> pLexeme);

//    ET_ReturnCode eDifficultFormLabelToGramHash(const CEString sLabel);

    void HandleDbException(CException&);
};

}   // namespace Hlib

#endif // C_DICTIONARY_H_INCLUDED