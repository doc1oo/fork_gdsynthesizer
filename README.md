# GDSYNTHESIZER

GDSYNTHESIZER is software tone generator and SMF player using GDEXTENSION for GodotEngine.

## What is GDSYNTHESIZER?

GDSYNTHESIZER is:
 - an extension of audio_stream_player that can be use by Godot engine
 - wave table base synthesizer
 - midi standard file player that may support format 0 and 1 
 - polyphony 32 or more
 - with variable parameters for 256 instruments
 - with 3 oscillator that are 5 types base wave forms
 - with variable envelope params i.e. attack slope time, decay harf-life time sustain rate and release slope time
 - with 2 types noises i.e. white and pink that can be added 
 - with random vibrator that make frequency distribution noise
 - with 3 delay functions for each instruments
 - with 2 LFOs for each AM and FM moduration
 - currently for only Windows and WEB(html5) platforms

GDSYNTHESIZER dose note have:
 - GUI
 - sound effect filter

Target Godot Version is ver.4.2.1.
This project is just a hobby of the author and may not necessarily work the way you want it to.

## How to build

GDSYNTHESIZER is builded with min-GW64 and scons.

GDSYNTHESIZER depends on godot-cpp.
So prepare the necessary files using the git command as below.

```
mkdir your_directory
cd your_directory
git clone https://github.com/soyokuyo/gdsynthesizer gdsynthesizer
cd gdsynthesizer
git clone --depth 1 https://github.com/godotengine/godot-cpp.git -b godot-4.2.1-stable godot-cpp-4.2.1-stable
```

Directory structure may be below.
```
your_directory/
    godot-cpp-4.2.1-stable/
    project/
    src/
    .gitignore
    SConstruct
    (and some other files)
```

- debug build for Windows
```
scons platform=windows use_mingw=yes
```

- release build for Windows
```
scons platform=windows use_mingw=yes target=template_release 
```

- debug build for WEB
```
scons platform=web target=template_debug
```

- release build for WEB
```
scons platform=web target=template_release
```


## how to include your Godot Engine project

make "bin" directory under "res://" and then copy gdsynthesizer.gdextension file and gdextension files as below.

```
res://
   bin/
      gdsynthesizer.gdextension
      libgdsynthesizer.windows.template_debug.x86_64.dll
      libgdsynthesizer.windows.template_release.x86_64.dll
      libgdsynthesizer.web.template_release.wasm32.wasm
      libgdsynthesizer.web.template_debug.wasm32.wasm
```

put GDSynthesizer node on your scene.
atach script with your scene and modify it.

The easiest way is as follows.

1. put your SMF file e.g. "sample.mid" in "res://" directory.
2. modify _ready() and _process() functions as below.

```
extends GDSynthesizer

func _ready()->void:
	init_synthe(4.0)
	var res:int = load_midi("res://sample.mid")
	if res == 1:
		print("open success")
	else:
		print("open failure")

	play(0.0)

func _process(delta:float)->void:
	feed_data(delta)

```

GDSYNTHESIZER is variable tone generator, so you can modify tone with  parameter edeitting.
But actualy, editing parameters is a little complicated.

To check how your SMF sounds with SYNTHESIZER, I've made a working GUI example named "KuyoSynthe" using GD SYNTHESIZER.
Find it from following GITHUB and try it out.
https://github.com/soyokuyo/kuyosynthe

By using KuyoSynthe, you can easily check how your SMF sounds, and you may find out which parameters to change to make it even better.
If you look into KuyoSynthe, you may find out how to use FGSYNTHESOZSE.

KuyoSynthe is also a complete example implementation for using GDSYNTHESIZSER.

Additionally, you may be able to find some explanations on my blog. 
https://junk-box.net/toy/

However, most of article will be written in Japanese.
