@echo off
echo MessageWait lib build begin

if defined VS100COMNTOOLS (
call "%VS100COMNTOOLS%\vsvars32.bat")

devenv MessageWait.vcxproj /Rebuild "Release" 
devenv MessageWait.vcxproj /Rebuild "Debug" 

rmdir /s /q .\debug
rmdir /s /q .\release

echo build MessageWait lib over
echo on