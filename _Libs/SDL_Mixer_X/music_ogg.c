/*
  SDL_mixer:  An audio mixer library based on the SDL library
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* $Id$ */

#ifdef OGG_MUSIC

/* This file supports Ogg Vorbis music streams */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL_mixer_ext.h"
#include "dynamic_ogg.h"
#include "music_ogg.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

/* This is the format of the audio mixer data */
static SDL_AudioSpec mixer;

/* Initialize the Ogg Vorbis player, with the given mixer settings
   This function returns 0, or -1 if there was an error.
 */
int OGG_init(SDL_AudioSpec *mixerfmt)
{
    mixer = *mixerfmt;
    return(0);
}

/* Set the volume for an OGG stream */
void OGG_setvolume(OGG_music *music, int volume)
{
    music->volume = volume;
}

static size_t sdl_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    return SDL_RWread((SDL_RWops *)datasource, ptr, size, nmemb);
}

static int sdl_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
    return (int)SDL_RWseek((SDL_RWops *)datasource, offset, whence);
}

static long sdl_tell_func(void *datasource)
{
    return (long)SDL_RWtell((SDL_RWops *)datasource);
}

/* Load an OGG stream from an SDL_RWops object */
OGG_music *OGG_new_RW(SDL_RWops *src, int freesrc)
{
    OGG_music *music;
    ov_callbacks callbacks;

    if(!Mix_Init(MIX_INIT_OGG))
        return(NULL);

    SDL_memset(&callbacks, 0, sizeof(callbacks));
    callbacks.read_func = sdl_read_func;
    callbacks.seek_func = sdl_seek_func;
    callbacks.tell_func = sdl_tell_func;

    music = (OGG_music *)SDL_malloc(sizeof * music);
    if(music)
    {
        vorbis_info *vi;
        vorbis_comment *ptr;
        int isLength = 0;
        int i;
        ogg_int64_t total;

        /* Initialize the music structure */
        SDL_memset(music, 0, (sizeof * music));
        music->src = src;
        music->freesrc = freesrc;
        OGG_stop(music);
        OGG_setvolume(music, MIX_MAX_VOLUME);
        music->section = -1;
        music->channels = 0;

        music->mus_title = NULL;
        music->mus_artist = NULL;
        music->mus_album = NULL;
        music->mus_copyright = NULL;

        MyResample_zero(&music->resample);

        music->loop         = -1;
        music->loop_start   = -1;
        music->loop_end     =  0;
        music->loop_len     =  0;

        if(vorbis.ov_open_callbacks(src, &music->vf, NULL, 0, callbacks) < 0)
        {
            SDL_SetError("Not an Ogg Vorbis audio stream");
            SDL_free(music);
            return(NULL);
        }

        vi = vorbis.ov_info(&music->vf, -1);
        music->channels = vi->channels;

        /* Parse comments and extract title and loop points */
        ptr = ov_comment(&music->vf, -1);

        for(i = 0; i < ptr->comments; i++)
        {
            int   paramLen = ptr->comment_lengths[i] + 1;
            char *param = (char *)SDL_malloc(paramLen);
            char *argument  = param;
            char *value     = param;
            memset(param, 0, paramLen);
            memcpy(param, ptr->user_comments[i], ptr->comment_lengths[i]);
            value = strchr(param, '=');
            if(value == NULL)
            {
                value = param + paramLen - 1; /* set null */
            } else {
                *(value++) = '\0';
            }

            #ifdef __USE_ISOC99
#define A_TO_OGG64(x) (ogg_int64_t)atoll(x)
            #else
#define A_TO_OGG64(x) (ogg_int64_t)atol(x)
            #endif

            if(strcasecmp(argument, "LOOPSTART") == 0)
                music->loop_start = A_TO_OGG64(value);
            else if(strcasecmp(argument, "LOOPLENGTH") == 0)
            {
                music->loop_len = A_TO_OGG64(value);
                isLength = 1;
            }
            else if(strcasecmp(argument, "LOOPEND") == 0)
            {
                isLength = 0;
                music->loop_end = A_TO_OGG64(value);
            }
            else if(strcasecmp(argument, "TITLE") == 0)
            {
                music->mus_title = (char *)SDL_malloc(sizeof(char) * strlen(value) + 1);
                strcpy(music->mus_title, value);
            }
            else if(strcasecmp(argument, "ARTIST") == 0)
            {
                music->mus_artist = (char *)SDL_malloc(sizeof(char) * strlen(value) + 1);
                strcpy(music->mus_artist, value);
            }
            else if(strcasecmp(argument, "ALBUM") == 0)
            {
                music->mus_album = (char *)SDL_malloc(sizeof(char) * strlen(value) + 1);
                strcpy(music->mus_album, value);
            }
            else if(strcasecmp(argument, "COPYRIGHT") == 0)
            {
                music->mus_copyright = (char *)SDL_malloc(sizeof(char) * strlen(value) + 1);
                strcpy(music->mus_copyright, value);
            }

            SDL_free(param);
        }

#undef A_TO_OGG64

        if(isLength == 1)
            music->loop_end = music->loop_start + music->loop_len;
        else
            music->loop_len = music->loop_end - music->loop_start;

        total = ov_pcm_total(&music->vf, -1);
        if(((music->loop_start >= 0) || (music->loop_end > 0)) &&
           ((music->loop_start < music->loop_end) || (music->loop_end == 0)) &&
           (music->loop_start < total) &&
           (music->loop_end <= total))
        {
            if(music->loop_start < 0)
                music->loop_start = 0;
            if(music->loop_end == 0)
                music->loop_end = total;
            music->loop = 1;
            music->loop_len_ch = music->channels;
        }
    }
    else
    {
        SDL_OutOfMemory();
        return(NULL);
    }

    return (music);
}

