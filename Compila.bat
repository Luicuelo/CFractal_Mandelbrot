del *.exe
chcp 65001
cl.exe /Zi /nologo /EHsc /Ot  /FoD:\Luis\C\Pfractal_2\intermediate\ /FdD:\Luis\C\Pfractal_2\intermediate\ /FeD:\Luis\C\Pfractal_2\Pfractal.exe D:\Luis\C\Pfractal_2/*.c D:\Luis\C\Pfractal_2/source/*.c user32.lib Gdi32.lib Comctl32.lib menu.res
