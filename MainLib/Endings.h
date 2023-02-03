#ifndef ENDINGS_H_INCLUDED
#define ENDINGS_H_INCLUDED

#include <vector>
#include <map>
#include "SqliteWrapper.h"
#include "Enums.h"
#include "EString.h"

using namespace std;

namespace Hlib
{

class CLexeme;
class CInflection;

class CEndings
{
public:
    CEndings(shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection) 
        : m_spLexeme(spLexeme), m_spInflection(spInflection), m_ullDbKey(0)
    {
        m_sEnding.SetVowels(CEString::g_szRusVowels);
    }

    virtual ~CEndings()
    {}

    virtual void Reset()
    {
        m_vecEndings.clear();
    }

    int iCount()
    {
        return (int)m_vecEndings.size();
    }

    ET_ReturnCode eGetEnding(int iSeqNum, CEString& sEnding, int64_t& llEndingKey);
    ET_ReturnCode eGetEnding (CEString& s_, int64_t& llKey)
    {
        return eGetEnding (0, s_, llKey);
    }

protected:
    void ReportDbError();
    std::vector<pair<uint64_t, CEString> > m_vecEndings;

    shared_ptr<CLexeme> m_spLexeme;
    shared_ptr<CInflection> m_spInflection;
    uint64_t m_ullDbKey;
    CEString m_sEnding;

    ET_Gender m_eGender;
    ET_Number m_eNumber;
    ET_Case m_eCase;
    ET_Person m_ePerson;
    ET_Animacy m_eAnimacy;
    ET_StressLocation m_eStressPos;
    ET_StemAuslaut m_eStemAuslaut;
    int m_iInflectionType;

};      // class CEndings

class CNounEndings : public CEndings
{
public:
    CNounEndings(shared_ptr<CLexeme>, shared_ptr<CInflection>);
    ET_ReturnCode eSelect(ET_Number, ET_Case, ET_StressLocation);
};

class CAdjLongEndings : public CEndings
{
public:
    CAdjLongEndings(shared_ptr<CLexeme>, shared_ptr<CInflection>, ET_Subparadigm);

    ET_ReturnCode eSelect(ET_Subparadigm, ET_Gender, ET_Number, ET_Case, ET_Animacy);

protected:
    ET_Subparadigm m_eSubparadigm;

};

class CAdjShortEndings : public CEndings
{
public:
    CAdjShortEndings(shared_ptr<CLexeme>, shared_ptr<CInflection>, ET_Subparadigm);

    ET_ReturnCode eSelect(ET_Gender, ET_Number, ET_StressLocation);

protected:
    ET_Subparadigm m_eSubparadigm;

};

class CAdjPronounEndings : public CEndings
{
public:
    CAdjPronounEndings(shared_ptr<CLexeme>, shared_ptr<CInflection>);

    ET_ReturnCode eSelect(ET_Gender, ET_Number, ET_Case, ET_Animacy);

};

class CPersonalEndings : public CEndings
{
public:
    CPersonalEndings(shared_ptr<CLexeme>, shared_ptr<CInflection>);

    ET_ReturnCode eSelect(ET_Person, ET_Number, ET_StressLocation, ET_StemAuslaut);

};

class CInfinitiveEndings : public CEndings
{
public:
    CInfinitiveEndings(shared_ptr<CLexeme>, shared_ptr<CInflection>);

    ET_ReturnCode eSelect(int iInflectionType);

};

class CPastTenseEndings : public CEndings
{
public:
    CPastTenseEndings(shared_ptr<CLexeme>, shared_ptr<CInflection>);

    ET_ReturnCode eSelect(ET_Gender, ET_Number, ET_StemAuslaut);

};

class CImperativeEndings : public CEndings
{
public:
    CImperativeEndings(shared_ptr<CLexeme>, shared_ptr<CInflection>);

    ET_ReturnCode eSelect(ET_Number, int iType, bool bIsVariant = false);

};

class CAdverbialEndings : public CEndings
{
public:
    CAdverbialEndings(shared_ptr<CLexeme>, shared_ptr<CInflection>);

    ET_ReturnCode eSelect(ET_Subparadigm, bool bHusherStem, bool bVStem, bool bIsVariant);
    ET_ReturnCode eSelectPastAdvAugmentedEndings(ET_Subparadigm, bool bIsVariant);
};

class CComparativeEndings : public CEndings
{
public:
    CComparativeEndings(shared_ptr<CLexeme>, shared_ptr<CInflection>);

    ET_ReturnCode eSelect(bool bVelarStem, bool bIsVariant);
};

} // namespace Hlib

#endif // ENDINGS_H_INCLUDED
