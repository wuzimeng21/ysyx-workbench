#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

// Audio stream buffer (matches NEMU autoconf.h)
#define SB_ADDR  0xa1200000
#define SB_SIZE  0x10000

static int audio_initialized = 0;

void __am_audio_init() {
  audio_initialized = 0;
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = SB_SIZE;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  // Write audio parameters to NEMU audio device registers
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, 1);
  audio_initialized = 1;
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  if (audio_initialized) {
    stat->count = inl(AUDIO_COUNT_ADDR);
  } else {
    stat->count = 0;
  }
}

static int sbuf_wr_pos = 0;  // write position in NEMU stream buffer

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  if (!audio_initialized || ctl->buf.start == NULL) return;

  int len = (uint8_t *)ctl->buf.end - (uint8_t *)ctl->buf.start;
  if (len <= 0) return;

  // Write to NEMU stream buffer at current position (ring buffer)
  uint8_t *sbuf = (uint8_t *)(uintptr_t)SB_ADDR;
  for (int i = 0; i < len; i++) {
    sbuf[sbuf_wr_pos] = ((uint8_t *)ctl->buf.start)[i];
    sbuf_wr_pos = (sbuf_wr_pos + 1) % SB_SIZE;
  }

  // Notify NEMU audio handler with byte count
  outl(AUDIO_COUNT_ADDR, len);
}
