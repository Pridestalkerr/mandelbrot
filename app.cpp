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
			stateChanged = true; //select proper events
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
			std::vector <int> hist(256);
			for(int py = 0; py < 400; ++py)
		        for(int px = 0; px < 500; ++px)
		        {
		        	/*int r = (int)(9*(1-t)*t*t*t*255);
					int g = (int)(15*(1-t)*(1-t)*t*t*255);
					int b =  (int)(8.5*(1-t)*(1-t)*(1-t)*t*255);*/
					double t = double(mb.getTable()[py][px]) / double(255);
		            img.setPixel(px, py, sf::Color(int(9*(1-t)*t*t*t*255), int(15*(1-t)*(1-t)*t*t*255), int(8.5*(1-t)*(1-t)*(1-t)*t*255)));

				}
			stateChanged = false;
			texture.loadFromImage(img);
			sprite.setTexture(texture);
			window.draw(sprite);
			window.display();
		}
	}
    return 0;
}