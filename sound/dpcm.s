.segment "DPCM"

.export _dpcm_superhot
.export _dpcm_superhot_len

_dpcm_superhot:
    .incbin "sound/superhot.dmc"

_dpcm_superhot_len = * - _dpcm_superhot
