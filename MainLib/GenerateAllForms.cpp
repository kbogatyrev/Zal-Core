#include <iostream>
#include <memory>

#include <toml.hpp>

#include "Singleton.h"
#include "Dictionary.h"

using namespace Hlib;

int main(int argc, char* argv[]) 
{
    if (argc < 2)
    {
        ERROR_LOG(L"Argument expected: \'generate_all_forms.toml\'");
        return H_ERROR_INVALID_ARG;
    }

    toml::table config;

    try
    {
        string path = argv[1];
        config = toml::parse_file(path);
    }
    catch (const toml::parse_error& err)
    {
        CEString sMsg(L"Unable to parse config.");
        sMsg += Hlib::CEString::sFromUtf8(err.description().data());
        ERROR_LOG(sMsg)
            return H_ERROR_UNEXPECTED;
    }

#ifdef WIN32
    auto utf8DbPath = config["paths"]["db_path_windows"].as_string()->get();
#else
    auto utf8DbPath = config["paths"]["db_path_linux"].as_string()->get();
#endif

    auto sDbPath = Hlib::CEString::sFromUtf8(utf8DbPath);

//    _setmode(_fileno(stdout), _O_U16TEXT);

//    std::cout << L"Hello world!" << std::endl;
     
    auto pSingleton = Hlib::Singleton::pGetInstance();

    shared_ptr<CDictionary> spDictionary;
    auto rc = pSingleton->eGetDictionary(spDictionary);
    spDictionary->eSetDbPath(sDbPath);
    rc = spDictionary->eGenerateAllForms();
    if (rc != H_NO_ERROR && rc != H_NO_MORE) {
        std::cout << "Error " << rc << std::endl;
    }
    return 0;
}
