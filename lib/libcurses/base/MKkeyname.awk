# $OpenBSD: src/lib/libcurses/base/MKkeyname.awk,v 1.4 2003/03/17 21:54:44 millert Exp $
# $From: MKkeyname.awk,v 1.18 2000/12/10 02:25:23 tom Exp $
##############################################################################
# Copyright (c) 1999,2000 Free Software Foundation, Inc.                     #
#                                                                            #
# Permission is hereby granted, free of charge, to any person obtaining a    #
# copy of this software and associated documentation files (the "Software"), #
# to deal in the Software without restriction, including without limitation  #
# the rights to use, copy, modify, merge, publish, distribute, distribute    #
# with modifications, sublicense, and/or sell copies of the Software, and to #
# permit persons to whom the Software is furnished to do so, subject to the  #
# following conditions:                                                      #
#                                                                            #
# The above copyright notice and this permission notice shall be included in #
# all copies or substantial portions of the Software.                        #
#                                                                            #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    #
# THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    #
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        #
# DEALINGS IN THE SOFTWARE.                                                  #
#                                                                            #
# Except as contained in this notice, the name(s) of the above copyright     #
# holders shall not be used in advertising or otherwise to promote the sale, #
# use or other dealings in this Software without prior written               #
# authorization.                                                             #
##############################################################################
BEGIN {
	print "/* generated by MKkeyname.awk */"
	print ""
	print "#include <ncurses_cfg.h>"
	print "#include <stdlib.h>"
	print "#include <string.h>"
	print "#include <curses.h>"
	print "#include <tic.h>"
	print ""
	print "const struct kn _nc_key_names[] = {"
}

/^[^#]/ {
	printf "\t{ \"%s\", %s },\n", $1, $1;
	}

END {
	printf "\t{ 0, 0 }};\n"
	print ""
	print "NCURSES_EXPORT(NCURSES_CONST char *) keyname (int c)"
	print "{"
	print "\tint i;"
	print "\tstatic char name[20];"
	print "\tchar *p;"
	print "\tsize_t psize;"
	print ""
	print "\tfor (i = 0; _nc_key_names[i].name != 0; i++)"
	print "\t\tif (_nc_key_names[i].code == c)"
	print "\t\t\treturn (NCURSES_CONST char *)_nc_key_names[i].name;"
	print "\tif (c >= 256) return \"UNKNOWN KEY\";"
	print "\tp = name;"
	print "\tpsize = sizeof(name);"
	print "\tif (c >= 128) {"
	print "\t\tstrlcpy(p, \"M-\", psize);"
	print "\t\tp += 2;"
	print "\t\tpsize -= 2;"
	print "\t\tc -= 128;"
	print "\t}"
	print "\tif (c < 0)"
	print "\t\tsnprintf(p, psize, \"%d\", c);"
	print "\telse if (c < 32)"
	print "\t\tsnprintf(p, psize, \"^%c\", c + '@');"
	print "\telse if (c == 127)"
	print "\t\tstrlcpy(p, \"^?\", psize);"
	print "\telse"
	print "\t\tsnprintf(p, psize, \"%c\", c);"
	print "\treturn (NCURSES_CONST char *)name;"
	print "}"
}
