
@echo off

setlocal enabledelayedexpansion


:: ---------------------------------------------------
:: default config
:: ---------------------------------------------------
set TMPSIG="sig.tmp"



:: ---------------------------------------------------
:: Tools
:: ---------------------------------------------------

:: Get SHELL directory.
set "TOOLDIR=%~dp0"

:: mkdiff
set "MKDIFF=%TOOLDIR\%mkdiff\mkdiff.exe"

:: signature
set "MKCERT=%TOOLDIR\%mkimg\mk_cert_openssl.exe"
set "FLCERT=%TOOLDIR\%mkimg\file_cert.exe"
set "SIGKEY_FILE=%TOOLDIR\%mkimg\signing_key.pem"

:: ---------------------------------------------------
:: Arguments
:: ---------------------------------------------------

:: number of old version: -o 
set OLD_VERNUM=""
:: path of old version directory: -s 
set OLD_IMGPATH=""

:: number of new version: -n 
set NEW_VERNUM=""       
:: path of new version directory: -d 
set NEW_IMGPATH=""
      
:: path of FOTA package file: -p
set PACKAGE_PATH=""

:: # secure boot version: -b
set SECBOOT_VER="0"

:: signature flag: -k
set /a SIGNATURE=0

:: ---------------------------------------------------
:: Parse arguments
:: ---------------------------------------------------

:parse
if not "%1"=="" (
    if "%1"=="-o" (
        if "%2"=="" (
            call :usage
        )
        set OLD_VERNUM=%2
        shift
    ) else if "%1"=="-s" (
        if "%2"=="" (
            call :usage
        )
        set OLD_IMGPATH=%2
        shift
    ) else if "%1"=="-n" (
        if "%2"=="" (
            call :usage
        )
        set NEW_VERNUM=%2
        shift
    ) else if "%1"=="-d" (
        if "%2"=="" (
            call :usage
        )
        set NEW_IMGPATH=%2
        shift
    ) else if "%1"=="-p" (
        if "%2"=="" (
            call :usage
        )
        set PACKAGE_PATH=%2
        shift
    ) else if "%1"=="-b" (
        if "%2"=="" (
            call :usage
        )
        set SECBOOT_VER=%2
        shift
    ) else if "%1"=="-k" (
        set /a SIGNATURE=1
    ) 
    shift
    goto :parse
)

:: ---------------------------------------------------
:: Check arguments valid
:: ---------------------------------------------------
if %OLD_VERNUM%=="" (
    call :usage 
) else if %OLD_IMGPATH%=="" (
    call :usage 
) else if %NEW_VERNUM%=="" (
    call :usage 
) else if %NEW_IMGPATH%=="" (
    call :usage 
) else if %PACKAGE_PATH%=="" (
    call :usage 
)

:: ---------------------------------------------------
:: Generate OTA package.
:: ---------------------------------------------------
call :mkdiff

:: ---------------------------------------------------
:: Signature OTA package.
:: ---------------------------------------------------
if %SIGNATURE% neq 0 (
    call :signature
)

:MAIN_END
	goto :EOF


:: ---------------------------------------------------
:: Functions ......
:: ---------------------------------------------------

:usage
    @echo.  
    echo Usage:
    echo   mkOTAPack.bat ^<-o old_ver^> ^<-s old_path^> ^<-n new_ver^> ^<-d new_path^> ^<-p pack_path^> ^[-b secboot^] ^[-k^]
    @echo.
    pause 
    exit 0                                                                                             
exit /b 0

:judge_run_result
    if %errorlevel% neq 0 (
        del /q %TMPSIG%
        pause
        exit 1
    )
exit /b 0

:mkdiff
    echo "==================================== make OTA package ===================================="
    %MKDIFF% -f patch -o %OLD_VERNUM% -s %OLD_IMGPATH% -n %NEW_VERNUM% -d %NEW_IMGPATH% -p %PACKAGE_PATH% -b %SECBOOT_VER%
    call :judge_run_result
exit /b 0

:signature
    echo "================================= signature OTA package ==================================="
    %MKCERT% --certVer=%SECBOOT_VER% --sign_key=%SIGKEY_FILE% --mergeHead=0x1 --srcFile=%PACKAGE_PATH% --signature=%TMPSIG%
    call :judge_run_result
    %FLCERT% --srcFile=%PACKAGE_PATH% --certFile=%TMPSIG% --outFile=%PACKAGE_PATH%
    call :judge_run_result
    del /q %TMPSIG%
exit /b 0

::---------------------------------------------------------------------------------
:: EOF Funciton Definition (EXIT)
::---------------------------------------------------------------------------------
:EOF