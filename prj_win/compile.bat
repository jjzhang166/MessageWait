@echo off
echo protobuf_net lib build begin

if defined VS100COMNTOOLS (
call "%VS100COMNTOOLS%\vsvars32.bat")

devenv protobuf_net.vcxproj /Rebuild "Release" 
devenv protobuf_net.vcxproj /Rebuild "Debug" 

rmdir /s /q .\debug
rmdir /s /q .\release

echo build protobuf_net lib over
echo on