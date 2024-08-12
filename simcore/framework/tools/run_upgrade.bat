@echo off

echo "=== executing upgrade-db ... ==="
"%~dp0\upgrade-db.exe" %1 %2

REM "%~dp0\..\txSimService\updator-scenario.exe" %1 %2

echo "=== executing upgrade_pblog ... ==="
CALL "%~dp0\upgrade_pblog.bat" %1 %2

echo "=== executing upgrade_mil ... ==="
CALL "%~dp0\upgrade_mil.bat" %1 %2

echo "=== executing upgrade_sensor ... ==="
"%~dp0\upgrade_sensor.exe" %1 %2

echo "=== executing upgrade_grading_report ... ==="
"%~dp0\upgrade_grading_report.exe" %1 %2

if exist "%~dp0\UpdateSceneV2.exe" (
    echo "=== executing upgrade_scensor_v2 ... ==="
    "%~dp0\UpdateSceneV2.exe" %1 %2
) else (
	echo "not find UpdateSceneV2.exe"
)
