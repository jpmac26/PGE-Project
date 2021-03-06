#/bin/bash
bak=~+

if [[ "$OSTYPE" == "msys"* ]]; then
    ./build.bat
    exit 0
fi

#flags
flag_pause_on_end=true
QMAKE_EXTRA_ARGS=""
MAKE_EXTRA_ARGS="-r -j 4"
flag_debugThisScript=false

for var in "$@"
do
    case "$var" in
        --help)
            echo ""
            echo -e "=== \e[44mBuild script for PGE Project\e[0m ==="
            echo ""
            echo -e "\E[4mSYNTAX:\E[0m"
            echo ""
            echo -e "    $0 \e[90m[<arg1>] [<arg2>] [<arg2>] ...\e[0m"
            echo ""
            echo -e "\E[4mAVAILABLE ARGUMENTS:\E[0m"
            echo ""

            echo -e "--- Actions ---"
            echo -e " \E[1;4mlupdate\E[0m          - Update the translations"
            echo -e " \E[1;4mlrelease\E[0m         - Compile the translations"
            echo -e " \E[1;4mclean\E[0m            - Remove all object files and caches to build from scratch"
            echo -e " \E[1;4mrepair-submodules\E[0m- Repair invalid or broken submodules"
            echo -e " \E[1;4misvalid\E[0m          - Show validation state of dependencies"
            echo -e ""

            echo -e "--- Flags ---"
            echo -e " \E[1;4mno-pause\E[0m         - Don't pause script on completion'"
            echo -e " \E[1;4msilent-make\E[0m      - Don't print build commands for each building file"
            printf  " \E[1;4muse-ccache\E[0m       - Use the CCache to speed-up build process"
            if [[ ! -f /usr/bin/ccache && ! -f /bin/ccache && ! -f /usr/local/bin/ccache ]]; then
                printf " \E[0;4;41;37m<ccache is not installed!>\E[0m"
            fi
            printf "\n"
            echo ""

            echo -e "--- Disable building of components ---"
            echo -e " \E[1;4mnoeditor\E[0m         - Skip building of PGE Editor compoment"
            echo -e " \E[1;4mnoengine\E[0m         - Skip building of PGE Engine compoment"
            echo -e " \E[1;4mnocalibrator\E[0m     - Skip building of Playable Character Calibrator compoment"
            echo -e " \E[1;4mnomaintainer\E[0m     - Skip building of PGE Maintainer compoment"
            echo -e " \E[1;4mnomanager\E[0m        - Skip building of PGE Manager compoment"
            echo -e " \E[1;4mnogifs2png\E[0m       - Skip building of GIFs2PNG compoment"
            echo -e " \E[1;4mnopng2gifs\E[0m       - Skip building of PNG2GIFs compoment"
            echo -e " \E[1;4mnolazyfixtool\E[0m    - Skip building of LazyFixTool compoment"
            echo ""

            echo "--- Special ---"
            echo -e " \E[1;4mdebugscript\E[0m      - Show some extra information to debug this script"
            echo ""

            echo "--- For fun ---"
            echo -e " \E[1;4mcolors\E[0m           - Prints various blocks of different color with showing their codes"
            echo -e " \E[1;4mcool\E[0m             - Prints some strings inside the lines (test of printLine command)"
            echo ""

            echo -e "==== \e[43mIMPORTANT!\e[0m ===="
            echo "This script is designed for Linux and macOS operating systems."
            echo "If you trying to start it under Windows, it will automatically start"
            echo "the build.bat script instead of this."
            echo "===================="
            echo ""
            exit 1
            ;;
        no-pause)
                flag_pause_on_end=false
            ;;
        silent-make)
                MAKE_EXTRA_ARGS="${MAKE_EXTRA_ARGS} -s"
            ;;
        colors)
            for((i=0;i<=1;i++))
            do
                printf "="
                for((j=0;j<=7;j++))
                do
                    printf "\E[${i};3${j};4${j}m"
                    printf "[${i};3${j};4${j}]"
                    printf "\E[0;00m "
                done
                printf "=\n"
            done
            exit 0
            ;;
        use-ccache)
                if [[ "$OSTYPE" == "linux-gnu" ]]; then
                    QMAKE_EXTRA_ARGS="$QMAKE_EXTRA_ARGS -spec linux-g++ CONFIG+=useccache"
                else
                    QMAKE_EXTRA_ARGS="$QMAKE_EXTRA_ARGS CONFIG+=useccache"
                fi
            ;;
        clean)
                echo "======== Remove all cached object files and automatically generated Makefiles ========"
                if [[ "$OSTYPE" == "msys"* ]]; then
                    ./clean_make.bat nopause
                    BinDir=bin-w32
                else
                    ./clean_make.sh nopause
                    BinDir=bin
                fi

                if [ -d ./$BinDir/_build_x32 ]; then
                    echo "removing $BinDir/_build_x32 ..."
                    rm -Rf ./$BinDir/_build_x32
                fi

                if [ -d ./$BinDir/_build_x64 ]; then
                    echo "removing $BinDir/_build_x64 ..."
                    rm -Rf ./$BinDir/_build_x64
                fi

                echo 'removing Dependencies build cache ...'

                ./clear_deps.sh
                echo "==== Clear! ===="
                exit 0;
            ;;

        repair-submodules)
            #!!FIXME!! Implement parsing of submodules list and fill this array automatically
            #NOTE: Don't use "git submodule foreach" because broken submodule will not shown in it's list!
            SUBMODULES="_Libs/FreeImage"
            SUBMODULES="${SUBMODULES} _Libs/QtPropertyBrowser"
            SUBMODULES="${SUBMODULES} _Libs/sqlite3"
            SUBMODULES="$SUBMODULES _common/PGE_File_Formats"
            SUBMODULES="$SUBMODULES _common/PgeGameSave/submodule"
            # \===============================================================================
            for s in $SUBMODULES
            do
                if [ -d $s ];then
                    echo "Remove folder ${s}..."
                    rm -Rf $s
                fi
            done
            echo "Fetching new submodules..."
            git submodule init
            git submodule update
            echo ""
            git submodule foreach git checkout master
            git submodule foreach git pull origin master
            echo ""
            echo "==== Fixed! ===="
            exit 0;
            ;;

        # Enable debuggin of this script by showing states of inernal variables with pauses
        debugscript)
            flag_debugThisScript=true
            ;;

        # Disable building of some compnents
        noeditor)
            QMAKE_EXTRA_ARGS="${QMAKE_EXTRA_ARGS} CONFIG+=${var}"
            ;;
        noengine)
            QMAKE_EXTRA_ARGS="${QMAKE_EXTRA_ARGS} CONFIG+=${var}"
            ;;
        nocalibrator)
            QMAKE_EXTRA_ARGS="${QMAKE_EXTRA_ARGS} CONFIG+=${var}"
            ;;
        nogifs2png)
            QMAKE_EXTRA_ARGS="${QMAKE_EXTRA_ARGS} CONFIG+=${var}"
            ;;
        nopng2gifs)
            QMAKE_EXTRA_ARGS="${QMAKE_EXTRA_ARGS} CONFIG+=${var}"
            ;;
        nolazyfixtool)
            QMAKE_EXTRA_ARGS="${QMAKE_EXTRA_ARGS} CONFIG+=${var}"
            ;;
        nomanager)
            QMAKE_EXTRA_ARGS="${QMAKE_EXTRA_ARGS} CONFIG+=${var}"
            ;;
        nomaintainer)
            QMAKE_EXTRA_ARGS="${QMAKE_EXTRA_ARGS} CONFIG+=${var}"
            ;;
    esac
