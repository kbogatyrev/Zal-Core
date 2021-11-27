cmake_minimum_required (VERSION 3.16)

project (Zal-Core VERSION 4.6.3)

if (MSVC)
	add_compile_definitions(
		WIN32
		_WINDOWS
		_UNICODE
		UNICODE
		_DEBUG
		_CONSOLE
	)
    add_compile_options(
		/JMC /ifcOutput "out/build/x64-Debug/" /GS /W3 /Zc:wchar_t 
		/Zi /Gm- /Od #/Fd "out/build/x64-Debug/ZalMainLib.pdb" 
		/Zc:inline /fp:precise #/D "_DEBUG" 
		/errorReport:prompt /WX- /Zc:forScope /RTC1 /Gd /MDd /std:c++17 /FC 
#		/Fa "out/build/x64-Debug" /EHsc /nologo /Fo "out/build/x64-Debug" /Fp "out/build/x64-Debug/ZalMainLib.pch" 
		/diagnostics:column 
	)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_library (ZalMainLib SHARED)

target_include_directories (ZalMainLib PUBLIC
	../HLib
	../ThirdParty/Sqlite
	../ThirdParty/sha1
)

target_sources (ZalMainLib PRIVATE
	MainLib/Analytics.cpp
	MainLib/Dictionary.cpp
#	MainLib/dllmain.cpp
	MainLib/Endings.cpp
	MainLib/FormBuilderAdjComparative.cpp
	MainLib/FormBuilderAdjLong.cpp
	MainLib/FormBuilderAdjShort.cpp
	MainLib/FormBuilderAspectPair.cpp
	MainLib/FormBuilderBase.cpp
	MainLib/FormBuilderBaseConj.cpp
	MainLib/FormBuilderBaseDecl.cpp
	MainLib/FormBuilderNouns.cpp
	MainLib/FormBuilderPronounAdj.cpp
	MainLib/FormBuilderVerbImpv.cpp
	MainLib/FormBuilderVerbNonFinite.cpp
	MainLib/FormBuilderVerbPast.cpp
	MainLib/FormBuilderVerbPers.cpp
	MainLib/Lexeme.cpp
	MainLib/MainLib.cpp
	MainLib/Mainlib.def
	MainLib/Parser.cpp
	MainLib/ParsingTree.cpp
	MainLib/Transcriber.cpp
	MainLib/Verifier.cpp
	MainLib/WordForm.cpp
	../ThirdParty/Sqlite/sqlite3.c
)

add_custom_command(TARGET ZalMainLib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ZalMainLib> ../../../../Zal-Windows/TestApp/bin/debug
)
