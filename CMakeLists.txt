cmake_minimum_required(VERSION 3.7)
project(dos-app)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "-O3 -ffast-math -g")
add_compile_options(-fpermissive)
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "src/*.cpp")
include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(main ${sources})
target_link_libraries(main ${CMAKE_SOURCE_DIR}/allegro/lib/djgpp/liballeg.a)
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/allegro/include)
find_package(Python3 REQUIRED COMPONENTS Interpreter)

file(GLOB tmx_files CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/RawAssetData/*.tmx")
set(bin_files "")

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/assets/STRINGS.json ${CMAKE_CURRENT_SOURCE_DIR}/assets/STRINGS.BIN
	COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/BuildScripts/CompileStrings.py
	DEPENDS ${tmx_files} ${CMAKE_CURRENT_SOURCE_DIR}/BuildScripts/CompileStrings.py
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

foreach(tmx_file ${tmx_files})
	get_filename_component(tmx_name ${tmx_file} NAME_WE)
	set(tsx_file "${CMAKE_CURRENT_SOURCE_DIR}/RawAssetData/${tmx_name}.tsx")
	set(bin_file "${CMAKE_CURRENT_SOURCE_DIR}/assets/MAPS/${tmx_name}.bin")
	add_custom_command(
		OUTPUT ${bin_file}
		COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/BuildScripts/TMXConv.py ${tmx_file} ${bin_file}
		DEPENDS ${tmx_file} ${tsx_file} ${CMAKE_CURRENT_SOURCE_DIR}/assets/STRINGS.json ${CMAKE_CURRENT_SOURCE_DIR}/BuildScripts/TMXConv.py
	)
	list(APPEND bin_files ${bin_file})
endforeach()

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/assets/GAMEDB.BIN
	COMMAND Python3::Interpreter ${CMAKE_CURRENT_SOURCE_DIR}/BuildScripts/CompileDB.py
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/db.json ${CMAKE_CURRENT_SOURCE_DIR}/BuildScripts/CompileDB.py
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}	
)

add_custom_target(assets
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/assets
	${CMAKE_BINARY_DIR}
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/GAMEDB.BIN ${CMAKE_CURRENT_SOURCE_DIR}/assets/STRINGS.BIN ${bin_files}
)
add_dependencies(main assets)