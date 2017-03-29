cmake_minimum_required(VERSION 3.4)


################################################################################
# define our project
project(ColorPicker4MoDao)

################################################################################
# set our executable
# add_executable(ColorPicker4MoDao src/main.cxx)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_executable(ColorPicker4MoDao WIN32 src/main.cxx)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_executable(ColorPicker4MoDao MACOSX_BUNDLE src/main.cxx)
endif()

target_sources(ColorPicker4MoDao PRIVATE src/resource.cxx)
target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/GlobalEventHook.hxx)
target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/GlobalEventHook.cxx)
target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/ColorPickerCanvas.hxx)
target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/ColorPickerCanvas.cxx)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/GlobalEventHook.win.cxx)
    target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/ColorPickerCanvas.win.cxx)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/GlobalEventHook.mac.mm)
    target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/ColorPickerCanvas.mac.mm)
    set_target_properties(ColorPicker4MoDao PROPERTIES MACOSX_BUNDLE_INFO_PLIST
                          ${CMAKE_CURRENT_SOURCE_DIR}/res/Info.plist.in)
endif()


################################################################################
# using this handle Qt5 static link
include(3rd/StaticQt5Base/StaticLinkQt5.cmake)
# I don't like qt keywords
add_definitions(-DQT_NO_KEYWORDS)
# auto generate moc files for the target
smart_qt5_static_link(ColorPicker4MoDao)
smart_qt5_static_link_os_extra(ColorPicker4MoDao)


################################################################################
# set our c++ compiler

if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)

    add_definitions(-DUNICODE)
    add_definitions(-D_UNICODE)

    set(C_FLAGS_COMM "/nologo /Zc:wchar_t- /Zc:strictStrings /MP /W3")
    set(CXX_FLAGS_COMM "/nologo /Zc:wchar_t- /Zc:strictStrings /MP /W3 /GR /EHsc /std:c++14")

    set(C_FLAGS_DEBUG "/DDEBUG /MTd")
    set(CXX_FLAGS_DEBUG "/DDEBUG /MTd")

    set(C_FLAGS_RELEASE "/DNDEBUG /MT")
    set(CXX_FLAGS_RELEASE "/DNDEBUG /MT")

    set(LINKER_FLAGS_COMM "/NOLOGO /DYNAMICBASE /NXCOMPAT /INCREMENTAL:NO /DEBUG")

    set(LINKER_FLAGS_DEBUG "")
    set(LINKER_FLAGS_RELEASE "")

else() # we thing its gcc or clang

    set(C_FLAGS_COMM "-Wall")
    set(CXX_FLAGS_COMM "-Wall -std=c++14")

    set(C_FLAGS_DEBUG "-g")
    set(CXX_FLAGS_DEBUG "-g")

    set(C_FLAGS_RELEASE "-g -O2")
    set(CXX_FLAGS_RELEASE "-g -O2")

    set(LINKER_FLAGS_COMM "")

    set(LINKER_FLAGS_DEBUG "")
    set(LINKER_FLAGS_RELEASE "")
endif()


set(C_FLAGS_DEBUG "${C_FLAGS_DEBUG} ${C_FLAGS_COMM}")
set(CXX_FLAGS_DEBUG "${CXX_FLAGS_DEBUG} ${CXX_FLAGS_COMM}")

set(C_FLAGS_RELEASE "${C_FLAGS_RELEASE} ${CXX_FLAGS_COMM}")
set(CXX_FLAGS_RELEASE "${CXX_FLAGS_RELEASE} ${CXX_FLAGS_COMM}")

set(EXE_LINKER_FLAGS_DEBUG "${LINKER_FLAGS_DEBUG} ${LINKER_FLAGS_COMM}")
set(EXE_LINKER_FLAGS_RELEASE "${LINKER_FLAGS_RELEASE} ${LINKER_FLAGS_COMM}")

function(formal_and_set_flags flag_name)

    set(${flag_name} "${CMAKE_${flag_name}} ${${flag_name}}")
    string(STRIP ${${flag_name}} ${flag_name})

    if("${${flag_name}}" STREQUAL "")
        return()
    endif()

    string(REPLACE " " ";" ${flag_name} ${${flag_name}})
    list(SORT ${flag_name})
    list(REMOVE_DUPLICATES ${flag_name})
    string(REPLACE ";" " " ${flag_name} "${${flag_name}}")
    string(STRIP ${${flag_name}} ${flag_name})

    # set the CMAKE_xxx counterpart
    set(CMAKE_${flag_name} ${${flag_name}} PARENT_SCOPE)

endfunction()

formal_and_set_flags(C_FLAGS_DEBUG)
formal_and_set_flags(CXX_FLAGS_DEBUG)

formal_and_set_flags(C_FLAGS_RELEASE)
formal_and_set_flags(CXX_FLAGS_RELEASE)

formal_and_set_flags(EXE_LINKER_FLAGS_DEBUG)
formal_and_set_flags(EXE_LINKER_FLAGS_RELEASE)

