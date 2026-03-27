"""Convert WAV to NES DPCM format."""
import wave
import struct
import sys

INPUT = "sound/superhot.wav"
OUTPUT = "sound/superhot.dmc"
TARGET_RATE = 8363  # NES DPCM rate index 8 (~8.4kHz)

def wav_to_samples(path):
    """Read WAV and return mono 8-bit samples."""
    with wave.open(path, 'rb') as w:
        nch = w.getnchannels()
        sw = w.getsampwidth()
        rate = w.getframerate()
        nframes = w.getnframes()
        raw = w.readframes(nframes)
    
    # Convert to list of float samples -1..1
    samples = []
    for i in range(nframes):
        offset = i * nch * sw
        if sw == 2:
            val = struct.unpack_from('<h', raw, offset)[0] / 32768.0
        elif sw == 1:
            val = (raw[offset] - 128) / 128.0
        else:
            val = 0
        samples.append(val)
    
    # Resample to target rate
    ratio = TARGET_RATE / rate
    new_len = int(len(samples) * ratio)
    resampled = []
    for i in range(new_len):
        src = i / ratio
        idx = int(src)
        if idx >= len(samples) - 1:
            idx = len(samples) - 2
        frac = src - idx
        val = samples[idx] * (1 - frac) + samples[idx + 1] * frac
        resampled.append(val)
    
    # Normalize
    peak = max(abs(v) for v in resampled) or 1
    resampled = [v / peak for v in resampled]
    
    # Convert to 0-127 range (NES DPCM uses 7-bit output, 0-127)
    return [int((v + 1) * 63.5) for v in resampled]

def encode_dpcm(samples):
    """Encode samples as NES 1-bit delta PCM."""
    output = []
    level = 64  # Start at midpoint
    byte = 0
    bit = 0
    
    for s in samples:
        if s > level:
            byte |= (1 << bit)
            level += 2  # NES DPCM step size is always 2
            if level > 127:
                level = 127
        else:
            # bit stays 0
            level -= 2
            if level < 0:
                level = 0
        
        bit += 1
        if bit == 8:
            output.append(byte)
            byte = 0
            bit = 0
    
    # Pad last byte
    if bit > 0:
        output.append(byte)
    
    # DPCM length must be (n * 16) + 1 bytes
    # Pad to next valid length
    while (len(output) - 1) % 16 != 0:
        output.append(0)
    
    return bytes(output)

samples = wav_to_samples(INPUT)
print(f"Resampled: {len(samples)} samples at {TARGET_RATE}Hz")
print(f"Duration: {len(samples)/TARGET_RATE:.2f}s")

dpcm = encode_dpcm(samples)
print(f"DPCM size: {len(dpcm)} bytes")

with open(OUTPUT, 'wb') as f:
    f.write(dpcm)

print(f"Written to {OUTPUT}")
