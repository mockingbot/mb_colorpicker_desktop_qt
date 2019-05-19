cmake_minimum_required(VERSION 3.4)

# helper function for show message
function(smart_cute_status_message txt)
    if(${SHOW_SMART_CUTE_LOG_MESAGE})
        message(STATUS ${txt})
    endif()
endfunction()

# make sure the executeable exist
find_program(QT_RCC_EXECUTABLE NAMES rcc)
if(QT_RCC_EXECUTABLE)
    message(STATUS "Use ${QT_RCC_EXECUTABLE}")
else()
    message(WARNING "Can Not Find Qt rcc")
endif()

find_program(QT_UIC_EXECUTABLE NAMES uic)
if(QT_UIC_EXECUTABLE)
    message(STATUS "Use ${QT_UIC_EXECUTABLE}")
else()
    message(WARNING "Can Not Find Qt uic")
endif()

find_program(QT_MOC_EXECUTABLE NAMES moc)
if(QT_MOC_EXECUTABLE)
    message(STATUS "Use ${QT_MOC_EXECUTABLE}")
else()
    message(FATAL_ERROR "Can Not Find Qt moc")
endif()

find_program(QT_QMAKE_EXECUTABLE NAMES qmake)
if(QT_QMAKE_EXECUTABLE)
    message(STATUS "Use ${QT_QMAKE_EXECUTABLE}")
else()
    message(FATAL_ERROR "Can Not Find Qt qmake")
endif()

# Get Qt version from moc
execute_process(COMMAND ${QT_MOC_EXECUTABLE} -v OUTPUT_VARIABLE QT_MOC_VERSION_LOG
                                                ERROR_VARIABLE QT_MOC_VERSION_LOG)

string(STRIP ${QT_MOC_VERSION_LOG} QT_MOC_VERSION_LOG)
string(REGEX MATCHALL "([A-Za-z\ \(]*)([0-9\.]+)([A-Za-z\ \)]*)"
                       MOC_VERSION_REGEX_RESULT ${QT_MOC_VERSION_LOG})
set(QT_MOC_VERSION_LIST_TXT ${CMAKE_MATCH_2})
string(REPLACE "." ";" QT_MOC_VERSION_LIST ${QT_MOC_VERSION_LIST_TXT})

list(GET QT_MOC_VERSION_LIST 0 QT_MAJOR_VERSION)
list(GET QT_MOC_VERSION_LIST 1 QT_MINOR_VERSION)
list(GET QT_MOC_VERSION_LIST 2 QT_PATCH_VERSION)

message(STATUS "Qt Version: ${QT_MAJOR_VERSION}.${QT_MINOR_VERSION}.${QT_PATCH_VERSION}")

macro(qmake_query target out)
    execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query ${target} OUTPUT_VARIABLE ${out}_log)
    string(STRIP "${${out}_log}" ${out})
endmacro()

# Locate info from qmake
qmake_query(QT_INSTALL_BINS qt_install_bins)
get_filename_component(qt_bin_path ${qt_install_bins} ABSOLUTE)
message(STATUS "Qt Bin Path: ${qt_bin_path}")

qmake_query(QT_INSTALL_HEADERS qt_install_headers)
get_filename_component(qt_include_path ${qt_install_headers} ABSOLUTE)
message(STATUS "Qt Include Path: ${qt_include_path}")

qmake_query(QT_INSTALL_LIBS qt_install_libs)
get_filename_component(qt_lib_path ${qt_install_libs} ABSOLUTE)
message(STATUS "Qt Library Path: ${qt_lib_path}")

qmake_query(QT_INSTALL_PLUGINS qt_install_plugins)
get_filename_component(qt_plugins_path ${qt_install_plugins} ABSOLUTE)
message(STATUS "Qt Plugins Path: ${qt_plugins_path}")


