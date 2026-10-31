# CS675 Assignment 2 — One Last Cup

| Name | Roll number |
|---|---|
| Jashanpreet Singh | 23B0985 |
| Shreekar S Naik | 22B4518 |

## Build and run

```sh
# macOS dependencies: brew install glfw glew glm ffmpeg
# Ubuntu: sudo apt install libglfw3-dev libglew-dev libglm-dev ffmpeg
make
./a2-animate-minifig
```

Run from the repository root. The program uses C++17 and OpenGL 3.3 core, with GLFW, GLEW and GLM. `make test` checks animation logic; `make clean` removes the executable. There are no command-line modes.

## Story

The café is dim at closing time, initially lit by the chef’s warm lantern. A tired police officer switches on a cool flashlight and approaches during a patrol. The chef notices the visitor, gives a small greeting and brings the café lighting back. The officer turns toward the chef after approaching. Instead of rushing to close, the chef picks up the last takeaway coffee and offers it to the visitor. Both pause during the hand-off; the officer accepts the cup, acknowledges the kindness and leaves through the back doorway. The chef waves goodbye and resumes closing.

The chef carries a warm hanging lantern in the left hand; the officer holds a cool flashlight in the right. Their other hands are free to serve and receive the cup. These are different props with different lighting behaviour, rather than matching lights on their clothes.

| Time | Action and camera |
|---|---|
| 0–9 s | Dim overview: the chef’s lantern and officer’s flashlight establish the portable lights; officer turns toward the aisle |
| 9–19 s | Officer-follow view: approach through the central aisle |
| 19–29 s | Chef-follow view: turn toward counter, forward reach, visible contact, lift and turn into the aisle |
| 29–37 s | Officer-follow view: two steps, turn, offer and deliberate hand-off |
| 37–43 s | Officer-follow view: chef releases the cup and the officer accepts the takeaway |
| 43–49 s | Same side two-shot: thank-you and reply, without a reverse-angle cut |
| 49–68 s | Wide departure, brief body-mounted coffee/portrait shot at 50.8–54.2 s, then wide aisle walk and doorway exit |

The movement includes eased starts/stops and pauses between actions. The main point lights turn on at 12 and 14 seconds, after the portable lights have been introduced. The exit is given extra time instead of accelerating the character to keep a 60-second running time. Story positions and poses are saved in the keyframe file. The C++ player interpolates them and applies a reusable walking cycle only on flagged intervals. Each path segment has a matching facing direction, the returning knee bends, and the lowest foot edge is placed at floor height using A1’s original limb dimensions. The fixed feet retain a toy-like rocking motion because A1 has no ankle joints. This is an inexpensive animation approximation: planned paths avoid furniture, and four foot-corner heights determine the floor offset. There is no general collision engine, contact solver or inverse-kinematics rig. There are 33 keyframes from frame 0 to 2040 at 30 FPS; inclusive recording produces 2041 images, approximately 68.03 seconds.

## Models and scene

