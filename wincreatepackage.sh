# this script is designed to be called by cmake in an msys2 mingw64 environment
# $1 should be the package output directory


echo "win Create Package"
mkdir -p $1/lib
cp -r $MSYSTEM_PREFIX/lib/gdk-pixbuf-2.0 $1/lib/
mkdir -p $1/share
mkdir -p $1/share/glib-2.0
cp -r $MSYSTEM_PREFIX/share/icons $1/share/
cp -r $MSYSTEM_PREFIX/share/glib-2.0/schemas $1/share/glib-2.0/
