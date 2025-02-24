#include <emscripten.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "share/compat.h"
#include "FLAC/metadata.h"
#include "FLAC/stream_encoder.h"

static void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data);

#define READSIZE 1024

static unsigned total_samples = 0; /* can use a 32-bit number due to WAVE size limitations */
static char *input_file = "input.pcm";
static char *output_file = "output.flac";

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

EXTERN EMSCRIPTEN_KEEPALIVE
int encode(unsigned channels, unsigned sample_rate, unsigned bps,unsigned _total_samples, unsigned compression)
{
    FLAC__bool ok = true;
    FLAC__StreamEncoder *encoder = 0;
    FLAC__StreamEncoderInitStatus init_status;
    FLAC__StreamMetadata *metadata[2];
    FLAC__StreamMetadata_VorbisComment_Entry entry;

    if (channels == 1){
        total_samples = _total_samples / 2;
    } else if (channels == 2){
        total_samples = _total_samples / 4;
    } else {
        fprintf(stderr, "channel range [1-2]");
        return 1;
    }

    FLAC__byte buffer[READSIZE/*samples*/ * 2/*bytes_per_sample*/ * channels/*channels*/]; /* we read the WAVE data into here */
    FLAC__int32 pcm[READSIZE/*samples*/ * channels/*channels*/];
    FILE *fin;
    
    if((fin = fopen(input_file, "rb")) == NULL) {
        fprintf(stderr, "ERROR: opening %s for output\n", input_file);
        return 1;
    }

    /* allocate the encoder */
    if((encoder = FLAC__stream_encoder_new()) == NULL) {
        fprintf(stderr, "ERROR: allocating encoder\n");
        fclose(fin);
        return 1;
    }

    ok &= FLAC__stream_encoder_set_verify(encoder, true);
    ok &= FLAC__stream_encoder_set_compression_level(encoder, compression);
    ok &= FLAC__stream_encoder_set_channels(encoder, channels);
    ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bps);
    ok &= FLAC__stream_encoder_set_sample_rate(encoder, sample_rate);
    ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, total_samples);

    /* now add some metadata; we'll add some tags and a padding block */
    if(ok) {
        if(
            (metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) == NULL ||
            (metadata[1] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL ||
            /* there are many tag (vorbiscomment) functions but these are convenient for this particular use: */
            !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", "Some Artist") ||
            !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false) || /* copy=false: let metadata object take control of entry's allocated string */
            !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "YEAR", "1984") ||
            !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false)
        ) {
            fprintf(stderr, "ERROR: out of memory or tag error\n");
            ok = false;
        } else {

            metadata[1]->length = 1234; /* set the padding length */

            ok = FLAC__stream_encoder_set_metadata(encoder, metadata, 2);

        }
    }

    /* initialize encoder */
    if(ok) {
        init_status = FLAC__stream_encoder_init_file(encoder, output_file, progress_callback, /*client_data=*/NULL);
        if(init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
            fprintf(stderr, "ERROR: initializing encoder: %s\n", FLAC__StreamEncoderInitStatusString[init_status]);
            ok = false;
        }
    }

    /* read blocks of samples from WAVE file and feed to encoder */
    if(ok) {
        size_t left = (size_t)total_samples;
        while(ok && left) {
            size_t need = (left>READSIZE? (size_t)READSIZE : (size_t)left);
            
            if(fread(buffer, channels*(bps/8), need, fin) != need) {
                fprintf(stderr, "ERROR: reading from WAVE file\n");
                ok = false;
            }
            else {
                /* convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC */
                size_t i;
                for(i = 0; i < need*channels; i++) {
                    /* inefficient but simple and works on big- or little-endian machines */
                    pcm[i] = (FLAC__int32)(((FLAC__int16)(FLAC__int8)buffer[channels*i+1] << 8) | (FLAC__int16)buffer[channels*i]);                
                }
                /* feed samples to encoder */
                ok = FLAC__stream_encoder_process_interleaved(encoder, pcm, need);
            }
            left -= need;
        }
    }

    ok &= FLAC__stream_encoder_finish(encoder);

    fprintf(stderr, "encoding: %s\n", ok? "succeeded" : "FAILED");
    fprintf(stderr, "   state: %s\n", FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);

    /* now that encoding is finished, the metadata can be freed */
    FLAC__metadata_object_delete(metadata[0]);
    FLAC__metadata_object_delete(metadata[1]);

    FLAC__stream_encoder_delete(encoder);
    fclose(fin);

    return 0;
}

void progress_callback(const FLAC__StreamEncoder *encoder, FLAC__uint64 bytes_written, FLAC__uint64 samples_written, unsigned frames_written, unsigned total_frames_estimate, void *client_data)
{
    (void)encoder, (void)client_data;
    fprintf(stderr, "wrote %llu bytes, %llu/%u samples, %u/%u frames\n", bytes_written, samples_written, total_samples, frames_written, total_frames_estimate);
}