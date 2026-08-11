#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Angle.hpp>
#include <vector>

//window
const char* window_title = "Mario Bros";
const unsigned window_width = 800;
const unsigned window_height = 600;
const float max_frame_rate = 60;

//terreno
const sf::Vector2f ground_size = {800.0f, 50.0f};
const sf::Color ground_color = sf::Color(139, 69, 19);
const sf::Vector2f ground_pos = {0.0f, 550.0f};

struct Mondo {
    sf::RectangleShape ground; 

    Mondo(); 
    void draw(sf::RenderWindow& window);
};

Mondo::Mondo() {
    ground.setSize(ground_size);
    ground.setPosition(ground_pos);
    ground.setFillColor(ground_color);
}

void Mondo::draw(sf::RenderWindow& window) {
    window.draw(ground);
}

//handle
void handle_close (sf::RenderWindow& window)
{
    window.close();
}
void handle_resize (const sf::Event::Resized& resized, sf::RenderWindow& window)
{   
    float aspect = static_cast<float>(window_width)/static_cast<float>(window_height);
    sf::Vector2u ws = resized.size;
    float new_aspect = static_cast<float>(ws.x)/static_cast<float>(ws.y);
    if (new_aspect < aspect)
        ws = {ws.x,static_cast<unsigned>(ws.x/aspect)};
        else
        ws = {static_cast<unsigned>(ws.y*aspect),ws.y};
    window.setSize(ws);
}
/*
template <typename T>
void handle(const T &, State &gs)
{
    // All unhandled events will end up here
}*/

int main(){
    sf::RenderWindow window(sf::VideoMode ({window_width, window_height}), window_title);
    window.setFramerateLimit(max_frame_rate);
    window.setMinimumSize(window.getSize());

    sf::Clock clock;
    Mondo mondo;

    while(window.isOpen()){

        float dt = clock.restart().asSeconds();

        window.handleEvents(
            [&window](const sf::Event::Closed&) { handle_close (window); },
            [&window](const sf::Event::Resized& event) { handle_resize (event, window); }
        );

        window.clear(sf::Color::Black);
        mondo.draw(window);
        window.display();
    }
}