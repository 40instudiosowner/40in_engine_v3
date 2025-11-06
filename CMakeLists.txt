
cmake_minimum_required (VERSION 3.8)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("40in_engine_v3")


# Устанавливаем стандарт
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Обязательно указываем, что Qt6 ищем через CMake
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)
find_package(Vulkan REQUIRED)

find_program(GLSLC_EXECUTABLE glslc
    HINTS ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/shaderc
)
if (NOT GLSLC_EXECUTABLE)
  message(FATAL_ERROR "glslc not found! Install shaderc via vcpkg with tools binary")
endif()

# Автокомпиляция шейдеров (GLSL  SPIR-V)
file(GLOB SHADER_SOURCES shaders/*.vert shaders/*.frag)
set(COMPILED_SHADERS "")

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPIRV ${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILE_NAME}.spv)
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders
        COMMAND ${GLSLC_EXECUTABLE} ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILE_NAME}"
    )
    list(APPEND COMPILED_SHADERS ${SPIRV})
endforeach()

# Собираем все UI файлы из папки ui
file(GLOB UI_FILES ui/*.ui)

# Добавляем источник в исполняемый файл этого проекта.
add_executable (40in_engine_v3 
"40in_engine_v3.cpp" 

"main_window.h" 
"main_window.cpp"
"vulkan_window.h"
"vulkan_window.cpp"
"vulkan_renderer.h"
"vulkan_renderer.cpp"

${UI_FILES}
${COMPILED_SHADERS})

# Подключаем Qt-шные заголовки (модули)
target_link_libraries(40in_engine_v3 PRIVATE
    Qt6::Gui
    Qt6::Widgets
    Vulkan::Vulkan
)

if(NOT DEFINED VCPKG_ROOT)
    set(VCPKG_ROOT "C:/vcpkg") # или твой путь
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    #set(QT_BIN_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/bin")
    #set(QT_PLUGINS_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/Qt6/plugins")
    set(QT_BIN_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/debug/bin")
    set(QT_PLUGINS_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/debug/Qt6/plugins")
    # Копируем необходимые Qt DLL в выходную директорию после сборки
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
           
            "${QT_BIN_DIR}/Qt6Guid.dll"
            "${QT_BIN_DIR}/Qt6Widgetsd.dll"
            "${QT_BIN_DIR}/vulkan-1.dll"

            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
else()
    set(QT_BIN_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/bin")
    set(QT_PLUGINS_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/Qt6/plugins")
    # Копируем необходимые Qt DLL в выходную директорию после сборки
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different

            "${QT_BIN_DIR}/Qt6Gui.dll"
            "${QT_BIN_DIR}/Qt6Widgets.dll" 
            "${QT_BIN_DIR}/vulkan-1.dll"
            "${QT_BIN_DIR}/libpng16.dll"
            "${QT_BIN_DIR}/harfbuzz.dll"
            "${QT_BIN_DIR}/freetype.dll"
            "${QT_BIN_DIR}/icuin74.dll"
            "${QT_BIN_DIR}/icuuc74.dll"
            "${QT_BIN_DIR}/zstd.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# Копируем папку platforms
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${QT_PLUGINS_DIR}/platforms"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms"
)

# включаем MOC, RCC, UIC для .ui, .qrc и сигналов/слотов
set_target_properties(40in_engine_v3 PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)
