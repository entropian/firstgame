#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "globaldata.h"
#include "input.h"
#include "mat.h"
#include "camera.h"
#include "translator.h"
#include "selected.h"
#include "track.h"
#include "ship.h"
#include "linegrid.h"
#include "ray.h"

extern GlobalData g;
extern Input g_input;
extern void getNormalizedWindowCoord(float&, float&, const unsigned int, const unsigned int);
void moveCamera(Camera&, const Input&, const float);

enum EditorAction
{
    NONE,
    READ_FROM_FILE,
    WRITE_TO_FILE,    
    REMOVE_SELECTED_BOX,
    COPY_SELECTED_BOX,
    ADD_NEW_BOX,
    SELECT_BOX,
    ADD_SELECT,
    DESELECT,
    DESELECT_ALL,
    CHANGE_BOX_LENGTH,
    MOVE_SELECTED_BOX,
    DRAG_MOVE_CAMERA,
    TURN_PERSPECTIVE_CAMERA,
    ZOOM
};

class Editor
{
public:
    Editor(Track& t, const Ship& s, const float a_r, const float fov, const Mat4 p_transform)
        :track(t), ship(s), translator(p_transform), selected(t), bwfd(p_transform), aspect_ratio(a_r),
        line_grid(GRID_UNIT, 0.0f, 500, p_transform)
    {
        pers_camera = PerspectiveCamera(Vec3(0.0f, 0.0f, -1.0f),
                             Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 1.0f, 4.0f), fov, aspect_ratio);
        const float view_volume_width = 20.0f;
        const float view_volume_height = view_volume_width / aspect_ratio;
        ortho_camera_z = OrthographicCamera(Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 10.0f),
                                      view_volume_width, view_volume_height);
        ortho_camera_x = OrthographicCamera(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(-10.0f, 0.0f, 0.0f),
                              view_volume_width, view_volume_height);
        ortho_camera_y = OrthographicCamera(Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f),
                                      Vec3(0.0f, 10.0f, 0.0f),
                                      view_volume_width, view_volume_height);
        ortho_transform_x = Mat4::makeOrthographic(view_volume_width, view_volume_height, 0.001f, 200.0f);
        ortho_transform_y = ortho_transform_x;
        ortho_transform_z = ortho_transform_x;
        pers_transform = p_transform;

        box_hit_side = -1;
        raycast_hit_point;            
        left_clicking = false;
        clicking_on_selected_box = false;
        click_to_move_box = false;
        last_active_key = nullptr;
    }
    void frame()
    {
        // Camera movement
        moveCamera(pers_camera, g_input, g.dt);
        
        int hit_box_index = -1;
        EditorAction editor_action = NONE;
        {
            if(last_active_key && !*(last_active_key))
            {
                last_active_key = nullptr;
            }
            if(last_active_key && *(last_active_key)) 
            {
                editor_action = NONE;
            }else if(g_input.left_click)
            {
                if(!left_clicking)
                {
                    float click_x, click_y;
                    getNormalizedWindowCoord(click_x, click_y, g_input.left_click_x, g_input.left_click_y);
                    // The left click coordinates determines which viewport/camera is active
                    // for the duration that left click is held down
                    calcActiveCamAndNewClickCoords(click_x, click_y);
                    Ray ray = active_camera->calcRayFromScreenCoord(click_x, click_y);
                    float t;
                    if(selected.getNumSelected() > 0)
                    {
                        if(active_camera == &pers_camera)
                        {
                            if(translator.rayIntersect(t, ray))
                            {
                                click_to_move_box = true;
                                editor_action = NONE;
                            }else
                            {
                                hit_box_index = track.rayIntersectTrack(box_hit_side, t, ray);
                                if(hit_box_index == -1)
                                {
                                    if(g_input.left_ctrl)
                                    {
                                        editor_action = NONE;
                                    }else
                                    {
                                        editor_action = DESELECT_ALL;
                                    }
                                }else if(selected.hasIndex(hit_box_index))
                                {
                                    if(g_input.left_ctrl)
                                    {
                                        editor_action = DESELECT;
                                    }else
                                    {
                                        raycast_hit_point = ray.calcPoint(t);
                                        clicking_on_selected_box = true;
                                        editor_action = NONE;
                                    }
                                }else
                                {
                                    raycast_hit_point = ray.calcPoint(t);
                                    if(g_input.left_ctrl)
                                    {
                                        editor_action = ADD_SELECT;
                                    }else
                                    {
                                        editor_action = SELECT_BOX;
                                    }
                                }
                            }
                        }else // Left clicked on an ortho view
                        {
                            hit_box_index = track.rayIntersectTrack(box_hit_side, t, ray);
                            if(hit_box_index == -1)
                            {
                                if(!g_input.left_ctrl)
                                {
                                    editor_action = DESELECT_ALL;
                                }                                
                            }else if(selected.hasIndex(hit_box_index))
                            {
                                click_to_move_box = true;
                            }else 
                            {
                                if(g_input.left_ctrl)
                                {
                                    editor_action = ADD_SELECT;
                                }else
                                {
                                    editor_action = SELECT_BOX;
                                }
                            }
                        }
                    }else // Nothing is selected at the moment
                    {                            
                        hit_box_index = track.rayIntersectTrack(box_hit_side, t, ray);
                        if(hit_box_index == -1)
                        {
                            editor_action = NONE;
                        }else
                        {
                            raycast_hit_point = ray.calcPoint(t);
                            editor_action = SELECT_BOX;
                        }               
                    }
                    left_clicking = true;
                }else // Holding down left click
                {
                    if(click_to_move_box)
                    {
                        editor_action = MOVE_SELECTED_BOX;
                    }else if(clicking_on_selected_box)
                    {
                        editor_action = CHANGE_BOX_LENGTH;
                    }else
                    {
                        editor_action = NONE;
                    }
                }
            }else if(left_clicking && !g_input.left_click)
            {
                if(selected.getNumSelected() > 0)
                {
                    selected.syncSelectedMinMax();
                }
                click_to_move_box = false;
                clicking_on_selected_box = false;
                left_clicking = false;
                active_camera = nullptr;
                editor_action = NONE;
            }else if(g_input.right_click)
            {
                if(!right_clicking)
                {
                    if(!g.editor_multi_view)
                    {
                        editor_action = TURN_PERSPECTIVE_CAMERA;
                    }else
                    {
                        float click_x, click_y;
                        getNormalizedWindowCoord(click_x, click_y, g_input.right_click_x, g_input.right_click_y);
                        calcActiveCamAndNewClickCoords(click_x, click_y);
                        if(active_camera == &(pers_camera))
                        {
                            editor_action = TURN_PERSPECTIVE_CAMERA;
                        }else
                        {
                            editor_action = DRAG_MOVE_CAMERA;
                        }
                    }
                    right_clicking = true;
                }else
                {
                    if(g.editor_multi_view)
                    {
                        if(active_camera == &(pers_camera))
                        {
                            editor_action = TURN_PERSPECTIVE_CAMERA;
                        }else
                        {
                            editor_action = DRAG_MOVE_CAMERA;
                        }
                    }else
                    {
                        editor_action = TURN_PERSPECTIVE_CAMERA;
                    }
                }
            }else if(right_clicking && !g_input.right_click)
            {
                right_clicking = false;
                active_camera = nullptr;
                editor_action = NONE;
            }else if(g_input.scrolling && g.editor_multi_view)
            {
                float cursor_x, cursor_y;
                getNormalizedWindowCoord(cursor_x, cursor_y, g.cursor_x, g.cursor_y);           
                calcActiveCamAndNewClickCoords(cursor_x, cursor_y);
                if(active_camera != &pers_camera)
                {
                    editor_action = ZOOM;
                }
            }else
            {
                // Keyboard actions
                if(g_input.o)
                {
                    editor_action = WRITE_TO_FILE;
                    last_active_key = &(g_input.o);
                }else if(g_input.b && selected.getNumSelected()> 0)
                {
                    editor_action = REMOVE_SELECTED_BOX;
                    last_active_key = &(g_input.b);
                }else if(g_input.c && selected.getNumSelected()> 0)
                {
                    editor_action = COPY_SELECTED_BOX;
                    last_active_key = &(g_input.c);
                }else if(g_input.n)
                {
                    editor_action = ADD_NEW_BOX;
                    last_active_key = &(g_input.n);
                }else if(g_input.p)
                {
                    editor_action = READ_FROM_FILE;
                    last_active_key = &(g_input.p);
                }
            }
        }
        if(hit_box_index != -1)
        {
            printf("hit_box_index %d\n", hit_box_index);
        }
        switch(editor_action)
        {
        case NONE:
            break;
        case READ_FROM_FILE:
        {
            std::string input_file_name;
            std::cout << "Input file name: ";
            std::cin >> input_file_name;
            track.deleteBoxes();
            track.readFromFile(input_file_name.c_str());
        } break;   
        case WRITE_TO_FILE:
        {
            std::string output_file_name;
            std::cout << "Output file name: ";
            std::cin >> output_file_name;
            track.writeToFile(output_file_name.c_str());
        } break;
        case REMOVE_SELECTED_BOX:
        {
            selected.remove();
        } break;
        case COPY_SELECTED_BOX:
        {
            selected.copyAndSelect();
        } break;
        case ADD_NEW_BOX:
        {
            // Place new box in track
            // take camera z axis, move some distance forward, then project it on to the xz plane at y = 0
            // make a box there
            Vec3 camera_z_axis = -pers_camera.getZAxis();
            Vec3 camera_pos = pers_camera.getPosition();
            const float dist = 5.0f;
            Vec3 box_center = camera_z_axis * dist + camera_pos;
            box_center[1] = 0.0f;
            Box new_box(box_center, 1.0f, 1.0f, 1.0f);
            if(selected.getNumSelected()> 0)
            {
                selected.deselectAll();
            }
            selected.select(track.addBox(new_box));
        } break;
        case SELECT_BOX:
        {
            if(selected.getNumSelected()> 0)
            {
                selected.deselectAll();
            }
            selected.select(hit_box_index);
        } break;
        case ADD_SELECT:
        {
            selected.select(hit_box_index);
        } break;
        case DESELECT:
        {
            selected.deselect(hit_box_index);
        } break;
        case DESELECT_ALL:
        {
            selected.deselectAll();
        } break;
        case CHANGE_BOX_LENGTH:
        {
            float x_norm = g.cursor_movement_x / g.window_width * aspect_ratio;
            float y_norm = g.cursor_movement_y / g.window_height;
            Vec3 cursor_vec = Vec3(active_camera->getCameraTransform() * Vec4(x_norm, y_norm, 0.0f, 0.0f));
            Vec3 cam_pos = active_camera->getPosition();                            
            Vec3 box_normal = selected.getSideNormal(box_hit_side);
            float dist_cam_to_hitpoint = fabs((cam_pos - raycast_hit_point).length());
            float amount = dot(cursor_vec, box_normal) * dist_cam_to_hitpoint;
            selected.changeLength(box_hit_side, amount);
        } break;
        case MOVE_SELECTED_BOX:
        {
            float x_norm = g.cursor_movement_x / g.window_width * aspect_ratio;
            float y_norm = g.cursor_movement_y / g.window_height;
            Vec3 cursor_vec = Vec3(active_camera->getCameraTransform() * Vec4(x_norm, y_norm, 0.0f, 0.0f));
            if(g.editor_multi_view && active_camera != &pers_camera)
            {
                OrthographicCamera* ortho_cam_ptr = reinterpret_cast<OrthographicCamera*>(active_camera);
                const float drag_move_multiplier = ortho_cam_ptr->getViewWidth();
                selected.move(cursor_vec * drag_move_multiplier);
            }else
            {
                Vec3 cam_pos = active_camera->getPosition();            
                Vec3 hit_point_to_cam = raycast_hit_point - cam_pos;
                Vec3 scaled_cursor_vec = cursor_vec * hit_point_to_cam.length();
                translator.moveSelected(selected, scaled_cursor_vec);
            }
        } break;
        case DRAG_MOVE_CAMERA:
        {
            float x_norm = g.cursor_movement_x / g.window_width * aspect_ratio;
            float y_norm = g.cursor_movement_y / g.window_height;
            OrthographicCamera* ortho_cam_ptr = reinterpret_cast<OrthographicCamera*>(active_camera);
            const float drag_move_multiplier = ortho_cam_ptr->getViewWidth() * 3.0f;
            Vec3 cursor_vec = Vec3(active_camera->getCameraTransform() * Vec4(-x_norm, -y_norm, 0.0f, 0.0f));
            cursor_vec *= drag_move_multiplier;
            active_camera->setPosAndOrientation(active_camera->getPosition() +
                                                cursor_vec, active_camera->getEulerAng());
        } break;
        case TURN_PERSPECTIVE_CAMERA:
        {
            pers_camera.turn(g.cursor_movement_x, g.cursor_movement_y);
        } break;
        case ZOOM:
        {
            OrthographicCamera* ortho_cam_ptr = reinterpret_cast<OrthographicCamera*>(active_camera);
            float new_view_height = ortho_cam_ptr->getViewHeight() + g_input.scroll_y;

            {
                ortho_cam_ptr->setViewHeight(new_view_height);
                float new_view_width = new_view_height * aspect_ratio;
                Mat4 new_ortho = Mat4::makeOrthographic(new_view_width, new_view_height, 0.001f, 200.0f);
                if(active_camera == &ortho_camera_x)
                {
                    ortho_transform_x = new_ortho;
                }else if(active_camera == &ortho_camera_y)
                {
                    ortho_transform_y = new_ortho;
                }else if(active_camera = &ortho_camera_z)
                {
                    ortho_transform_z = new_ortho;
                }
            }
        } break;
        default:
            break;
        }

        if(!g.editor_multi_view)
        {
            Mat4 view_transform = pers_camera.getViewTransform();
            persViewDraw(view_transform);
        }else
        {
            // bottom left
            glViewport(0, 0, g.window_width / 2, g.window_height / 2);
            Mat4 view_transform = pers_camera.getViewTransform();
            persViewDraw(view_transform);

            // top left, x view
            updateProjTransform(ortho_transform_x);
            line_grid.setModelTransform(
                //Mat4::makeTranslation(ortho_camera_x.getPosition() + Vec3(0.1f, 0.0f, 0.0f)) *
                Mat4::makeZRotation(90.0f));
            glViewport(0, g.window_height / 2, g.window_width / 2, g.window_height / 2);
            view_transform = ortho_camera_x.getViewTransform();
            orthoViewDraw(view_transform);

            // top right, y view
            updateProjTransform(ortho_transform_y);
            line_grid.setModelTransform(Mat4::makeTranslation(Vec3(0.0f, 8.0f, 0.0f)));
            glViewport(g.window_width / 2, g.window_height / 2, g.window_width / 2, g.window_height / 2);
            view_transform = ortho_camera_y.getViewTransform();
            orthoViewDraw(view_transform);

            // top right, z view
            updateProjTransform(ortho_transform_z);
            line_grid.setModelTransform(
                Mat4::makeXRotation(90.0f));

            glViewport(g.window_width / 2, 0, g.window_width / 2, g.window_height / 2);
            view_transform = ortho_camera_z.getViewTransform();
            orthoViewDraw(view_transform);

            //printCameraLocations();
            line_grid.setModelTransform(Mat4());
            glViewport(0, 0, g.window_width, g.window_height);
            updateProjTransform(pers_transform);
        }
    }

    void printCameraLocations()
    {
        std::cout << "Camera locations\n";
        pers_camera.getPosition().print();
        ortho_camera_x.getPosition().print();
        ortho_camera_y.getPosition().print();
        ortho_camera_z.getPosition().print();
    }

