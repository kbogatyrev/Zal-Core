cmake_minimum_required (VERSION 3.16)

project (Zal-Core VERSION 4.6.3)

if (MSVC)
	if (CMAKE_BUILD_TYPE MATCHES "Debug")
		message (STATUS "Windows debug mode")
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
#			/Fa "out/build/x64-Debug" /EHsc /nologo /Fo "out/build/x64-Debug" /Fp "out/build/x64-Debug/ZalMainLib.pch" 
			/diagnostics:column 
		)
	else()
		message (STATUS "Windows release mode")
		add_compile_definitions(
			WIN32
			_WINDOWS
			_UNICODE
			UNICODE
			_CONSOLE
			NDEBUG
		)
		add_compile_options(
			/ifcOutput "out/build/x64-Release/" /GS /GL /W3 /Gy /Zc:wchar_t 
			/Zi /Gm- /O2 #/Fd"C:\dev\Zal-Windows\lib\x64\Release\ZalMainLib.pdb" 
			/Zc:inline /fp:precise 
			/errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MD 
			/std:c++17 /FC 
			#/Fa"x64\Release\" 
			/EHsc /nologo 
			#/Fo"x64\Release\" /Fp"x64\Release\ZalMainLib.pch" 
			/diagnostics:column 
		)
	endif()
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

if (CMAKE_BUILD_TYPE MATCHES "Debug")
	add_custom_command(TARGET ZalMainLib POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ZalMainLib> ../../../../Zal-Windows/TestApp/bin/debug
	)
else()
	add_custom_command(TARGET ZalMainLib POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ZalMainLib> ../../../../Zal-Windows/TestApp/bin/release
	)
endif()
