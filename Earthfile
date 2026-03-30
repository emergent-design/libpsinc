VERSION 0.8

jammy:
	FROM ubuntu:22.04

noble:
	FROM ubuntu:24.04

trixie:
	FROM debian:trixie

image:
	ARG DISTRIBUTION=noble

	FROM +$DISTRIBUTION
	ENV DEBIAN_FRONTEND noninteractive
	ENV DEBCONF_NONINTERACTIVE_SEEN true
	WORKDIR /code

	RUN apt-get -q update && apt-get install -y --no-install-recommends ca-certificates curl build-essential cmake clang fakeroot chrpath dh-exec

	RUN curl -so /usr/share/keyrings/emergent.gpg https://apt.emergent-design.co.uk/emergent.gpg \
		&& echo "deb [signed-by=/usr/share/keyrings/emergent.gpg] https://apt.emergent-design.co.uk/public $DISTRIBUTION main" > /etc/apt/sources.list.d/emergent.list

	RUN apt-get -q update && apt-get install -y --no-install-recommends libfreeimage-dev libusb-1.0-0-dev libtbb-dev libemergent-dev

build:
	FROM +image
	COPY --dir include packages src CMakeLists.txt .
	RUN cmake -B build \
		&& make -j8 -C build

package:
	ARG DISTRIBUTION=noble
	FROM +build
	RUN cd packages && dpkg-buildpackage -b -uc -us
	SAVE ARTIFACT --keep-ts libpsinc*.deb AS LOCAL build/$DISTRIBUTION/

all-dists:
	BUILD +package --DISTRIBUTION=noble --DISTRIBUTION=trixie

check:
	BUILD +build

psinc-all:
	BUILD --platform=linux/amd64 --platform=linux/arm64 +all-dists

appimage:
	FROM +build --DISTRIBUTION=noble
	RUN apt-get update && apt-get install -y --no-install-recommends qt6-base-dev

	COPY --dir iconograph/include iconograph/resources iconograph/src iconograph/ui iconograph/CMakeLists.txt ./iconograph

	RUN cd iconograph \
		&& cmake -B build \
		&& make -j8 -C build

	RUN mkdir -p packages/appdir/usr/bin packages/appdir/usr/lib \
		&& cp iconograph/build/iconograph packages/appdir/usr/bin/ \
		&& cp build/libpsinc.so.0 packages/appdir/usr/lib/libpsinc.so.0

	RUN cd packages \
		&& curl -sJL "https://github.com/probonopd/go-appimage/releases/download/continuous/appimagetool-940-x86_64.AppImage" -o appimagetool.AppImage \
		&& chmod a+x appimagetool.AppImage \
		&& ./appimagetool.AppImage --appimage-extract \
		&& ARCH=x86_64 ./squashfs-root/usr/bin/appimagetool -s deploy appdir/usr/share/applications/iconograph.desktop \
		&& ARCH=x86_64 VERSION=$(cat version) ./squashfs-root/usr/bin/appimagetool ./appdir

	SAVE ARTIFACT --keep-ts packages/Iconograph*.AppImage AS LOCAL build/

# The following will be broken since the switch to cmake - to be addressed at a later date
windows:
	# ARG EMERGENT=0.0.39
	ARG EMERGENT=0.1.3
	ARG PREMAKE=5.0.0-alpha16
	# FROM DOCKERFILE packages/
	FROM teadriven/essential-qt-mingw:5.15
	WORKDIR /code

	RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates curl p7zip-full unzip

	# Install libusb
	RUN mkdir libusb && cd libusb \
		&& curl -Ls https://github.com/libusb/libusb/releases/download/v1.0.23/libusb-1.0.23.7z --output libusb.7z \
		&& p7zip -d libusb.7z \
		&& cp -r include/libusb-1.0 /usr/x86_64-w64-mingw32/include/ \
		&& cp MinGW64/dll/libusb-1.0.dll /usr/x86_64-w64-mingw32/lib/ \
		&& cd .. && rm -rf libusb

	# Install freeimage
	RUN curl -Ls https://downloads.sourceforge.net/project/freeimage/Binary%20Distribution/3.18.0/FreeImage3180Win32Win64.zip --output FreeImage.zip \
		&& unzip FreeImage.zip \
		&& cp FreeImage/Dist/x64/FreeImage.h /usr/x86_64-w64-mingw32/include/ \
		&& cp FreeImage/Dist/x64/FreeImage.dll /usr/x86_64-w64-mingw32/lib/freeimage.dll \
		&& rm -rf FreeImage && rm FreeImage.zip

	# Install other dependencies
	RUN curl -Ls -o premake.deb https://github.com/emergent-design/premake-pkg/releases/download/v$PREMAKE/premake_$PREMAKE-0ubuntu1_amd64.deb \
		&& dpkg -i premake.deb

	RUN curl -Ls -o libemergent-dev.deb https://github.com/emergent-design/libemergent/releases/download/v$EMERGENT/libemergent-dev_${EMERGENT}_all.deb \
		&& dpkg -i libemergent-dev.deb \
		&& ln -s /usr/include/emergent /usr/x86_64-w64-mingw32/include/

	# Build lib
	COPY --dir include packages src iconograph premake5.lua .
	RUN premake5 --os=windows gmake && make CXX=x86_64-w64-mingw32-g++ -j$(nproc)

	# Build iconograph
	RUN cd iconograph \
		&& PATH=/opt/qt5-win-x64/bin:$PATH qmake -spec win32-g++ iconograph.pro \
		&& make -j$(nproc)
	# RUN make -f iconograph.make -j$(nproc)
	RUN cd packages && ./windows
	SAVE ARTIFACT --keep-ts packages/*.7z AS LOCAL build/windows/
