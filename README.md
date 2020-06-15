**This repo is very likely abandoned in favor of osp-magnum**

**osp-magnum:** https://github.com/TheOpenSpaceProgram/osp-magnum

osp-magnum is based on many of the lessons learned from this repo, so really nothing much gone to waste.

# Urho3D Open Space Program
Another Open Space Program project, but made with Urho3D.

![screenshot](https://cdn.discordapp.com/attachments/421375838953537546/553854430005690368/Screenshot_2019-03-09_00-16-52.png "This is what it looks like at the time this README is being written")

Build instructions can be found in the [Wiki](https://github.com/TheOpenSpaceProgram/urho-osp/wiki/).

## Contributing

If you have any cool ideas, discuss in the [Discord Server](https://discord.gg/7xFsKRg). This is an open source project, yet there's mostly one guy working on it, so a ton of technical documentation is being written in the [Wiki](https://github.com/TheOpenSpaceProgram/urho-osp/wiki).

### Technical Features
 * Loading Resources from Files
   * Parts use modified glTF format (.sturdy.gltf)
     * Can be created directly in Blender
   * Functionality can be added to parts (Machines)
     * MachineRocket - A Rocket engine
     * MachinePutMoreHere - Add more Machines
 * Large Universe
   * Coordinate system is practically infinite
     * A tree of "Satellites"
   * Active Physics Areas (ActiveArea)
     * Floating Origin
     * There can be multiple of these
 * Planet Level of Detail
   * Subdivides an Icosahedron
   * Generating chunks for a solid surface
   * Logarithmic Depth Buffer (disabled)
   * Doesn't kill the computer and runs smoothly on a chromebook

### (Planned) Gameplay Features
 * Atmosphereic Flight
 * Building and Flying spacecraft
 * Built-in Mod Manager
 * Multiplayer
 * Swappable solar systems 
 * Way too early and technical to really say much here, but there's big lists of features in a google doc somewhere.

 * Assembling spacecraft from these parts
   * Parts are dragged together into a single rigid body that has physics
 * Piloting spacecraft
   * Rockets with throttle and instantaneous torque control
   * Spacecraft can reach orbit (uses physics engine)



## Screenshots

Precise arrangement of Marlin-1D rockets and a few cubes

![screenshot](https://cdn.discordapp.com/attachments/418085421747011585/603044840300871730/unknown.png "")

Fridge with rockets on it

![screenshot](https://cdn.discordapp.com/attachments/418085421747011585/603694930279530498/unknown.png "")

Multiple Astronomical Bodies

![screenshot](https://cdn.discordapp.com/attachments/421375838953537546/608817108666548265/Screenshot_2019-08-07_17-17-36.png "")

## Old Media:

[Cube rockets](http://www.youtube.com/watch?v=hFYCftKDFMg)

Tests on subdividing spheres
![screenshot](https://cdn.discordapp.com/attachments/325425261069860875/415682532626137089/Screenshot_2018-02-20_17-32-48.png "It looks pretty but it's completely wrong.")

![screenshot](https://cdn.discordapp.com/attachments/425003724633669633/428391873720090639/Screenshot_2018-03-27_20-14-23.png "This too is completely wrong.")

Cube in orbit
![screenshot](https://cdn.discordapp.com/attachments/425003724633669633/451141764582080533/Screenshot_2018-05-29_14-29-53.png "An inverse square force towards the center.")

More accurate model of our planet
![screenshot](https://cdn.discordapp.com/attachments/425003724633669633/448727538706153472/Screenshot_2018-05-22_23-02-33.png "This the truth, don't let the government fool you.")

