@echo off
setlocal

:: Path Configuration
set "root_folder=C:\Users\yosprout\Desktop\Codes\Github\PacMan-CRC48"
set "dest_folder=%root_folder%\PacMan"
set "exe_name=CRC48-PacMan.exe"
set "release_build=%root_folder%\build\Desktop_Qt_6_10_1_MinGW_64_bit-Release"
set "qt_bin=C:\Qt\6.10.1\mingw_64\bin"

echo [1/4] Creating destination folder...
if not exist "%dest_folder%" mkdir "%dest_folder%"

echo [2/4] Copying resources and executable...
copy /Y "%root_folder%\Map.txt" "%dest_folder%\Map.txt"
copy /Y "%root_folder%\yo_stylesheet.qss" "%dest_folder%\yo_stylesheet.qss"
copy /Y "%release_build%\%exe_name%" "%dest_folder%\%exe_name%"

echo [3/4] Setting up Qt environment variables...
set "PATH=%qt_bin%;%PATH%"

echo [4/4] Running windeployqt...
windeployqt --no-translations "%dest_folder%\%exe_name%"

echo.
echo ==========================================
echo Deployment Complete!
echo Output Path: %dest_folder%
echo ==========================================
pause
