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

	RUN apt-get -q update && apt-get install -y --no-install-recommends libfreeimage-dev libusb-1.0-0-dev libtbb-dev libemergent-dev libentity-dev

build:
	FROM +image
	COPY --dir include packages src CMakeLists.txt .
	RUN cmake -B build \
		&& make -j8 -C build psinc

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

flasc:
	FROM +build --DISTRIBUTION=noble
	RUN make -j8 -C build flasc
	COPY packages/flasc.png .
	RUN curl -sJL "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" -o linuxdeploy.AppImage \
		&& chmod a+x linuxdeploy.AppImage \
		&& APPIMAGE_EXTRACT_AND_RUN=1 ./linuxdeploy.AppImage -e build/flasc --appdir appdir --create-desktop-file -i flasc.png --output appimage
	SAVE ARTIFACT flasc*.AppImage AS LOCAL build/

windows:
	FROM teadriven/essential-qt-mingw:6.11-2
	# FROM DOCKERFILE packages/docker-qt6/

	WORKDIR /code

	RUN curl -so /usr/share/keyrings/emergent.gpg https://apt.emergent-design.co.uk/emergent.gpg \
		&& echo "deb [signed-by=/usr/share/keyrings/emergent.gpg] https://apt.emergent-design.co.uk/public noble main" > /etc/apt/sources.list.d/emergent.list \
		&& apt-get -q update \
		&& apt-get install -y --no-install-recommends libemergent-dev libentity-dev p7zip-full unzip \
		&& ln -s /usr/include/emergent /usr/x86_64-w64-mingw32/include/ \
		&& ln -s /usr/include/entity /usr/x86_64-w64-mingw32/include/

	# Install libusb
	RUN mkdir libusb && cd libusb \
		&& curl -Ls https://github.com/libusb/libusb/releases/download/v1.0.29/libusb-1.0.29.7z --output libusb.7z \
		&& p7zip -d libusb.7z \
		&& mkdir -p /usr/x86_64-w64-mingw32/include/libusb-1.0 \
		&& cp -r include/libusb.h /usr/x86_64-w64-mingw32/include/libusb-1.0/ \
		&& cp MinGW64/dll/libusb-1.0.dll /usr/x86_64-w64-mingw32/lib/ \
		&& cd .. && rm -rf libusb

	# Install freeimage
	RUN curl -Ls https://downloads.sourceforge.net/project/freeimage/Binary%20Distribution/3.18.0/FreeImage3180Win32Win64.zip --output FreeImage.zip \
		&& unzip FreeImage.zip \
		&& cp FreeImage/Dist/x64/FreeImage.h /usr/x86_64-w64-mingw32/include/ \
		&& cp FreeImage/Dist/x64/FreeImage.dll /usr/x86_64-w64-mingw32/lib/freeimage.dll \
		&& rm -rf FreeImage && rm FreeImage.zip


	COPY --dir packages include src CMakeLists.txt .
	COPY --dir iconograph/include iconograph/resources iconograph/src iconograph/ui iconograph/CMakeLists.txt ./iconograph

	RUN cmake -DCMAKE_TOOLCHAIN_FILE=/opt/qt6-host/toolchain-mingw.cmake -B build \
		&& make -j8 -C build

	RUN cd iconograph \
		&& cmake -DCMAKE_TOOLCHAIN_FILE=/opt/qt6-host/toolchain-mingw.cmake -DQT_HOST_PATH=/opt/qt6-host -DQt6_ROOT=/usr/local/Qt-6.11.0 -B build \
		&& make -j8 -C build

	RUN cd packages && ./windows
	SAVE ARTIFACT --keep-ts packages/*.7z AS LOCAL build/windows/

# -DQt6_ROOT=/usr/local/Qt-6.11.0
