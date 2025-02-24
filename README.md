## WebAssembly FLACEncoder

Extract PCM from WebAudio API (decodeAudioData) and generate 16kHz 16bit Monoral FLAC.

I decided to develop this because [mmig/libflac.js](https://github.com/mmig/libflac.js/tree/master) does not generate a seek table when encoding.

TestPage: https://otmb.github.io/FLACEncoder/

## Install & Build

podman is required for execution.
If you use docker, please rewrite the Makefile.

```
// Install
$ make docker-build

// Build
$ make wasm-build
```

## References

- [xiph/flac](https://github.com/xiph/flac)

