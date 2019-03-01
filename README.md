# Subgraph Flow

`sgflow` is a launcher application that runs in the Sway window manager. It
is similar to applications such as `dmenu` and `rofi`. 

![Screenshot)(sgflow_screenshot.png)

## Audience

`sgflow` is intended to be usable by any person who needs a launcher for their 
Sway desktop. However, it will also include optional (via compilation and 
configuration) features specifically for users of Subgraph Citadel -- namely 
features to support the paradigms of Citadel such as `realms`.

It may also work in other window managers that are based on `wlroots` though
it is not designed to nor has this been tested.

## Design

The main difference between `sgflow` and other launchers that work in Sway is 
that it is a Wayland application that renders itself via Wayland as an overlay 
onto a layer surface. Some other launchers are essentially X11 applications 
that are displayed along-side other applications and so they don't feel like 
they are part of the desktop.

`sgflow` is modelled after the applications overview in GNOME. It uses the 
same back-end functions to find and launch `.desktop` files. It also provides
some of the same search features -- such as searching by desktop application 
category, etc. But it does not support search providers or user-customizable
searches/launch actions.

It is implemented as a GTK application that is set up to render on a layer 
surface (thanks to `Waybar` for this inspiration).

Currently, `sgflow` is written in C. This was not my first choice. It will 
be re-written in Rust once the `wlroots-rs` bindings support layer shells.

## Building

Assuming that the required libraries are installed, run the following commands
to build `sgflow`:

```.bash
$ meson build
$ ninja -C build
```



