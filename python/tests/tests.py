#! /usr/bin/env python
# -*- coding: UTF-8 -*-


# PyLasso -- Python bindings for Lasso library
#
# Copyright (C) 2004 Entr'ouvert
# http://lasso.entrouvert.org
# 
# Authors: Nicolas Clapies <nclapies@entrouvert.com>
#          Valery Febvre <vfebvre@easter-eggs.com>
#          Emmanuel Raviart <eraviart@entrouvert.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


import imp
import sys
import unittest


testSuites = (
    'login_tests',
    )

for testSuite in testSuites:
    fp, pathname, description = imp.find_module(testSuite)
    try:                                             
        module = imp.load_module(testSuite, fp, pathname, description)
    finally:                                                   
        if fp:
            fp.close()
    if not module:
        print 'Unable to load test suite:', testSuite

    if module.__doc__:
        doc = module.__doc__
    else:
        doc = testSuite

    print
    print '-' * len(doc)
    print doc
    print '-' * len(doc)

    unittest.TextTestRunner(verbosity=2).run(module.allTests)

