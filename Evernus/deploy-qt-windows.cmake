set(WINDEPLOYQT "D:/Qt/Qt5.12.7/5.12.7/msvc2017_64/bin/windeployqt.exe")
set(COMPONENT_NAME_MAIN "Evernus")
set(CMAKE_CURRENT_SOURCE_DIR "D:/github/evernus/Evernus")


execute_process(COMMAND ${WINDEPLOYQT} --qmldir=${CMAKE_CURRENT_SOURCE_DIR}/qml  ${COMPONENT_NAME_MAIN}/data/bin WORKING_DIRECTORY ${CPACK_TEMPORARY_INSTALL_DIRECTORY}/packages)
