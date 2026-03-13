# Planck

![Planck](https://i.imgur.com/q2M3uEU.jpg)

A compact 40% (12x4) ortholinear keyboard kit made and sold by OLKB and Massdrop. [More info on qmk.fm](https://qmk.fm/planck/)

* Keyboard Maintainer: [Jack Humbert](https://github.com/jackhumbert)
* Hardware Supported: Planck PCB rev1, rev2, rev3, rev4, Teensy 2.0
* Hardware Availability: [OLKB.com](https://olkb.com), [Massdrop](https://www.massdrop.com/buy/planck-mechanical-keyboard?mode=guest_open)

Make example for this keyboard (after setting up your build environment):

    make planck/rev4:default

Flashing example for this keyboard:

    make planck/rev4:default:flash

See the [build environment setup](getting_started_build_tools) and the [make instructions](getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](newbs).

## Bootloader

Enter the bootloader in 3 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (usually the top left key or Escape) and plug in the keyboard
* **Physical reset button**: Briefly press the button on the back of the PCB - some may have pads you must short instead
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available
