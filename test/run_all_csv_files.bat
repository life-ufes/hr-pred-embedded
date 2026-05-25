@echo off

rem Print target directory
echo target directory: %1

set "target_dir=%~1"
if "%target_dir:~-1%"=="\" set "target_dir=%target_dir:~0,-1%"
set "output_dir=data_output\%target_dir%"

@REM rem Change to the directory containing this script
@REM cd %1

rem Activate conda environment
call conda activate hr-pred
if errorlevel 1 (
	echo Failed to activate conda environment hr-pred
	exit /b 1
)

rem Iterate over all CSV files in target directory and run main.py for each file
for %%f in ("%target_dir%\*.csv") do (
	echo Processing file: %%f
    rem Run main.py with the current CSV file as input
	python main.py -f "%%~ff" -p COM5 -b 115200 -l %%~nf -o "%output_dir%"
    if errorlevel 1 (
        echo Failed to process file %%f
        @REM exit /b 1
    )

	rem Toggle DTR on COM5 to reset the board (use PowerShell)
	powershell -NoProfile -Command "try { $p = new-Object System.IO.Ports.SerialPort 'COM5',115200,None,8,one; $p.Open(); $p.DtrEnable = $false; Start-Sleep -Milliseconds 200; $p.DtrEnable = $true; Start-Sleep -Milliseconds 200; $p.Close() } catch { Write-Error \"Failed to toggle DTR on COM5: $_\"; exit 1 }"
)

echo All files processed.
