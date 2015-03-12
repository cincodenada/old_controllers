BUILD_DIR=`ls -dtr /tmp/build* | tail -n1`
BUILD_FILE=`ls -dtr $BUILD_DIR/*.cpp.hex | tail -n1 | perl -n -e'/\/([^\/]*)\.cpp\.hex/ && print $1'`

echo "Running make_asm for $BUILD_FILE in $BUILD_DIR"

sh make_asm.sh $BUILD_DIR $BUILD_FILE
