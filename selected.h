#pragma once

#include "box.h"
// Think about changing box colors, and drawing wireframes
// Update center
// Selected should behave similarly to a single box
// Add a new method to Manipulator that attaches it to a Selected
// Add a new method to Manipulator that lets it move selected
// move manipulator into selected?

class Selected
{
public:
    Selected(Track& t, const Mat4& proj_transform)
        :track(t), wireframe_drawer(proj_transform), selected_color(0.5f, 0.5f, 0.5f)
    {}

    bool rayIntersect(float &t, int& hit_side, const Ray& ray)
    {
        float min_t = FLT_MAX;
        int min_hit_side;
        bool hit = false;
        for(int i = 0; i < selected_boxes.size(); i++)
        {
            int face = -1;
            float tmp_t = selected_boxes[i]->rayIntersect(face, ray);
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
        for(int i = 0; i < selected_boxes.size(); i++)
        {
            selected_boxes[i]->setColor(box_colors[i]);
            track.removeBox(selected_boxes[i]);            
        }
        selected_boxes.clear();
        box_colors.clear();
    }

    // Copy selected boxes
    void copyAndSelect()
    {
        for(int i = 0; i < selected_boxes.size(); i++)
        {
            Box new_box = selected_boxes[i]->makeCopy();
            selected_boxes[i]->setColor(box_colors[i]);
            Box* new_box_ptr = track.addBox(new_box);
            selected_boxes[i] = new_box_ptr;
        }
    }

    Vec3 getSideNormal(const int hit_side)
    {
        if(selected_boxes.size() > 0)
        {
            return selected_boxes[0]->getSideNormal(hit_side);
        }
        return Vec3();
    }

    void changeLength(const int side_num, const float amount)
    {
        for(int i = 0; i < selected_boxes.size(); i++)
        {
            selected_boxes[i]->changeLength(side_num, amount);
        }
        // TODO: bound_all
    }

    void move(const Vec3& v)
    {
        for(int i = 0; i < selected_boxes.size(); i++)
        {
            selected_boxes[i]->move(v);
        }
        bound_all.min += v;
        bound_all.max += v;
    }

    bool select(Box* box)
    {
        for(int i = 0; i < selected_boxes.size(); i++)
        {
            if(selected_boxes[i] == box)
            {
                return false;
            }
        }
        if(selected_boxes.size() == 0)
        {
            bound_all.min = box->min;
            bound_all.max = box->max;
        }else
        {
            bound_all.enlargeTo(box->min);
            bound_all.enlargeTo(box->max);
        }
        box_colors.push_back(box->getColor());
        box->setColor(selected_color);
        selected_boxes.push_back(box);            
        return true;
    }

    bool find(const Box* box)
    {
        for(int i = 0; i < selected_boxes.size(); i++)
        {
            if(selected_boxes[i] == box)
            {
                return true;
            }
        }
        return false;
    }

    bool deselect(const Box* box)
    {
        for(int i = 0; i < selected_boxes.size(); i++)
        {
            if(selected_boxes[i] == box)
            {
                for(int j = i; j < selected_boxes.size()-1; j++)
                {
                    selected_boxes[j] = selected_boxes[j+1];
                    box_colors[j] = box_colors[j+1];
                }
                selected_boxes.erase(selected_boxes.end() - 1);
                box_colors.erase(box_colors.end() - 1);
                bound_all.min = selected_boxes[0]->min;
                bound_all.max = selected_boxes[0]->max;
                for(int k = 1; k < selected_boxes.size(); k++)
                {
                    bound_all.enlargeTo(selected_boxes[i]->min);
                    bound_all.enlargeTo(selected_boxes[i]->max);
                }
                return true;
            }
        }
        return false;
    }

    void deselectAll()
    {
        for(int i = 0; i < selected_boxes.size(); i++)
        {
            selected_boxes[i]->setColor(box_colors[i]);
        }
        selected_boxes.clear();
        box_colors.clear();
        bound_all = BBox();
    }
    int getNumSelected() const
    {
        return selected_boxes.size();
    }

    // NOTE temporary
    Box* getBoxPtr(const int index)
    {
        if(selected_boxes.size() > index)
        {
            return selected_boxes[index];
        }
        return nullptr;
    }

    Vec3 getCenter()
    {
        return bound_all.getCenter();
    }
private:
    Track& track;
    BoxWireframeDrawer wireframe_drawer;
    BBox bound_all;
    Vec3 selected_color;
    std::vector<Box*> selected_boxes;
    std::vector<Vec3> box_colors;
};
