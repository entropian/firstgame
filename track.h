#pragma once
#include "box.h"
#include "ray.h"
#include <vector>
#include "vec.h"

class Track
{
public:
    Track()
    {}

    void addBox(const Box& box)
    {
        boxes.push_back(box);
    }

    Box* rayIntersectTrack(int& face, const Ray& ray)
    {
        float min_t = TMAX;
        int index = -1, min_face = -1;
        for(int i = 0; i < boxes.size(); i++)
        {
            int side;
            float t = boxes[i].rayIntersect(side, ray);
            if(t < min_t)
            {
                min_t = t;
                index = i;
                min_face = side;
            }
        }
        face = min_face;
        if(index != -1)
        {
            return &(boxes[index]);
        }
        return nullptr;
    }

    // Determine if ship bbox collide with track
    // Assume all track boxes are the size of ship's bbox or bigger
    // Therefore each side of ship's bbox can touch up to 4 boxes
    // 4 * 6 sides = 24
    int bboxCollideWithTrack(Box* colliding_boxes[24], bool opposing_axis[3], const BBox& bbox)
    {
        int count = 0;
        Vec3 center_dir(0.0f, 0.0f, 0.0f);

        opposing_axis[0] = false;
        opposing_axis[1] = false;
        opposing_axis[2] = false;
        
        Vec3 ship_center = (bbox.max - bbox.min) * 0.5f + bbox.min;
        for(int i = 0; i < boxes.size(); i++)
        {
            if(bbox.bboxIntersect(boxes[i]))
            {
                colliding_boxes[count] = &(boxes[i]);
                Vec3 box_center = (boxes[i].max - boxes[i].min) * 0.5f +
                    boxes[i].min;
                Vec3 center_displacement = ship_center - box_center;
                for(int j = 0; j < 3; j++)
                {
                    if(center_dir[j] == 0.0f)
                    {
                        if(center_displacement[j] != 0.0f)
                        {
                            center_dir[j] = center_displacement[j];
                        }
                    }
                    if(center_displacement[j] * center_dir[j] < 0.0f)
                    {
                        opposing_axis[j] = true;
                    }
                }
                count++;
            }
        }
        return count;
    }
private:
    std::vector<Box> boxes;
};
