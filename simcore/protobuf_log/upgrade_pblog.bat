@echo off
echo %1%
echo %2%

set service_sys_dir=%2%\sys\service_data
set service_data_dir=%2%\data\service_data
set script_dir=%service_sys_dir%\script

set sim_data_dir=%service_data_dir%\sim_data
set pblog_dir_data=%service_data_dir%\sim_data\pblog

::check and copy post_process to script_dir
if not exist %script_dir% (
    mkdir %script_dir%
)

::create sim_data_dir as needed
if not exist %sim_data_dir% (
    mkdir %sim_data_dir%
)

if not exist %pblog_dir_data% (
    mkdir %pblog_dir_data%
)

set post_script=%1%\service\data\script\post_process.exe
copy /Y /B %post_script% %script_dir%
set post_script_save_agent=%1%\service\data\script\pb_save_agent.exe
copy /Y /B %post_script_save_agent% %script_dir%
