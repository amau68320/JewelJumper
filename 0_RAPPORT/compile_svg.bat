@echo off
set fname=%1
set prog="C:\Program Files\Inkscape\inkscape.com"
if not exist %prog% set prog="F:\Program Files\Inkscape\inkscape.com"
%prog% -D -z --file=%fname% --export-pdf=%fname:~0,-4%.pdf --export-latex
echo %ERRORLEVEL%
pause
