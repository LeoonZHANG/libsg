cmake_minimum_required(VERSION 3.3)


# project
#########
project(libxml)

set(lib_name xml)
set(lib_name_static ${lib_name})
## Take different names for static lib and dynamic lib on Windows, or they're .lib file will conflicted.
if(WIN32)
    set(lib_name_static lib${lib_name_static})
endif()
set(target_static lib${lib_name}-static)


# files
#######
set(src_lib
    buf.c
	c14n.c
	catalog.c
	chvalid.c
	debugXML.c
	dict.c
	DOCBparser.c
	encoding.c
	entities.c
	error.c
	globals.c
	hash.c
	HTMLparser.c
	HTMLtree.c
	legacy.c
	list.c
	nanoftp.c
	nanohttp.c
	parser.c
	parserInternals.c
	pattern.c
	relaxng.c
	SAX2.c
	SAX.c
	schematron.c
	threads.c
	tree.c
	uri.c
	valid.c
	xinclude.c
	xlink.c
	xmlIO.c
	xmlmemory.c
	xmlreader.c
	xmlregexp.c
	xmlmodule.c
	xmlsave.c
	xmlschemas.c
	xmlschemastypes.c
	xmlunicode.c
	xmlwriter.c
	xpath.c
	xpointer.c
	xmlstring.c
    )


# build
#######
add_definitions(-DWIN32 -D_WINDOWS -D_MBCS -DNOLIBTOOL)
include_directories(include)
if(WIN32)
    include_directories(../libiconv/include)
endif()


# targets
#########
add_library(${target_static} STATIC ${src_lib})

set_target_properties(${target_static} PROPERTIES
    OUTPUT_NAME ${lib_name_static})
