setlocal

if not defined CMAKE set CMAKE=\\arias\CadDoctor_dev\DevEnv\Tools\CMake3.23.1\bin\cmake.exe
rem Change CMAKE for your environment.
rem set CMAKE=D:\Program\CMake\bin\cmake.exe

set HWLOC_X86=E:\Develop3\hwloc-win63-build-2.8.0
set HWLOC_X64=E:\Develop3\hwloc-win64-build-2.8.0

set BLD_DIR=vs2019
set GENERATOR="Visual Studio 16 2019"

set ENV_DIR=%CD%
pushd .
%~d0
cd "%~dp0.."
set ROOT_DIR=%CD%
popd

call :MAKE_WIN64
call :MAKE_WIN64_STATIC
pause
endlocal
goto :EOF

:MAKE_WIN64
call :MAKE_1_ENV %BLD_DIR%.x64 %GENERATOR% x64 "RelWithDebInfo"
call :MAKE_1_ENV %BLD_DIR%.x64 %GENERATOR% x64 "Debug"
goto :EOF

:MAKE_WIN64_STATIC
call :MAKE_1_ENV %BLD_DIR%.x64.static %GENERATOR% x64 "RelWithDebInfo" Static
call :MAKE_1_ENV %BLD_DIR%.x64.static %GENERATOR% x64 "Debug" Static
goto :EOF

:MAKE_1_ENV
pushd .
setlocal
set WORK_DIR=%~1
set CMAKE_GENERATOR=%~2
set ARCH=%~3
set BUILD_CONFIG=%~4
set BUILD_STATIC=%~5

mkdir "%WORK_DIR%"
cd "%WORK_DIR%"
if "%BUILD_STATIC%" == "Static" (
  "%CMAKE%" -DTBB_TEST=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_HWLOC_2_5_LIBRARY_PATH=%HWLOC_X64%\lib\libhwloc.lib -DCMAKE_HWLOC_2_5_INCLUDE_PATH=%HWLOC_X64%\include -DCMAKE_HWLOC_2_5_DLL_PATH=%HWLOC_X64%\bin\libhwloc-15.dll -G "%CMAKE_GENERATOR%" -A %ARCH% "%ROOT_DIR%"
) else (
  "%CMAKE%" -DTBB_TEST=OFF -DCMAKE_HWLOC_2_5_LIBRARY_PATH=%HWLOC_X64%\lib\libhwloc.lib -DCMAKE_HWLOC_2_5_INCLUDE_PATH=%HWLOC_X64%\include -DCMAKE_HWLOC_2_5_DLL_PATH=%HWLOC_X64%\bin\libhwloc-15.dll -G "%CMAKE_GENERATOR%" -A %ARCH% "%ROOT_DIR%"
)
if not "%BUILD_CONFIG%" == "" (
  "%CMAKE%" --build . --config "%BUILD_CONFIG%"
)

endlocal
popd
goto :EOF
