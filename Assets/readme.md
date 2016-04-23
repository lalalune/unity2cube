L3D Cube script for Unity3D! Works with the big cubes, but can also be easily modified to work for the small ones!
This will work on Android, iOS and computer– so if you have an old phone lying around, you can use it to control your cube!
The NoSleep.cs script will keep your phone awake, just make sure it's plugged into a charger :)

Setup:

First thing you'll want to do is put your Device ID and token in for your Particle, which you can get from the Particle site.

Since I have seen literally 0 documentation on working with the big cubes on CubeTube, everything is currently being done through Particle Build.

Upload the .ino file included, or paste the code into a new file on Particle Buid. Upload to your Particle, and everything should be set.

If you have one cube, add one "send data" script to an object and enter your Device ID and Token. IP and port will be set automatically.

The way it works is that raycasts are fired at your 3d object. When they collide, they store the color by tag– so if you want your object to be color 1, tag is "color 1"

Since the cube is 16x16x16 (or 8x8x8), objects need to fit inside that space (in Unity Units) in order for the Raycasts to hit them. If they go out of that space they will be ignored.

These colors are 8 bit values (2-bit RGB color). Bits 1 and 2 are red, bits 3 and 4 are green, 5 and 6 are blue, 7 and 8 are nothing but could be used for brightness
An int of value 64 (1 0 0 0 0 0 0 0) would be red. 68 (1 0 0 0 0 0 1 0 0) would be red with some blue. Etc etc.
We're working on a better way to encode this. Please e-mail me if you beat me to it.

The "empty" field in the SendData file determines what color all of the pixels that don't have an object in them will be. This is black by default.

There is current no brightness available. This would be easy enough to code in the data send and listener, but it would require some thought in how to assign brightness to objects.
Again, e-mail me if you figure this out.

FPS on the send-data determines how many frames will be sent over UDP. 10-15 seems to work well, 30 FPS would be fine for a fast dedicated Wi-Fi connection. I've done 60 FPS on a smaller cube just fine.

The X Y Z offsets are to offset the raycast points, in case you want multiple cubes.

If you have any questions, please e-mail me!