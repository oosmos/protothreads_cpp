#!/usr/bin/env python

import os
import platform

system = platform.system()

def Remove(Filename):
    os.system('rm -f %s' % Filename)

if system == 'Windows':
    Remove('*.exe')
    Remove('*.obj')
    Remove('*.ilk')
    Remove('*.pdb')
    Remove('*.suo')
    Remove('*.sln')
elif system == 'Linux':
    Remove('*.o')
    Remove('thread_test')
