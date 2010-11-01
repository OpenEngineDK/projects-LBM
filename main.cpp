// main
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

// OpenEngine stuff
#include <Meta/Config.h>

#include <Core/Engine.h>
#include <Devices/IMouse.h>
#include <Devices/IKeyboard.h>
#include <Display/Camera.h>
#include <Display/Frustum.h>
#include <Display/PerspectiveViewingVolume.h>
#include <Logging/Logger.h>
#include <Logging/StreamLogger.h>
#include <Resources/Directory.h>
#include <Resources/DirectoryManager.h>
#include <Renderers/TextureLoader.h>
#include <Renderers/IRenderingView.h>
#include <Scene/RenderStateNode.h>
#include <Scene/SceneNode.h>
#include <Utils/Timer.h>

// SDL extension
#include <Display/SDLEnvironment.h>

// Generic handlers
#include <Utils/MoveHandler.h>
#include <Utils/QuitHandler.h>

// OpenGL stuff
#include <Renderers/OpenGL/Renderer.h>
#include <Display/OpenGL/RenderCanvas.h>
#include <Renderers/OpenGL/RenderingView.h>

// Project files
#include "LBPhysics.h"
#include "SBVBox.h"

// name spaces that we will be using.
using namespace OpenEngine;
using namespace OpenEngine::Core;
using namespace OpenEngine::Devices;
using namespace OpenEngine::Display;
using namespace OpenEngine::Geometry;
using namespace OpenEngine::Logging;
using namespace OpenEngine::Renderers::OpenGL;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Utils;

Engine* engine;
IEnvironment* env;
IFrame* frame;
Display::IRenderCanvas* canvas;
Renderer* renderer;
IMouse* mouse;
IKeyboard* keyboard;
ISceneNode* scene;
Camera* camera;
Frustum* frustum;
IRenderingView* renderingview;
TextureLoader* textureloader;

bool useShader = true;

class RenderStateHandler : public IListener<KeyboardEventArg> {
    RenderStateNode* node;
public:
    RenderStateHandler(RenderStateNode* n) : node(n) {            
        node->DisableOption(RenderStateNode::WIREFRAME);
        node->DisableOption(RenderStateNode::TANGENT);
    }
    void Handle(KeyboardEventArg arg) {
        if (arg.type == EVENT_PRESS && arg.sym == KEY_g){
            node->ToggleOption(RenderStateNode::WIREFRAME);
        }
        if (arg.type == EVENT_PRESS && arg.sym == KEY_b){
            node->ToggleOption(RenderStateNode::TANGENT);
        }
        /*
        if (arg.type == EVENT_PRESS && arg.sym == KEY_n){
            multiplier += 0.025;
            if (multiplier > 1.0) multiplier = 1.0;
        }
        if (arg.type == EVENT_PRESS && arg.sym == KEY_m){
            multiplier -= 0.025;
            if (multiplier < 0.0) multiplier = 0.0;
        }
        */
    }
};

// Forward declarations ... ffs c++
void SetupDisplay();
void SetupRendering();

int main(int argc, char** argv) {
    // create a logger to std out    
    Logger::AddLogger(new StreamLogger(&std::cout));
    logger.info << "execution binary: " << argv[0] << logger.end;
    logger.info << "current working directory: " 
                << Directory::GetCWD()<< logger.end;


    // setup the engine
    engine = new Engine;

    SetupDisplay();

    DirectoryManager::AppendPath("projects/LBM/data/");

    scene = new SceneNode();

    SetupRendering();

    Utils::Timer timer;
    timer.Start();

    logger.info << "time elapsed: "
                << timer.GetElapsedTime() << logger.end;

    // Register the handler as a listener on up and down keyboard events.
    MoveHandler* move_h = new MoveHandler(*camera, *(env->GetMouse()));
    keyboard->KeyEvent().Attach(*move_h);
    engine->InitializeEvent().Attach(*move_h);
    engine->ProcessEvent().Attach(*move_h);
    engine->DeinitializeEvent().Attach(*move_h);

    QuitHandler* quit_h = new QuitHandler(*engine);
    keyboard->KeyEvent().Attach(*quit_h);

    LBPhysicsPtr lbp = LBPhysics::Create();
    engine->InitializeEvent().Attach(*lbp);
    engine->ProcessEvent().Attach(*lbp);
    textureloader->Load( lbp, Renderers::TextureLoader::RELOAD_IMMEDIATE );

    SBVBox* box = new SBVBox(*camera, lbp);
    engine->InitializeEvent().Attach(*box);
    engine->ProcessEvent().Attach(*box);

    scene->AddNode(box);

    engine->Start();

    // Return when the engine stops.
    return EXIT_SUCCESS;
}

void SetupDisplay(){
    // setup display and devices
    //env = new SDLEnvironment(1440,900,32,FRAME_FULLSCREEN);
    env = new SDLEnvironment(800,600);
    frame    = &env->CreateFrame();
    mouse    = env->GetMouse();
    keyboard = env->GetKeyboard();
    engine->InitializeEvent().Attach(*env);
    engine->ProcessEvent().Attach(*env);
    engine->DeinitializeEvent().Attach(*env);

    // setup camera
    camera  = new Camera(*(new PerspectiveViewingVolume(1, 4000)));
    frustum = new Frustum(*camera);

    camera->SetPosition(Vector<3, float>(0.0, 0.0, 10.0));
    camera->LookAt(0.0, 0.0, 0.0);
}


void SetupRendering(){
    renderer = new Renderer();
    textureloader = new TextureLoader(*renderer);
    renderingview = new RenderingView();

    renderer->InitializeEvent().Attach(*renderingview);
    renderer->ProcessEvent().Attach(*renderingview);
    canvas = new Display::OpenGL::RenderCanvas();
    canvas->SetViewingVolume(frustum);
    canvas->SetRenderer(renderer);
    canvas->SetScene(scene);
    frame->SetCanvas(canvas);

    renderer->PreProcessEvent().Attach(*textureloader);

    renderer->SetBackgroundColor(Vector<4, float>(0.5, 0.5, 1.0, 1.0));
}
