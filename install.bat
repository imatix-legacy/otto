@echo off
zip -f otto
copy otto.bat \usr\bin
copy otto.d   \usr\bin
copy otto.fmt \usr\bin
copy otto     \usr\bin
copy c.bat    \usr\bin
copy otto.zip i:\site\pub\tools
copy c        ..\sfl
copy c        ..\smt
copy c        ..\studio
copy c        ..\gslgen
copy c.cmd    ..\sfl
copy c.cmd    ..\smt
copy c.cmd    ..\studio
copy c.cmd    ..\gslgen
copy c.bat    ..\sfl
copy c.bat    ..\smt
copy c.bat    ..\studio
copy c.bat    ..\gslgen
