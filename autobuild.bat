@echo off

if "%1"=="" (
    set buildType=Release
) else (
    set buildType=%1
)

if "%2"=="" (
    set packProject=NWLan
) else (
    set packProject=%2
)

set msbuild="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"
set installer="D:\qt-installer\bin\binarycreator.exe"
set binaryDir=D:\mywork\product\NWLan

set buildApp="..\buildApp"

::https://stackoverflow.com/questions/7005951/batch-file-find-if-substring-is-in-string-not-in-a-file
for /f "delims=:" %%i in (%packProject%/.module) do (
    echo.%%i | findstr /C:"dll">nul && (
        if exist ..\build%%i (
            rd /s /q ..\build%%i
        )
        mkdir ..\build%%i

        echo %buildApp%\%buildType%\plugin
        cmake -S ..\%%i -DCMAKE_BUILD_TYPE=%buildType% -A x64 -B ..\build%%i -DMS=%buildApp%\%buildType%\plugin
        %msbuild% ..\build%%i\ALL_BUILD.vcxproj /p:Configuration=%buildType%
    )
)

xcopy %buildApp%\%buildType%\plugin\%buildType%\* "%binaryDir%\qtinstall\mypackages\content\data\plugin" /e /y
rd /s /q %buildApp%\%buildType%\plugin\%buildType%

cd pack
WINPACK.exe  ../%packProject%/config_.json
cd ..

%installer% -c %binaryDir%\qtinstall\config\config.xml -p %binaryDir%\qtinstall\mypackages NWLanV4.exe -v 