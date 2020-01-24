project_name="dynamic_prem"
build_name="rt_druid_gh65"

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
echo -n "Your username will be $USER. Please enter a password: "
read -s PASS
# downloaded
#ECLIPSE_TOOLCHAIN_ARCHIVE=eclipse-rtdruid3-photon-linux-gtk-x86_64_20181217_gh55.tar.gz
ECLIPSE_TOOLCHAIN_ARCHIVE=eclipse-rtdruid3-photon-linux-gtk-x86_64_20190524_gh65.tar.gz
echo
cd $parent_path
docker build --build-arg USERNAME=$USER --build-arg PASSWORD=$PASS --build-arg ECLIPSE_TOOLCHAIN_ARCHIVE=$ECLIPSE_TOOLCHAIN_ARCHIVE -t $project_name/$build_name:latest .
