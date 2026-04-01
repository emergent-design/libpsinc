# teadriven/essential-qt-mingw

FROM ubuntu:24.04

RUN apt-get -q update \
	&& apt-get install -y --no-install-recommends ca-certificates curl build-essential cmake ninja-build git mingw-w64

RUN curl -Ls https://download.qt.io/official_releases/qt/6.11/6.11.0/submodules/qtbase-everywhere-src-6.11.0.tar.xz -o qt.xz \
	&& tar xf qt.xz

# build the host tools
RUN mkdir -p /opt/qt6-host && cd /opt/qt6-host \
	&& /qtbase-everywhere-src-6.11.0/configure -developer-build -nomake tests -no-opengl -release \
	&& cmake --build . --target host_tools

COPY toolchain-mingw.cmake /opt/qt6-host/

# RUN update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix \
# 	&& update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix

# build Qt for windows
RUN mkdir -p build && cd build \
	&& cmake  -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/opt/qt6-host/toolchain-mingw.cmake -DQT_HOST_PATH=/opt/qt6-host ../qtbase-everywhere-src-6.11.0/ \
	&& cat config.summary \
	&& cmake --build . --parallel 16 \
	&& cmake --install .

# cleanup
RUN apt-get clean \
	&& rm -rf /var/lib/apt/lists/* \
	&& rm -rf qtbase-everywhere-src-6.11.0 \
	&& rm -rf build \
	&& rm qt.xz