/* Ignore loop points if found */
void OGG_IgnoreLoop(OGG_music *music)
{
    if(music) music->loop = -1;
}


/* Start playback of a given OGG stream */
void OGG_play(OGG_music *music)
{
    music->playing = 1;
}

/* Return non-zero if a stream is currently playing */
int OGG_playing(OGG_music *music)
{
    return(music->playing);
}

/* Read some Ogg stream data and convert it for output */
static void OGG_getsome(OGG_music *music)
{
    int section;

    long len;
    Uint8 data[4096];
    SDL_AudioCVT *cvt;
    struct MyResampler *res;
    ogg_int64_t pcmpos;
    SDL_zero(data);

    len = sizeof(data);
    /* Align length to the channels position */
    len -= len % (music->channels * sizeof(Uint16));

    #ifdef OGG_USE_TREMOR
    len = vorbis.ov_read(&music->vf, data, sizeof(data), &section);
    #else
    len = vorbis.ov_read(&music->vf, (char *)data, len, 0, 2, 1, &section);
    #endif
    pcmpos = ov_pcm_tell(&music->vf);
    if((music->loop == 1) && (pcmpos >= music->loop_end))
    {
        len -= ((pcmpos - music->loop_end) * music->loop_len_ch) * sizeof(Uint16);
        ov_pcm_seek(&music->vf, music->loop_start);
    }

    if(len <= 0)
    {
        if(len == 0)
            music->playing = 0;
        return;
    }
    cvt = &music->cvt;
    res = &music->resample;
    if(section != music->section)
    {
        vorbis_info *vi;
        vi = vorbis.ov_info(&music->vf, -1);

        MyResample_init(res,
                        (int)vi->rate,
                        mixer.freq,
                        vi->channels,
                        AUDIO_S16);
        SDL_BuildAudioCVT(cvt,
                          AUDIO_S16,
                          (Uint8)vi->channels,
                          mixer.freq/*vi->rate HACK: Don't use SDL's resamplers*/,
                          mixer.format,
                          mixer.channels,
                          mixer.freq);
        if(cvt->buf)
            SDL_free(cvt->buf);
        cvt->buf = (Uint8 *)SDL_malloc(sizeof(data) * (size_t)cvt->len_mult * (size_t)res->len_mult);
        music->section = section;
    }

    if(cvt->buf)
    {
        if(res->needed)
        {
            MyResample_addSource(res, data, (int)len);
            MyResample_Process(res);
            SDL_memcpy(cvt->buf, res->buf, (size_t)res->buf_len);
            cvt->len = res->buf_len;
            cvt->len_cvt = res->buf_len;
        }
        else
        {
            cvt->len = (int)len;
            cvt->len_cvt = (int)len;
            SDL_memcpy(cvt->buf, data, (size_t)len);
        }
        if(cvt->needed)
            SDL_ConvertAudio(cvt);
        music->len_available = music->cvt.len_cvt;
        music->snd_available = music->cvt.buf;
    }
    else
    {
        SDL_SetError("Out of memory");
        music->playing = 0;
    }
}

/* Play some of a stream previously started with OGG_play() */
int OGG_playAudio(OGG_music *music, Uint8 *snd, int len)
{
    int mixable;

    while((len > 0) && music->playing)
    {
        if(!music->len_available)
            OGG_getsome(music);
        mixable = len;
        if(mixable > music->len_available)
            mixable = music->len_available;
        if(music->volume == MIX_MAX_VOLUME)
            SDL_memcpy(snd, music->snd_available, (size_t)mixable);
        else
        {
            SDL_MixAudioFormat(snd, music->snd_available, mixer.format,
                               (Uint32)mixable, music->volume);
        }
        music->len_available -= mixable;
        music->snd_available += mixable;
        len -= mixable;
        snd += mixable;
    }

    return len;
}

/* Stop playback of a stream previously started with OGG_play() */
void OGG_stop(OGG_music *music)
{
    music->playing = 0;
}

/* Close the given OGG stream */
void OGG_delete(OGG_music *music)
{
    if(music)
    {
        if(music->cvt.buf)
            SDL_free(music->cvt.buf);
        if(music->freesrc)
            SDL_RWclose(music->src);
        if(music->mus_title)
            SDL_free(music->mus_title);
        if(music->mus_artist)
            SDL_free(music->mus_artist);
        if(music->mus_album)
            SDL_free(music->mus_album);
        if(music->mus_copyright)
            SDL_free(music->mus_copyright);
        vorbis.ov_clear(&music->vf);
        SDL_free(music);
    }
}

/* Jump (seek) to a given position (time is in seconds) */
void OGG_jump_to_time(OGG_music *music, double time)
{
    #ifdef OGG_USE_TREMOR
    vorbis.ov_time_seek(&music->vf, (ogg_int64_t)(time * 1000.0));
    #else
    vorbis.ov_time_seek(&music->vf, time);
    #endif
}

#endif /* OGG_MUSIC */
