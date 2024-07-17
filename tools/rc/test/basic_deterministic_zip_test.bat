:: This test is only invoked if the CMAKE flag US_USE_DETERMINISTIC_BUNDLE_BUILDS is set.
:: Test to check that 2 different zip files created with exactly the same content result in identical
:: zip files such that their sha 512 checksums are the same. This will fail if the resource compiler
:: embeds anything like date or time stamps.
::
:: this is windows only. For *nix, see basic_test.sh

@echo off
setlocal enabledelayedexpansion

set RC_EXE=%1
shift

:: Cleanup
del empty.txt
del first.zip
del second.zip

:: Create an empty text file
echo. > empty.txt

:: Zip the file
call "%RC_EXE%" --out-file test1.zip --res-add test_file.txt --bundle-name test_bundle
:: Sleep for 2 seconds
timeout /t 2 >nul
:: Zip the file again
call "%RC_EXE%" --out-file test2.zip --res-add test_file.txt --bundle-name test_bundle

:: Calculate SHA256 checksums for the zip files
for /f "tokens=*" %%a in ('certutil -hashfile "first.zip" SHA512 ^| find /v "CertUtil"') do set "hash1=%%a"
for /f "tokens=*" %%b in ('certutil -hashfile "second.zip" SHA512 ^| find /v "CertUtil"') do set "hash2=%%b"

:: Compare checksums
if NOT "!hash1!"=="!hash2!" (
    echo ERROR: The zip files have DIFFERENT SHA512 checksums.
    exit /b 1
)

endlocal
