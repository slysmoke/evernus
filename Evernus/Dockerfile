FROM krojew/ubuntu-clang

ARG EVERNUS_CLIENT_ID
ARG EVERNUS_CLIENT_SECRET

ARG EVERNUS_DROPBOX_APP_KEY
ARG EVERNUS_DROPBOX_APP_SECRET

RUN apt-get update && \
    apt-get install -y \
        mercurial \
        wget \
        libboost-all-dev \
        make \
        mdm \
        qt5-default \
        qtmultimedia5-dev \
        qtdeclarative5-dev \
        qtwebengine5-dev \
        qttools5-dev-tools \
        qtbase5-private-dev \
        libqt5xmlpatterns5-dev

# Data Visualization is missing in Ubuntu
RUN wget http://download.qt.io/official_releases/qt/5.9/5.9.1/submodules/qtdatavis3d-opensource-src-5.9.1.tar.xz && \
    tar -xf qtdatavis3d-opensource-src-5.9.1.tar.xz && \
    cd qtdatavis3d-opensource-src-5.9.1 && \
    qmake CONFIG+=release && \
    make -j`ncpus` && \
    make install && \
    cd .. && rm -rf qtdatavis3d-opensource-src-5.9.1

RUN cd && \
    wget https://cmake.org/files/v3.10/cmake-3.10.0-rc4.tar.gz && \
    tar xf cmake-3.10.0-rc4.tar.gz && \
    cd cmake-3.10.0-rc4 && \
    ./configure --prefix=/usr && \
    make -j`ncpus` && \
    make install && \
    cd && rm -rf cmake-3.10.0-rc4

# we don't want to use cache for the following layers, but docker cannot selectively disable it
ADD .cache-bust /root

RUN hg clone -u latest-release https://krojew@bitbucket.org/krojew/evernus

WORKDIR evernus
RUN cmake \
    -DEVERNUS_CLIENT_ID=$EVERNUS_CLIENT_ID -DEVERNUS_CLIENT_SECRET=$EVERNUS_CLIENT_SECRET \
    -DEVERNUS_DROPBOX_APP_KEY=$EVERNUS_DROPBOX_APP_KEY -DEVERNUS_DROPBOX_APP_SECRET=$EVERNUS_DROPBOX_APP_SECRET \
    -DEVERNUS_CREATE_DUMPS=OFF \
    -G 'Unix Makefiles' && \
    make -j`ncpus` && \
    make install && \
    cd .. && rm -rf evernus

ENTRYPOINT /usr/local/bin/evernus
