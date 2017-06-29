#pragma once

#include "box.h"
// Think about changing box colors, and drawing wireframes
// move manipulator into selected?

class Selected
{
public:
    Selected(Track& t)
        :track(t), selected_color(0.5f, 0.5f, 0.5f)
    {}

    bool rayIntersect(float &t, int& hit_side, const Ray& ray)
    {
        float min_t = FLT_MAX;
        int min_hit_side;
        bool hit = false;
        for(int i = 0; i < box_indices.size(); i++)
        {
            int face = -1;
            float tmp_t = track.getBoxAtIndex(box_indices[i]).rayIntersect(face, ray);
            if(tmp_t < min_t)
            {
                min_t = tmp_t;
                min_hit_side = face;
                hit = true;
            }
        }
        t = min_t;
        hit_side = min_hit_side;
        return hit;
    }

    // Remove selected boxes from track
    void remove()
    {
        for(int i = 0; i < box_indices.size(); i++)
        {
            track.getBoxAtIndex(box_indices[i]).setColor(box_colors[i]);
            track.removeBox(box_indices[i]);            
        }
        // No old indices are kept after track removes any box
        box_indices.clear();
        box_colors.clear();
    }

    // Copy selected boxes
    void copyAndSelect()
    {
        for(int i = 0; i < box_indices.size(); i++)
        {
            Box new_box = track.getBoxAtIndex(box_indices[i]).makeCopy();
            track.getBoxAtIndex(box_indices[i]).setColor(box_colors[i]);
            int new_box_index = track.addBox(new_box);
            box_indices[i] = new_box_index;
        }
    }

    Vec3 getSideNormal(const int hit_side)
    {
        if(box_indices.size() > 0)
        {
            return track.getBoxAtIndex(0).getSideNormal(hit_side);
        }
        return Vec3();
    }

    void changeLength(const int side_num, const float amount)
    {
        for(int i = 0; i < box_indices.size(); i++)
        {
            track.getBoxAtIndex(box_indices[i]).changeLength(side_num, amount);
        }
        bound_all.changeLength(side_num, amount);
    }

    void move(const Vec3& v)
    {
        for(int i = 0; i < box_indices.size(); i++)
        {
            track.getBoxAtIndex(box_indices[i]).move(v);
        }
        bound_all.min += v;
        bound_all.max += v;
    }

    bool select(const int index)
    {
        for(int i = 0; i < box_indices.size(); i++)
        {
            if(box_indices[i] == index)
            {
                return false;
            }
        }
        Box& box = track.getBoxAtIndex(index);
        if(box_indices.size() == 0)
        {            
            bound_all.min = box.min;
            bound_all.max = box.max;
        }else
        {
            bound_all.enlargeTo(box.min);
            bound_all.enlargeTo(box.max);
        }
        // this call to push_back is for making room for the new index
        box_colors.push_back(Vec3());
        box_indices.push_back(-1); 
        // Keep box_indices in non-increasing order
        int i;
        for(i = 0; i < box_indices.size() - 1 && box_indices[i] > index; i++){}
        for(int j = box_indices.size() - 1; j > i; j--)
        {
            box_indices[j] = box_indices[j-1];
            box_colors[j] = box_colors[j-1];
        }
        box_indices[i] = index;
        box_colors[i] = box.getColor();
        box.setColor(selected_color);
        return true;
    }

    bool hasIndex(const int index)
    {
        for(int i = 0; i < box_indices.size(); i++)
        {
            if(box_indices[i] == index)
            {
                return true;
            }
        }
        return false;
    }

    bool deselect(const int index)
    {
        for(int i = 0; i < box_indices.size(); i++)
        {
            if(box_indices[i] == index)
            {
                track.getBoxAtIndex(box_indices[i]).setColor(box_colors[i]);
                box_indices.erase(box_indices.begin() + i);
                box_colors.erase(box_colors.begin() + i);
                Box& tmp_box = track.getBoxAtIndex(0);
                bound_all.min = tmp_box.min;
                bound_all.max = tmp_box.max;
                for(int k = 1; k < box_indices.size(); k++)
                {
                    bound_all.enlargeTo(track.getBoxAtIndex(box_indices[i]).min);
                    bound_all.enlargeTo(track.getBoxAtIndex(box_indices[i]).max);
                }
                return true;
            }
        }
        return false;
    }

    void deselectAll()
    {
        for(int i = 0; i < box_indices.size(); i++)
        {
            track.getBoxAtIndex(box_indices[i]).setColor(box_colors[i]);
        }
        box_indices.clear();
        box_colors.clear();
        bound_all = BBox();
    }
    int getNumSelected() const
    {
        return box_indices.size();
    }

    // NOTE temporary
    Box& getBox(const int index)
    {
        //if(index > -1 && box_indices.size() > index)
        {
            return track.getBoxAtIndex(box_indices[index]);
        }
        //return nullptr;
    }

    Vec3 getCenter()
    {
        return bound_all.getCenter();
    }
private:
    Track& track;
    BBox bound_all;
    Vec3 selected_color;
    std::vector<int> box_indices;
    std::vector<Vec3> box_colors;
};
