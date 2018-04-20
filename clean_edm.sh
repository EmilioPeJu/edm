#!/bin/bash
# Bash script to clean up a distributed edm tree
# 
# SYNOPSIS:
#     clean_edm.sh [-nocvs] path
#
# OPTIONS:
#     -nocvs clean the tree as files, not as a cvs tree
#     path   path to the edm distribution tree to clean (default .)

CopyCvsTree () {
# Routine to copy only the files checked out from CVS in one directory 
# tree to another directory tree
#    $1 - Root of source directory tree (must have CVS directories)
#    $2 - Root of destination directory tree

    for dir in `awk -F/ '/^[^\/]/{print $2}' $1/CVS/Entries*` ; do
	if [ -f $1/${dir}/CVS/Entries ] ; then
	    if [ ! -d $2/$dir ] ; then
		mkdir $2/$dir
	    fi
	    CopyCvsTree $1/$dir $2/$dir
        else
            echo $1/${dir}/CVS/Entries does not exist - not creating the new directory
        fi
    done

    for file in `awk -F/ '/^\//{print $2}' $1/CVS/Entries` ; do
        cp $1/$file $2/$file
    done
}

CvsMove () {
    mv $1 $2
    cvs rm $1
    cvs add $2
}

RenameLib () {
# Routine to rename all occurrance of a library name in one tree to another name
#    $1 - Path to root of tree
#    $2 - Old library name
#    $3 - New library name

    # Rename all the files which include the first name to the second
    for file in `find $1 -name .svn -prune -o -name CVS -prune -o -name "*${2}*" -print` ; do
	$MV $file `echo $file | sed s/$2/$3/g`
    done

    # Change all the occurences of the first library name to the second
    for file in `grep -rl $2 $1 | egrep -v 'CVS|\.svn|clean_edm.sh'`; do
	ed -s $file <<EOF
,s/$2/$3/g
w
EOF
    done
}

distribution=0;

case "$1" in
    -nocvs)
	shift
	RM='rm -f'
	MV='mv'
	RMDIR='rm -rf'
	;;
    -svn)
        shift
        RM='svn remove --force'
        MV='svn move --force'
	RMDIR='svn remove --force'
	;;
    -cvs)
	shift
        RM='cvs rm -f'
        MV='CvsMove'
        RMDIR='cvs rm -Rf'
	;;
    -dist)
	shift
        distribution=1;
	# Make the new directory
	if [ ! -d $2 ] ; then
	    mkdir $2
	fi

	CopyCvsTree $1 $2
	shift
	RM='rm -f'
	MV='mv'
        RMDIR='rm -rf'
	;;
    *)
        RM='cvs rm -f'
        MV='CvsMove'
        RMDIR='cvs rm -Rf'
	;;
esac

if [ $# -eq 0 ] ; then
   new=.
else
   new=$1
fi

# Remove files with strange names
$RM ${new}'/util/iprpc/osf/iptest/makefile;3'
$RM ${new}'/util/net/vms/.$fdl$knob.vdb'

# Clear the execution bit off of all files
find $new -name .svn -prune -o -name CVS -prune -o -type f -perm -111 -exec chmod 664 '{}' \;
if [ -f $new/clean_edm.sh ] ; then chmod 775 $new/clean_edm.sh; fi

if [ $distribution -eq 1 ] ; then
    exit;
fi

# Remove executables which are not text executables.
file `find $new -name .svn -prune -o -name CVS -prune -o -type f -print` | grep executable | grep -v 'text executable' | awk -F: '{print $1}' > /tmp/clean_edm_$$
$RM `cat /tmp/clean_edm_$$`
rm -f /tmp/clean_edm_$$

# Remove all the shodif files - these refer to a directory called util1, which isn't distributed
$RM `find $new -name .svn -prune -o -name CVS -prune -o -name shodif -print`

# Remove imagelib, moveLibs and userLib - these are not included in the build
# imagelib is duplicated in pnglib and giflib, and moveLibs and userLib etc seem to be dead ends
$RMDIR $new/imagelib $new/moveLibs $new/userLib $new/archivePlot $new/html ${new}/util/thread/vxworks/cur

# Change the names of the long library names to more recognisable ones.
RenameLib $new cf322683-513e-4570-a44b-7cdd7cae0de5 EdmGif
RenameLib $new 57d79238-2924-420b-ba67-dfbecdf03fcd EdmPng
RenameLib $new 3014b6ee-f439-11d2-ad99-00104b8742df EdmDoc
RenameLib $new 7e1b4e6f-239d-4650-95e6-a040d41ba633 EdmChoiceButton
RenameLib $new 114135a4-6f6c-11d3-95bc-00104b8742df EdmUtil
RenameLib $new cfcaa62e-8199-11d3-a77f-00104b8742df EdmLib

# Change all the occurances of LIBRARY to LIBRARY_HOST in Makefiles
for file in `grep -rl "LIBRARY *=" ${new}/*/Makefile`; do
ed -s $file <<EOF
,s/LIBRARY *=/LIBRARY_HOST =/g
w
EOF
done

# Change all the occurance of PROD to PROD_HOST in edmMain/Makefile
ed -s ${new}/edmMain/Makefile <<EOF
,s/PROD *=/PROD_HOST =/g
w
EOF

# Move edmObjects, edmPrintDef  and edmObjects from edmMain to setup
if [ -f $new/edmMain/edmObjects   ] ; then $MV $new/edmMain/edmObjects   $new/setup/edmObjects ; fi
if [ -f $new/edmMain/edmPvObjects ] ; then $MV $new/edmMain/edmPvObjects $new/setup/edmPvObjects ; fi
if [ -f $new/edmMain/edmPrintDef  ] ; then $MV $new/edmMain/edmPrintDef  $new/setup/edmPrintDef ; fi

# Remove duplicate copies of fonts.list and colors.list 
$RM $new/edmMain/fonts.list
$RM $new/util/graph/vms/fonts.list
$RM $new/edmMain/colors.list
$RM $new/edmMain/default.scheme
