cmake_minimum_required(VERSION 3.16)

project(SummerCake VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置动态链接
set(BUILD_SHARED_LIBS ON)

# 查找Qt 6.10.0组件
find_package(Qt6 6.10.0 REQUIRED COMPONENTS
    Widgets
    Network
    Core
    Gui
)

# 资源文件
set(RESOURCES
    resources/resources.qrc
)

# 包含头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# 源文件
set(SOURCES
    src/main.cpp
    src/git/gitmanager.cpp
    src/ai/aimanager.cpp
    src/ai/aiprovider.cpp
    src/ai/openaiprovider.cpp
    src/widgets/mainwindow.cpp
    src/widgets/aisettingdialog.cpp
    src/widgets/aifloatwidget.cpp
    src/widgets/filestatusmodel.cpp
    src/widgets/commithistorymodel.cpp
    src/widgets/branchmodel.cpp
    src/widgets/remotemodel.cpp
)

# 头文件
set(HEADERS
    src/git/gitmanager.h
    src/ai/aimanager.h
    src/ai/aiprovider.h
    src/ai/openaiprovider.h
    src/widgets/mainwindow.h
    src/widgets/aisettingdialog.h
    src/widgets/aifloatwidget.h
    src/widgets/filestatusmodel.h
    src/widgets/commithistorymodel.h
    src/widgets/branchmodel.h
    src/widgets/remotemodel.h
)

# UI文件
set(UIS
    src/widgets/mainwindow.ui
    src/widgets/aisettingdialog.ui
)

# 创建可执行文件
qt_add_executable(SummerCake
    ${SOURCES}
    ${HEADERS}
    ${UIS}
    ${RESOURCES}
)

# 链接Qt库
target_link_libraries(SummerCake PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Core
    Qt6::Gui
)

# 设置输出目录
set_target_properties(SummerCake PROPERTIES
    WIN32_EXECUTABLE ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
)

# 安装配置
install(TARGETS SummerCake
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 部署Qt依赖
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SummerCake)
endif()
