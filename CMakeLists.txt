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

target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/DaemonModeHost.hxx)
target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/DaemonModeHost.cxx)
target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/GlobalEventHook.hxx)
target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/GlobalEventHook.cxx)
target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/ColorPickerHost.hxx)
target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/ColorPickerHost.cxx)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/DaemonModeHost.win.cxx)
    target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/GlobalEventHook.win.cxx)
    target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/ColorPickerHost.win.cxx)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/DaemonModeHost.mac.mm)
    target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/GlobalEventHook.mac.mm)
    target_sources(ColorPicker4MoDao PRIVATE src/ColorPicker/ColorPickerHost.mac.mm)
    set_target_properties(ColorPicker4MoDao PROPERTIES MACOSX_BUNDLE_INFO_PLIST
                          ${CMAKE_CURRENT_SOURCE_DIR}/res/Info.plist.in)
endif()


if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set_target_properties(ColorPicker4MoDao PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dst/Windows/x64)
    else()
        set_target_properties(ColorPicker4MoDao PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dst/Windows/x32)
    endif()
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set_target_properties(ColorPicker4MoDao PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dst/macOS)
endif()

################################################################################
# using this handle Qt5 static link
include(SmartCute.cmake)
# I don't like qt keywords
add_definitions(-DQT_NO_KEYWORDS)

smart_cute_link(ColorPicker4MoDao RELEASE Core Gui Widgets)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    smart_cute_link(ColorPicker4MoDao RELEASE platforms/qwindows)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    smart_cute_link(ColorPicker4MoDao RELEASE platforms/qcocoa)
endif()

set_target_properties(ColorPicker4MoDao PROPERTIES LINK_FLAGS "/SUBSYSTEM:Console")

# auto generate resource files for the target
smart_cute_add_resource(ColorPicker4MoDao res/resource.xml)

################################################################################
# set our c++ compiler

if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    add_definitions(-D__CURRENT_FUNCTION_NAME__=__FUNCTION__)
else() # we thing its gcc or clang
    add_definitions(-D__CURRENT_FUNCTION_NAME__=__PRETTY_FUNCTION__)
endif()


if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)

    add_definitions(-DUNICODE)
    add_definitions(-D_UNICODE)

    set(C_FLAGS_COMM "/nologo /Zc:wchar_t- /Zc:strictStrings /MP /W3")
    set(CXX_FLAGS_COMM "/nologo /Zc:wchar_t- /Zc:strictStrings /MP /W3 /GR /EHsc")

    if(QT_FRAMEWORK_IS_STATIC_RUNTIME_LIB)
        set(C_FLAGS_DEBUG "/DDEBUG /MTd")
        set(CXX_FLAGS_DEBUG "/DDEBUG /MTd")

        set(C_FLAGS_RELEASE "/DNDEBUG /MT")
        set(CXX_FLAGS_RELEASE "/DNDEBUG /MT")
    else()
        set(C_FLAGS_DEBUG "/DDEBUG /MDd")
        set(CXX_FLAGS_DEBUG "/DDEBUG /MDd")

        set(C_FLAGS_RELEASE "/DNDEBUG /MD")
        set(CXX_FLAGS_RELEASE "/DNDEBUG /MD")
    endif()

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

# clean predefined
set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_RELEASE "")

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

