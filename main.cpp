#include "sg3d.h"

Graphics g(800, 600, "SG3D - Playground"); // Create a new SG graphic
sg3d g3d; // Create a new SG3D graphic

sg3d::cam camera = { // Create camera
    0.0f, // rotation y
    0.0f, // rotation x
    0.0f, // rotation z
    0.0f, // z position
    -3.0f, // y position
    0.0f, // x position
    0.0f, // COY
    0.0f, // COX
    0.0f, // COZ
    1 // Clipping plane
};

// Some variables for the example
float lookspeed = 0.01;
float movespeed = 0.05;
bool lookleft = false;
bool lookright = false;
bool walkforward = false;
bool walkbackward = false;
bool strafeleft = false;
bool straferight = false;

// Create a position to summon the pillar and cube model
sg3d::tri pillar1 = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, g._RGB(0, 0, 0)};
sg3d::tri cube1 = {{2, 5, 0}, {2, 5, 0}, {2, 5, 0}, {0, 0, 0}, {0, 0, 0}, g._RGB(0, 0, 0)};

// Import the pillar and cube model
std::vector<sg3d::tri> pillar_object1 = g3d.load_object("src/models/complex/pillar.sg3d", pillar1, g);
std::vector<sg3d::tri> cube_object1 = g3d.load_object("src/models/primitive/cube.sg3d", cube1, g);

void Render()
{
    // SG 2d render function
    
    g3d.setCurrentCamera(camera); // Set the camera

    std::vector<sg3d::obj> world = { // Create the world
        g3d.pack_tris( pillar_object1 ),
        g3d.pack_tris( cube_object1 )
    };

    g3d.drawWorld(world, g); // Draw the world

    g._DrawLine(400, 290, 400, 310, g._RGB(255, 0, 0)); // Some SG 2d lines drawn
    g._DrawLine(390, 300, 410, 300, g._RGB(255, 0, 0));

    // Simple camera controls
    if (lookleft)
        camera.roy -= lookspeed;
    if (lookright)
        camera.roy += lookspeed;
    if (walkforward)
    {
        camera.xpos += sin(camera.roy) * movespeed;
        camera.zpos += -cos(camera.roy) * movespeed;
    }
    if (walkbackward)
    {
        camera.xpos -= sin(camera.roy) * movespeed;
        camera.zpos -= -cos(camera.roy) * movespeed;
    }
    if (strafeleft)
    {
        camera.zpos += sin(camera.roy) * movespeed;
        camera.xpos += cos(camera.roy) * movespeed;
    }
    if (straferight)
    {
        camera.zpos -= sin(camera.roy) * movespeed;
        camera.xpos -= cos(camera.roy) * movespeed;
    }
}

void Keyboard(KeySym ks)
{
    // SG 2d keyboard inputs
    
    switch (ks)
    {
        case XK_Right:
            lookleft = true;
            break;
        case XK_Left:
            lookright = true;
            break;
        case XK_a:
            strafeleft = true;
            break;
        case XK_d:
            straferight = true;
            break;
        case XK_w:
            walkforward = true;
            break;
        case XK_s:
            walkbackward = true;
            break;
    }
    if (!g.keydown) { lookleft = lookright = walkforward = walkbackward = strafeleft = straferight = false; }
}

int main(int argc, char** argv)
{
    // Birth
    g._InitWindow();
    // Life
    g._WindowLoop(Render, Keyboard);
    // Death
    g._CloseWindow();
    return 0;
}
