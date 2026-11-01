@echo off
setlocal

rem Corre todo el flujo de la Parte 3: compila, genera los 3 lotes y los simula con la Parte 1 y la Parte 2.
rem
rem Uso:  correr_simulacion.bat [semilla] ["capacidades"]
rem   semilla       por defecto 42 (la misma semilla genera siempre los mismos lotes)
rem   capacidades   entre comillas y separadas por espacio, por defecto "0 10 100 1000 5000" (0 = sin cache)
rem Ejemplo:  correr_simulacion.bat 7 "0 50 500"
rem
rem Los resultados salen por pantalla y se guardan en reporte.txt (se borra el anterior al empezar).
rem Si no queres que espere una tecla al final, definir antes NOPAUSE=1.

cd /d "%~dp0"
set PATH=C:\CodeBlocks\MinGW\bin;%PATH%

set RC=0
set SEMILLA=1
set CAPS=0 10 100 1000 5000
if not "%~1"=="" set SEMILLA=%~1
if not "%~2"=="" set CAPS=%~2

echo === Compilando generador y simuladores ===
mingw32-make generador sim_p1 sim_p2
if errorlevel 1 goto error

if not exist datos mkdir datos
if exist reporte.txt del reporte.txt

rem el primer lote ademas pide el encabezado general del reporte (-e), todos piden su titulo de seccion (-s)
call :lote concentrado "-e %SEMILLA% -s"
if errorlevel 1 goto error
call :lote disperso "-s"
if errorlevel 1 goto error
call :lote ordenado "-s"
if errorlevel 1 goto error

echo.
echo === Listo. Resultados guardados en reporte.txt ===
goto fin

:error
echo.
echo *** Hubo un error, se corta la ejecucion ***
set RC=1

:fin
if not defined NOPAUSE pause
exit /b %RC%

rem genera un lote y lo simula con la Parte 1 y con la Parte 2.  %1 = nombre del lote, %2 = opciones del simulador
:lote
echo.
echo === Lote %~1, semilla %SEMILLA% ===
bin\generador.exe %~1 %SEMILLA%
if errorlevel 1 exit /b 1

bin\sim_p1.exe %~2 datos\posteos.bin datos\usuarios_%~1.bin %CAPS%
if errorlevel 1 exit /b 1

bin\sim_p2.exe datos\posteos.bin datos\usuarios_%~1.bin %CAPS%
if errorlevel 1 exit /b 1

exit /b 0
