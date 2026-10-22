set(prefix "${ROOT_BINARY}/consumer-prefix")
execute_process(COMMAND "${CMAKE_COMMAND}" --install "${ROOT_BINARY}" --prefix "${prefix}" --config "${CONFIG}"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}" -S "${CONSUMER_SOURCE}" -B "${ROOT_BINARY}/consumer-build"
    "-DCMAKE_PREFIX_PATH=${prefix};${DEPENDENCY_PREFIX}" "-DTEST_VISUALIZATION=${VISUALIZATION}"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_COMMAND}" --build "${ROOT_BINARY}/consumer-build" --config "${CONFIG}"
    COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${ROOT_BINARY}/consumer-build" -C "${CONFIG}" --output-on-failure
    COMMAND_ERROR_IS_FATAL ANY)
