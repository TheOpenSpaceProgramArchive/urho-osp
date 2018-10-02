# Urho3D Open Space Program
An experiment by Capital-Asterisk, to see if (another) Open Space Program can be made with Urho3D. This repository is at the point where 10N thrust rockets can be built out of different sized cubes, and brought to life with the physics engine. The code has some hidden things on the insides, such as a way of organizing usable parts and performance curves.

[Watch this](http://www.youtube.com/watch?v=hFYCftKDFMg)

## Building from source:

### Linux:
**Requires packages used by Urho3D:** https://github.com/urho3d/Urho3D/wiki/Getting-started-in-Linux

**1.** Let's create and enter a new directory for OSP:
```bash
mkdir ~/OSP && cd ~/OSP
```
**2.** Clone `urho-osp` and the Urho3D game engine
```bash
git clone https://github.com/TheOpenSpaceProgram/urho-osp
git clone https://github.com/urho3d/Urho3D
```
**3.** Build and install Urho3D
```bash
cd Urho3D
cmake .
sudo make install
```
**4.** Copy `CMake` folder to the `urho-osp` cloned repository:
```bash
cp -r CMake ../urho-osp/CMake
```
**5.** Set the `URHO3D_HOME` environment variable in `.bashrc`:
```bash
echo 'export URHO3D_HOME="~/OSP/Urho3D"' >> ~/.bashrc && source ~/.bashrc
```
**6.** Build `urho-osp`:
```bash
cd ../urho-osp
cmake .
make
```

## Screenshots of things that previously happened on this repository:

Tests on subdividing spheres
![alt text](https://cdn.discordapp.com/attachments/325425261069860875/415682532626137089/Screenshot_2018-02-20_17-32-48.png "It looks pretty but it's completely wrong.")

![alt text](https://cdn.discordapp.com/attachments/425003724633669633/428391873720090639/Screenshot_2018-03-27_20-14-23.png "This too is completely wrong.")

Cube in orbit
![alt text](https://cdn.discordapp.com/attachments/425003724633669633/451141764582080533/Screenshot_2018-05-29_14-29-53.png "An inverse square force towards the center.")

More accurate model of our planet
![alt text](https://cdn.discordapp.com/attachments/425003724633669633/448727538706153472/Screenshot_2018-05-22_23-02-33.png "This the truth, don't let the goverment fool you.")
