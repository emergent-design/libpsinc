# teadriven/essential-qt-mingw

FROM ubuntu:22.04

RUN apt-get update \
	&& apt-get install -y --no-install-recommends build-essential mingw-w64 git cmake \
	&& rm -rf /var/lib/apt/lists/*

RUN update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix \
	&& update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix

# Build Qt (fixes a missing include)
RUN git clone git://code.qt.io/qt/qt5.git \
	&& cd qt5 && git checkout 5.15 \
	&& ./init-repository --module-subset=essential \
	&& sed -i "/^QT_BEGIN_NAMESPACE/i #include <limits>" qtbase/src/corelib/text/qbytearraymatcher.h \
	&& ./configure -make libs -nomake examples -nomake tests -device-option \
		CROSS_COMPILE=x86_64-w64-mingw32- -xplatform win32-g++ \
		-qt-zlib -qt-libpng -qt-freetype -qt-harfbuzz -qt-pcre \
		-no-glib -no-icu -no-iconv -no-dbus -no-opengl \
		-skip qt3d -skip qtactiveqt -skip qtandroidextras -skip qtcanvas3d -skip qtconnectivity -skip qtdeclarative \
		-skip qtdoc -skip qtdocgallery -skip qtfeedback -skip qtgraphicaleffects -skip qtlocation -skip qtmacextras \
		-skip qtmultimedia -skip qtpim -skip qtpurchasing -skip qtqa -skip qtquickcontrols -skip qtquickcontrols2 \
		-skip qtrepotools -skip qtscript -skip qtsensors -skip qtserialbus -skip qtsvg -skip qtsystems -skip qttools \
		-skip qttranslations -skip qtwayland  -skip qtwebchannel -skip qtwebengine -skip qtwebsockets -skip qtwebview \
		-skip qtwinextras -skip qtx11extras -skip qtxmlpatterns \
		-opensource -confirm-license -release -prefix /opt/qt5-win-x64 \
	&& make -j"$(nproc)" && make install \
	&& cd .. && rm -rf qt5
