const fs = require('fs');
const Module = require('./wasm/encodeFlac.js');
// ffmpeg -i test.mp4 -vn -ac 2 -ar 16000 -f s16le test.pcm
const CHANNELS = 2, SAMPLE_RATE = 16000, BPS = 16, COMPRESSION = 5;

async function main(){
    const pcmBin = fs.readFileSync("test.pcm");
    Module.FS.writeFile("input.pcm", pcmBin);

    const samples = pcmBin.length;
    const ret = Module._encode(CHANNELS, SAMPLE_RATE, BPS, samples, COMPRESSION);
    if (ret == 0){
        const flacBin = Module.FS.readFile("output.flac");
        fs.writeFileSync("output.flac", flacBin);
        console.log("succeeded", flacBin.length);
    }
}
Module.onRuntimeInitialized = () => {
    main().catch(err => {
        console.error(err);
        process.exit(1);
    });
};
// Module.stderr = function(text) { /*console.log(text);*/ }