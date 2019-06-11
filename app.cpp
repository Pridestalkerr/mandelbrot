#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "mandelbrot.hpp"

int main()
{
	//Mandelbrot mb(1000, 1000, std::make_pair(0.266-0.032, 0.266+0.032), std::make_pair(-0.925-0.032, -0.925+0.032)); //cool point to check
	
	mbs::Mandelbrot mb(400, 500, std::make_pair(-1.2, 1.2), std::make_pair(-2.0, 1.0)); //800x1000 window, range for Im, range for Re


	sf::RenderWindow window(sf::VideoMode(500, 400), "Mandelbrot");
	sf::Image img;
	img.create(500, 400);
	sf::Texture texture;
	sf::Sprite sprite;
	bool stateChanged = true;
	while(window.isOpen())
	{
		sf::Event event;
		if(window.waitEvent(event))
		{
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
							stateChanged = true;
							mb.setOffset(-0.2, 0);
							break;
						case sf::Keyboard::S:
							//y - zoom
							stateChanged = true;
							mb.setOffset(0.2, 0);
							break;
						case sf::Keyboard::A:
							//x - zoom
							stateChanged = true;
							mb.setOffset(0, -0.2);
							break;
						case sf::Keyboard::D:
							//x + zoom
							stateChanged = true;
							mb.setOffset(0, 0.2);
							break;
						case sf::Keyboard::Add:
							//focus
							stateChanged = true;
							mb.focus(0.85);
							break;
						case sf::Keyboard::Subtract:
							//focus
							stateChanged = true;
							mb.focus(1/0.85);
							break;
					}
			}
		}
		if(stateChanged)
		{
			for(int py = 0; py < 400; ++py)
		        for(int px = 0; px < 500; ++px)
		            img.setPixel(px, py, sf::Color(mb.getTable()[py][px], mb.getTable()[py][px]*2, mb.getTable()[py][px]*3%255));
			stateChanged = false;
			texture.loadFromImage(img);
			sprite.setTexture(texture);
			window.draw(sprite);
			window.display();
		}
	}
    return 0;
}