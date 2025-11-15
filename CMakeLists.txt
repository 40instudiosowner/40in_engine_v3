cmake_minimum_required(VERSION 3.25)
project(40in_engine_v3 LANGUAGES CXX)

# Требуем C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# -----------------------------------------------------------------------------
# Найдём Qt и Vulkan
# -----------------------------------------------------------------------------
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
find_package(Vulkan REQUIRED)

# -----------------------------------------------------------------------------
# Shader compiler (glslc)
# -----------------------------------------------------------------------------
find_program(GLSLANG_VALIDATOR_EXECUTABLE glslangValidator
    HINTS
        ${VULKAN_SDK}
        ${VULKAN_SDK}/bin
)

if (NOT GLSLANG_VALIDATOR_EXECUTABLE)
    message(FATAL_ERROR "glslc not found! Make sure shaderc (with tools) is installed via Conan.")
else()
    message(STATUS "Found glslc: ${GLSLANG_VALIDATOR_EXECUTABLE}")
endif()

# -----------------------------------------------------------------------------
# Автокомпиляция шейдеров (GLSL → SPIR-V)
# -----------------------------------------------------------------------------
file(GLOB SHADER_SOURCES shaders/*.vert shaders/*.frag)
set(COMPILED_SHADERS "")

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPIRV ${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILE_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders
        COMMAND ${GLSLANG_VALIDATOR_EXECUTABLE} --target-env vulkan1.0 ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILE_NAME}"
    )
    list(APPEND COMPILED_SHADERS ${SPIRV})
endforeach()

# -----------------------------------------------------------------------------
# UI и исходники
# -----------------------------------------------------------------------------
# Путь к ui файлам
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

file(GLOB UI_FILES  ui/*.ui)

file(GLOB SRC_FILES src/*.cpp)

file(GLOB INC_FILES src/*.h)

add_executable(${PROJECT_NAME}
	${SRC_FILES}

    ${UI_FILES}
    ${COMPILED_SHADERS}
)

# -----------------------------------------------------------------------------
# Qt автоматические инструменты
# -----------------------------------------------------------------------------
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# -----------------------------------------------------------------------------
# Линковка библиотек
# -----------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Gui
    Qt6::Widgets
    Vulkan::Vulkan
)

# -----------------------------------------------------------------------------
# Настройка окружения (опционально)
# -----------------------------------------------------------------------------
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DEBUGGER_ENVIRONMENT
        "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>;%PATH%"
    )
endif()

# -----------------------------------------------------------------------------
# Копирование плагинов Qt (Windows)
# -----------------------------------------------------------------------------
if(WIN32)
    get_target_property(_qt_bin_dir Qt6::Widgets IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${_qt_bin_dir}" DIRECTORY)
    set(QT_PLUGINS_DIR "${QT_BIN_DIR}/../plugins")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${QT_PLUGINS_DIR}/platforms"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms"
    )
endif()
