# Execute Git commands to retrieve information from the version control.
execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${ENGINE_PROJECT_DIR}" rev-list --all --count
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE GIT_ENGINE_CHANGE_NUMBER)

execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${ENGINE_PROJECT_DIR}" log -1 --format=%h
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE GIT_ENGINE_COMMIT_HASH)

execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${ENGINE_PROJECT_DIR}" log -1 --format=%ci
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE GIT_ENGINE_COMMIT_DATE)

execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${ENGINE_PROJECT_DIR}" rev-parse --abbrev-ref HEAD
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE GIT_ENGINE_BRANCH_NAME)

execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${GAME_PROJECT_DIR}" rev-list --all --count
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE GIT_GAME_CHANGE_NUMBER)

execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${GAME_PROJECT_DIR}" log -1 --format=%h
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE GIT_GAME_COMMIT_HASH)

execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${GAME_PROJECT_DIR}" log -1 --format=%ci
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE GIT_GAME_COMMIT_DATE)

execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${GAME_PROJECT_DIR}" rev-parse --abbrev-ref HEAD
    OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE GIT_GAME_BRANCH_NAME)

# Save retrieved information as a config file.
configure_file(${INPUT_FILE} ${OUTPUT_FILE})
