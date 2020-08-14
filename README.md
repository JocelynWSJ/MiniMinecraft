MiniMinecraft
======================
https://www.youtube.com/watch?v=UM3sVK046Ks
https://www.bilibili.com/video/BV1AE411J7rR


My part:
--------------

**Game Engine Update Function and Player Physics**

1. Player: Use bool status[12] to record mode. Player's position can use Camera's position whose y need to minus one.

2. Fly and jump: Both actions can be divided into rising process and landing process. In the rising process, give Player::velocity[1] a positive initial value and use int MyGl::flyLastFrame; to record the number of consecutive frames. Player::velocity[1] need to contact decrease based on flyLastFrame.

3. Swimming
- Altering the player's physics and controls when the player is touching a WATER or LAVA block to simulate swimming.
- Adding overlay to cover the screen when player is swimming.

4. Gravity: Give Player::velocity[1] a negative initial value in MyGL::timerUpdate(). And then make changes according to the mode or detection, if necessary.

5. Collision Detection: Detection is divided into horizontal direction detection and vertical direction detection. 
bool MyGL::HerizCollisionDetect(glm::vec3 pos, glm::vec3 movetrend);
bool MyGL::VertCollisionDetect(glm::vec3 pos, glm::vec3 movetrend);
In the flying mode, only horizontal detection is needed. And in the other condition, detection of both direction are needed.

**Multithreaded Terrain Generation**
1. Adding multithreading functionality to prevent the gameplay from slowing down when new terrain is generated to expand the world. Adding Worker class to complete FBM calculation in each thread.

**Day and night cycle**

I used the raycast method to create a procedural sky background. 
In Miniminecraft, one day lasts 2400 seconds, there are four main periods: Daytime, Sunset/Dusk, Nighttime, Sunrise/Dawn. The light direction and color also changed with sun and moon. To achieve smooth change of the sky color, I used mix function to generate color.
Most code is in sky.frag.glsl, sky.vert.glsl.

1. Daytime: (0~1200)
   Day time is the longest period in a day, lasting 1200 seconds. During this time, sun moves from east to west. The light direction will also change with it.

2. Sunset/Dusk: (1200~1380)
   Sun descends on the western horizon. The sun will appear larger when it falls. The sky becomes darker, while the sky near sun changed to orange.

3. Night Time: (1380~2220)
   The moon rises from east and stars shows up(I used Worley noise in 3D space to generate stars). The intensity of moonlight is half of sunlight. 

4. Sunrise/Dawn: (2220~2400)
   Sun rises up from east. The sun will also appear larger as it rises. Sky becomes lighter, while the sky near sun become orange.
