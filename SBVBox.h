#ifndef _SBV_BOX_
#define _SBV_BOX_

#include <Core/IListener.h>
#include <Display/Camera.h>
#include <Geometry/Box.h>
#include <Geometry/Line.h>
#include <Geometry/Plane.h>
#include <Geometry/Polygon.h>
#include <Geometry/Tests.h>
#include <Math/Vector.h>
#include <Meta/OpenGL.h>
#include <Resources/ITexture3D.h>
#include <Renderers/IRenderer.h>
#include <Scene/ISceneNodeVisitor.h>
#include <Scene/PostProcessNode.h>

#include <vector>

using namespace OpenEngine;
using Core::IListener;
using Core::InitializeEventArg;
using Core::ProcessEventArg;
using Geometry::Box;
using Geometry::Line;
using Geometry::Plane;
using Geometry::Polygon;
using Geometry::Tests;
using Math::Vector;
using Scene::RenderNode;
using Scene::PostProcessNode;
using Scene::ISceneNodeVisitor;
using Renderers::RenderingEventArg;


/***
 * Slice Based Volumetric Box
 */
class SBVBox 
: public IListener<OpenEngine::Core::InitializeEventArg>,
    public IListener<OpenEngine::Core::ProcessEventArg>,
    public RenderNode {
 private:
    Display::Camera& camera;
    Box* box;
    Resources::ITexture3DPtr tex;
    unsigned int halfNumberOfSlices;
    std::vector<Polygon> polygons;
    bool useRayCaster, debug;

    PostProcessNode* rayCaster;

 public:
    SBVBox(Display::Camera& camera, Resources::ITexture3DPtr tex, PostProcessNode* pp = NULL) 
        : camera(camera), tex(tex), rayCaster(pp) {
        Vector<3,float> center(0.5f);
        Vector<3,float> relCorner(0.5f);
        box = new Box(center, relCorner);
        halfNumberOfSlices = 100;
        useRayCaster = false;
        debug = false;
    }

    void Handle(OpenEngine::Core::InitializeEventArg arg) {
        // handle initialize
    }

    void Handle(OpenEngine::Core::ProcessEventArg arg) {

        // get camera and calculate position and rotation
        Vector<3,float> camPos = camera.GetPosition();
        Vector<3,float> camDir(0.0f,0.0f,-1.0f);
        camDir = camera.GetDirection().RotateVector(camDir);

        // for each plane 
        polygons.clear();
        for (int slice = -halfNumberOfSlices; 
             slice<=(int)halfNumberOfSlices; slice++) {
            Vector<3,float> pointOnPlane = box->GetCenter();
            float thickness = (box->GetCorner().GetLength() / (float)(halfNumberOfSlices+1));
            float length = -slice * thickness;
            pointOnPlane += camDir * length;
            Plane plane(camDir, pointOnPlane);
            //logger.info << "plane:" << plane.ToString() << logger.end;

            // calc new intersection points
            Polygon polygon;
            bool intersects = Tests::Intersects(*box, plane, &polygon);
            //logger.info << "intersects:" << intersects << logger.end;
            if (intersects) {
                // sort points based on angle projected onto camera
                polygon.SortPoints();
                polygons.push_back(polygon);
            }
        }
    }

    void Apply(RenderingEventArg arg, ISceneNodeVisitor& v) {
        if (rayCaster != NULL && useRayCaster == true) {
            v.VisitPostProcessNode(rayCaster);
        }else{
            // bind 3d texture
            glBindTexture(GL_TEXTURE_3D, tex->GetID());
            //logger.info << "binding texture with id: " << tex->GetID() << logger.end;
            // save gl state
            GLboolean tex3d = glIsEnabled(GL_TEXTURE_3D);
            GLboolean b = glIsEnabled(GL_BLEND);
            
            // set gl start
            glEnable(GL_TEXTURE_3D);
            
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            
            // for each polygon back to front
            //     render the polygon
            glColor4f(1.0f,1.0f,1.0f,1.0f);
            for (unsigned int i=0; i<polygons.size(); i++) {
                Polygon polygon = polygons[i];
                
                //logger.info << "POLYGON BEGIN" << logger.end;
                glBegin(GL_POLYGON);
                for (unsigned int p=0; p<polygon.NumberOfPoints(); p++) {
                    Vector<3,float> point = polygon.GetPoint(p);
                    
                    // calculate texture coordinate
                    Vector<3,float> texCoord = point;
                    //texCoord /= box->GetCorner().GetLength();
                    //texCoord *= 0.5;
                    //texCoord += Vector<3,float>(0.5f);
                    
                    // apply texture coordinate
                    float* pointer2 = &(texCoord[0]);
                    glTexCoord3fv(pointer2);
                    
                    // draw point
                float* pointer = &(point[0]);
                glVertex3fv(pointer);
                }
                glEnd();
            }
            
            // unbind texture
            glBindTexture(GL_TEXTURE_3D, 0);
            
            // reset gl state
            if (!tex3d)
                glDisable(GL_TEXTURE_3D);
            if (!b)
                glDisable(GL_BLEND);

            /*
            // render polygons as lines
            for (unsigned int i=0; i<polygons.size(); i++) {
                Polygon polygon = polygons[i];
                unsigned int noe = polygon.NumberOfPoints();
                for (unsigned int p=0; p<noe; p++) {
                    Vector<3,float> point1 = polygon.GetPoint(p);
                    Vector<3,float> point2 = polygon.GetPoint((p+1)%noe);
                    Line line(point1,point2);
                    //logger.info << "Polygon" << polygon.ToString() << logger.end;
                    //Vector<3,float> color(p/((float)polygon.NumberOfPoints()),0.0f,0.0f);
                    Vector<3,float> color(1-(i/((float)polygons.size())),0.0f,0.0f);
                    //Vector<3,float> color(1.0f,0.0f,0.0f);
                    arg.renderer.DrawLine(line, color, 2);
                    Vector<3,float> color2(0.0f,1.0f,0.0f);
                    arg.renderer.DrawPoint(point1, color2, 4);
                    arg.renderer.DrawPoint(point2, color2, 4);
                }
                //Vector<3,float> color3(0.0f,0.0f,1.0f);
                //arg.renderer.DrawPoint(polygon.GetCenterOfGravity(), color3, 9);
            }
            */
        }
        
        if (debug) {
            // render box as lines
            std::vector<Line> lines = box->GetBoundingLines();
            for (unsigned int i=0; i<lines.size(); i++) {
                Line line = lines[i];
                //logger.info << "Line" << line.ToString() << logger.end;
                Vector<3,float> color(1.0f);
                arg.renderer.DrawLine(line, color, 2);
            }
        }
    }

    void ToggleDebugInfo () {
        debug = !debug;
        if (debug)
            logger.info << "using debug info" << logger.end;
        else
            logger.info << "turning off debug info" << logger.end;
    }

    void ToggleRenderTechnique() {
        useRayCaster = !useRayCaster;
        if (useRayCaster)
            logger.info << "switching to raycaster" << logger.end;
        else
            logger.info << "switching to slice based render" << logger.end;
    }
};

#endif // _SBV_BOX_
