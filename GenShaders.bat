@echo off
pushd run\assets\shaders

powershell foreach ($file in (Get-ChildItem *.frag)) { echo "-------------------------"; echo "Compiling: $file.Name"; glslc.exe $file -c }
powershell foreach ($file in (Get-ChildItem *.vert)) { echo "-------------------------"; echo "Compiling: $file.Name"; glslc.exe $file -c }

popd
