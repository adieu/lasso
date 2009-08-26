#! /usr/bin/env python

from cStringIO import StringIO
import glob
import re
import sys
import os

enable_wsf = 0

if '-wsf' in sys.argv:
    enable_wsf = 1

if len(sys.argv) == 2+enable_wsf:
    srcdir = sys.argv[1]
else:
    srcdir = '.'

wsf = ['lasso_disco_', 'lasso_dst_', 'lasso_is_', 'lasso_profile_service_',
        'lasso_discovery', 'lasso_wsf', 'lasso_interaction_', 'lasso_utility_',
        'lasso_sa_', 'lasso_soap_binding', 'lasso_authentication', 'lasso_wsse_',
        'lasso_sec_', 'lasso_idwsf2', 'lasso_wsf2', 'lasso_wsa_',
        'lasso_wsu_']
if enable_wsf:
    wsf = []

fd = StringIO()

print >> fd, "/* This file has been autogenerated; changes will be lost */"
print >> fd, ""
print >> fd, "typedef GType (*type_function) ();"
print >> fd, ""

header_files = []
for header_file in glob.glob('%s/*/*.h' % srcdir) + glob.glob('%s/*/*/*.h' % srcdir):
    if ('/id-wsf/' in header_file or '/id-wsf-2.0' in header_file) and not enable_wsf:
        continue
    header_files.append(header_file)
    try:
        type = re.findall('lasso_.*get_type', open(header_file).read())[0]
    except IndexError:
        continue
    for t in wsf:
        if type.startswith(t):
            break
    else:
        print >> fd, "extern GType %s();" % type

print >> fd, ""
print >> fd, "type_function functions[] = {"
for header_file in header_files:
    try:
        type = re.findall('lasso_.*get_type', open(header_file).read())[0]
    except IndexError:
        continue
    for t in wsf:
        if type.startswith(t):
            break
    else:
        print >> fd, "\t%s," % type
print >> fd, "\tNULL"
print >> fd, "};"

file('types.c', 'w').write(fd.getvalue())