done

#=============Detect directory that contains script=====================
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
    SCRDIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
    SOURCE="$(readlink "$SOURCE")"
    [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
SCRDIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
#=======================================================================
echo $SCRDIR
cd $SCRDIR
source ./_common/functions.sh
#=======================================================================

if [ -f "$SCRDIR/_paths.sh" ]
then
    source "$SCRDIR/_paths.sh"
else
    echo ""
    echo "_paths.sh is not exist! Run \"generate_paths.sh\" first!"
    errorofbuild
fi

PATH=$QT_PATH:$PATH
LD_LIBRARY_PATH=$QT_LIB_PATH:$LD_LIBRARY_PATH

if $flag_debugThisScript; then
    echo "QMAKE_EXTRA_ARGS = ${QMAKE_EXTRA_ARGS}"
    echo "MAKE_EXTRA_ARGS = ${MAKE_EXTRA_ARGS}"
    pause
fi

checkForDependencies()
{
    libPref="lib"
    dlibExt="so"
    slibExt="a"
    osDir="linux_default"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        dlibExt="dylib"
        slibExt="a"
        libPref="lib"
        osDir="macos"
    elif [[ "$OSTYPE" == "linux-gnu" || "$OSTYPE" == "linux" ]]; then
        osDir="linux"
        libPref="lib"
    elif [[ "$OSTYPE" == "freebsd"* ]]; then
        osDir="freebsd"
        libPref="lib"
    elif [[ "$OSTYPE" == "msys"* ]]; then
        dlibExt="a"
        osDir="win32"
        libPref="lib"
    fi
    libsDir=$SCRDIR/_Libs/_builds/$osDir/

    HEADS="SDL2/SDL.h"
    HEADS="${HEADS} SDL2/SDL_mixer_ext.h"
    HEADS="${HEADS} FreeImageLite.h"
    HEADS="${HEADS} mad.h"
    HEADS="${HEADS} sqlite3.h"
    HEADS="${HEADS} freetype2/ft2build.h"
    HEADS="${HEADS} FLAC/all.h"
    HEADS="${HEADS} luajit-2.1/lua.h"
    HEADS="${HEADS} luajit-2.1/luajit.h"
    HEADS="${HEADS} ogg/ogg.h"
    HEADS="${HEADS} vorbis/codec.h"
    HEADS="${HEADS} vorbis/vorbisfile.h"
    HEADS="${HEADS} vorbis/vorbisenc.h"
    for head in $HEADS
    do
        if [[ $1 == "test" ]]
        then
            echo "Checking include ${head}..."
        fi
        if [[ ! -f ${libsDir}/include/${head} ]]
        then
            lackOfDependency
        fi
    done

    DEPS="FLAC.$slibExt"
    DEPS="${DEPS} ogg.$slibExt"
    DEPS="${DEPS} vorbis.$slibExt"
    DEPS="${DEPS} vorbisfile.$slibExt"
    DEPS="${DEPS} mad.$slibExt"
    DEPS="${DEPS} SDL2.$slibExt"
    DEPS="${DEPS} freetype.$slibExt"
    DEPS="${DEPS} sqlite3.$slibExt"
    for lib in $DEPS
    do
        if [[ $1 == "test" ]]
        then
            echo "Checking library ${libPref}${lib}..."
        fi
        if [[ ! -f ${libsDir}/lib/${libPref}${lib} ]]
        then
            lackOfDependency
        fi
    done
}

# Check input arguments again
for var in "$@"
do
    case "$var" in
        isvalid)
            checkForDependencies "test"
            printf "=== \E[37;42mOK!\E[0m ===\n\n"
            exit 0
            ;;
        cool)
            printLine "Yeah!"                   "\E[0;42;37m" "\E[0;34m"
            printLine "This must be cool!"      "\E[0;42;36m" "\E[0;35m"
            printLine "Really?"                 "\E[0;42;35m" "\E[0;31m"
            printLine "You are cool!?"          "\E[0;42;34m" "\E[0;32m"
            printLine "Yeah!"                   "\E[0;42;33m" "\E[0;36m"
            exit 0
            ;;
        lupdate)
            echo ""
            echo "Running translation refreshing...";

            printLine "Editor" "\E[0;42;37m" "\E[0;34m"
            ${QT_PATH}/lupdate Editor/pge_editor.pro

            printLine "Engine" "\E[0;42;37m" "\E[0;34m"
            ${QT_PATH}/lupdate Engine/pge_engine.pro

            printLine "Done!" "\E[0;42;37m" "\E[0;32m"
            exit 0;
            ;;
        lrelease)
            echo ""
            echo "Running translation compilation...";

            printLine "Editor" "\E[0;42;37m" "\E[0;34m"
            ${QT_PATH}/$LRelease Editor/pge_editor.pro

            printLine "Engine" "\E[0;42;37m" "\E[0;34m"
            ${QT_PATH}/$LRelease Engine/pge_engine.pro

            printLine "Done!" "\E[0;42;37m" "\E[0;32m"
            exit 0;
            ;;
    esac
