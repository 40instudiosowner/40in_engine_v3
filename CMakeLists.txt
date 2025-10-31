
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
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (40in_engine_v3 "40in_engine_v3.cpp" "40in_engine_v3.h")

# Подключаем Qt-шные заголовки (модули)
target_link_libraries(40in_engine_v3 PRIVATE
    Qt6::Core
    Qt6::Widgets
)

if(NOT DEFINED VCPKG_ROOT)
    set(VCPKG_ROOT "C:/vcpkg") # или твой путь
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(QT_BIN_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/bin")
    set(QT_PLUGINS_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/Qt6/plugins")
else()
    set(QT_BIN_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/bin")
    set(QT_PLUGINS_DIR "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/Qt6/plugins")
endif()


# Копируем необходимые Qt DLL в выходную директорию после сборки
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_BIN_DIR}/Qt6Cored.dll"
        "${QT_BIN_DIR}/Qt6Guid.dll"
        "${QT_BIN_DIR}/Qt6Widgetsd.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Копируем папку platforms
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${QT_PLUGINS_DIR}/platforms"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms"
)

# (необязательно) включаем MOC, RCC, UIC для .ui, .qrc и сигналов/слотов
set_target_properties(40in_engine_v3 PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)