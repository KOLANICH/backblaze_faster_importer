#This is free and unencumbered software released into the public domain.
#Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means.
#In jurisdictions that recognize copyright laws, the author or authors of this software dedicate any and all copyright interest in the software to the public domain. We make this dedication for the benefit of the public at large and to the detriment of our heirs and successors. We intend this dedication to be an overt act of relinquishment in perpetuity of all present and future rights to this software under copyright law.
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#For more information, please refer to <https://unlicense.org/>

#set(SQLITE_CPP_REPO "https://github.com/SRombauts/${projectName}.git")
set(SQLITE_CPP_ZIP "https://github.com/KOLANICH/SQLiteCPP/archive/fixes_by_KOLANICH.zip")

function(downloadSQLiteCppIfNeeded projectName)
	set(VAR_NAME "${projectName}_download")
	FetchContent_Declare(
		"${VAR_NAME}"
		URL "${SQLITE_CPP_ZIP}" #git submodules update cannot be disabled for now, so using a zip instead
		#GIT_REPOSITORY "${SQLITE_CPP_REPO}"
		#GIT_SHALLOW TRUE
		#GIT_PROGRESS TRUE
		#SOURCE_DIR "${${projectName}_DIR}"
		TLS_VERIFY 1
	)
	FetchContent_MakeAvailable("${VAR_NAME}")
	FetchContent_GetProperties("${VAR_NAME}"
		SOURCE_DIR "${projectName}_DIR"
	)
	set("${projectName}_DIR" "${${projectName}_DIR}" PARENT_SCOPE)
	set("${projectName}_INCLUDE_DIR" "${${projectName}_DIR}/include" PARENT_SCOPE)
	set("${projectName}_SOURCE_DIR" "${${projectName}_DIR}/src" PARENT_SCOPE)
endfunction()