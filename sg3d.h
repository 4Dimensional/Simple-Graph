// Simple graphics 3d
// Handles graphical inputs



#include "sg.h"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <vector>

class sg3d {
public:
    struct p3d {
        float x, y, z;
    };

    struct p2d {
        float x, y;
    };

    struct tri {
        p3d p1, p2, p3, rot, rot_center;
        unsigned long int color;
    };

    struct clr {
        int r, g, b;
    };

    struct cam {
        float roy,rox,roz,zpos,ypos,xpos,coy,cox,coz;
        int clippingPlane;
    };

    struct obj {
        std::vector<tri> tris;
    };

    obj pack_tris(std::vector<tri> tris)
    {
        obj ret;
        ret.tris.resize(tris.size());
        ret.tris = tris;
        if (cameraenabled)
        {
            for (size_t i = 0; i < tris.size(); ++i)
            {
                ret.tris[i].p1.x += current_camera.xpos; ret.tris[i].p2.x += current_camera.xpos; ret.tris[i].p3.x += current_camera.xpos;
                ret.tris[i].p1.y += current_camera.ypos; ret.tris[i].p2.y += current_camera.ypos; ret.tris[i].p3.y += current_camera.ypos;
                ret.tris[i].p1.z += current_camera.zpos; ret.tris[i].p2.z += current_camera.zpos; ret.tris[i].p3.z += current_camera.zpos;
                ret.tris[i].rot.x += current_camera.rox;
                ret.tris[i].rot.y += current_camera.roy;
                ret.tris[i].rot.z += current_camera.roz;
                ret.tris[i].rot_center.x += current_camera.cox;
                ret.tris[i].rot_center.y += current_camera.coy;
                ret.tris[i].rot_center.z += current_camera.coz;
            }
        }
        return ret;
    }

    std::vector<tri> load_object(std::string file_location, tri adder, Graphics g)
    {
        /*
            object structure:
            First       Second      Third         Rotation    Rot Center   Color
            0.0 0.0 0.0 1.0 0.0 0.0 -1.0 -1.0 0.0 0.0 0.0 0.0 0.0 0.0 10.0 255 0 255
        */

        std::vector<tri> tris;

        std::fstream file(file_location);

        std::string line;

        p3d p1, p2, p3;
        p3d trot;
        p3d trot_center;
        clr tcolor;

        if (file.is_open())
        {
            while (file>>p1.x && file>>p1.y && file>>p1.z && file>>p2.x && file>>p2.y && file>>p2.z && file>>p3.x && file>>p3.y && file>>p3.z && file>>trot.x && file>>trot.y && file>>trot.z && file>>trot_center.x && file>>trot_center.y && file>>trot_center.z && file>>tcolor.r && file>>tcolor.g && file>>tcolor.b)
            {
                p1.x += adder.p1.x; p2.x += adder.p2.x; p3.x += adder.p3.x;
                p1.y += adder.p1.y; p2.y += adder.p2.y; p3.y += adder.p3.y;
                p1.z += adder.p1.z; p2.z += adder.p2.z; p3.z += adder.p3.z;

                trot.x += adder.rot.x; trot.y += adder.rot.y; trot.z += adder.rot.z;
                trot_center.x += adder.rot_center.x; trot_center.y += adder.rot_center.y; trot_center.z += adder.rot_center.z;

                tri t_tri = { p1, p2, p3, trot, trot_center, g._RGB(tcolor.r, tcolor.g, tcolor.b) };
                tris.push_back(t_tri);
            }
        }
        else
        {
            std::cout << "Unable to load object" << std::endl;
            file.close();
            return tris;
        }

        file.close();

        std::cout << "Loaded object" << std::endl;

        return tris;
    }

