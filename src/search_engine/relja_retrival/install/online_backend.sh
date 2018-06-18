##### Boost
# Don't need to install -dev versions to run precompiled binaries, however
# it seems that ubuntu doesn't have default packages such as libboost-thread
# but only e.g. libboost-thread1.55.0 so it is not possible to automatically
# install. One option would be a regexpression, but that would fail as ubuntu
# for some reason often comes with two different versions of boost which
# then creates hell. So installing -dev is the only possible automatic option,
# otherwise one needs to manually see which version to install.

sudo apt-get install \
    libboost-thread-dev \
    libboost-system-dev \
    libboost-random-dev \
    libboost-program-options-dev \
    libboost-filesystem-dev \
    libboost-date-time-dev

##### libmagick++
# Like boost, it's just simpler to install the -dev version
sudo apt-get install \
    libmagick++-dev

##### libprotobuf-dev
sudo apt-get install \
    libprotobuf-dev

##### FASTANN
echo "Now install FASTANN"
