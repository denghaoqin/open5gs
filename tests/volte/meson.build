# Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>

# This file is part of Open5GS.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

testapp_volte_sources = files('''
    pcscf-fd-path.h
    pcscf-fd-path.c

    abts-main.c
    volte-test.c
'''.split())

testapp_volte_exe = executable('volte',
    sources : testapp_volte_sources,
    c_args : [testunit_core_cc_flags,
        '-DFD_EXT_DIR="@0@"'.format(freediameter_extensions_builddir)],
    dependencies : libtestapp_dep)

test('volte', testapp_volte_exe, is_parallel : false, suite: 'app')
