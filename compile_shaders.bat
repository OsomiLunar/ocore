@echo off
call :treeProcess
goto :eof

:treeProcess
for %%f in (*.vert *.frag *.comp *.geom) do (
	"%VULKAN_SDK%/Bin/glslangValidator.exe" -q -V -e main "%%~ff" -o "%%~nf%%~xf.spv"
)

for /D %%d in (*) do (
    cd %%d
    call :treeProcess
    cd ..
)