    void save_object(obj o, tri subtractor, std::string location)
    {
        std::ofstream file(location);
        if (file)
        {
            for (size_t i = 0; i < o.tris.size(); ++i)
            {
                tri t = o.tris[i];
                tri s = subtractor;
                cam c = {
                    0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f
                };
                if (cameraenabled)
                    c = current_camera;
                clr deconstruct = undorgb(t.color);
                file<<t.p1.x-s.p1.x-c.xpos<<" ";file<<t.p1.y-s.p1.y-c.ypos<<" ";file<<t.p1.z-s.p1.z-c.zpos<<" ";
                file<<t.p2.x-s.p2.x-c.xpos<<" ";file<<t.p2.y-s.p2.y-c.ypos<<" ";file<<t.p2.z-s.p2.z-c.zpos<<" ";
                file<<t.p3.x-s.p3.x-c.xpos<<" ";file<<t.p3.y-s.p3.y-c.ypos<<" ";file<<t.p3.z-s.p3.z-c.zpos<<" ";
                file<<t.rot.x-s.rot.x-c.rox<<" ";file<<t.rot.y-s.rot.y-c.roy<<" ";file<<t.rot.z-s.rot.z-c.roz<<" ";
                file<<t.rot_center.x-s.rot_center.x-c.cox<<" ";file<<t.rot_center.y-s.rot_center.y-c.coy<<" ";file<<t.rot_center.z-s.rot_center.z-c.coz<<" ";
                file<<deconstruct.r<<" ";file<<deconstruct.g<<" ";file<<deconstruct.b<<std::endl;
            }
        }
        file.close();
    }

    void drawWorld(std::vector<obj> objs, Graphics g)
    {
        size_t max_size = 0;
        for (size_t i = 0; i < objs.size(); ++i)
        {
            max_size += objs[i].tris.size();
        }
        std::vector<tri> all_tris;
        all_tris.resize(max_size);
        int r = 0;
        for (size_t i = 0; i < objs.size(); ++i)
        {
            for (size_t j = 0; j < objs[i].tris.size(); ++j)
            {
                all_tris[r] = objs[i].tris[j];
                r++;
            }
        }
        drawTriArray(all_tris, g);
    }

    void setCurrentCamera(cam c)
    {
        current_camera = c;
        cameraenabled = true;
    }

private:
    clr undorgb(unsigned int color)
    {
        int red, green, blue;

        // Does not give desired results, but it still works
        red = color>>8;
        blue = color;
        green = (color>>16)>>8;

        return {red, green, blue};
    }

    bool cameraenabled = false;
    cam current_camera;

    static p3d rot3d(p3d p1, p3d rot)
    {
        p3d r1;

        p3d p2 = { p1.x, p1.y, p1.z };

        r1.x = (cos(rot.x)*cos(rot.y))*p2.x + (cos(rot.x)*sin(rot.y)*sin(rot.z) - sin(rot.x)*cos(rot.y))*p2.y + (cos(rot.x)*sin(rot.y)*cos(rot.z) + sin(rot.x)*sin(rot.z))*p2.z;
        r1.y = (sin(rot.x)*cos(rot.y))*p2.x + (sin(rot.x)*sin(rot.y)*sin(rot.z) + cos(rot.x)*cos(rot.z))*p2.y + (sin(rot.x)*sin(rot.y)*cos(rot.z) - cos(rot.x)*sin(rot.z))*p2.z;
        r1.z = (-sin(rot.y))*p2.x + (cos(rot.y)*sin(rot.z))*p2.y + (cos(rot.y)*cos(rot.z))*p2.z;

        return r1;
    }