done

# Validate built dependencies!
checkForDependencies

#=======================================================================
# build translations of the editor
#cd Editor
#$LRelease *.pro
#checkState
#cd ../Engine
#$LRelease -idbased *.pro
#checkState
#cd ..
#=======================================================================
# build all components
echo "Running $QMake..."
if [[ "$OSTYPE" == "linux-gnu" || "$OSTYPE" == "linux" ]]; then
    $QMake CONFIG+=release CONFIG-=debug QTPLUGIN.platforms=qxcb QMAKE_TARGET.arch=$(uname -m) $QMAKE_EXTRA_ARGS
else
    $QMake CONFIG+=release CONFIG-=debug $QMAKE_EXTRA_ARGS
fi
checkState

#=======================================================================
echo "Building..."
TIME_STARTED=$(date +%s)
make $MAKE_EXTRA_ARGS
checkState
TIME_ENDED=$(date +%s)
TIME_PASSED=$(($TIME_ENDED-$TIME_STARTED))
#=======================================================================
# copy data and configs into the build directory
echo "Installing..."
make -s install
checkState

#=======================================================================
echo ""

show_time $TIME_PASSED
printLine "BUILT!" "\E[0;42;37m" "\E[0;32m"

cd $bak
if $flag_pause_on_end ; then
    pause
fi
exit 0
