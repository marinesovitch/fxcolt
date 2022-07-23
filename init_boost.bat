@REM initializes the boost submodule in the 3rdParty subdirectory, only headers are needed
git submodule update --init --recursive
cd 3rdParty/boost
call .\bootstrap.bat
.\b2.exe headers
