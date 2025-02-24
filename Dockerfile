FROM emscripten/emsdk
WORKDIR /

RUN wget https://github.com/xiph/flac/archive/refs/tags/1.5.0.tar.gz
RUN tar xvzf 1.5.0.tar.gz

RUN mkdir -p /flac/dist
WORKDIR /flac/dist

RUN embuilder build ogg
RUN emcmake cmake /flac-1.5.0 \
-DBUILD_CXXLIBS=0 \
-DBUILD_PROGRAMS=0 \
-DBUILD_EXAMPLES=0 \
-DBUILD_TESTING=0 \
-DBUILD_DOCS=0 \
-DWITH_FORTIFY_SOURCE=0 \
-DWITH_STACK_PROTECTOR=0 \
-DINSTALL_MANPAGES=0 \
-DINSTALL_PKGCONFIG_MODULES=0 \
-DINSTALL_CMAKE_CONFIG_MODULE=0 \
-DWITH_OGG=0 \
-DBUILD_SHARED_LIBS=1 \
-DENABLE_MULTITHREADING=0


RUN emmake make -j4 && emmake make install
WORKDIR /src
