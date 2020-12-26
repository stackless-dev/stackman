rem Build with visualstudio2017buildtools
echo Building %1
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\Common7\Tools\VsDevCmd.bat"
msbuild stackman.sln /p:Platform=%1
