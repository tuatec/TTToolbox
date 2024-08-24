@echo off
set prefix_path=C:\Program Files\Epic Games
set Engine=5
set Version=4
set PluginName=TTToolbox
set Plugin_Dir=%~dp0
set Plugin_Version=0.6
set PackageDir=%~dp0\zz_packaging

for /f "skip=2 tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\%Engine%.%Version%" /v "InstalledDirectory"') do set "EngineDirectory=%%b"

call "%EngineDirectory%\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -Plugin="%Plugin_Dir%\%PluginName%.uplugin" -Package="%PackageDir%\%PluginName%_v%Plugin_Version%_UE%Engine%.%Version%_win64_release" -Rocket"

set pathBefore=%cd%
cd %PackageDir%

powershell -command "Compress-Archive -Force -Path '%PackageDir%\%PluginName%_v%Plugin_Version%_UE%Engine%.%Version%_win64_release' -DestinationPath '%PackageDir%\%PluginName%_v%Plugin_Version%_UE%Engine%.%Version%_win64_release.zip'"

cd %pathBefore%