private:
    void persViewDraw(const Mat4& view_transform)
    {
        updateViewTransform(view_transform);
        ship.draw();
        track.draw();
        glEnable(GL_BLEND);    
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        line_grid.draw();
        glDisable(GL_BLEND);
        if(selected.getNumSelected()> 0)
        {
            glDisable(GL_DEPTH_TEST);
            for(int i = 0; i < selected.getNumSelected(); i++)
            {
                bwfd.drawWireframeOnBox(selected.getBox(i), view_transform);
            }
            translator.moveTo(selected.getCenter());
            translator.draw(view_transform);
            glEnable(GL_DEPTH_TEST);
        }
    }

    void orthoViewDraw(const Mat4& view_transform)
    {
        updateViewTransform(view_transform);
        ship.draw();
        track.draw();
        glEnable(GL_BLEND);    
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        line_grid.draw();
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        if(selected.getNumSelected()> 0)
        {
            glDisable(GL_DEPTH_TEST);
            for(int i = 0; i < selected.getNumSelected(); i++)
            {
                bwfd.drawWireframeOnBox(selected.getBox(i), view_transform);
            }
            glEnable(GL_DEPTH_TEST);
        }        
    }

    void updateViewTransform(const Mat4& view_transform)
    {
        ship.updateDynamicUniforms(view_transform);
        track.setViewTransform(view_transform);
        line_grid.setViewTransform(view_transform);
    }

    void updateProjTransform(const Mat4& proj_transform)
    {
        ship.setProjTransform(proj_transform);
        track.setProjTransform(proj_transform);
        line_grid.setProjTransform(proj_transform);
        translator.setProjTransform(proj_transform);
        bwfd.setProjTransform(proj_transform);
    }

    void calcActiveCamAndNewClickCoords(float& x, float& y)
    {
        float click_x = x, click_y = y;
        if(g.editor_multi_view)
        {
            if(click_x < 0.0f)
            {
                click_x = click_x * 2.0f + 1.0f;
                if(click_y < 0.0f)
                {
                    active_camera = reinterpret_cast<Camera*>(&pers_camera);                         
                    click_y = click_y * 2.0f + 1.0f;
                }else
                {
                    active_camera = reinterpret_cast<Camera*>(&ortho_camera_x);
                    click_y = click_y * 2.0f - 1.0f;
                }
            }else
            {
                click_x = click_x * 2.0f - 1.0f;
                if(click_y < 0.0f)
                {
                    active_camera = reinterpret_cast<Camera*>(&ortho_camera_z);
                    click_y = click_y * 2.0f + 1.0f;
                }else
                {
                    active_camera = reinterpret_cast<Camera*>(&ortho_camera_y);
                    click_y = click_y * 2.0f - 1.0f;
                }
            }
        }else
        {
            active_camera = &pers_camera;
        }
        x = click_x;
        y = click_y;
    }
    
    PerspectiveCamera pers_camera;
    OrthographicCamera ortho_camera_x;
    OrthographicCamera ortho_camera_y;
    OrthographicCamera ortho_camera_z;
    Mat4 pers_transform;
    Mat4 ortho_transform_x;
    Mat4 ortho_transform_y;
    Mat4 ortho_transform_z;
    Translator translator;
    Selected selected;
    LineGrid line_grid;
    BoxWireframeDrawer bwfd;
    Track& track;
    const Ship& ship;
    Camera* active_camera;

    int box_hit_side;
    Vec3 raycast_hit_point;
    // TODO: move these left and right clicking into g?
    bool left_clicking;
    bool right_clicking;
    bool clicking_on_selected_box;
    bool click_to_move_box;
    int* last_active_key;
    float aspect_ratio;
};
