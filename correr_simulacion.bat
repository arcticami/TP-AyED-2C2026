@echo off
setlocal

rem Corre todo el flujo de la Parte 3: compila, genera los 3 lotes y los simula con la Parte 1 y la Parte 2.
rem
rem Uso:  correr_simulacion.bat [semilla] ["capacidades"]
rem   semilla       por defecto 1 (la misma semilla genera siempre los mismos lotes)
rem   capacidades   entre comillas y separadas por espacio, por defecto "0 10 100 1000 5000" (0 = sin cache)
rem Ejemplo:  correr_simulacion.bat 7 "0 50 500"
rem
rem Va mostrando por pantalla lo que ejecuta y, a medida que termina cada parte, agrega su resultado a
rem reporte.txt (se borra el anterior al empezar). Cada tabla aparece cuando termina de correr.
rem Si no queres que espere una tecla al final, definir antes NOPAUSE=1.

cd /d "%~dp0"
set PATH=C:\CodeBlocks\MinGW\bin;%PATH%

set RC=0
set SEMILLA=1
set CAPS=0 10 100 1000 5000
if not "%~1"=="" set SEMILLA=%~1
if not "%~2"=="" set CAPS=%~2

echo === Compilando ===
mingw32-make generador sim_p1 sim_p2
if errorlevel 1 goto error

if not exist datos mkdir datos
if exist reporte.txt del reporte.txt

echo.
echo === Generando lotes, semilla %SEMILLA% ===
for %%L in (concentrado disperso ordenado) do (
    bin\generador.exe %%L %SEMILLA%
    if errorlevel 1 goto error
)

rem datos generales, una sola vez arriba del reporte (los lee del archivo el propio simulador)
echo.
echo === Datos generales ===
(
echo semilla  : %SEMILLA%
echo archivos : datos\posteos.bin, datos\usuarios_^<lote^>.bin
) > reporte.tmp
bin\sim_p1.exe datos\posteos.bin datos\usuarios_concentrado.bin >> reporte.tmp
if errorlevel 1 goto error
call :anexar

call :simular concentrado CONCENTRADO
if errorlevel 1 goto error
call :simular disperso DISPERSO
if errorlevel 1 goto error
call :simular ordenado ORDENADO
if errorlevel 1 goto error

echo.
echo === Listo. Reporte guardado en reporte.txt ===
goto fin

:error
echo.
echo *** Hubo un error, se corta la ejecucion ***
set RC=1

:fin
if exist reporte.tmp del reporte.tmp
if not defined NOPAUSE pause
exit /b %RC%

rem muestra reporte.tmp por pantalla, lo agrega a reporte.txt y lo borra
:anexar
type reporte.tmp
type reporte.tmp >> reporte.txt
del reporte.tmp
exit /b 0

rem simula un lote con la Parte 1 y con la Parte 2.  %1 = lote, %2 = titulo de la seccion
:simular
echo.
(
echo ----------------------------------------------------------------------------------------------------
echo %~2
echo ----------------------------------------------------------------------------------------------------
echo.
) > reporte.tmp
call :anexar

echo [%~1] simulando Parte 1 - lista simple, capacidades: %CAPS% ...
bin\sim_p1.exe datos\posteos.bin datos\usuarios_%~1.bin %CAPS% > reporte.tmp
if errorlevel 1 exit /b 1
call :anexar

echo [%~1] simulando Parte 2 - lista doble + indice arbol, capacidades: %CAPS% ...
bin\sim_p2.exe datos\posteos.bin datos\usuarios_%~1.bin %CAPS% > reporte.tmp
if errorlevel 1 exit /b 1
call :anexar

exit /b 0
