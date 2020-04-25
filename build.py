#!/usr/bin/env python

import os
import platform

system = platform.system() 

if system == 'Windows':
    os.system('cl /nologo /W4 /Zi /EHsc -wd4127 -wd4702 -wd4127 static_threads.cpp    os_windows.cpp')
    print('')
    os.system('cl /nologo /W4 /Zi /EHsc -wd4127 -wd4702 -wd4127 object_threads.cpp os_windows.cpp')
    print('')
    os.system('cl /nologo /W4 /Zi /EHsc -wd4127 -wd4702 -wd4127 test_threads.cpp   os_windows.cpp')
elif system == 'Linux':
    os.system('g++ -o static_threads static_threads.cpp os_linux.cpp')
    os.system('g++ -o object_threads object_threads.cpp os_linux.cpp')
    os.system('g++ -o test_threads   test_threads.cpp   os_linux.cpp')
