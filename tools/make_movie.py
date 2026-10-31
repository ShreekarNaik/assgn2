"""Encode captured frames with an original synthesized soundtrack. Run at repo root."""
import math
from pathlib import Path
import shutil
import struct
import subprocess
import wave

frames = sorted(Path('frames').glob('frame-*.ppm'))
if len(frames) < 1800:
    raise SystemExit('Capture the full 60-second animation first: run ./a2-animate-minifig and press L, then R')
if [p.name for p in frames] != [f'frame-{i:05d}.ppm' for i in range(len(frames))]:
    raise SystemExit('Missing or misnumbered frames')
if not shutil.which('ffmpeg'):
    raise SystemExit('Install ffmpeg first')
Path('output').mkdir(exist_ok=True)
rate = 22050
duration = len(frames) / 30
# Original simple chord progression; no sampled or copyrighted recordings.
with wave.open('output/soundtrack.wav', 'wb') as out:
    out.setparams((1, 2, rate, 0, 'NONE', 'not compressed'))
    chords = [(130.81,164.81,196), (110,130.81,164.81),
              (87.31,110,130.81), (98,123.47,146.83)]
    for second in range(math.ceil(duration)):
        data = bytearray()
        for j in range(rate):
            t = second + j/rate
            if t >= duration:
                break
            chord = chords[int(t/4) % 4]
            pulse = (t % .5)/.5
            envelope = math.sin(math.pi*pulse)**2
            volume = .06 if t < 14 or t > 58 else .12
            fade = max(0, min(1, t, duration-t))
            sample = volume*fade*sum(math.sin(2*math.pi*f*t) for f in chord)/3
            sample += .04*fade*envelope*math.sin(2*math.pi*chord[int(t*2)%3]*2*t)
            data += struct.pack('<h', int(sample*32767))
        out.writeframes(data)
subprocess.run(['ffmpeg','-hide_banner','-loglevel','warning','-n','-framerate','30','-i','frames/frame-%05d.ppm',
                '-i','output/soundtrack.wav','-c:v','libx264','-crf','18',
                '-pix_fmt','yuv420p','-c:a','aac','-shortest','output/one-last-cup.mp4'], check=True)
Path('screenshots').mkdir(exist_ok=True)
subprocess.run(['ffmpeg','-hide_banner','-loglevel','warning','-n','-i',str(frames[1050]),
                '-frames:v','1','-update','1','screenshots/movie.jpg'], check=True)
print('Ready: output/one-last-cup.mp4 and screenshots/movie.jpg')
