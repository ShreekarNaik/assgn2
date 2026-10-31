"""Apply suggested TIMESTAMPS to S-recorded poses, leaving all values untouched.

The assignment permits editing timestamps after recording. Back up and inspect
manual pose edits as usual; this helper only saves retyping the timing column.
"""
from pathlib import Path
import shutil

plan = Path('authoring/story_notes.txt').read_text().splitlines()
recording = Path('keyframes.txt')
rows = recording.read_text().splitlines()
if len(rows) != len(plan):
    raise SystemExit('Save each planned pose with S first; row counts differ.')
if any(len(row.split()) != 48 for row in rows + plan):
    raise SystemExit('Expected timestamp and 47 state fields in each row.')
number = 1
while Path(f'authoring/keyframes-before-retime-{number}.txt').exists():
    number += 1
shutil.copy2(recording, f'authoring/keyframes-before-retime-{number}.txt')
retimed = [suggestion.split(maxsplit=1)[0] + ' ' + saved.split(maxsplit=1)[1]
           for suggestion, saved in zip(plan, rows)]
recording.write_text('\n'.join(retimed) + '\n')
print(f'Retimed {len(rows)} S-recorded poses; all scene values preserved.')
