@echo off

set "cpp_files="

cd cpp
for /r %%t in (*.cc *.cpp *.h) do (
  call set "cpp_files=%%cpp_files%% %%t"
)
cd ..

cd windows/runner
for /r %%t in (*.cc *.cpp *.h) do (
  call set "cpp_files=%%cpp_files%% %%t"
)
cd ../..

if not "%cpp_files%"=="" (
    clang-format -i -style=google %cpp_files%
)