The minifigure builders come from the final [classroom A1 repository](https://github.com/Computer-Graphics-IITB/cs675-computer-graphics-2026-a1-model-shreekarnaik), commit `d34ada0`. Both models retain the submitted geometry and hierarchy:

```text
Torso (root rotation XYZ, translation XYZ)
├── Neck (Y) → Head → Hat
├── Left arm (shoulder X) → Left hand (wrist Y)
├── Right arm (shoulder X) → Right hand (wrist Y)
└── Pelvis
    ├── Left thigh (hip X) → Left lower leg (knee X) → Left foot
    └── Right thigh (hip X) → Right lower leg (knee X) → Right foot
```

There are 15 degrees of freedom per figure. A1 has no elbow joints. The original torso details, face geometry and hats remain. Wrist transforms are read from that same hierarchy to place the cup and handheld light props. No body parts or joints are added to or removed from the original figures.

The scene is a fully enclosed diner: cream walls, teal lower panels, brass trim, a framed night window, a walnut counter, shelves and containers, pastries on plates, side tables, cups and stools. Side seating leaves clear central and right-hand aisles. Four walls and a ceiling enclose the room; rectangular wall panels leave an actual window opening and an exit doorway. The environment map is visible through those openings. The officer passes through the doorway. All scene geometry uses the existing box and cylinder primitives. Small scene-building helpers and nested loops generate repeated furniture, window bars, shelf containers and lamp frames. The scene is organized into room, furniture and light/prop builders. There is no general scene-description engine or extra modelling dependency.

## Textures and lighting

Five original PPM textures are included in `textures/`:

| Texture | Use |
|---|---|
| chef.ppm | Large checks on the chef's torso |
| police.ppm | Blue uniform stripes and gold trim on the officer's torso |
| tiles.ppm | Quiet cream/green floor tiles |
| wood.ppm | Walnut counter and tabletops |
| sky.ppm | Equirectangular midnight environment panorama |

Planar UV projection chooses a coordinate pair from the face normal. Repeating texture coordinates and mipmapped filtering keep the materials readable at different distances. A small P3 PPM loader avoids an extra image-library dependency. All textures are included in the submission.

Lighting is per-pixel Blinn–Phong. The vertex shader supplies world-space positions and inverse-transpose transformed normals; the fragment shader combines material colour, texture colour, ambient light, attenuated diffuse light and specular highlights. A soft exposure curve prevents bright personal lights from clipping colours.

Two independent scene point lights sit beneath the pendant fixtures at (-6, 2.78, 0.5) and (6, 2.78, 0.5). One is warm and one gently cool. The pendants frame the side aisles, keeping them clear of the close camera views while lighting the characters from the front. Their attenuation is `2.2 / (1 + 0.08d + 0.025d²)`.

The chef's lantern is a warm point light in a brass cage, hanging from the existing left hand. The officer's flashlight is a cool spotlight fixed to the existing right-hand frame. A forward-and-down carrying pose aims the flashlight along the walking route rather than straight under the feet. The spotlight direction follows the arm and wrist; a smooth cone transitions between 16° and 30°. It casts a visible pool onto the floor when aimed down. The lantern uses attenuation strength 1.2 and the flashlight 4.0 with the same distance denominator as the pendant lights. Both lights are independently toggleable. The chef holds the lantern slightly forward and leaves it lit throughout the exchange, keeping the cage and warm light contribution visible rather than hiding them against the body. Their glass/lens and pendant undersides brighten when their corresponding lights are on. Ambient light preserves readability during closing. Shadows are not implemented.

A 160-unit cube surrounds the entire scene, all three cameras and the lights. Its shader samples the environment panorama from the world-space direction. This distant sky is unlit; scene and minifigure textures receive lighting.

## Cameras

All cameras use perspective projection and near/far planes of 0.1/250. The interior overview uses 74°, the chef follow view 48°, and the officer views 65°.

- Camera 1 sees the complete scene. Its initial eye is (0, 7.9, 6.4), looking at (0, -0.3, -0.5), inside the room below the ceiling. It remains a stable overview during the opening and ending.
- Camera 2 follows the chef; camera 3 follows the police officer. Their eye and target follow the figure’s root translation, with eye offsets (6, 2.8, -1.5) for the chef and (1, 2.8, 4.5) for the officer, and target offsets (0, 1, 0) and (0, 0.65, 0) respectively. The chef’s side view exposes the forward reach and cup contact without putting the officer in front of them. The chef faces the counter and reaches forward. The hand rests on the cup before ownership changes; a separate lift precedes the first step. The cup’s giver/receiver grip offsets differ by 180°, preserving world orientation at the transfer. Its open handle is staged to read in pickup and exchange views. Keeping the view orientation stable avoids camera spins when the characters turn to exchange the cup; each camera always looks at and moves with its figure.

The officer camera also has a body-mounted SnorriCam mode: local eye (0.9, 1.45, 3.5), target (0, 1.45, 0) and up direction are transformed with the root, keeping the officer steady while the café moves behind him. It is used from 50.8–54.2 seconds during the departure, with wide views before and after to preserve the route. F toggles the mode and its state is saved in the keyframe file.

C cycles through all three. Follow offsets are fixed; the overview eye and target can be edited and keyframed.

## Keymap

| Key | Function |
|---|---|
| Tab | Select chef / police for posing |
| C | Cycle overview / chef-follow / police-follow camera |
| F | Toggle body-mounted SnorriCam mode for the officer-follow camera |
| M | Enable/disable the reusable walking cycle for the selected figure during playback |
| F1 / F2 | Toggle left / right pendant point light |
| F3 / F4 | Toggle chef lantern / officer flashlight |
| G | Cycle cup placement: counter / chef right hand / officer left hand |
| A / Shift+A | Increase / decrease cup tilt |
| S | Append the actual current scene state to keyframes.txt, incrementing timestamp by 10 |
| L | Load keyframes.txt and print the number loaded |
| P | Play the loaded animation |
| R | Play and save every animation frame |
| Escape | Stop playback/recording; quit when idle |
| Backspace | Reset selected figure |
| X / Y / Z | Rotate selected figure's root around X / Y / Z |
| Left / Right | Translate figure along X |
| Up / Down | Translate figure along Y |
| = / - | Translate figure along Z |
| 1 | Rotate neck around Y |
| 2 / 3 | Rotate left / right shoulder around X |
| 4 / 5 | Rotate left / right hip around X |
| 6 / 7 | Bend left / right knee |
| 8 / 9 | Rotate left / right wrist around Y |
| Shift + rotation key | Reverse the rotation direction |
| J / K / O | Increment overview eye X / Y / Z |
| U / V / W | Increment overview target X / Y / Z |
| Shift + camera adjustment | Reverse camera adjustment |

On macOS, use Fn with F1–F4 if the keyboard assigns those keys to system functions. Figure posing retains A1's fixed increments on press/repeat. Playback owns the scene until it finishes or Escape is pressed. Back up the supplied story before saving new keyframes: S appends to the file.

## Animation authoring: 33 story poses and one reusable walk

The final file contains **33 story keyframes**, not individually recorded footfalls. They describe the meaningful poses, positions, turns, cup contacts, light changes and camera cuts. A small C++ walking function supplies the repeated steps between those poses. M enables/disables that cycle for the selected figure, and both figures’ walk flags are saved in the keyframe file.

**S saves the current scene; L loads the file; P plays; R captures.** For manual authoring, set a pose/position and the required light/camera/walk states, then press S. S uses timestamps 0,10,20... by default; adjust the timestamp column to set the intended pacing. Walking requires only start/stop positions and a walk flag, rather than dozens of per-step poses. N and the former suggestion queue have been removed.

The walk is a deliberately small approximation based on distance travelled along the keyframed route. It alternates hips/knees and offsets the root using the lowest foot corners from A1’s original dimensions. It does not use collision detection, inverse kinematics or a physics engine. Disable M for deliberately posed or stationary legs; their recorded angles still interpolate normally. The original hierarchy is unchanged.

The current story was planned with AI assistance. `tools/create_story.py` contains 33 named story times and prepares reference states in `authoring/story_notes.txt`; it does not write the final file. The supplied states were staged during automated verification and saved through the app’s S handler, then reloaded with L. This is not evidence of human keypresses or a claim that the authors personally hand-posed the supplied file. `tools/retime_recording.py` changes only the timestamp column of that recording, preserving all saved values, as checked against the original recording. The helpers and Codex assistance are disclosed so the animation has an inspectable provenance rather than an invented manual-authoring story.

The normal interactive S/L/P/R path needs no Python helper. Both authors should review, reproduce and adjust these 33 poses themselves before submitting, understand the small walking function, and finalize their own declaration. The assignment does not explicitly settle scripted planning assistance; do not claim instructor permission that has not been given.

## Keyframe format and playback

Each plain-text line has an integer frame timestamp followed by 47 numeric fields, with no labels:

1. Four light states (0/1), then camera index (0/1/2).
2. Overview eye XYZ and target XYZ.
3. Active figure (0 chef / 1 police).
4. Chef's 15 A1 degrees of freedom, then officer's 15.
5. Cup holder (0 counter / 1 chef / 2 officer), tilt in degrees, officer body-camera mode, and chef/officer walk-cycle flags (all mode flags are 0/1).

The per-figure order is root rotation XYZ, translation XYZ, neck Y, left/right shoulder X, left/right hip X, left/right knee X and left/right wrist Y. Light states, camera index, cup holder, body-camera mode, walk flags and selected figure change at keyframe boundaries. Other fields interpolate linearly; root rotations use the shortest signed angular difference. The loader rejects malformed fields, nonfinite values and unordered timestamps, and reports the keyframe count.

GLFW supplies `glfwGetTime`, but has no timer-callback registration API. The application's `FrameTimer::dispatch` uses that clock to invoke a callback at 1/30-second deadlines from the window event loop. There is no sleep loop: input events remain responsive. Every animation frame is rendered in order, even when capture takes longer than real time. The exported movie is therefore exactly 30 FPS without dropped animation frames.

## Recording and movie

```sh
./a2-animate-minifig
# Press L, then R in the window. Wait for “Playback complete”.
python3 tools/make_movie.py
```

Recording reads the back buffer before swap, corrects pack alignment, flips rows vertically and writes `frames/frame-00000.ppm` onward. It checks available disk space first. If a previous capture exists, it is moved to the next unused `frames-previous-N` directory; no old frames are deleted. Keep the window dimensions fixed while recording. A Retina 1920×1080 capture takes roughly 13 GB; the delivered movie uses 1280×720 frames, roughly 5.7 GB before encoding.

The movie script combines the images into H.264 video with an original synthesized chord soundtrack encoded as AAC. No external music recordings or samples are used. It writes `output/one-last-cup.mp4` and extracts the hand-off frame to `screenshots/movie.jpg`. Existing encoded outputs are protected from overwriting.

**YouTube:** Add your uploaded video URL here before submission.

![Night Shift Café — frame from the movie](screenshots/movie.jpg)

## Code layout

| File | Responsibility |
|---|---|
| src/main.cpp | Keyboard commands, timer callback, playback and framebuffer recording |
| src/scene.hpp/.cpp | Scene geometry, cameras, lights and scene-state transfer |
| src/minifig.hpp/.cpp | Final A1 builders, joint controls and pose access |
| src/node.hpp/.cpp | Node buffers, transforms, textures and recursive rendering |
| src/geometry.hpp/.cpp | Triangle geometry, normals and UVs |
| src/animation.hpp/.cpp | Keyframe parsing, saving and interpolation |
| src/texture.hpp/.cpp | PPM texture loading |
| src/gl_context.hpp/.cpp | GLFW window and input setup |
| src/shader_util.hpp/.cpp | Shader compilation and linking |
| shaders/ | World-space vertex shader and per-pixel lighting shader |
| tools/create_story.py | Reference notes for 33 assisted story poses; does not write the final file |
| tools/retime_recording.py | Apply timestamps to S-recorded rows without changing any scene values |
| tools/make_movie.py | Frame encoding, original soundtrack and movie screenshot |

## Sources and declaration

The A1 source is our submitted classroom version linked above. A1 acknowledged the CS475/CS675 tutorials, GLFW/GLEW/GLM documentation, LEGO proportions and Claude as a coding helper. OpenAI Codex assisted with this A2 implementation, visual iteration and report. Textures and synthesized audio were created for this scene; no third-party art or music is bundled.

**Declaration draft — both authors must review before submission:** This assignment extends our own submitted A1 minifigures. We acknowledge the references and AI coding assistance above and take responsibility for the implementation we submit and our ability to explain it in the viva. We will not represent assisted code as work completed without assistance.

## Validation

The program builds with `-Wall -Wextra`, and the animation tests pass. Independent GPT-6.1 Sol reviewers at low reasoning effort rated the revised rendered cut 9.0/10, judging student-level staging, motion/contact, camera framing and story coherence rather than photorealistic rendering. A separate source/rubric audit passed the implemented requirements; final submission caveats are listed below. A real OpenGL run checked all three cameras, four light switches and cup controls. The cup-transfer hand positions differ by less than 0.00005 world units. Pickup, hand-off, takeaway acknowledgement, lighting and departure frames were inspected during visual iteration.

The production timer and recording paths generated the full 2041-frame sequence at 1280×720. The encoded production video matches the independently reviewed cut exactly on all 2041 decoded video frames (SSIM 1.0); the final audio is regenerated to the full duration. The encoded H.264/AAC movie is 68.03 seconds long at 30 FPS; full video/audio decoding completes without errors. The README JPG is captured frame 1050, during the hand-off. YouTube upload and the final author-reviewed declaration remain manual submission steps.

## Submission

Include the Makefile, README.md, final keyframes.txt, source, shaders, texture images and movie JPG. Do not submit frames/, frames-previous-*/, output/, output-previous-*/, backups/, authoring/, executables or object files. Texture images must remain included; no ignore rule hides them. Run `make clean` before packaging. Keep the repository private and submit through the course destination.

Before submitting, both authors should understand the code, finalize their declaration and add the YouTube URL. The movie itself and captured frame sequences must stay outside the submitted archive.
