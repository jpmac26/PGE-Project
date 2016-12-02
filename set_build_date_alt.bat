del .\pge_version.h
set LAST_COMMIT_DATE < git log -1 --format=%cd
echo "//DO NOT EDIT THIS FILE DIRECTLY!!!" > .\pge_version.h
echo "//EDIT \"pge_version_h_template\" INSTEAD!!!" >> .\pge_version.h
echo >> .\pge_version.h
type .\pge_version_h_template >> .\pge_version.h
echo >> ./pge_version.h
echo "#define _CURRENT_BUILD_DATE \""%LAST_COMMIT_DATE%"\"" >> .\pge_version.h
echo >> .\pge_version.h
echo "#endif" >> .\pge_version.h
echo >> .\pge_version.h
echo "//DO NOT EDIT THIS FILE DIRECTLY!!!" >> .\pge_version.h
echo "//EDIT \"pge_version_h_template\" INSTEAD!!!" >> .\pge_version.h
