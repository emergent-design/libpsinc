VERSION 0.6

bionic:
	FROM ubuntu:18.04

focal:
	FROM ubuntu:20.04

jammy:
	FROM ubuntu:22.04

image:
	ARG TARGETARCH
	ARG DISTRIBUTION=bionic
	ARG PREMAKE=5.0.0-alpha16

	FROM +$DISTRIBUTION
	ENV DEBIAN_FRONTEND noninteractive
	ENV DEBCONF_NONINTERACTIVE_SEEN true
	WORKDIR /code
	RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates curl build-essential clang fakeroot chrpath dh-exec
	RUN curl -Ls -o premake.deb https://github.com/emergent-design/premake-pkg/releases/download/v$PREMAKE/premake_$PREMAKE-0ubuntu1_$TARGETARCH.deb \
		&& dpkg -i premake.deb
	RUN apt-get install -y --no-install-recommends libfreeimage-dev libusb-1.0-0-dev

deps:
	ARG EMERGENT=0.0.39

	FROM +image
	RUN curl -Ls -o libemergent-dev.deb https://github.com/emergent-design/libemergent/releases/download/v$EMERGENT/libemergent-dev_${EMERGENT}_all.deb \
		&& dpkg -i libemergent-dev.deb

build:
	FROM +deps
	COPY --dir include packages src premake5.lua strip .
	RUN premake5 gmake && make -j$(nproc)

package:
	FROM +build
	ARG DISTRIBUTION=bionic
	RUN cd packages && dpkg-buildpackage -b -uc -us
	# SAVE ARTIFACT libpsinc-dev_*.deb libpsinc-dev.deb
	# SAVE ARTIFACT libpsinc0_*.deb libpsinc0.deb
	SAVE ARTIFACT libpsinc*.deb AS LOCAL build/

all:
	BUILD --platform=linux/amd64 --platform=linux/arm64 +package --DISTRIBUTION=bionic
	BUILD --platform=linux/amd64 --platform=linux/arm64 +package --DISTRIBUTION=focal
	BUILD --platform=linux/amd64 --platform=linux/arm64 +package --DISTRIBUTION=jammy


appimage:
	FROM --build-arg DISTRIBUTION=bionic +deps
	RUN apt-get update && apt-get install -y --no-install-recommends qtbase5-dev qt5-default libqt5serialport5-dev file
	COPY --dir include packages src premake5.lua strip resources ui iconograph.pro .
	RUN premake5 gmake && make -j$(nproc)
	RUN qmake -o iconograph.make CONFIG+=release iconograph.pro \
		&& make -f iconograph.make -j$(nproc)
	RUN mkdir -p packages/appdir/usr/bin packages/appdir/usr/lib \
		&& cp bin/iconograph packages/appdir/usr/bin/ \
		&& cp lib/libpsinc.so packages/appdir/usr/lib/libpsinc.so.0
	RUN cd packages \
		&& curl -sJL "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" -o linuxdeployqt.AppImage \
		&& chmod a+x linuxdeployqt.AppImage \
		&& ./linuxdeployqt.AppImage --appimage-extract \
		&& unset QTDIR; unset QT_PLUGIN_PATH; unset LD_LIBRARY_PATH \
		&& squashfs-root/usr/bin/linuxdeployqt appdir/usr/share/applications/iconograph.desktop -bundle-non-qt-libs \
		&& squashfs-root/usr/bin/linuxdeployqt appdir/usr/share/applications/iconograph.desktop -appimage
	SAVE ARTIFACT packages/Iconograph*.AppImage AS LOCAL build/


mingw:
	FROM --build-arg DISTRIBUTION=jammy +deps
	RUN apt-get update && apt-get install -y --no-install-recommends mingw-w64 p7zip-full unzip git cmake
	RUN update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix \
   		&& update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix

   	# Build Qt
   	RUN git clone git://code.qt.io/qt/qt5.git && cd qt5 && git checkout 5.15 && ./init-repository --module-subset=essential
   	# Fix missing include
   	RUN sed -i "/^QT_BEGIN_NAMESPACE/i #include <limits>" qt5/qtbase/src/corelib/text/qbytearraymatcher.h

	RUN cd qt5 && ./configure -make libs -nomake examples -nomake tests -device-option \
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
		&& make -j"$(nproc)" && make install && cd .. && rm -rf qt5

	# Install libusb
	RUN mkdir libusb && cd libusb \
		&& curl -L https://github.com/libusb/libusb/releases/download/v1.0.23/libusb-1.0.23.7z --output libusb.7z \
		&& p7zip -d libusb.7z \
		&& cp -r include/libusb-1.0 /usr/x86_64-w64-mingw32/include/ \
		&& cp MinGW64/dll/libusb-1.0.dll /usr/x86_64-w64-mingw32/lib/ \
		&& cd .. && rm -rf libusb

	# Install freeimage
	RUN curl -L https://downloads.sourceforge.net/project/freeimage/Binary%20Distribution/3.18.0/FreeImage3180Win32Win64.zip --output FreeImage.zip \
		&& unzip FreeImage.zip \
		&& cp FreeImage/Dist/x64/FreeImage.h /usr/x86_64-w64-mingw32/include/ \
		&& cp FreeImage/Dist/x64/FreeImage.dll /usr/x86_64-w64-mingw32/lib/freeimage.dll \
		&& rm -rf FreeImage && rm FreeImage.zip

windows:
	FROM +mingw
	# Dependencies
	RUN ln -s /usr/include/emergent /usr/x86_64-w64-mingw32/include/
	# Build lib
	COPY --dir include packages src resources ui iconograph.pro premake5.lua .
	RUN premake5 --os=windows gmake && make CXX=x86_64-w64-mingw32-g++ -j$(nproc)
	# Build iconograph
	RUN PATH=/opt/qt5-win-x64/bin:$PATH qmake -spec win32-g++ -o iconograph.make iconograph.pro
	RUN make -f iconograph.make -j$(nproc)
	RUN cd packages && ./windows
	SAVE ARTIFACT packages/*.7z AS LOCAL build/windows/