@echo off
rem ---------------------------------------------------------------------------
rem  Borsa Patronu - tek tikla derleme.
rem  Cift tikla: oyunu derler (borsa_patronu.exe).
rem  Diger hedefler icin komut satirindan:  build.bat test | web | paket | sun
rem  Ayrintilar: python build.py --help
rem ---------------------------------------------------------------------------
python "%~dp0build.py" %*
if errorlevel 1 (
    echo.
    echo DERLEME BASARISIZ - yukaridaki mesaja bak.
)
echo.
pause
