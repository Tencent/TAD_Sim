@echo off

SET CUR_DIR=%~dp0
SET SRC_DIR=%CUR_DIR%\..\..\

SET /p docker_file="Input docker file (Default is %CUR_DIR%\Dockerfile): "
IF "%docker_file%"=="" (
    SET docker_file=%CUR_DIR%\Dockerfile
    ECHO =^> Using default Dockerfile.
)

SET /p version="Input tag prefix (Default is cloud): "
IF "%version%"=="" (
    SET version=cloud
    ECHO =^> Using default tag prefix.
)

SET /p docker_repo="Input repo type (Default is csighub.tencentyun.com/simcloud/sim_hadmap): "
IF "%docker_repo%"=="" (
    SET docker_repo=csighub.tencentyun.com/simcloud/sim_hadmap
    ECHO =^> Using default repo.
)

FOR /f "tokens=*" %%a IN ('powershell.exe -Command "Get-Date -Format 'yyyyMMdd.HHmmss'"') DO SET "time_stamp=%%a"
FOR /f "tokens=* delims=/" %%a IN ('git rev-parse --abbrev-ref HEAD') DO SET git_branch=%%a
FOR /f "tokens=* delims=" %%a IN ('git rev-parse --short HEAD') DO SET git_rev=%%a
SET docker_tag=%docker_repo%:%version%_%git_branch%.%git_rev%.%time_stamp%

COPY "%USERPROFILE%\.gitconfig" "%SRC_DIR%"
COPY "%USERPROFILE%\.git-credentials" "%SRC_DIR%"

ECHO In %SRC_DIR%
ECHO =^> Building: %docker_tag%

docker build -t "%docker_tag%" -f "%docker_file%" %SRC_DIR%

DEL %SRC_DIR%\.gitconfig %SRC_DIR%\.git-credentials

PAUSE > NUL
