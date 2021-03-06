#This is free and unencumbered software released into the public domain.
#Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means.
#In jurisdictions that recognize copyright laws, the author or authors of this software dedicate any and all copyright interest in the software to the public domain. We make this dedication for the benefit of the public at large and to the detriment of our heirs and successors. We intend this dedication to be an overt act of relinquishment in perpetuity of all present and future rights to this software under copyright law.
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#For more information, please refer to <https://unlicense.org/>

set(SQLITE_BASE_URI "https://sqlite.org")
set(SQLITE_DOWNLOAD_PAGE_URI "${SQLITE_BASE_URI}/download.html")


function(downloadSQLiteIfNeeded projectName)
	set("DOWNLOADED_DOWNLOAD_HTML_FILE_NAME" "${CMAKE_BINARY_DIR}/sqlite_download.html")
	if(EXISTS "${${projectName}_DOWNLOADED_DOWNLOAD_HTML_FILE_NAME}")
	else()
		file(DOWNLOAD "${SQLITE_DOWNLOAD_PAGE_URI}" "${DOWNLOADED_DOWNLOAD_HTML_FILE_NAME}" TLS_VERIFY SHOW_PROGRESS)
	endif()
	
	file(READ "${DOWNLOADED_DOWNLOAD_HTML_FILE_NAME}" DOWNLOADED_DOWNLOAD_HTML)
	string(REGEX MATCH ",'(20[0-9][0-9]/sqlite-amalgamation-[0-9]+.zip)'\\)" "" "${DOWNLOADED_DOWNLOAD_HTML}")

	set(SQLITE_URI_PART "${CMAKE_MATCH_1}")
	set(SQLITE_AMALGAMATION_URI "${SQLITE_BASE_URI}/${SQLITE_URI_PART}")

	message(STATUS "${projectName} amalgamation file URI: ${SQLITE_BASE_URI}/${SQLITE_URI_PART}")

	set(VAR_NAME "${projectName}_download")
	FetchContent_Declare(
		"${VAR_NAME}"
		URL "${SQLITE_AMALGAMATION_URI}"
		#SOURCE_DIR "${SQLITE_AMALGAMATION_SOURCE_DIR}"
		TLS_VERIFY 1
		
		PATCH_COMMAND ""
		UPDATE_COMMAND ""
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
		CMAKE_COMMAND ""
	)
	
	FetchContent_MakeAvailable("${VAR_NAME}")
	FetchContent_GetProperties("${VAR_NAME}"
		SOURCE_DIR "${projectName}_AMALGAMATION_SOURCE_DIR"
	)
	set("${projectName}_AMALGAMATION_SOURCE_DIR" "${${projectName}_AMALGAMATION_SOURCE_DIR}" PARENT_SCOPE)
	message(STATUS "${projectName} amalgamation dir: ${${projectName}_AMALGAMATION_SOURCE_DIR}")
endfunction()