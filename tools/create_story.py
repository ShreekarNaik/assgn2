"""Plan 33 story poses. The app's S recorder writes the final keyframes.

Run from the repository root. This writes authoring/story_notes.txt for
assisted planning, never keyframes.txt. The application has no pose-import key.
"""
import math
from pathlib import Path

# A1 degree-of-freedom order, matching Minifig::pose(). Named indices keep
# the authored actions readable without introducing a separate rig system.
(ROOT_X, HEADING, ROOT_Z, X, Y, Z, NECK, LEFT_SHOULDER, RIGHT_SHOULDER,
 LEFT_HIP, RIGHT_HIP, LEFT_KNEE, RIGHT_KNEE, LEFT_WRIST, RIGHT_WRIST) = range(15)

FPS = 30
DURATION = 68
FLOOR_Y = -1.9


def ease(time, start, end):
    amount = max(0.0, min(1.0, (time - start) / (end - start)))
    return amount * amount * (3.0 - 2.0 * amount)


def blend(time, start, end, first, last):
    return first + (last - first) * ease(time, start, end)


def figure(x, z, heading):
    return [0, heading, 0, x, -0.1, z] + [0] * 9


def author_frame(time):
    chef = figure(-1.6, -2.0, 0)
    officer = figure(3.4, 1.8, -25)
    # Lower the lantern while turning at the counter, then carry it forward.
    chef[LEFT_SHOULDER] = (-25 * ease(time, 1, 3) * (1 - ease(time, 21, 22))
                           -25 * ease(time, 28.2, 30))
    chef[NECK] = -20 * ease(time, 1, 3) * (1 - ease(time, 8, 10))

    # Turn toward the path first, walk forward, stop, then face the chef.
    heading = math.degrees(math.atan2(-2.698, -2.6))
    officer[HEADING] = blend(time, 5.5, 7.5, -25, heading)
    arrival = ease(time, 8, 19)
    officer[X] = 3.4 - 2.698 * arrival
    officer[Z] = 1.8 - 2.6 * arrival
    officer[HEADING] = blend(time, 19, 21, officer[HEADING], -90)

    chef[NECK] += 30 * ease(time, 8, 11) * (1 - ease(time, 20, 23))
    chef[RIGHT_SHOULDER] = -45 * ease(time, 12, 13) * (1 - ease(time, 15, 17))
    # Aim the carried flashlight down the path, not straight under the feet.
    officer[RIGHT_SHOULDER] = blend(time,20,21,-40,-15)
    officer[RIGHT_SHOULDER] -= 20 * ease(time,21,22) * (1-ease(time,23,24))

    # Face the counter, reach FORWARD, touch, pause, then lift.
    chef[HEADING] = 180 * ease(time, 22, 24)
    chef[RIGHT_SHOULDER] += -31 * ease(time, 24, 25.5)
    chef[RIGHT_SHOULDER] += -19 * ease(time, 26.2, 27)
    holder = 0 if time < 26 else 1 if time < 36 else 2
    # Turn into the aisle before taking two slow forward steps.
    chef[HEADING] = blend(time, 27, 28, chef[HEADING], 0)
    advance = ease(time, 28, 30)
    chef[Z] = -2 + 1.2 * advance
    chef[HEADING] = blend(time, 30, 31, chef[HEADING], 90)
    chef[RIGHT_SHOULDER] -= 25 * ease(time, 31, 33)
    officer[LEFT_SHOULDER] = -75 * ease(time, 32, 35)
    chef[RIGHT_SHOULDER] *= 1 - ease(time, 37, 39)
    chef[HEADING] = blend(time, 39, 42, chef[HEADING], 30)

    # Takeaway: accept at chest height, look at the cup and acknowledge the chef.
    # A1 has no elbows, so this avoids an implausible attempt to reach the mouth.
    officer[LEFT_SHOULDER] = blend(time, 39, 41, officer[LEFT_SHOULDER], -55)
    officer[NECK] = 18 * ease(time, 39, 40) * (1 - ease(time, 42, 43))
    officer[RIGHT_SHOULDER] += -40 * ease(time, 43, 44) * (1 - ease(time, 45, 46))
    chef[RIGHT_SHOULDER] += -50 * ease(time, 45, 46) * (1 - ease(time, 48, 49))

    officer[RIGHT_SHOULDER] -= 25 * ease(time,49,50.5)

    # Exit along the clear right-hand aisle and through the back door.
    officer[HEADING] = blend(time, 49, 50.5, officer[HEADING], 90)
    across = ease(time, 50.5, 57)
    officer[X] += (6.4 - 0.702) * across
    officer[HEADING] = blend(time, 57, 58, officer[HEADING], 180)
    out = ease(time, 58, 65.5)
    officer[Z] -= 5.6 * out
    chef[NECK] += 30 * ease(time, 50, 54)
    chef[RIGHT_SHOULDER] += -40 * ease(time, 51, 52) * (1 - ease(time, 54, 55))

    left = int(time >= 12)
    right = int(14 <= time < 67)
    lantern = 1
    flashlight = int(5 <= time < 21 or time >= 50)
    camera = (0 if time < 9 else 2 if time < 19 else 1 if time < 29
              else 2 if time < 49 else 0)
    body_camera = int(50.8 <= time < 54.2)
    if body_camera:
        camera = 2
        # Settle the takeaway at chest height; no impossible backward head turn.
        officer[LEFT_SHOULDER] -= 10 * ease(time,50.8,51.5) * (1-ease(time,53,54.2))
    overview = [0, 7.9, 6.4, 0, -.3, -.5]
    return ([left, right, lantern, flashlight, camera] + overview + [0]
            + chef + officer + [holder, 0, body_camera,
                int(28<=time<30),int(8<=time<19 or 50.5<=time<57 or 58<=time<65.5)])


# These are STORY poses, not per-step leg poses. Walking is a reusable C++
# cycle enabled by M and stored as a flag for each figure.
TIMES = [0,5,8,9,12,14,19,22,24,25.5,26,27,28,29,30,31,33,35,36,
         37,39,43,46,49,50.5,50.8,52,54.2,57,58,65.5,67,68]
Path('authoring').mkdir(exist_ok=True)
rows=[]
for time in TIMES:
    values=author_frame(time)
    assert len(values)==47
    rows.append(str(round(time*FPS))+' '+' '.join(f'{v:.7g}' for v in values))
Path('authoring/story_notes.txt').write_text('\n'.join(rows)+'\n')
print(f'Prepared {len(rows)} assisted story-pose notes; no final file written.')
