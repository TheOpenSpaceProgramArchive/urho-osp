# Urho3D Open Space Program
Another Open Space Program project, but made with Urho3D.

![screenshot](https://cdn.discordapp.com/attachments/421375838953537546/553854430005690368/Screenshot_2019-03-09_00-16-52.png "This is what it looks like at the time this README is being written")

Build instructions can be found in the [wiki](https://github.com/TheOpenSpaceProgram/urho-osp/wiki/Building-from-Source).

### Working Features
 * Loading resources from files
   * Parts use glTF format (.sturdy.gltf)
   * Functionality can be added to parts (Rocket that apply thrust)
   * Can be created directly in Blender
 * Assembling spacecraft from these parts
   * Parts are dragged together into a single rigid body that has physics
 * Piloting spacecraft
   * Rockets with throttle and instantaneous torque control
   * Spacecraft can reach orbit (uses physics engine)
 * Large universe
   * Coordinate system can reach 400× the distance to Alpha Centauri
   * Multiple areas can have active physics (capable, but not used yet)
   * Floating Origin
 * Planet Level of Detail
   * Subdivides an Icosahedron
   * Generating chunks for a solid surface
   * Logarithmic Depth Buffer (disabled)
   * Doesn't kill the computer and runs smoothly on a chromebook

Features listed are not 100% complete, but work.

### Currently in Progress

 * A real way to build spacecraft (known as entities in the codebase)
   * Better Interface for the construction scene
   * Different kinds of structural attachments for parts
   * Proper data structure for entities

## Old Media:

[Cube rockets](http://www.youtube.com/watch?v=hFYCftKDFMg)

Tests on subdividing spheres
![screenshot](https://cdn.discordapp.com/attachments/325425261069860875/415682532626137089/Screenshot_2018-02-20_17-32-48.png "It looks pretty but it's completely wrong.")

![screenshot](https://cdn.discordapp.com/attachments/425003724633669633/428391873720090639/Screenshot_2018-03-27_20-14-23.png "This too is completely wrong.")

Cube in orbit
![screenshot](https://cdn.discordapp.com/attachments/425003724633669633/451141764582080533/Screenshot_2018-05-29_14-29-53.png "An inverse square force towards the center.")

More accurate model of our planet
![screenshot](https://cdn.discordapp.com/attachments/425003724633669633/448727538706153472/Screenshot_2018-05-22_23-02-33.png "This the truth, don't let the government fool you.")

