@echo off
set Source=%~dp0\lib
set Target=%~dp0\bin

REM �����ļ�
xcopy %Source%\*.dll %Target%\lib /h /s /y

PUSimulator.exe
