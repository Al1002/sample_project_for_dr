#include <game_engine/std_includes.hpp>

#include <game_engine/colors.h>  // #defined RGB_COLORs
#include <game_engine/objects.hpp>
#include <game_engine/events.hpp>
#include <game_engine/base_handler.hpp>
#include <game_engine/engine.hpp>
#include <game_engine/graphic_system.hpp>
#include <game_engine/physics.hpp>
#include <game_engine/button.hpp>


class BirdHandler : public Handler<KeyboardEvent, PhysicsObject>
{
public:
    void handle(shared_ptr<KeyboardEvent> e) override
    {
        if(!e->is_down)
            return;
        if (e->sdl_event.keysym.sym == 'w' || e->sdl_event.keysym.sym == ' ' || e->sdl_event.keysym.sym == SDLK_UP)
        {
            getOwner()->body->SetLinearVelocity({0, -600.0f / 1024});
            getOwner()->get<AudioPlayer>(1)->play();
        }
    }
};

class PipeSpawner : public Object2D
{
public:
    Clock time;

    void loop(double delta) override
    {
        if(time.get_time() < 2.5)
            return;
        else
            time.start_timer();

        add(make_shared<Object2D>());
        get<Object2D>(0)->offset = {500, 400 + (float)(rand() % 300)};

        get(0)->add(get<BlueprintFactory>("/Templates")->build("green_pipe_above"));
        get<Sprite>("Object2D/Sprite")->scaleX(100);
        get<Sprite>("Object2D/Sprite")->offset = {0, 75};
        get(0)->add(make_shared<PhysicsObject>(get<Sprite>("Object2D/Sprite")->offset, get<Sprite>("Object2D/Sprite")->getSize(), b2_kinematicBody));
        
        get(0)->add(get<BlueprintFactory>("/Templates")->build("green_pipe_bellow"));
        get<Sprite>("Object2D/Sprite_1")->scaleX(100);
        get<Sprite>("Object2D/Sprite_1")->offset = {0, -75 - get<Sprite>("Object2D/Sprite_1")->getSize().y};
        get(0)->add(make_shared<PhysicsObject>(get<Sprite>("Object2D/Sprite_1")->offset, get<Sprite>("Object2D/Sprite_1")->getSize(), b2_kinematicBody));
        
        get(0)->attachLoopBehaviour([](Object *self, double delta){
            shared_ptr<Object2D> t_self = static_pointer_cast<Object2D>(self->shared_from_this());
            t_self->offset.x -= 100 * delta;
            if(t_self->offset.x < -100)
                t_self->getParent()->removeChild(t_self->getName());
        });
        getEngine()->add(removeChild(0));
    }
};


class StartGameButton : public Button
{
public:
    bool isStart = false;
    void onClick()override
    {
        if(isStart)
            return;
        else
            isStart = true; 
        getEngine()->add(make_shared<Object>("Game"));
        
        // bird
        auto bird = make_shared<PhysicsObject>(Vect2f(0,0), Vect2f(6,6), b2_dynamicBody);
        bird->attachInitBehaviour([](Object *self){
            static_cast<Object2D*>(self)->offset = {100, 100};
            shared_ptr<Sprite> sprite = self->get<BlueprintFactory>("/Templates")->build<Sprite>("bird");
            sprite->scaleX(84);
            sprite->setDrawHeight(2);
            self->add(sprite);
            self->add(make_shared<AudioPlayer>("../exec_env/resources/sfx_jump.mp3"));
            self->get<AudioPlayer>(1)->setVolume(25);
            self->attachHandler(make_shared<BirdHandler>());
        });
        getEngine()->get("Game")->add(bird);

        // pipes
        getEngine()->get("Game")->add(make_shared<PipeSpawner>());
    }
};

#include <exception>

#ifdef __WIN32__ // the mingw SDL expects WinMain
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdL0, -ine, int nShowCmd)
#endif

int main(int argc, char **argv)
{
    const Vect2i screen_size = {400, 720};
    Engine::enable();

    auto e = make_shared<Engine>(screen_size, Vect2f(0, 2000));

    e->add(e->gsys->loadTexture("../exec_env/resources/flappy_sprite_sheet.png"));
    e->add(make_shared<BlueprintFactory>("Templates"));
    
    // save sprites
    e->get<BlueprintFactory>("Templates")->addBlueprint(
        make_shared<Sprite>(e->get<Texture>("Texture"), Vect2i(148 * 0, 0), Vect2i(144, 256)),
        "background"
    );
    e->get<BlueprintFactory>("Templates")->addBlueprint(
        make_shared<Sprite>(e->get<Texture>("Texture"), Vect2i(148 * 2, 0), Vect2i(160, 56)),
        "floor"
    );
    e->get<BlueprintFactory>("Templates")->addBlueprint(
        make_shared<AnimatedSprite>(3, 10, e->get<Texture>("Texture"), Vect2i(0, 512 - 28), Vect2i(28, 28)),
        "bird"
    );
    e->get<BlueprintFactory>("Templates")->addBlueprint(
        make_shared<Sprite>(e->get<Texture>("Texture"), Vect2i(28 * 2, 512 - 190), Vect2i(28 * 1, 162)),
        "green_pipe_bellow"
    );
    e->get<BlueprintFactory>("Templates")->addBlueprint(
        make_shared<Sprite>(e->get<Texture>("Texture"), Vect2i(28 * 3, 512 - 190), Vect2i(28 * 1, 162)),
        "green_pipe_above"
    );

    // backround
    e->add(e->get<BlueprintFactory>("Templates")->build("background"));
    e->get<Sprite>("Sprite")->offset = screen_size / 2;
    e->get<Sprite>("Sprite")->scaleY(screen_size.y);
    e->get<Sprite>("Sprite")->setDrawHeight(-2);
    
    // floor
    e->add(make_shared<PhysicsObject>(Vect2f(0, 0), Vect2f(480, 168), b2_staticBody));
    e->get<PhysicsObject>("PhysicsObject")->offset = {screen_size.x / 2.0f, 650};
    e->get("PhysicsObject")->add(e->get<BlueprintFactory>("Templates")->build("floor"));
    
    // floor sprite
    e->get<Sprite>("PhysicsObject/Sprite")->scaleX(480);
    e->get<Sprite>("PhysicsObject/Sprite")->setDrawHeight(1);
    e->get<Sprite>("PhysicsObject/Sprite")->attachLoopBehaviour([](Object *self, double delta){
        shared_ptr<Object2D> t_self = static_pointer_cast<Object2D>(self->shared_from_this());
        t_self->offset.x -= 100 * delta;
        if(t_self->offset.x < -36)
            t_self->offset.x = 0;
    });

    // start game 
    e->add(make_shared<StartGameButton>());
    e->get<Button>("Button")->base_size = screen_size;

    e->start();

    Engine::disable();
    return 0;
}
