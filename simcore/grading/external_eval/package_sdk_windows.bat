@echo off
:: Disable command echoing to prevent cluttering the command prompt output

:: Save the directory path of the current script into 'thisSDKDir'
set thisSDKDir=%~dp0

:: Return to the original script directory
cd %thisSDKDir%

:: Copy necessary header files of third-party libraries
xcopy ..\..\framework\src\*.h .\txSimGradingSDK\deps\include\framework\ /R /Y
xcopy ..\..\..\common\map_sdk\hadmap\include .\txSimGradingSDK\deps\include\map_sdk\ /E /R /Y
xcopy ..\..\..\common\message\build\*.h .\txSimGradingSDK\deps\include\message\ /R /Y

:: Copy necessary lib files of third-party libraries
xcopy ..\..\..\common\map_sdk\hadmap\lib .\txSimGradingSDK\deps\lib\map_sdk\x64\Release\ /E /R /Y

:: Copy necessary bin files of third-party libraries
:: xcopy ..\build\bin\Release\*.dll .\txSimGradingSDK\deps\bin\ /E /R /Y

:: Copy necessary header files of txSimGrading
xcopy ..\actors\*.h .\txSimGradingSDK\include\actors\ /R /Y
xcopy ..\eval\*.h .\txSimGradingSDK\include\eval\ /R /Y
xcopy ..\factory\eval_factory.h .\txSimGradingSDK\include\factory\ /R /Y
xcopy ..\factory\eval_factory_interface.h .\txSimGradingSDK\include\factory\ /R /Y
xcopy ..\manager\*.h .\txSimGradingSDK\include\manager\ /R /Y
xcopy ..\stateflow\*.h .\txSimGradingSDK\include\stateflow\ /R /Y
xcopy ..\utils\*.h .\txSimGradingSDK\include\utils\ /R /Y

:: Copy necessary lib files of txSimGrading
xcopy ..\build\bin\Release\sim_msg.lib .\txSimGradingSDK\lib\x64\Release\ /E /R /Y
xcopy ..\build\bin\Release\eval_actors.lib .\txSimGradingSDK\lib\x64\Release\ /E /R /Y
xcopy ..\build\bin\Release\eval_base.lib .\txSimGradingSDK\lib\x64\Release\ /E /R /Y
xcopy ..\build\bin\Release\manager.lib .\txSimGradingSDK\lib\x64\Release\ /E /R /Y
xcopy ..\build\bin\Release\factory.lib .\txSimGradingSDK\lib\x64\Release\ /E /R /Y
xcopy ..\build\bin\Release\utils.lib .\txSimGradingSDK\lib\x64\Release\ /E /R /Y
xcopy ..\build\bin\Release\stateflow.lib .\txSimGradingSDK\lib\x64\Release\ /E /R /Y
xcopy ..\build\bin\Release\eval_no_check_kpis.lib .\txSimGradingSDK\lib\x64\Release\ /E /R /Y

:: Copy the source codes and scripts of txSimGrading
xcopy .\src .\txSimGradingSDK\src\ /E /R /Y
xcopy .\CMakeLists.txt .\txSimGradingSDK\ /R /Y
xcopy .\external_grading_kpi.json .\txSimGradingSDK\ /R /Y
xcopy .\build_windows_vs2022.bat .\txSimGradingSDK\ /R /Y

:: Use the tar command to compress related files and package them into SDK
tar zcvf txSimGradingSDK.tar.gz txSimGradingSDK

:: Remove the temporary file folders
rd /s /q .\txSimGradingSDK

:: Change the working directory back to the original directory where the script was run
cd %thisSDKDir%
