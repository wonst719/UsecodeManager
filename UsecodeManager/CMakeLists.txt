cmake_minimum_required(VERSION 3.8)

set(USECODE_MANAGER_SOURCES
	"common.h"
	"compile.cpp"
	"decompile.cpp"
	"file.cpp"
	"file.h"
	"main.cpp"
	"usecode.cpp"
	"usecode.h"
)

add_executable(UsecodeManager ${USECODE_MANAGER_SOURCES})
