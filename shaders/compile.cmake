get_filename_component(SHADER_NAME "${SHADER_FILE}" NAME)
get_filename_component(SHADER_DIR "${SHADER_FILE}" DIRECTORY)

if(WIN32)
    execute_process(
        COMMAND "${SHADER_DIR}/compile.bat" "${SHADER_FILE}" "${TARGET_DIR}/${SHADER_NAME}.spv"
        RESULT_VARIABLE RES
    )
elseif(LINUX)
    execute_process(
        COMMAND "${SHADER_DIR}/compile.sh" "${SHADER_FILE}" "${TARGET_DIR}/${SHADER_NAME}.spv"
        RESULT_VARIABLE RES
    )
else()
    set(RES 1)
endif()

if(NOT RES)
    message(" -> ${SHADER_NAME}")
else()
    message(" x> ${SHADER_NAME}")
endif()
