#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "mandelbrot_gpu.hpp"

int main()
{
    //Mandelbrot mb(1000, 1000, std::make_pair(0.266-0.032, 0.266+0.032), std::make_pair(-0.925-0.032, -0.925+0.032)); //cool point to check
    
    mbs::Mandelbrot <> mb(1200, 960, {-2.0, 1.0}, {-1.2, 1.2}); //500x400 (W x H) window, range for Re, range for Im

    sf::RenderWindow window(sf::VideoMode(1200, 960), "Mandelbrot");

    sf::Image img;
    img.create(1200, 960);

    sf::Texture texture;
    sf::Sprite sprite;

    bool state_changed = true;

    while(window.isOpen())
    {
        sf::Event event;
        if(window.waitEvent(event))
        {
            state_changed = true;
            switch(event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;

                case sf::Event::KeyPressed:
                    switch(event.key.code)
                    {
                        case sf::Keyboard::W:
                            //y + zoom
                            state_changed = true;
                            mb.set_offset(0, -0.2);
                            break;
                        case sf::Keyboard::S:
                            //y - zoom
                            state_changed = true;
                            mb.set_offset(0, 0.2);
                            break;
                        case sf::Keyboard::A:
                            //x - zoom
                            state_changed = true;
                            mb.set_offset(-0.2, 0);
                            break;
                        case sf::Keyboard::D:
                            //x + zoom
                            state_changed = true;
                            mb.set_offset(0.2, 0);
                            break;
                        case sf::Keyboard::Add:
                            //focus
                            state_changed = true;
                            mb.focus(0.85);
                            break;
                        case sf::Keyboard::Subtract:
                            //focus
                            state_changed = true;
                            mb.focus(1/0.85);
                            break;
                    }
            }
        }
        if(state_changed)
        {
            //std::vector <int> hist(256);
            for(int py = 0; py < 960; ++py)
                for(int px = 0; px < 1200; ++px)
                {
                    double t = double(mb.data()[py * 1200 + px]) / double(255);
                    int r = (int)(9*(1-t)*t*t*t*255);
                    int g = (int)(15*(1-t)*(1-t)*t*t*255);
                    int b =  (int)(8.5*(1-t)*(1-t)*(1-t)*t*255);
                    img.setPixel(px, py, sf::Color(int(9*(1-t)*t*t*t*255), int(15*(1-t)*(1-t)*t*t*255), int(8.5*(1-t)*(1-t)*(1-t)*t*255)));
                    // mbs::RGB color = mbs::generateColor(mb.data()[py][px]);
                    // img.setPixel(px, py, sf::Color(color.R - 1, color.G - 1, color.B - 1));
                }

            state_changed = false;
            texture.loadFromImage(img);
            sprite.setTexture(texture);
            window.draw(sprite);
            window.display();
        }
    }
    return 0;
}