# Fetch qt depend library from prl file
function(find_qt_depend_library_from_prl_file who prl_file_path)
    if(NOT EXISTS ${prl_file_path})
        message(FATAL_ERROR "File ${prl_file_path} Not Exist")
    endif()

    # Figure out this qt library is build for static or not
    file(READ ${prl_file_path} ${who}_PRL_TXT)
    string(REPLACE "\n" ";" ${who}_PRL_TXT_LIST ${${who}_PRL_TXT})

    # Check if we get the right file contents
    foreach(head PRL_BUILD_DIR PRO_INPUT PRL_TARGET PRL_CONFIG PRL_VERSION PRL_LIBS)
        foreach(var ${${who}_PRL_TXT_LIST})
            if(${var} MATCHES "^QMAKE_${head} =")
                set(${who}_${head} ${var})
            endif()
        endforeach()
    endforeach()


    # Build QMAKE_PRL_CONFIG list
    if(NOT ${${who}_PRL_CONFIG} MATCHES "^QMAKE_PRL_CONFIG =")
        message(FATAL_ERROR "Failed To Match QMAKE_PRL_CONFIG in ${prl_file_path}")
    endif()

    string(REGEX REPLACE "^QMAKE_PRL_CONFIG = " "" ${who}_PRL_CONFIG_BODY ${${who}_PRL_CONFIG})
    string(STRIP ${${who}_PRL_CONFIG_BODY} ${who}_PRL_CONFIG_BODY)

    if("${${who}_PRL_CONFIG_BODY}" STREQUAL "")
        message(FATAL_ERROR "Empty QMAKE_PRL_CONFIG in ${a_prl_file}")
    endif()
    string(REPLACE " " ";" ${who}_PRL_CONFIG_LIST ${${who}_PRL_CONFIG_BODY})
    set(${who}_PRL_CONFIG_LIST ${${who}_PRL_CONFIG_LIST})
    list(SORT ${who}_PRL_CONFIG_LIST)
    list(REMOVE_DUPLICATES ${who}_PRL_CONFIG_LIST)

    # Check if this is static/dynamic build
    if("staticlib" IN_LIST ${who}_PRL_CONFIG_LIST AND "static" IN_LIST ${who}_PRL_CONFIG_LIST)
        smart_cute_status_message("Found ${who} Static Library Build")
        set(${who}_IS_STATIC_LIBRARY True PARENT_SCOPE)
    elseif("dll" IN_LIST ${who}_PRL_CONFIG_LIST AND "shared" IN_LIST ${who}_PRL_CONFIG_LIST)
        smart_cute_status_message("Found ${who} Dynamic Library Build")
        set(${who}_IS_DYNAMIC_LIBRARY TRUE PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Failed Find Right Library Type in ${prl_file_path}")
    endif()

    # Check if this is release or debug build
    if("release" IN_LIST ${who}_PRL_CONFIG_LIST)
        smart_cute_status_message("Found ${who} Release Build Version")
        set(${who}_IS_REALSE_BUILD TRUE PARENT_SCOPE)
    elseif("debug" IN_LIST ${who}_PRL_CONFIG_LIST)
        smart_cute_status_message("Found ${who} Debug Build Version")
        set(${who}_IS_DEBUG_BUILD TRUE PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Failed Find Right Build Type in ${prl_file_path}")
    endif()

    # Check if this is static_runtime build
    if("static_runtime" IN_LIST ${who}_PRL_CONFIG_LIST)
        smart_cute_status_message("Found ${who} Static Runtime Build")
        set(${who}_IS_STATIC_RUNTIME_LIBRARY TRUE PARENT_SCOPE)
    else()
        smart_cute_status_message("Not Found ${who} Static Runtime Build")
        set(${who}_IS_STATIC_RUNTIME_LIBRARY FALSE PARENT_SCOPE)
    endif()

    # loacte the library (**.lib) file with same name of this **.prl file
    get_filename_component(prl_file_dir ${prl_file_path} DIRECTORY)
    get_filename_component(self_lib_name ${prl_file_path} NAME_WE)
    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        find_library(${who}_dep_lib_self ${self_lib_name} PATHS ${prl_file_dir})
    else()
        if(${self_lib_name} MATCHES "^lib(.+)")
            set(self_lib_name ${CMAKE_MATCH_1})
            find_library(${who}_dep_lib_self ${self_lib_name} PATHS ${prl_file_dir})
        endif()
    endif()
    if(NOT ${who}_dep_lib_self)
        message(FATAL_ERROR "Library ${self_lib_name} Not Found for ${prl_file_path}")
    else()
        list(APPEND ${who}_DEP_LIBS ${${who}_dep_lib_self})
    endif()

    # shared build also need this value
    set(${who}_DEP_LIBS ${${who}_DEP_LIBS} PARENT_SCOPE)

    # Skip Empty QMAKE_PRL_LIBS
    if(NOT ${who}_PRL_LIBS)
        return()
    endif()

    if(NOT ${${who}_PRL_LIBS} MATCHES "^QMAKE_PRL_LIBS =")
        message(FATAL_ERROR "Failed To Match QMAKE_PRL_LIBS in ${prl_file_path}")
    endif()

    string(REGEX REPLACE "^QMAKE_PRL_LIBS = " "" ${who}_PRL_LIBS_BODY ${${who}_PRL_LIBS})
    string(STRIP ${${who}_PRL_LIBS_BODY} ${who}_PRL_LIBS_BODY)

    if("${${who}_PRL_LIBS_BODY}" STREQUAL "")
        message(STATUS "Skip Empty ${who}_PRL_LIBS")
        return()
    endif()

    string(REPLACE " " ";" ${who}_PRL_LIBS_LIST ${${who}_PRL_LIBS_BODY})

    # store the depend lib information
    set(${who}_DEP_LIBS_L_PATH) # library search path specified by -L command
    set(${who}_DEP_L_PREFIX_LIBS) # library prefix with -l command
    set(${who}_DEP_QT_INSTALL_LIBS_PREFIX_LIBS) # library prefix with [QT_INSTALL_LIBS]
    set(${who}_DEP_DIRECT_SPEC_LIBS) # library like "***.lib" user32.lib
    set(${who}_DEP_FRAMEWORKS) # library prefix with -framework macOS only??

    set(curret_list_idx 0)
    list(LENGTH ${who}_PRL_LIBS_LIST list_count)
    foreach(_ ${${who}_PRL_LIBS_LIST})
        if(NOT ${list_count} GREATER ${curret_list_idx})
            break()
        endif()

        list(GET ${who}_PRL_LIBS_LIST ${curret_list_idx} var)

        # For -LXXXX
        if(${var} MATCHES "^\\-L(.+)$")
            set(l_path ${CMAKE_MATCH_1})
            if(${l_path} MATCHES "^\\$\\$\\[QT_(.+)\\]$")
                # skip
                # message(">>>>>........${l_path}..............<<")
            else()
                if(NOT EXISTS ${l_path})
                    message(WARNING "None Exist -L Path ${l_path}")
                else()
                    list(APPEND ${who}_DEP_LIBS_L_PATH ${l_path})
                endif()
            endif()

            math(EXPR curret_list_idx ${curret_list_idx}+1)
            continue()
        endif()

       # For -luser32 , usually for common system library
        if(${var} MATCHES "^\\-l([-A-Za-z0-9_\\.]+)$")
            list(APPEND ${who}_DEP_L_PREFIX_LIBS ${CMAKE_MATCH_1})
            math(EXPR curret_list_idx ${curret_list_idx}+1)
            continue()
        endif()

        # for $$[QT_INSTALL_LIBS]\\(.+)
        if(${var} MATCHES "^\\$\\$\\[QT_INSTALL_LIBS\\](\\\\\\\\|/)(.+)$")
            list(APPEND ${who}_DEP_QT_INSTALL_LIBS_PREFIX_LIBS ${CMAKE_MATCH_2})
            math(EXPR curret_list_idx ${curret_list_idx}+1)
            continue()
        endif()

        if(${var} MATCHES "^\\$\\$\\[QT_HOST_LIBS\\](\\\\\\\\|/)(.+)$")
            message(WARNING "Found $$[QT_HOST_LIBS] in ${a_prl_file}")
            math(EXPR curret_list_idx ${curret_list_idx}+1)
            continue()
        endif()

        # For "C:\\Program Files (x86)\\Windows Kits\\10\\lib\\um\\x64/d2d1.lib"
        if(${var} MATCHES "^\"")
            set(tmp_var ${var})
            math(EXPR curret_list_idx ${curret_list_idx}+1)
            list(GET ${who}_PRL_LIBS_LIST ${curret_list_idx} var_next)
            set(tmp_var "${tmp_var} ${var_next}")

            while(NOT "${tmp_var}" MATCHES "\"$")
                math(EXPR curret_list_idx ${curret_list_idx}+1)
                list(GET ${who}_PRL_LIBS_LIST ${curret_list_idx} var_next)
                set(tmp_var "${tmp_var} ${var_next}")
            endwhile()

            list(APPEND ${who}_DEP_DIRECT_SPEC_LIBS ${tmp_var})
            math(EXPR curret_list_idx ${curret_list_idx}+1)
            continue()
        endif()

        # for (.+).lib (.+).a
        if(${var} MATCHES "^([-A-Za-z0-9_\\.]+)\\.(lib|a)$")
            list(APPEND ${who}_DEP_DIRECT_SPEC_LIBS ${CMAKE_MATCH_1})
            math(EXPR curret_list_idx ${curret_list_idx}+1)
            continue()
        endif()

        # for -framework UIKit
        if(${var} STREQUAL -framework)
            math(EXPR curret_list_idx ${curret_list_idx}+1)
            list(GET ${who}_PRL_LIBS_LIST ${curret_list_idx} var_next)
            list(APPEND ${who}_DEP_FRAMEWORKS ${var_next})
            math(EXPR curret_list_idx ${curret_list_idx}+1)
            continue()
        endif()

        message(FATAL_ERROR "Unkwown Library Type ${var} in ${prl_file_path}")

    endforeach()


    # loacte the library (**.lib) file with same name of this **.prl file
    get_filename_component(prl_file_dir ${prl_file_path} DIRECTORY)
    get_filename_component(self_lib_name ${prl_file_path} NAME_WE)
    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        find_library(${who}_dep_lib_self ${self_lib_name} PATHS ${prl_file_dir})
    else()
        if(${self_lib_name} MATCHES "^lib(.+)")
            set(self_lib_name ${CMAKE_MATCH_1})
            find_library(${who}_dep_lib_self ${self_lib_name} PATHS ${prl_file_dir})
        endif()
    endif()
    if(NOT ${who}_dep_lib_self)
        message(FATAL_ERROR "Library ${self_lib_name} Not Found for ${prl_file_path}")
    else()
        list(APPEND ${who}_DEP_LIBS ${${who}_dep_lib_self})
    endif()

    # sort and remove duplicates
    if(${who}_DEP_LIBS_L_PATH)
        list(SORT ${who}_DEP_LIBS_L_PATH)
        list(REMOVE_DUPLICATES ${who}_DEP_LIBS_L_PATH)
    endif()
    if(${who}_DEP_L_PREFIX_LIBS)
        list(SORT ${who}_DEP_L_PREFIX_LIBS)
        list(REMOVE_DUPLICATES ${who}_DEP_L_PREFIX_LIBS)
    endif()
    if(${who}_DEP_QT_INSTALL_LIBS_PREFIX_LIBS)
        list(SORT ${who}_DEP_QT_INSTALL_LIBS_PREFIX_LIBS)
        list(REMOVE_DUPLICATES ${who}_DEP_QT_INSTALL_LIBS_PREFIX_LIBS)
    endif()
    if(${who}_DEP_DIRECT_SPEC_LIBS)
        list(SORT ${who}_DEP_DIRECT_SPEC_LIBS)
        list(REMOVE_DUPLICATES ${who}_DEP_DIRECT_SPEC_LIBS)
    endif()
    if(${who}_DEP_FRAMEWORKS)
        list(SORT ${who}_DEP_FRAMEWORKS)
        list(REMOVE_DUPLICATES ${who}_DEP_FRAMEWORKS)
    endif()

    # start real file these libraries
    foreach(var ${${who}_DEP_L_PREFIX_LIBS})
        find_library(${who}_dep_lib_${var} ${var})
        if(NOT ${who}_dep_lib_${var})
            message(FATAL_ERROR "-l Prefix Library ${var} Not Found for ${prl_file_path}")
        else()
            list(APPEND ${who}_DEP_LIBS ${${who}_dep_lib_${var}})
        endif()
    endforeach(var)

    foreach(var ${${who}_DEP_QT_INSTALL_LIBS_PREFIX_LIBS})
        find_library(${who}_dep_lib_${var} ${var} PATHS ${qt_lib_path} NO_DEFAULT_PATH)
        if(NOT ${who}_dep_lib_${var})
            message(FATAL_ERROR "$[QT_INSTALL_LIBS] Prefix Library ${var} Not Found for ${prl_file_path}")
        else()
            list(APPEND ${who}_DEP_LIBS ${${who}_dep_lib_${var}})
        endif()
    endforeach(var)

    foreach(var ${${who}_DEP_DIRECT_SPEC_LIBS})
        if(var MATCHES "^\"(.+)\"$")
            string(REGEX REPLACE "^\"" "" var ${var})
            string(REGEX REPLACE "\"$" "" var ${var})
            file(TO_CMAKE_PATH ${var} var)
            if(EXISTS ${var})
                get_filename_component(lib_dir ${var} DIRECTORY)
                get_filename_component(lib_name ${var} NAME_WE)
                find_library(${who}_dep_lib_${lib_name} ${lib_name} PATHS ${lib_dir} NO_DEFAULT_PATH)
                if(NOT ${who}_dep_lib_${lib_name})
                    message(FATAL_ERROR "Direct Specified Library ${var} Not Found for ${prl_file_path}")
                else()
                    list(APPEND ${who}_DEP_LIBS ${${who}_dep_lib_${lib_name}})
                endif()
            else()
                message(FATAL_ERROR "Direct Specified Library ${var} Not Exist for ${prl_file_path}")
             endif()
        else()
            find_library(${who}_dep_lib_${var} ${var})
            if(NOT ${who}_dep_lib_${var})
                message(FATAL_ERROR "Direct Specified Library ${var} Not Found for ${prl_file_path}")
            else()
                list(APPEND ${who}_DEP_LIBS ${${who}_dep_lib_${var}})
            endif()
        endif()
    endforeach(var)

    foreach(var ${${who}_DEP_FRAMEWORKS})
        find_library(qt_dep_lib_${var} ${var})
        if(NOT qt_dep_lib_${var})
            message(FATAL_ERROR "Library 4 ${var} Not Found for ${prl_file_path}")
        else()
            list(APPEND ${who}_DEP_LIBS ${qt_dep_lib_${var}})
        endif()
    endforeach(var)


    if(${who}_DEP_LIBS)
        list(SORT ${who}_DEP_LIBS)
        list(REMOVE_DUPLICATES ${who}_DEP_LIBS)
    endif()
    foreach(item ${${who}_DEP_LIBS})
        smart_cute_status_message("${item}")
    endforeach()

    set(${who}_DEP_LIBS ${${who}_DEP_LIBS} PARENT_SCOPE)

endfunction()

set(SHOW_SMART_CUTE_LOG_MESAGE true)
set(SHOW_SMART_CUTE_LOG_MESAGE false)


# handle prl files in lib dir
file(GLOB qt_prl_files RELATIVE ${qt_lib_path} ${qt_lib_path}/*.prl)
foreach(a_prl_file ${qt_prl_files})
    if(NOT ${a_prl_file} MATCHES "^(lib)?Qt5([A-Za-z]+).prl")
        # skip
    else()
        set(name ${CMAKE_MATCH_2})
        smart_cute_status_message("----${qt_lib_path}/${a_prl_file}")
        find_qt_depend_library_from_prl_file(${name} ${qt_lib_path}/${a_prl_file})
    endif()
endforeach()

# handle prl files in plugins dir
file(GLOB_RECURSE qt_plugins_path_list RELATIVE ${qt_plugins_path} ${qt_plugins_path}/*.prl)
foreach(a_prl_file ${qt_plugins_path_list})
    if(NOT ${a_prl_file} MATCHES "^([A-Za-z]+)/([A-Za-z]+).prl")
        # skip
    else()
        set(name ${CMAKE_MATCH_1}/${CMAKE_MATCH_2})
        smart_cute_status_message("-----${qt_lib_path}/${a_prl_file}")
        find_qt_depend_library_from_prl_file(${name} ${qt_plugins_path}/${a_prl_file})
    endif()
endforeach()


# function to generate qt moc file
function(qt_moc_command cxx_head_file_path store_name)
    if(IS_ABSOLUTE ${cxx_head_file_path})
        file(TO_CMAKE_PATH ${cxx_head_file_path} input_file_path)
    else()
        file(TO_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${cxx_head_file_path} input_file_path)
    endif()

    if(NOT EXISTS ${input_file_path})
        message(FATAL_ERROR "Can not find file ${input_file_path}")
    endif()

    # set output_dir
    set(output_dir ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/moc.dir)

    # create generated store file path
    file(RELATIVE_PATH relative_file_path ${CMAKE_CURRENT_SOURCE_DIR} ${input_file_path})

    # generated file full path
    get_filename_component(tmp_abs_path ${output_dir}/${relative_file_path} ABSOLUTE)

    get_filename_component(generated_file_dirctory ${tmp_abs_path} DIRECTORY)
    get_filename_component(generated_file_name ${tmp_abs_path} NAME_WE)

    set(generated_file_full_path "${generated_file_dirctory}/${generated_file_name}.cxx")

    # create the cmake needed depend name
    string(REPLACE "." "_" generated_depend_name ${generated_file_full_path})
    string(REPLACE "/" "_" generated_depend_name ${generated_depend_name})
    string(REPLACE ":" "_" generated_depend_name ${generated_depend_name})

    execute_process(
        COMMAND         ${QT_MOC_EXECUTABLE} ${input_file_path}
        OUTPUT_VARIABLE output_msg
        ERROR_VARIABLE error_msg
    )

    if(error_msg)
        unset(${store_name} PARENT_SCOPE)
        return()
    endif()

    add_custom_command(
        OUTPUT    ${generated_file_full_path}
        COMMAND   ${QT_MOC_EXECUTABLE}
        ARGS      ${input_file_path} -o ${generated_file_full_path}
        DEPENDS   ${input_file_path}
        VERBATIM
    )

    if(NOT TARGET ${generated_depend_name})
        add_custom_target(${generated_depend_name}
            DEPENDS ${generated_file_full_path}
        )
    endif()

    # return generated file
    set(${store_name} ${generated_file_full_path} PARENT_SCOPE)
endfunction(qt_moc_command)


# function for handle qt resource file
function(qt_rcc_command resource_file_path store_name)

    # formal file path
    file(TO_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${resource_file_path} input_file_path)

    # check file exist
    if(NOT EXISTS ${input_file_path})
        message(FATAL_ERROR "Can not find file ${input_file_path}")
    endif()

    # set output_dir
    set(output_dir ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/rcc.dir)

    # create generated store file path
    file(RELATIVE_PATH relative_file_path ${CMAKE_CURRENT_SOURCE_DIR} ${input_file_path})

    # generated file full path
    get_filename_component(tmp_abs_path ${output_dir}/${relative_file_path} ABSOLUTE)

    get_filename_component(generated_file_dirctory ${tmp_abs_path} DIRECTORY)
    get_filename_component(generated_file_name ${tmp_abs_path} NAME_WE)

    set(generated_file_full_path "${generated_file_dirctory}/${generated_file_name}.cxx")

    # create the cmake needed depend name
    string(REPLACE "." "_" generated_depend_name ${generated_file_full_path})
    string(REPLACE "/" "_" generated_depend_name ${generated_depend_name})
    string(REPLACE ":" "_" generated_depend_name ${generated_depend_name})

    file(MD5 ${input_file_path} file_hash)

    execute_process(
        COMMAND         ${QT_RCC_EXECUTABLE} ${input_file_path} --list
        OUTPUT_VARIABLE depends_files_list
        ERROR_VARIABLE error_msg
    )

    if("${depends_files_list}" STREQUAL "")
        # sometime we get empty resource file
        set(${store_name} "" PARENT_SCOPE)
        set(resource_file_hash_list "" PARENT_SCOPE)
        return()
    endif()

    string(STRIP depends_files_list ${depends_files_list})
    string(REPLACE "\n" ";" depends_files_list ${depends_files_list})

    set(resource_included_file_list)

    foreach(one ${depends_files_list})
        string(STRIP one ${one})
        list(APPEND resource_included_file_list ${one})
    endforeach()

    add_custom_command(
        OUTPUT    ${generated_file_full_path}
        COMMAND   ${QT_RCC_EXECUTABLE}
        ARGS      ${input_file_path} -o ${generated_file_full_path} --name ${file_hash}
        DEPENDS   ${input_file_path} ${resource_included_file_list}
        VERBATIM
    )

    if(NOT TARGET ${generated_depend_name})
        add_custom_target(${generated_depend_name}
            DEPENDS ${generated_file_full_path}
        )
    endif()

    # return generated file
    set(${store_name} ${generated_file_full_path} PARENT_SCOPE)
    # return resource file hash
    set(resource_file_hash_list ${file_hash} PARENT_SCOPE)
endfunction(qt_rcc_command)


function(smart_cute_add_resource target_name resource_file_path)
    message(STATUS "SmartCute Compile Resource ${resource_file_path}")

    qt_rcc_command(${resource_file_path} generated_file_full_path)

    if("${generated_file_full_path}" STREQUAL "")
        return()
    endif()

    target_sources(${target_name} PRIVATE ${generated_file_full_path})

    list(APPEND all_resource_file_hash_list ${resource_file_hash_list})
    set(all_resource_file_hash_list ${all_resource_file_hash_list} PARENT_SCOPE)

endfunction()


function(smart_cute_link target_name build_type)
    message(STATUS "SmartCute Automatic Handle Qt For `${target_name}`")

    set(cxx_head_file_ext)
    list(APPEND cxx_head_file_ext .h)
    list(APPEND cxx_head_file_ext .hh)
    list(APPEND cxx_head_file_ext .hxx)
    list(APPEND cxx_head_file_ext .hpp)

    get_target_property(target_source_files ${target_name} SOURCES)

    foreach(one_file ${target_source_files})
        string(FIND ${one_file} "." _index REVERSE)
        string(SUBSTRING ${one_file} ${_index} -1 file_ext)
        if(NOT ${file_ext} IN_LIST cxx_head_file_ext)
            continue()
        endif()

        message(STATUS "SmartCute Compile Meta Object ${one_file}")
        qt_moc_command(${one_file} generated_file_full_path)
        if(generated_file_full_path)
            target_sources(${target_name} PRIVATE ${generated_file_full_path})
        endif()
    endforeach()

    set(qt_framework_modules)
    list(APPEND qt_framework_modules AccessibilitySupport)
    list(APPEND qt_framework_modules Concurrent)
    list(APPEND qt_framework_modules Core)
    list(APPEND qt_framework_modules DBus)
    list(APPEND qt_framework_modules DeviceDiscoverySupport)
    list(APPEND qt_framework_modules EdidSupport)
    list(APPEND qt_framework_modules EventDispatcherSupport)
    list(APPEND qt_framework_modules FbSupport)
    list(APPEND qt_framework_modules FontDatabaseSupport)
    list(APPEND qt_framework_modules Gui)
    list(APPEND qt_framework_modules Network)
    list(APPEND qt_framework_modules OpenGL)
    list(APPEND qt_framework_modules PrintSupport)
    list(APPEND qt_framework_modules Sql)
    list(APPEND qt_framework_modules Test)
    list(APPEND qt_framework_modules ThemeSupport)
    list(APPEND qt_framework_modules Widgets)
    list(APPEND qt_framework_modules WindowsUIAutomationSupport)
    list(APPEND qt_framework_modules Xml)

    if(NOT ((${build_type} STREQUAL "RELEASE") OR (${build_type} STREQUAL "DEBUG")))
        message(FATAL_ERROR smart_cute_link require specified `RELEASE` or `DEBUG`)
    endif()

    foreach(one_module ${ARGN})
        if(NOT ${one_module} IN_LIST qt_framework_modules)
            # message(FATAL_ERROR smart_cute_link link Unkwown module `${one_module}`)
        endif()
    endforeach()

    list(APPEND qt_framework_dep_libraries)
    foreach(var ${ARGN})
        if(${build_type} STREQUAL "RELEASE")
            list(APPEND qt_framework_dep_libraries ${${var}_DEP_LIBS})
        endif()
        if(${build_type} STREQUAL "DEBUG")
            list(APPEND qt_framework_dep_libraries ${${var}d_DEP_LIBS})
        endif()
    endforeach()

    if(CMAKE_SYSTEM_NAME STREQUAL Windows)
        # need include this for OpenGL on Windows
        if(opengles2 IN_LIST Gui_PRL_CONFIG_LIST)
            if(EXISTS ${qt_include_path}/QtANGLE/GLES2)
                target_include_directories(${target_name} PRIVATE ${qt_include_path}/QtANGLE)
            else()
                message(FATAL_ERROR "Failed Set OpenGLES2 Include Path")
            endif()
        endif()
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL Linux)
        # target_link_libraries(${target_name} -Xlinker --start-group)
    endif()

    if(qt_framework_dep_libraries)
        list(SORT qt_framework_dep_libraries)
        list(REMOVE_DUPLICATES qt_framework_dep_libraries)
    endif()

    foreach(item ${qt_framework_dep_libraries})
        message(STATUS "SmartCute Add Library ${item} for ${target_name}")
    endforeach(item)

    target_link_libraries(${target_name} ${qt_framework_dep_libraries})

    if(${build_type} STREQUAL "RELEASE")
        set(check_lib Core)
    endif()
    if(${build_type} STREQUAL "DEBUG")
        set(check_lib Cored)
    endif()

    # export this value so that the target can used for qt_plugins link
    if(${${check_lib}_IS_STATIC_LIBRARY})
        target_compile_definitions(${target_name} PRIVATE QT_FRAMEWORK_IS_STATIC_LIB)
    endif()

    # export this value so that the target can used for qt_plugins link
    if(${${check_lib}_IS_DYNAMIC_LIBRARY})
        target_compile_definitions(${target_name} PRIVATE QT_FRAMEWORK_IS_DYNAMIC_LIB)
    endif()

    # export this value so that the target can used for cmake
    if(${${check_lib}_IS_STATIC_LIBRARY})
        set(QT_FRAMEWORK_IS_STATIC_LIB TRUE PARENT_SCOPE)
    endif()

    # export this value so that the target can used for cmake
    if(${${check_lib}_IS_DYNAMIC_LIBRARY})
        set(QT_FRAMEWORK_IS_DYNAMIC_LIB TRUE PARENT_SCOPE)
    endif()

    # export this value so that the target can used for cmake
    if(${${check_lib}_IS_STATIC_RUNTIME_LIBRARY})
        set(QT_FRAMEWORK_IS_STATIC_RUNTIME_LIB TRUE PARENT_SCOPE)
    else()
        set(QT_FRAMEWORK_IS_STATIC_RUNTIME_LIB FALSE PARENT_SCOPE)
    endif()

    target_include_directories(${target_name} PRIVATE ${qt_include_path})

endfunction()

