Capybara
========

![Capybara](https://i.imgur.com/ZYwWS9m.png)

## What is this?

**Capybara** is a lightweight manager for [lemonbar](https://github.com/LemonBoy/bar), written in C.
It will start and manage lemonbar for you, and automagically update when bspwm changes.

## Great. How do I get it to work?

You'll need the following dependencies:

- [lemonbar](https://github.com/LemonBoy/bar) (obviously)
- [xtitle](https://github.com/baskerville/xtitle)
- the powerline fonts (specifically, Inconsolata for Powerline). [(you might be able to get them here)](https://github.com/powerline/fonts)
- the icomoon font (provided in this repository - install it on your system!)
- [pulseaudio-ctl](https://github.com/graysky2/pulseaudio-ctl) *(needed for volume)*
- mpc and a MPD server *(optional, only if you want the bar to display MPD info)*

Then, compile!

    $ gcc barm.c -o barm

If you don't want pulseaudio or mpd support, there are compile flags for that.
If you want to change the font, go edit the source yourself.

    $ gcc barm.c -o barm -DNO_PULSEAUDIO -DNO_MPD

Then, run it!

    $ ./barm

It should automagically update window title, current desktop (of focused monitor), load, and time.
If you want it to update instantly, send it a SIGUSR1 (this is needed if you want instant updates for volume, etc):

    $ kill -USR1 `pidof barm`

## How do I configure it?

You don't (at least not yet). It should be programmed intuitively enough for you to just dive in and make changes.
Patches welcome.

## Looks misaligned?

You probably don't have exactly the same screen as me. Change this line:

    if ((barfp = popen("lemonbar -g 1890x20+15+10...

Specify your own dimensions to lemonbar (after the -g option).

## It's broken!

I'd love it if you'd tell me. Feel free to report an issue on GitHub.
