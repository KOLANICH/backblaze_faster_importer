function(findBackBlazeAnalyticsDir)
	get_filename_component(BACKBLAZE_ANALYTICS_PARENT_DIR ${CMAKE_SOURCE_DIR} PATH)
	if(IS_DIRECTORY "${BACKBLAZE_ANALYTICS_PARENT_DIR}/backblaze_analytics")
		set(BACKBLAZE_ANALYTICS_PARENT_DIR "${BACKBLAZE_ANALYTICS_PARENT_DIR}" CACHE PATH "Path to parent dir of backblaze_analytics")
	else()
		set(BACKBLAZE_ANALYTICS_PARENT_DIR "BACKBLAZE_ANALYTICS_PARENT_DIR-NOTFOUND" CACHE PATH "Path to parent dir of backblaze_analytics")
	endif()
endfunction()

function(generateSchemaHeaders targetName)
	findBackBlazeAnalyticsDir()
	set(GENERATED_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/generated")
	add_custom_command(
		OUTPUT "${GENERATED_HEADERS_DIR}"
		PRE_BUILD
		COMMAND "${Python_EXECUTABLE}" ARGS "-m" "backblaze_analytics" "import" "generateC++Schema" "${GENERATED_HEADERS_DIR}"
		WORKING_DIRECTORY "${BACKBLAZE_ANALYTICS_PARENT_DIR}"
		DEPENDS "${BACKBLAZE_ANALYTICS_PARENT_DIR}/backblaze_analytics"
		COMMENT "Regenerating headers"
	)
	add_custom_target(
		regenerate_schema
		DEPENDS "${GENERATED_HEADERS_DIR}"
	)
	add_dependencies("${targetName}" regenerate_schema)
endfunction()