    static p3d calculateDist(tri t)
    {
        p3d p1 = t.p1;
        p3d p2 = t.p2;
        p3d p3 = t.p3;
        p3d rot = t.rot;
        p3d rot_center = t.rot_center;

        p3d pq1 = p1;
        p3d pq2 = p2;
        p3d pq3 = p3;

        // Rotate
        p3d r1, r2, r3;

        r1 = rot3d(pq1, rot);
        r2 = rot3d(pq2, rot);
        r3 = rot3d(pq3, rot);

        r1.x += rot_center.x;
        r1.y += rot_center.y;
        r1.z += rot_center.z;

        r2.x += rot_center.x;
        r2.y += rot_center.y;
        r2.z += rot_center.z;

        r3.x += rot_center.x;
        r3.y += rot_center.y;
        r3.z += rot_center.z;

        p3d avg = { (r1.x+r2.x+r3.x)/3, (r1.y+r2.y+r3.y)/3, (r1.z+r2.z+r3.z)/3 };

        return avg;
    }

    p2d projectPoint(p3d pnt, float FOV)
    {
        p2d ret;
        ret.x = pnt.x*(FOV/pnt.z);
        ret.y = pnt.y*(FOV/pnt.z);
        return ret;
    }

    void drawTri(p3d p1, p3d p2, p3d p3, p3d rot, p3d rot_center, unsigned int color, Graphics g)
    {
        short unsigned int w_wid = g.R_WINDOW_WIDTH;
        short unsigned int w_hei = g.R_WINDOW_HEIGHT;

        p3d pq1 = p1;
        p3d pq2 = p2;
        p3d pq3 = p3;

        // Rotate
        p3d r1, r2, r3;

        r1 = rot3d(pq1, rot);
        r2 = rot3d(pq2, rot);
        r3 = rot3d(pq3, rot);

        r1.x += rot_center.x;
        r1.y += rot_center.y;
        r1.z += rot_center.z;

        r2.x += rot_center.x;
        r2.y += rot_center.y;
        r2.z += rot_center.z;

        r3.x += rot_center.x;
        r3.y += rot_center.y;
        r3.z += rot_center.z;

        // Convert to screen
        p2d s1, s2, s3;

        p2d s1dif = projectPoint(r1, 3.14);
        p2d s2dif = projectPoint(r2, 3.14);
        p2d s3dif = projectPoint(r3, 3.14);

        s1 = { (s1dif.x*100)+(w_wid/2), (s1dif.y*100)+(w_hei/2) };
        s2 = { (s2dif.x*100)+(w_wid/2), (s2dif.y*100)+(w_hei/2) };
        s3 = { (s3dif.x*100)+(w_wid/2), (s3dif.y*100)+(w_hei/2) };

        /* Wireframe
        if ((r1.z+r2.z)/2 > 0)
            g._DrawLine(s1.x, s1.y, s2.x, s2.y, color);
        if ((r2.z+r3.z)/2 > 0)
            g._DrawLine(s2.x, s2.y, s3.x, s3.y, color);
        if ((r3.z+r1.z)/2 > 0)
            g._DrawLine(s3.x, s3.y, s1.x, s1.y, color);
        */

        if ((r1.z+r2.z)/2 > current_camera.clippingPlane && (r2.z+r3.z)/2 > current_camera.clippingPlane && (r3.z+r1.z)/2 > current_camera.clippingPlane)
        {
            XPoint pnts[3] = {{(short int)s1.x, (short int)s1.y}, {(short int)s2.x, (short int)s2.y}, {(short int)s3.x, (short int)s3.y}};
            g._DrawPoly(pnts, 3, color);
        }
    }

    static bool compareTwoTris(tri t1, tri t2)
    {
        return (calculateDist(t1).z > calculateDist(t2).z);
    }

    void drawTriArray(std::vector<tri> triarray, Graphics g)
    {
        std::vector<tri> ta;
        ta.resize(triarray.size());

        for (size_t i = 0; i < triarray.size(); ++i)
        {
            ta[i] = triarray[i];
        }

        std::sort(ta.begin(), ta.end(), compareTwoTris);

        for (size_t i = 0; i < triarray.size(); ++i)
        {
            drawTri(ta[i].p1, ta[i].p2, ta[i].p3, ta[i].rot, ta[i].rot_center, ta[i].color, g);
        }
    }
};
