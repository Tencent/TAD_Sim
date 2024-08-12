@echo off
powershell.exe -NoProfile -ExecutionPolicy Bypass "& {& '%~dp0play.ps1' %*}"
